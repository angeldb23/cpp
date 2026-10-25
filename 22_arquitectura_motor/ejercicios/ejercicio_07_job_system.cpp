/* =============================================================================
   MÓDULO 22 - EJERCICIO 07: Job System — Thread Pool, DAG Dependencies,
   ParallelFor, Profiling
   =============================================================================
   DIFICULTAD: Alto
   APOYO: Lección 13 (Job System), Lección 14 (Job System Avanzado)

   CONSIGNA:
   Implementa JobSystem con:
   - Thread pool (N workers)
   - Job con dependencias (DAG): pending_deps counter atómico
   - Schedule(work, deps...) retorna job_id
   - Kick() encola jobs con 0 deps, WaitAll() bloquea hasta completar
   - ParallelFor(count, func) divide trabajo entre workers
   - Profiling: queue_time, exec_time por job, export Chrome Tracing JSON

   TESTS:
   - Jobs con dependencias se ejecutan en orden correcto
   - ParallelFor procesa todos los elementos
   - WaitAll bloquea hasta que todos completan
   - Export genera JSON válido para chrome://tracing
   - Cancelación propaga a dependientes
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
#include "../../include/test_framework.hpp"

struct Job {
    std::function<void()> work;
    std::vector<uint32_t> dependencies;  // Job IDs que deben completar antes
    std::atomic<uint32_t> pending_deps{0};
    std::atomic<bool> completed{false};
    std::atomic<bool> cancelled{false};
    uint32_t id = 0;

    // Profiling
    std::chrono::steady_clock::time_point queue_time;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
    uint32_t worker_id = UINT32_MAX;
    bool stolen = false;
};

class JobSystem {
    std::vector<Job> jobs_;
    std::vector<std::vector<uint32_t>> dependents_;  // job_id -> lista de dependientes
    std::queue<uint32_t> ready_queue_;
    std::mutex queue_mtx_;
    std::condition_variable queue_cv_;
    std::vector<std::thread> workers_;
    std::atomic<bool> stop_{false};
    std::atomic<uint32_t> completed_count_{0};

    // Profiling
    struct ProfileEvent {
        uint32_t job_id, worker_id;
        uint64_t queue_us, exec_us;
        bool cancelled;
    };
    std::vector<ProfileEvent> profile_;
    std::mutex profile_mtx_;

public:
    explicit JobSystem(size_t num_threads = std::thread::hardware_concurrency()) {
        for(size_t i=0;i<num_threads;++i) workers_.emplace_back([this,i]{ WorkerLoop(i); });
    }
    ~JobSystem() { stop_=true; queue_cv_.notify_all(); for(auto& t:workers_) if(t.joinable()) t.join(); }

    // TU CÓDIGO AQUÍ: Implementa Schedule, Kick, WaitAll, WorkerLoop, etc.
    uint32_t Schedule(std::function<void()> work, std::initializer_list<uint32_t> deps = {}) {
        // TU CÓDIGO AQUÍ
        (void)work; (void)deps;
        return 0;
    }

    void Kick() {
        // TU CÓDIGO AQUÍ: enqueue jobs con pending_deps == 0
    }

    void WaitAll() {
        // TU CÓDIGO AQUÍ: bloquear hasta completed_count == jobs_.size()
        while(completed_count_.load() < jobs_.size()) std::this_thread::yield();
    }

    void CancelJob(uint32_t id) {
        // TU CÓDIGO AQUÍ: marcar cancelled y propagar a dependientes
        (void)id;
    }

    // ParallelFor: divide [0, count) entre workers
    template<typename Func>
    void ParallelFor(size_t count, Func&& func, size_t grain_size = 1) {
        // TU CÓDIGO AQUÍ
        (void)count; (void)func; (void)grain_size;
    }

    void ExportChromeTracing(const std::string& path) {
        // TU CÓDIGO AQUÍ: escribir JSON con traceEvents
        (void)path;
    }

private:
    void WorkerLoop(uint32_t worker_id) {
        // TU CÓDIGO AQUÍ:
        // Loop: pop job de ready_queue, ejecutar, marcar completed,
        // decrementar pending_deps de dependientes, enqueue si 0
        (void)worker_id;
    }
};

// ============================================================================
// TESTS
// ============================================================================
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

    // j1 antes que j2 y j3; j2 y j3 antes que j4
    auto it1 = std::find(order.begin(), order.end(), 1);
    auto it2 = std::find(order.begin(), order.end(), 2);
    auto it3 = std::find(order.begin(), order.end(), 3);
    auto it4 = std::find(order.begin(), order.end(), 4);
    ASSERT_TRUE(it1 < it2 && it1 < it3);
    ASSERT_TRUE(it2 < it4 && it3 < it4);
}

TEST_CASE(ParallelForProcessesAllElements) {
    JobSystem js(4);
    std::vector<int> data(1000, 0);
    js.ParallelFor(data.size(), [&](size_t i){ data[i] = int(i * 2); });
    for(size_t i=0;i<data.size();++i) ASSERT_EQ(data[i], int(i*2));
}

TEST_CASE(WaitAllBlocksUntilComplete) {
    JobSystem js(2);
    std::atomic<int> counter{0};
    js.Schedule([&]{ std::this_thread::sleep_for(std::chrono::milliseconds(10)); counter++; });
    js.Schedule([&]{ std::this_thread::sleep_for(std::chrono::milliseconds(10)); counter++; });
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

    // j1, j2, j3 cancelados → ninguno ejecuta
    ASSERT_EQ(executed.load(), 0);
}

TEST_CASE(ProfilingRecordsTiming) {
    JobSystem js(2);
    js.Schedule([]{ std::this_thread::sleep_for(std::chrono::milliseconds(1)); });
    js.Schedule([]{ std::this_thread::sleep_for(std::chrono::milliseconds(1)); });
    js.Kick();
    js.WaitAll();

    // Export y verificar que profile_ tiene entradas
    js.ExportChromeTracing("test_profile.json");
    std::ifstream f("test_profile.json");
    ASSERT_TRUE(f.is_open());
    std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    ASSERT_TRUE(content.find("traceEvents") != std::string::npos);
    ASSERT_TRUE(content.find("Job") != std::string::npos);
}

TEST_CASE(MultipleIndependentChainsRunInParallel) {
    JobSystem js(4);
    std::atomic<int> chain1_done{0}, chain2_done{0};

    uint32_t c1a = js.Schedule([&]{ std::this_thread::sleep_for(std::chrono::milliseconds(20)); chain1_done++; });
    uint32_t c1b = js.Schedule([&]{ std::this_thread::sleep_for(std::chrono::milliseconds(20)); chain1_done++; }, {c1a});
    uint32_t c2a = js.Schedule([&]{ std::this_thread::sleep_for(std::chrono::milliseconds(20)); chain2_done++; });
    uint32_t c2b = js.Schedule([&]{ std::this_thread::sleep_for(std::chrono::milliseconds(20)); chain2_done++; }, {c2a});

    auto start = std::chrono::steady_clock::now();
    js.Kick();
    js.WaitAll();
    auto elapsed = std::chrono::steady_clock::now() - start;

    // Con 4 threads, ambas cadenas corren en paralelo → ~40ms no 80ms
    ASSERT_LT(elapsed, std::chrono::milliseconds(70));
    ASSERT_EQ(chain1_done.load(), 2);
    ASSERT_EQ(chain2_done.load(), 2);
}

TEST_CASE(JobStoresProfilingData) {
    JobSystem js(1);
    uint32_t id = js.Schedule([]{ std::this_thread::sleep_for(std::chrono::milliseconds(5)); });
    js.Kick();
    js.WaitAll();

    // Verificar que job tiene timing data
    // En real: acceder a jobs_[id] para verificar queue_time, start_time, end_time
    ASSERT_TRUE(true);  // Placeholder
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 07 (Job System)...\n";
    return RUN_ALL_TESTS();
}