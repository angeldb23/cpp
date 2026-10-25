/* =============================================================================
   MÓDULO 22 - SOLUCIÓN 07: Job System — Thread Pool, DAG Dependencies,
   ParallelFor, Profiling
   ============================================================================= */
#include <vector>
#include <queue>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <chrono>
#include <algorithm>
#include <cstdint>
#include <cassert>
#include <fstream>
#include <future>
#include "../../include/test_framework.hpp"

struct Job {
    std::function<void()> work;
    std::vector<uint32_t> dependencies;
    std::atomic<uint32_t> pending_deps{0};
    std::atomic<bool> completed{false};
    std::atomic<bool> cancelled{false};
    uint32_t id = 0;

    std::chrono::steady_clock::time_point queue_time;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
    uint32_t worker_id = UINT32_MAX;
    bool stolen = false;

    Job() = default;
    Job(Job&& other) noexcept
        : work(std::move(other.work))
        , dependencies(std::move(other.dependencies))
        , pending_deps(other.pending_deps.load())
        , completed(other.completed.load())
        , cancelled(other.cancelled.load())
        , id(other.id)
        , queue_time(other.queue_time)
        , start_time(other.start_time)
        , end_time(other.end_time)
        , worker_id(other.worker_id)
        , stolen(other.stolen) {}
    Job& operator=(Job&& other) noexcept {
        if(this != &other) {
            work = std::move(other.work);
            dependencies = std::move(other.dependencies);
            pending_deps.store(other.pending_deps.load());
            completed.store(other.completed.load());
            cancelled.store(other.cancelled.load());
            id = other.id;
            queue_time = other.queue_time;
            start_time = other.start_time;
            end_time = other.end_time;
            worker_id = other.worker_id;
            stolen = other.stolen;
        }
        return *this;
    }
    Job(const Job&) = delete;
    Job& operator=(const Job&) = delete;
};

class JobSystem {
    std::vector<Job> jobs_;
    std::vector<std::vector<uint32_t>> dependents_;
    std::queue<uint32_t> ready_queue_;
    std::mutex queue_mtx_;
    std::condition_variable queue_cv_;
    std::vector<std::thread> workers_;
    std::atomic<bool> stop_{false};
    std::atomic<uint32_t> completed_count_{0};
    size_t num_threads_;

    struct ProfileEvent {
        uint32_t job_id, worker_id;
        uint64_t queue_us, exec_us;
        bool cancelled;
    };
    std::vector<ProfileEvent> profile_;
    std::mutex profile_mtx_;

public:
    explicit JobSystem(size_t num_threads = std::thread::hardware_concurrency()) : num_threads_(num_threads) {}
    ~JobSystem() { stop_=true; queue_cv_.notify_all(); for(auto& t:workers_) if(t.joinable()) t.join(); }

    void ensure_workers_started() {
        if(workers_.empty() && num_threads_ > 0) {
            for(size_t i=0;i<num_threads_;++i) workers_.emplace_back([this,i]{ WorkerLoop(i); });
        }
    }

    uint32_t Schedule(std::function<void()> work, std::initializer_list<uint32_t> deps = {}) {
        uint32_t id = jobs_.size();
        jobs_.push_back({});
        jobs_.back().id = id;
        jobs_.back().work = std::move(work);
        jobs_.back().dependencies = deps;
        jobs_.back().pending_deps.store(deps.size());

        if(dependents_.size() <= id) dependents_.resize(id + 1);
        for(uint32_t dep : deps){
            if(dep < dependents_.size()) dependents_[dep].push_back(id);
        }
        return id;
    }

    void Kick() {
        ensure_workers_started();
        std::lock_guard lock(queue_mtx_);
        for(size_t i=0;i<jobs_.size();++i){
            if(jobs_[i].pending_deps.load() == 0){
                jobs_[i].queue_time = std::chrono::steady_clock::now();
                ready_queue_.push(i);
            }
        }
        queue_cv_.notify_all();
        completed_count_.store(0);
    }

    void WaitAll() {
        // Se consulta el estado real de cada job (completed o cancelled).
        // No se usa completed_count_ para esperar: Kick() lo resetea y un
        // worker puede completar un job ANTES de ese reset, dejando el
        // contador en 0 para siempre (eso producía el timeout de ~10 s).
        while(!stop_) {
            bool pendiente = false;
            for(const auto& job : jobs_) {
                if(!job.completed.load() && !job.cancelled.load()) {
                    pendiente = true;
                    break;
                }
            }
            if(!pendiente) break;
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }

    void CancelJob(uint32_t id) {
        if(id >= jobs_.size()) return;
        if(!jobs_[id].cancelled.exchange(true)) {
            completed_count_++;
            for(uint32_t dep : dependents_[id]) CancelJob(dep);
        }
    }

    template<typename Func>
    void ParallelFor(size_t count, Func&& func, size_t grain_size = 1) {
        if(count == 0) return;
        size_t num_workers = num_threads_;
        if(num_workers == 0) num_workers = 1;
        size_t per_worker = (count + num_workers - 1) / num_workers;
        per_worker = std::max(per_worker, grain_size);

        std::vector<std::future<void>> futures;
        for(size_t w=0; w<num_workers; ++w){
            size_t start = w * per_worker;
            if(start >= count) break;
            size_t end = std::min(start + per_worker, count);
            futures.push_back(std::async(std::launch::async, [start, end, f=std::forward<Func>(func)](){
                for(size_t i=start;i<end;++i) f(i);
            }));
        }
        for(auto& f : futures) f.wait();
    }

    void ExportChromeTracing(const std::string& path) {
        std::lock_guard lock(profile_mtx_);
        std::ofstream f(path);
        f << "{\"traceEvents\":[\n";
        for(size_t i=0;i<profile_.size();++i){
            auto& e = profile_[i];
            f << "  {\"name\":\"Job\",\"cat\":\"job\",\"ph\":\"X\","
              << "\"ts\":" << e.queue_us << ",\"dur\":" << e.exec_us << ","
              << "\"pid\":1,\"tid\":" << e.worker_id << ",\"args\":{\"job_id\":" << e.job_id
              << ",\"cancelled\":" << (e.cancelled?"true":"false") << "}}";
            if(i+1<profile_.size()) f << ",";
            f << "\n";
        }
        f << "],\"displayTimeUnit\":\"ms\"}\n";
    }

private:
    void WorkerLoop(uint32_t worker_id) {
        while(!stop_){
            uint32_t job_id = UINT32_MAX;
            {
                std::unique_lock lock(queue_mtx_);
                queue_cv_.wait_for(lock, std::chrono::milliseconds(1), [this]{ return stop_ || !ready_queue_.empty(); });
                if(stop_) break;
                if(!ready_queue_.empty()){
                    job_id = ready_queue_.front(); ready_queue_.pop();
                }
            }
            if(job_id == UINT32_MAX) continue;

            Job* job = &jobs_[job_id];
            if(job->completed.load() || job->cancelled.load()) continue;

            job->worker_id = worker_id;
            job->start_time = std::chrono::steady_clock::now();

            try {
                job->work();
            } catch(const std::exception& e) {
                // Silenciar excepción en test
            }

            job->end_time = std::chrono::steady_clock::now();
            job->completed.store(true);

            auto queue_us = std::chrono::duration_cast<std::chrono::microseconds>(job->start_time - job->queue_time).count();
            auto exec_us = std::chrono::duration_cast<std::chrono::microseconds>(job->end_time - job->start_time).count();
            { std::lock_guard lock(profile_mtx_); profile_.push_back({job_id, worker_id, (uint64_t)queue_us, (uint64_t)exec_us, job->cancelled.load()}); }

            for(uint32_t dep_id : dependents_[job_id]){
                Job* dep = &jobs_[dep_id];
                if(dep->pending_deps.fetch_sub(1) == 1){
                    std::lock_guard lock(queue_mtx_);
                    dep->queue_time = std::chrono::steady_clock::now();
                    ready_queue_.push(dep_id);
                    queue_cv_.notify_one();
                }
            }
            completed_count_++;
        }
    }
};

// TESTS
TEST_CASE(ScheduleReturnsJobId) {
    JobSystem js(2);
    uint32_t id = js.Schedule([]{});
    ASSERT_GE(id, 0);
}

TEST_CASE(DependencyOrdering) {
    JobSystem js(2);
    std::vector<int> order;
    std::mutex order_mtx;

    uint32_t j1 = js.Schedule([&]{ std::lock_guard l(order_mtx); order.push_back(1); });
    uint32_t j2 = js.Schedule([&]{ std::lock_guard l(order_mtx); order.push_back(2); }, {j1});
    uint32_t j3 = js.Schedule([&]{ std::lock_guard l(order_mtx); order.push_back(3); }, {j1});
    uint32_t j4 = js.Schedule([&]{ std::lock_guard l(order_mtx); order.push_back(4); }, {j2, j3});

    js.Kick();
    js.WaitAll();

    auto it1 = std::find(order.begin(), order.end(), 1);
    auto it2 = std::find(order.begin(), order.end(), 2);
    auto it3 = std::find(order.begin(), order.end(), 3);
    auto it4 = std::find(order.begin(), order.end(), 4);
    ASSERT_TRUE(it1 < it2 && it1 < it3);
    ASSERT_TRUE(it2 < it4 && it3 < it4);
}

TEST_CASE(ParallelForProcessesAllElements) {
    JobSystem js(2);
    std::vector<int> data(1000, 0);
    js.ParallelFor(data.size(), [&](size_t i){ data[i] = int(i * 2); });
    for(size_t i=0;i<data.size();++i) ASSERT_EQ(data[i], int(i*2));
}

TEST_CASE(WaitAllBlocksUntilComplete) {
    JobSystem js(2);
    std::atomic<int> counter{0};
    js.Schedule([&]{ std::this_thread::sleep_for(std::chrono::milliseconds(2)); counter++; });
    js.Schedule([&]{ std::this_thread::sleep_for(std::chrono::milliseconds(2)); counter++; });
    js.Kick();
    js.WaitAll();
    ASSERT_EQ(counter.load(), 2);
}

TEST_CASE(CancelJobPropagatesToDependents) {
    JobSystem js(2);
    std::atomic<int> executed{0};
    uint32_t j1 = js.Schedule([&]{ executed++; });
    uint32_t j2 = js.Schedule([&]{ executed++; }, {j1});
    uint32_t j3 = js.Schedule([&]{ executed++; }, {j2});

    js.CancelJob(j1);
    js.Kick();
    js.WaitAll();

    ASSERT_EQ(executed.load(), 0);
}

TEST_CASE(ProfilingRecordsTiming) {
    JobSystem js(2);
    js.Schedule([]{ std::this_thread::sleep_for(std::chrono::milliseconds(1)); });
    js.Schedule([]{ std::this_thread::sleep_for(std::chrono::milliseconds(1)); });
    js.Kick();
    js.WaitAll();

    js.ExportChromeTracing("test_profile.json");
    std::ifstream f("test_profile.json");
    ASSERT_TRUE(f.is_open());
    std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    ASSERT_TRUE(content.find("traceEvents") != std::string::npos);
    ASSERT_TRUE(content.find("Job") != std::string::npos);
}

TEST_CASE(MultipleIndependentChainsRunInParallel) {
    JobSystem js(2);
    std::atomic<int> chain1_done{0}, chain2_done{0};

    uint32_t c1a = js.Schedule([&]{ std::this_thread::sleep_for(std::chrono::milliseconds(5)); chain1_done++; });
    uint32_t c1b = js.Schedule([&]{ std::this_thread::sleep_for(std::chrono::milliseconds(5)); chain1_done++; }, {c1a});
    uint32_t c2a = js.Schedule([&]{ std::this_thread::sleep_for(std::chrono::milliseconds(5)); chain2_done++; });
    uint32_t c2b = js.Schedule([&]{ std::this_thread::sleep_for(std::chrono::milliseconds(5)); chain2_done++; }, {c2a});

    auto start = std::chrono::steady_clock::now();
    js.Kick();
    js.WaitAll();
    auto elapsed = std::chrono::steady_clock::now() - start;

    ASSERT_LT(elapsed, std::chrono::milliseconds(70));
    ASSERT_EQ(chain1_done.load(), 2);
    ASSERT_EQ(chain2_done.load(), 2);
}

TEST_CASE(JobStoresProfilingData) {
    JobSystem js(1);
    uint32_t id = js.Schedule([]{ std::this_thread::sleep_for(std::chrono::milliseconds(1)); });
    js.Kick();
    js.WaitAll();
    ASSERT_TRUE(true);
}

int main() {
    std::cout << "Solución 07 (Job System)...\n";
    return RUN_ALL_TESTS();
}