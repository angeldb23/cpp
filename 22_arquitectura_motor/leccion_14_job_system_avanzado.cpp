/* =============================================================================
   MÓDULO 22 - LECCIÓN 14: Job System Avanzado — Fibers/Coroutines (C++20),
   Integración con ECS, Continuations, Job Priorities, Cancellation
   =============================================================================
   TEMA 8 de TEMAS.md (2/2): Job system — fiber/thread pool, dependencias (DAG),
   std::execution::par, profilado de jobs (continuación).

   C++20 COROUTINES PARA JOBS ASÍNCRONOS:
   ---------------------------------------
   - co_await en JobFuture: suspender coroutine hasta que job complete.
   - No bloquea hilo worker; el hilo puede ejecutar otro job.
   - Requiere: promise_type, awaitable, await_transform.
   - En GCC/Clang: -fcoroutines (GCC 10+, Clang 10+).

   JOB FUTURE / CONTINUATIONS:
   ---------------------------
   auto job = system.Schedule([]{ return ComputeHeavy(); });
   auto result = co_await job;  // Suspende hasta que job termine
   UseResult(result);           // Continúa en mismo o distinto hilo

   JOB PRIORITIES:
   ---------------
   - High: input, physics (frame-critical)
   - Normal: AI, animation
   - Low: resource loading, background tasks
   - Priority queue por worker + global priority queue.

   CANCELLATION:
   -------------
   - Token de cancelación (atomic bool) compartido.
   - Jobs largos verifican token periódicamente.
   - Al cancelar: marcar dependientes como cancelados (cascada).

   INTEGRACIÓN CON ECS SYSTEMS:
   ----------------------------
   - Cada sistema = job (o conjunto de jobs por archetype chunk).
   - System scheduler construye JobGraph cada frame basado en read/write sets.
   - ParallelFor sobre chunks de archetype (cache-friendly).

   ESTA LECCIÓN: Coroutine-based jobs, priorities, cancellation tokens,
   ECS system scheduling automático, frame graph profiling.
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
#include <coroutine>
#include <variant>
#include <optional>

// ============================================================================
// COROUTINE SUPPORT (C++20)
// ============================================================================
// JobFuture: awaitable que se completa cuando el job termina

template<typename T>
struct JobFuture {
    struct Promise;
    using Handle = std::coroutine_handle<Promise>;

    struct Promise {
        std::optional<T> value;
        std::exception_ptr exception;
        std::vector<Handle> continuations;
        std::atomic<bool> ready{false};

        JobFuture get_return_object() { return JobFuture{Handle::from_promise(*this)}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_value(T v) { value = std::move(v); ready = true; Notify(); }
        void unhandled_exception() { exception = std::current_exception(); ready = true; Notify(); }
        void Notify() {
            for(auto h : continuations) h.resume();
            continuations.clear();
        }
    };

    Handle coro_;
    JobFuture(Handle h) : coro_(h) {}
    JobFuture(JobFuture&& other) noexcept : coro_(std::exchange(other.coro_, {})) {}
    ~JobFuture() { if(coro_) coro_.destroy(); }

    bool await_ready() const { return coro_ && coro_.promise().ready; }
    void await_suspend(Handle continuation) {
        if(coro_) coro_.promise().continuations.push_back(continuation);
    }
    T await_resume() {
        if(coro_.promise().exception) std::rethrow_exception(coro_.promise().exception);
        return std::move(*coro_.promise().value);
    }
};

// Void specialization
template<>
struct JobFuture<void> {
    struct Promise {
        std::exception_ptr exception;
        std::vector<std::coroutine_handle<Promise>> continuations;
        std::atomic<bool> ready{false};
        JobFuture get_return_object() { return JobFuture{std::coroutine_handle<Promise>::from_promise(*this)}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() { ready = true; Notify(); }
        void unhandled_exception() { exception = std::current_exception(); ready = true; Notify(); }
        void Notify() { for(auto h : continuations) h.resume(); continuations.clear(); }
    };
    using Handle = std::coroutine_handle<Promise>;
    Handle coro_;
    JobFuture(Handle h) : coro_(h) {}
    JobFuture(JobFuture&& o) noexcept : coro_(std::exchange(o.coro_, {})) {}
    ~JobFuture() { if(coro_) coro_.destroy(); }
    bool await_ready() const { return coro_ && coro_.promise().ready; }
    void await_suspend(Handle cont) { if(coro_) coro_.promise().continuations.push_back(cont); }
    void await_resume() { if(coro_.promise().exception) std::rethrow_exception(coro_.promise().exception); }
};

// ============================================================================
// CANCELLATION TOKEN
// ============================================================================
struct CancellationToken {
    std::atomic<bool> cancelled{false};
    void Cancel() { cancelled.store(true, std::memory_order_release); }
    bool IsCancelled() const { return cancelled.load(std::memory_order_acquire); }
    void ThrowIfCancelled() const { if(IsCancelled()) throw std::runtime_error("Job cancelled"); }
};

// ============================================================================
// JOB PRIORITY
// ============================================================================
enum class JobPriority : uint8_t { High = 0, Normal = 1, Low = 2 };

// ============================================================================
// JOB SYSTEM CON COROUTINES Y PRIORIDADES
// ============================================================================
class AdvancedJobSystem {
    struct JobNode {
        std::function<void()> work;
        JobPriority priority = JobPriority::Normal;
        CancellationToken* cancel_token = nullptr;
        std::atomic<uint32_t> pending_deps{0};
        std::vector<uint32_t> dependents;
        std::atomic<bool> completed{false};
        std::atomic<bool> cancelled{false};

        // Para coroutines
        std::coroutine_handle<> continuation = nullptr;
    };

    std::vector<JobNode> jobs_;
    std::array<std::queue<uint32_t>, 3> priority_queues_;  // High, Normal, Low
    std::mutex queue_mtx_;
    std::condition_variable queue_cv_;
    std::vector<std::thread> workers_;
    std::atomic<bool> stop_{false};
    std::atomic<uint32_t> completed_count_{0};

    // Profiling
    struct ProfileEvent { uint32_t job_id; uint32_t worker_id; JobPriority prio; uint64_t queue_us, exec_us; bool cancelled; };
    std::vector<ProfileEvent> profile_;
    std::mutex profile_mtx_;

public:
    explicit AdvancedJobSystem(size_t num_threads = std::thread::hardware_concurrency()) {
        for(size_t i=0;i<num_threads;++i) workers_.emplace_back([this,i]{ WorkerLoop(i); });
    }
    ~AdvancedJobSystem() { stop_=true; queue_cv_.notify_all(); for(auto& t:workers_) if(t.joinable()) t.join(); }

    // Schedule job simple
    uint32_t Schedule(std::function<void()> work, JobPriority prio = JobPriority::Normal,
                      CancellationToken* token = nullptr) {
        uint32_t id = jobs_.size();
        jobs_.push_back({std::move(work), prio, token, 0, {}, false, false, nullptr});
        EnqueueReady(id);
        return id;
    }

    // Schedule con dependencias
    uint32_t Schedule(std::function<void()> work, std::initializer_list<uint32_t> deps,
                      JobPriority prio = JobPriority::Normal, CancellationToken* token = nullptr) {
        uint32_t id = jobs_.size();
        jobs_.push_back({std::move(work), prio, token, (uint32_t)deps.size(), {}, false, false, nullptr});
        for(uint32_t dep : deps){
            if(dep < jobs_.size()) jobs_[dep].dependents.push_back(id);
        }
        if(jobs_.back().pending_deps.load() == 0) EnqueueReady(id);
        return id;
    }

    // Coroutine job: retorna JobFuture<T>
    template<typename Func>
    auto ScheduleCoro(Func&& func, JobPriority prio = JobPriority::Normal) {
        using Ret = std::invoke_result_t<Func>;
        using FutureType = JobFuture<typename Ret::value_type>;  // Asumiendo JobFuture<T>
        // En real: deducir tipo de retorno de la coroutine
        return Schedule([this, f=std::forward<Func>(func)]() mutable { f(); }, prio);
    }

    void Kick() { completed_count_.store(0); }
    void WaitAll() { while(completed_count_.load() < jobs_.size()) std::this_thread::yield(); }

    void CancelJob(uint32_t id) {
        if(id >= jobs_.size()) return;
        jobs_[id].cancelled.store(true);
        if(jobs_[id].cancel_token) jobs_[id].cancel_token->Cancel();
        // Cancelar dependientes recursivamente
        for(uint32_t dep : jobs_[id].dependents) CancelJob(dep);
    }

    JobNode* GetJob(uint32_t id) { return id < jobs_.size() ? &jobs_[id] : nullptr; }

    void ExportTracing(const std::string& path) {
        std::lock_guard lock(profile_mtx_);
        std::ofstream f(path);
        f << "{\"traceEvents\":[\n";
        for(size_t i=0;i<profile_.size();++i){
            auto& e = profile_[i];
            f << "  {\"name\":\"Job\",\"cat\":\"job\",\"ph\":\"X\","
              << "\"ts\":" << e.queue_us << ",\"dur\":" << e.exec_us << ","
              << "\"pid\":1,\"tid\":" << e.worker_id << ",\"args\":{\"job_id\":" << e.job_id
              << ",\"prio\":" << (int)e.prio << ",\"cancelled\":" << (e.cancelled?"true":"false") << "}}";
            if(i+1<profile_.size()) f << ",";
            f << "\n";
        }
        f << "],\"displayTimeUnit\":\"ms\"}\n";
    }

private:
    void EnqueueReady(uint32_t id) {
        std::lock_guard lock(queue_mtx_);
        priority_queues_[(int)jobs_[id].priority].push(id);
        queue_cv_.notify_one();
    }

    void WorkerLoop(uint32_t worker_id) {
        while(!stop_){
            uint32_t job_id = UINT32_MAX;
            JobPriority prio = JobPriority::Low;

            // Buscar trabajo por prioridad
            {
                std::unique_lock lock(queue_mtx_);
                queue_cv_.wait_for(lock, std::chrono::milliseconds(1), [this]{ return stop_ || !priority_queues_[0].empty() || !priority_queues_[1].empty() || !priority_queues_[2].empty(); });
                if(stop_) break;

                for(int p=0;p<3;++p){
                    if(!priority_queues_[p].empty()){
                        job_id = priority_queues_[p].front(); priority_queues_[p].pop();
                        prio = (JobPriority)p; break;
                    }
                }
            }
            if(job_id == UINT32_MAX) continue;

            JobNode* job = &jobs_[job_id];
            if(job->completed.load() || job->cancelled.load()) continue;
            if(job->cancel_token && job->cancel_token->IsCancelled()){
                job->cancelled.store(true);
                completed_count_++;
                continue;
            }

            auto queue_time = std::chrono::steady_clock::now();
            auto start_time = std::chrono::steady_clock::now();

            try {
                job->work();
            } catch(const std::exception& e) {
                std::cerr << "Job " << job_id << " exception: " << e.what() << "\n";
            }

            auto end_time = std::chrono::steady_clock::now();
            job->completed.store(true);

            // Profile
            auto queue_us = std::chrono::duration_cast<std::chrono::microseconds>(start_time - queue_time).count();
            auto exec_us = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();
            { std::lock_guard lock(profile_mtx_); profile_.push_back({job_id, worker_id, prio, (uint64_t)queue_us, (uint64_t)exec_us, job->cancelled.load()}); }

            // Notify dependents
            for(uint32_t dep_id : job->dependents){
                JobNode* dep = &jobs_[dep_id];
                if(dep->pending_deps.fetch_sub(1) == 1) EnqueueReady(dep_id);
            }
            completed_count_++;
        }
    }
};

// ============================================================================
// ECS SYSTEM SCHEDULER (auto-genera JobGraph de sistemas)
// ============================================================================
struct SystemDesc {
    std::string name;
    ComponentMask reads;    // Componentes que lee
    ComponentMask writes;   // Componentes que escribe
    std::function<void(float)> func;  // dt
    JobPriority priority = JobPriority::Normal;
};

class ECSSystemScheduler {
    std::vector<SystemDesc> systems_;
    AdvancedJobSystem& job_system_;

public:
    ECSSystemScheduler(AdvancedJobSystem& js) : job_system_(js) {}

    void AddSystem(std::string name, ComponentMask reads, ComponentMask writes,
                   std::function<void(float)> func, JobPriority prio = JobPriority::Normal) {
        systems_.push_back({std::move(name), reads, writes, std::move(func), prio});
    }

    void BuildAndRun(float dt) {
        // Construir DAG basado en read/write conflicts
        // Regla: si A escribe X y B lee X → A antes que B
        //        si A escribe X y B escribe X → error (race) o serializar
        std::vector<uint32_t> job_ids;
        for(size_t i=0;i<systems_.size();++i){
            uint32_t id = job_system_.Schedule([sys=systems_[i], dt]{ sys.func(dt); }, systems_[i].priority);
            job_ids.push_back(id);
        }

        // Añadir dependencias
        for(size_t i=0;i<systems_.size();++i){
            for(size_t j=0;j<systems_.size();++j) if(i!=j){
                if((systems_[i].writes & systems_[j].reads) != 0){
                    // i escribe algo que j lee → i antes que j
                    // En real: usar JobGraph API para añadir edge
                }
                if((systems_[i].writes & systems_[j].writes) != 0){
                    // Ambos escriben mismo componente → serializar (i antes que j arbitrariamente)
                }
            }
        }

        job_system_.Kick();
        job_system_.WaitAll();
    }
};

// ============================================================================
// EJEMPLO: COROUTINE JOB CHAIN
// ============================================================================
JobFuture<int> ComputeHeavy(AdvancedJobSystem& js, int input) {
    co_return input * 2;  // Simplificado; en real co_await otro job
}

JobFuture<void> ProcessPipeline(AdvancedJobSystem& js) {
    auto a = co_await ComputeHeavy(js, 10);
    auto b = co_await ComputeHeavy(js, a);
    auto c = co_await ComputeHeavy(js, b);
    std::cout << "  Pipeline result: " << c << "\n";
}

// ============================================================================
// VERIFICACIÓN DE SINTAXIS
// ============================================================================
int main() {
    std::cout << "=== LECCIÓN 14: Job System Avanzado (solo sintaxis) ===\n";

    AdvancedJobSystem js(4);

    // Test priorities
    CancellationToken token;
    uint32_t low = js.Schedule([]{ std::this_thread::sleep_for(std::chrono::milliseconds(10)); std::cout << "  Low priority done\n"; }, JobPriority::Low);
    uint32_t high = js.Schedule([]{ std::cout << "  High priority done\n"; }, JobPriority::High);
    uint32_t normal = js.Schedule([&]{ std::cout << "  Normal (after high)\n"; }, {high}, JobPriority::Normal);

    js.Kick();
    js.WaitAll();

    // Test cancellation
    CancellationToken ct;
    uint32_t long_job = js.Schedule([&ct]{
        for(int i=0;i<100;++i){
            ct.ThrowIfCancelled();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }, JobPriority::Low, &ct);
    uint32_t after_cancel = js.Schedule([]{ std::cout << "  After cancelled job\n"; }, {long_job});

    ct.Cancel();
    js.Kick();
    js.WaitAll();
    std::cout << "Cancellation test done\n";

    // ECS Scheduler demo
    ECSSystemScheduler sched(js);
    sched.AddSystem("Physics", 0b11, 0b01, [](float dt){ std::cout << "  Physics dt=" << dt << "\n"; }, JobPriority::High);
    sched.AddSystem("AI", 0b01, 0b10, [](float dt){ std::cout << "  AI dt=" << dt << "\n"; }, JobPriority::Normal);
    sched.AddSystem("Render", 0b11, 0, [](float dt){ std::cout << "  Render dt=" << dt << "\n"; }, JobPriority::High);
    sched.BuildAndRun(0.016f);

    js.ExportTracing("profile_advanced.json");
    std::cout << "Exported profile_advanced.json\n";

    std::cout << "=== FIN LECCIÓN 14 ===\n";
    return 0;
}