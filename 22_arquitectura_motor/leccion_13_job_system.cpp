/* =============================================================================
   MÓDULO 22 - LECCIÓN 13: Job System — Fiber/Thread Pool, Dependencias (DAG),
   std::execution::par, Profiling de Jobs
   =============================================================================
   TEMA 8 de TEMAS.md (1/2): Job system — fiber/thread pool, dependencias (DAG),
   std::execution::par (puede requerir TBB con GCC; verificar y dejar pool propio
   como alternativa) y profilado de jobs.

   JOB SYSTEM ARQUITECTURA:
   ------------------------
   - Job = unidad de trabajo (función + datos + dependencias).
   - Thread Pool: N workers (típicamente num_cores - 1 para main thread).
   - Queue: work-stealing deque por worker (lock-free) + global queue.
   - Fibers (coroutines): para jobs que esperan (I/O, await other job) sin bloquear hilo.
     C++20 coroutines: co_await en job completion.

   DEPENDENCIAS (DAG):
   -------------------
   JobGraph: nodos = jobs, edges = dependencias (job A debe terminar antes que B).
   - Topological sort para orden de ejecución.
   - Counter de dependencias pendientes por job (atomic).
   - Cuando job termina → decrementar counters de dependientes; si 0 → enqueue.

   STD::EXECUTION::PAR (Parallel STL):
   -----------------------------------
   - std::sort(std::execution::par, ...) → parallel sort.
   - Requiere TBB (Threading Building Blocks) en GCC/libstdc++.
   - En Clang/libc++: puede usar backend propio.
   - Verificar al compilar: #ifdef __GLIBCXX__ → necesitar -ltbb.
   - Fallback: thread pool propio con parallel_for.

   PROFILING DE JOBS:
   ------------------
   - Timing por job (start/end timestamps, CPU cycles).
   - Queue wait time, execution time, steal count.
   - Frame graph: jobs como nodos, edges = dependencias, color = thread.
   - Chrome Tracing format (JSON) para visualizar en chrome://tracing.

   ESTA LECCIÓN: Thread pool work-stealing, JobGraph con DAG, parallel_for,
   fallback sin TBB, profiling con Chrome Tracing export.
   ============================================================================= */
#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <chrono>
#include <algorithm>
#include <memory>
#include <cstdint>
#include <cassert>
#include <array>
#include <unordered_map>
#include <fstream>

// ============================================================================
// CONFIGURACIÓN
// ============================================================================
constexpr size_t MAX_JOBS = 4096;
constexpr size_t MAX_DEPS = 16;

// ============================================================================
// WORK-STEALING DEQUE (simplificado, lock-based para claridad)
// ============================================================================
template<typename T>
class WorkStealingDeque {
    std::vector<T> buffer_;
    std::mutex mtx_;
    size_t head_ = 0, tail_ = 0, mask_;

public:
    explicit WorkStealingDeque(size_t capacity = 1024) {
        size_t cap = 1; while(cap < capacity) cap <<= 1;
        buffer_.resize(cap); mask_ = cap - 1;
    }

    // Push por owner (local thread) - solo tail
    bool PushBottom(T&& item) {
        std::lock_guard lock(mtx_);
        size_t next_tail = (tail_ + 1) & mask_;
        if(next_tail == head_) return false;  // Full
        buffer_[tail_] = std::move(item);
        tail_ = next_tail;
        return true;
    }

    // Pop por owner - desde tail (LIFO para cache locality)
    bool PopBottom(T& out) {
        std::lock_guard lock(mtx_);
        if(head_ == tail_) return false;
        tail_ = (tail_ - 1) & mask_;
        out = std::move(buffer_[tail_]);
        return true;
    }

    // Steal por thief - desde head (FIFO para fairness)
    bool Steal(T& out) {
        std::lock_guard lock(mtx_);
        if(head_ == tail_) return false;
        out = std::move(buffer_[head_]);
        head_ = (head_ + 1) & mask_;
        return true;
    }

    bool Empty() const { std::lock_guard lock(mtx_); return head_ == tail_; }
    size_t Size() const { std::lock_guard lock(mtx_); return (tail_ - head_) & mask_; }
};

// ============================================================================
// JOB DEFINITION
// ============================================================================
struct Job {
    using Func = std::function<void()>;

    Func work;
    std::array<uint32_t, MAX_DEPS> dependencies;  // Job IDs que deben completarse antes
    uint8_t num_deps = 0;
    std::atomic<uint32_t> pending_deps{0};        // Contador atómico de deps pendientes
    std::atomic<bool> completed{false};
    uint32_t id = 0;

    // Profiling
    std::chrono::steady_clock::time_point queue_time;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
    uint32_t worker_id = UINT32_MAX;
    bool stolen = false;
};

// ============================================================================
// JOB GRAPH (DAG)
// ============================================================================
class JobGraph {
    std::vector<Job> jobs_;
    std::vector<std::vector<uint32_t>> dependents_;  // job_id → lista de jobs que dependen de este

public:
    uint32_t AddJob(Job::Func work, std::initializer_list<uint32_t> deps = {}) {
        uint32_t id = jobs_.size();
        jobs_.push_back({});
        jobs_.back().id = id;
        jobs_.back().work = std::move(work);
        jobs_.back().num_deps = deps.size();
        jobs_.back().pending_deps.store(deps.size());
        for(uint32_t dep : deps){
            jobs_.back().dependencies[jobs_.back().num_deps - 1] = dep;  // Simplificado
            if(dep < dependents_.size()) dependents_[dep].push_back(id);
        }
        if(dependents_.size() <= id) dependents_.resize(id + 1);
        return id;
    }

    Job* GetJob(uint32_t id) { return id < jobs_.size() ? &jobs_[id] : nullptr; }
    const std::vector<uint32_t>& GetDependents(uint32_t id) const {
        return id < dependents_.size() ? dependents_[id] : empty_vec;
    }
    size_t JobCount() const { return jobs_.size(); }
    void Clear() { jobs_.clear(); dependents_.clear(); }

private:
    static const std::vector<uint32_t> empty_vec;
};
const std::vector<uint32_t> JobGraph::empty_vec;

// ============================================================================
// THREAD POOL CON WORK-STEALING
// ============================================================================
class JobSystem {
    std::vector<std::unique_ptr<WorkStealingDeque<uint32_t>>> queues_;
    std::vector<std::thread> workers_;
    std::atomic<bool> stop_{false};
    std::atomic<uint32_t> next_job_id_{0};
    std::mutex global_mtx_;
    std::condition_variable global_cv_;
    std::queue<uint32_t> global_queue_;

    JobGraph graph_;
    std::atomic<uint32_t> completed_count_{0};

    // Profiling
    struct ProfileEvent {
        uint32_t job_id;
        uint32_t worker_id;
        bool stolen;
        uint64_t queue_us, exec_us;
    };
    std::vector<ProfileEvent> profile_events_;
    std::mutex profile_mtx_;

public:
    explicit JobSystem(size_t num_threads = std::thread::hardware_concurrency()) {
        // Crear deques (uno por worker + 1 global)
        for(size_t i=0;i<=num_threads;++i) queues_.push_back(std::make_unique<WorkStealingDeque<uint32_t>>(2048));

        for(size_t i=0;i<num_threads;++i){
            workers_.emplace_back([this, i]{ WorkerLoop(i); });
        }
    }
    ~JobSystem() {
        stop_ = true; global_cv_.notify_all();
        for(auto& t : workers_) if(t.joinable()) t.join();
    }

    JobGraph& Graph() { return graph_; }

    // Enqueue job listo (pending_deps == 0)
    void EnqueueReady(uint32_t job_id) {
        Job* job = graph_.GetJob(job_id);
        if(!job) return;
        job->queue_time = std::chrono::steady_clock::now();
        // Push a queue del worker actual si hay, sino global
        // Para simplificar: global queue
        std::lock_guard lock(global_mtx_);
        global_queue_.push(job_id);
        global_cv_.notify_one();
    }

    // Kick: enqueue todos los jobs con 0 deps
    void Kick() {
        for(size_t i=0;i<graph_.JobCount();++i){
            Job* job = graph_.GetJob(i);
            if(job && job->pending_deps.load() == 0) EnqueueReady(i);
        }
        completed_count_.store(0);
    }

    // Wait for all jobs
    void WaitAll() {
        while(completed_count_.load() < graph_.JobCount()){
            std::this_thread::yield();
        }
    }

    // Parallel for (fallback sin TBB)
    template<typename Func>
    void ParallelFor(size_t count, Func&& func, size_t grain_size = 1) {
        if(count == 0) return;
        size_t num_workers = workers_.size();
        size_t per_worker = (count + num_workers - 1) / num_workers;
        per_worker = std::max(per_worker, grain_size);

        std::vector<std::future<void>> futures;
        for(size_t w=0; w<num_workers; ++w){
            size_t start = w * per_worker;
            if(start >= count) break;
            size_t end = std::min(start + per_worker, count);
            futures.push_back(std::async(std::launch::async, [start, end, &func](){
                for(size_t i=start;i<end;++i) func(i);
            }));
        }
        for(auto& f : futures) f.wait();
    }

    // Export Chrome Tracing JSON
    void ExportChromeTracing(const std::string& path) {
        std::lock_guard lock(profile_mtx_);
        std::ofstream f(path);
        f << "{\"traceEvents\":[\n";
        for(size_t i=0;i<profile_events_.size();++i){
            const auto& e = profile_events_[i];
            f << "  {\"name\":\"Job\",\"cat\":\"job\",\"ph\":\"X\","
              << "\"ts\":" << e.queue_us << ",\"dur\":" << (e.exec_us + e.queue_us) << ","
              << "\"pid\":1,\"tid\":" << e.worker_id << ",\"args\":{\"job_id\":" << e.job_id
              << ",\"stolen\":" << (e.stolen?"true":"false") << "}}";
            if(i+1<profile_events_.size()) f << ",";
            f << "\n";
        }
        f << "],\"displayTimeUnit\":\"ms\"}\n";
    }

private:
    void WorkerLoop(uint32_t worker_id) {
        WorkStealingDeque<uint32_t>* my_queue = queues_[worker_id].get();
        WorkStealingDeque<uint32_t>* global_queue = queues_.back().get();

        while(!stop_){
            uint32_t job_id = UINT32_MAX;
            bool stolen = false;

            // 1. Try local queue
            if(my_queue->PopBottom(job_id)){
                // Got local job
            }
            // 2. Try global queue
            else {
                std::unique_lock lock(global_mtx_);
                global_cv_.wait_for(lock, std::chrono::milliseconds(1), [this]{ return stop_ || !global_queue_.empty(); });
                if(stop_) break;
                if(!global_queue_.empty()){
                    job_id = global_queue_.front(); global_queue_.pop();
                }else{
                    // 3. Steal from other workers
                    for(size_t i=0;i<queues_.size()-1;++i){
                        if(i == worker_id) continue;
                        if(queues_[i]->Steal(job_id)){
                            stolen = true; break;
                        }
                    }
                }
            }

            if(job_id == UINT32_MAX) continue;

            Job* job = graph_.GetJob(job_id);
            if(!job || job->completed.load()) continue;

            // Execute
            job->worker_id = worker_id;
            job->stolen = stolen;
            job->start_time = std::chrono::steady_clock::now();

            job->work();

            job->end_time = std::chrono::steady_clock::now();
            job->completed.store(true);

            // Record profile
            auto queue_us = std::chrono::duration_cast<std::chrono::microseconds>(job->start_time - job->queue_time).count();
            auto exec_us = std::chrono::duration_cast<std::chrono::microseconds>(job->end_time - job->start_time).count();
            {
                std::lock_guard lock(profile_mtx_);
                profile_events_.push_back({job_id, worker_id, stolen, (uint64_t)queue_us, (uint64_t)exec_us});
            }

            // Notify dependents
            for(uint32_t dep_id : graph_.GetDependents(job_id)){
                Job* dep = graph_.GetJob(dep_id);
                if(dep && dep->pending_deps.fetch_sub(1) == 1){
                    EnqueueReady(dep_id);
                }
            }

            completed_count_++;
        }
    }
};

// ============================================================================
// EJEMPLO DE USO: Pipeline de frame típico
// ============================================================================
struct FrameData {
    std::vector<Entity> entities;
    std::vector<Transform> transforms;
    std::vector<Velocity> velocities;
};

using Entity = uint32_t;
struct Transform { float x,y,rot,sx,sy; };
struct Velocity { float vx,vy; };

void RunFramePipeline(JobSystem& js, FrameData& frame, float dt) {
    JobGraph& graph = js.Graph();
    graph.Clear();

    // Jobs del frame
    uint32_t job_input = graph.AddJob([&]{
        // Input polling
    });
    uint32_t job_physics = graph.AddJob([&]{
        // Physics step
        for(size_t i=0;i<frame.entities.size();++i){
            frame.transforms[i].x += frame.velocities[i].vx * dt;
            frame.transforms[i].y += frame.velocities[i].vy * dt;
        }
    }, {job_input});
    uint32_t job_ai = graph.AddJob([&]{
        // AI update
    }, {job_input});
    uint32_t job_animation = graph.AddJob([&]{
        // Animation update
    }, {job_physics});
    uint32_t job_culling = graph.AddJob([&]{
        // Frustum culling
    }, {job_physics});
    uint32_t job_render_prep = graph.AddJob([&]{
        // Render command generation
    }, {job_animation, job_culling});
    uint32_t job_audio = graph.AddJob([&]{
        // Audio update
    }, {job_input});
    uint32_t job_end = graph.AddJob([&]{
        // End frame
    }, {job_render_prep, job_audio});

    js.Kick();
    js.WaitAll();
}

// ============================================================================
// VERIFICACIÓN DE SINTAXIS
// ============================================================================
int main() {
    std::cout << "=== LECCIÓN 13: Job System (solo sintaxis) ===\n";

    JobSystem js(4);  // 4 worker threads

    // Test parallel_for
    std::vector<int> data(10000);
    js.ParallelFor(data.size(), [&](size_t i){ data[i] = int(i * 2); });
    std::cout << "ParallelFor: data[0]=" << data[0] << " data[9999]=" << data[9999] << "\n";

    // Test job graph
    JobGraph& graph = js.Graph();
    graph.Clear();

    uint32_t j1 = graph.AddJob([]{ std::cout << "  Job 1 (no deps)\n"; });
    uint32_t j2 = graph.AddJob([]{ std::cout << "  Job 2 (dep 1)\n"; }, {j1});
    uint32_t j3 = graph.AddJob([]{ std::cout << "  Job 3 (dep 1)\n"; }, {j1});
    uint32_t j4 = graph.AddJob([]{ std::cout << "  Job 4 (deps 2,3)\n"; }, {j2, j3});

    js.Kick();
    js.WaitAll();
    std::cout << "All jobs completed\n";

    // Export tracing
    js.ExportChromeTracing("profile.json");
    std::cout << "Exported profile.json (open in chrome://tracing)\n";

    std::cout << "=== FIN LECCIÓN 13 ===\n";
    return 0;
}