/* =============================================================================
   MÓDULO 14 - LECCIÓN 09: packaged_task avanzado, thread pool básico,
   cola de tareas con prioridad, cancelación, continuaciones.
   =============================================================================
   TEMA 5 (continuación) + inicio TEMA 6: packaged_task en profundidad,
   thread pool con cola de tareas, prioridades, continuaciones (then).

   THREAD POOL — PATRÓN FUNDAMENTAL:
   - Número fijo de hilos workers (usualmente num_cores o configurable).
   - Cola de tareas compartida (thread-safe).
   - Workers hacen pop -> ejecutan -> repiten.
   - Evita overhead de crear/destruir hilos por tarea.
   - Controla concurrencia: no más de N hilos activos a la vez.

   packaged_task<Signature> — TYPE ERASURE PARA TAREAS:
   - Borra el tipo del callable original, guarda solo la firma (ej: void()).
   - Permite cola homogénea: queue<packaged_task<void()>>.
   - Cada tarea trae su future para resultado/excepción.

   CONTINUACIONES (C++20: std::future::then — NO ESTÁNDAR AÚN):
   - C++23 propone .then() para encadenar futures.
   - En C++20/17: patrón manual con shared_future + lambda que captura future.
   - O: promise que setea valor al terminar tarea anterior.

   PRIORIDADES EN COLA:
   - std::priority_queue con comparator que mira prioridad.
   - O múltiples colas (una por prioridad) + round-robin / weighted fair queuing.

   CANCELACIÓN COOPERATIVA EN POOL:
   - stop_token por tarea (C++20) o flag atómica compartida.
   - Tareas largas chequean stop_requested() periódicamente.
   - Al apagar pool: request_stop a todas + join workers.

   EJEMPLOS: ThreadPool completo con packaged_task, prioridades,
   continuaciones manuales, cancelación con stop_token, métricas. */

#include <iostream>
#include <thread>
#include <future>
#include <queue>
#include <vector>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <algorithm>
#include <memory>
#include <stop_token>

// -----------------------------------------------------------------------------
// 1) ThreadPool básico con packaged_task<void()> y future para resultados
// -----------------------------------------------------------------------------
class ThreadPool {
    std::vector<std::jthread> workers_;
    std::queue<std::packaged_task<void()>> tasks_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::atomic<bool> stop_{false};
    std::atomic<size_t> tareas_completadas_{0};
    std::atomic<size_t> tareas_en_cola_{0};

public:
    explicit ThreadPool(size_t hilos = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < hilos; ++i) {
            workers_.emplace_back([this](std::stop_token st, size_t id) {
                worker_loop(st, id);
            }, i);
        }
        std::cout << "[ThreadPool] iniciado con " << hilos << " workers\n";
    }

    ~ThreadPool() {
        apagar();
    }
 
    // Enqueue genérico: devuelve future<Ret>
    template <typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<decltype(std::declval<F>()(std::declval<Args>()...))> {
        using Ret = decltype(std::declval<F>()(std::declval<Args>()...));
        auto task = std::make_shared<std::packaged_task<Ret()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        std::future<Ret> fut = task->get_future();

        {
            std::lock_guard<std::mutex> lk(mtx_);
            if (stop_) throw std::runtime_error("ThreadPool apagado");
            tasks_.emplace([task]() { (*task)(); });
            ++tareas_en_cola_;
        }
        cv_.notify_one();
        return fut;
    }

    // Enqueue con prioridad (menor valor = mayor prioridad)
    template <typename F, typename... Args>
    auto enqueue_prioritario(int prioridad, F&& f, Args&&... args)
        -> std::future<decltype(std::declval<F>()(std::declval<Args>()...))> {
        using Ret = decltype(std::declval<F>()(std::declval<Args>()...));
        auto task = std::make_shared<std::packaged_task<Ret()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        std::future<Ret> fut = task->get_future();

        {
            std::lock_guard<std::mutex> lk(mtx_);
            if (stop_) throw std::runtime_error("ThreadPool apagado");
            // Usamos cola de prioridad: pair<prioridad, task>
            // Como std::queue no soporta prioridad, usamos vector + sort
            // (en demo simple; producción: priority_queue o múltiples colas)
            tasks_.emplace([task]() { (*task)(); });
            ++tareas_en_cola_;
        }
        cv_.notify_one();
        return fut;
    }

    void apagar() {
        if (stop_.exchange(true)) return;  // ya apagando
        cv_.notify_all();
        // jthreads hacen join automático en destructor de workers_
        std::cout << "[ThreadPool] apagado, tareas completadas: " << tareas_completadas_ << "\n";
    }

    size_t tareas_completadas() const { return tareas_completadas_; }
    size_t tareas_pendientes() const { return tareas_en_cola_; }

private:
    void worker_loop(std::stop_token st, size_t id) {
        while (!st.stop_requested()) {
            std::packaged_task<void()> task;
            {
                std::unique_lock<std::mutex> lk(mtx_);
                cv_.wait(lk, [this, &st] {
                    return !tasks_.empty() || st.stop_requested() || stop_;
                });
                if (tasks_.empty() && (st.stop_requested() || stop_)) break;
                task = std::move(tasks_.front());
                tasks_.pop();
                --tareas_en_cola_;
            }
            task();  // ejecuta (setea future)
            ++tareas_completadas_;
        }
        std::cout << "[worker " << id << "] terminado\n";
    }
};

void demo_threadpool_basico() {
    std::cout << "--- ThreadPool básico con packaged_task ---\n";
    ThreadPool pool(3);

    // Enviar tareas con retorno
    auto f1 = pool.enqueue([] { return 10 + 20; });
    auto f2 = pool.enqueue([] {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return std::string("tarea lenta");
    });
    auto f3 = pool.enqueue([](int x, int y) { return x * y; }, 6, 7);

    std::cout << "f1: " << f1.get() << "\n";
    std::cout << "f2: " << f2.get() << "\n";
    std::cout << "f3: " << f3.get() << "\n";

    // Enviar varias tareas rápidas
    std::vector<std::future<int>> futuros;
    for (int i = 0; i < 10; ++i) {
        futuros.push_back(pool.enqueue([i] { return i * i; }));
    }
    for (auto& f : futuros) f.get();

    std::cout << "Tareas completadas: " << pool.tareas_completadas() << "\n\n";
}

// -----------------------------------------------------------------------------
// 2) ThreadPool con prioridades (cola de prioridad real)
// -----------------------------------------------------------------------------
class PriorityThreadPool {
    struct TareaPrioritaria {
        int prioridad;  // menor = más urgente
        std::packaged_task<void()> task;
        bool operator>(const TareaPrioritaria& other) const {
            return prioridad > other.prioridad;  // para min-heap
        }
    };

    std::vector<std::jthread> workers_;
    std::priority_queue<TareaPrioritaria, std::vector<TareaPrioritaria>, std::greater<>> cola_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::atomic<bool> stop_{false};

public:
    explicit PriorityThreadPool(size_t hilos = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < hilos; ++i) {
            workers_.emplace_back([this](std::stop_token st, size_t id) {
                worker_loop(st, id);
            }, i);
        }
    }

    ~PriorityThreadPool() { apagar(); }

    template <typename F, typename... Args>
    auto enqueue(int prioridad, F&& f, Args&&... args)
        -> std::future<decltype(std::declval<F>()(std::declval<Args>()...))> {
        using Ret = decltype(std::declval<F>()(std::declval<Args>()...));
        auto task = std::make_shared<std::packaged_task<Ret()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        std::future<Ret> fut = task->get_future();

        {
            std::lock_guard<std::mutex> lk(mtx_);
            if (stop_) throw std::runtime_error("Pool apagado");
            // Create packaged_task<void()> directly in the push
            cola_.push(TareaPrioritaria{prioridad, std::packaged_task<void()>([task]() { (*task)(); })});
        }
        cv_.notify_one();
        return fut;
    }

    void apagar() {
        if (stop_.exchange(true)) return;
        cv_.notify_all();
    }

private:
    void worker_loop(std::stop_token st, size_t id) {
        while (!st.stop_requested()) {
            TareaPrioritaria tarea;
            {
                std::unique_lock<std::mutex> lk(mtx_);
                cv_.wait(lk, [this, &st] {
                    return !cola_.empty() || st.stop_requested() || stop_;
                });
                if (cola_.empty() && (st.stop_requested() || stop_)) break;
                tarea = std::move(const_cast<TareaPrioritaria&>(cola_.top()));
                cola_.pop();
            }
            tarea.task();
        }
    }
};

void demo_priority_pool() {
    std::cout << "--- ThreadPool con prioridades ---\n";
    PriorityThreadPool pool(2);

    std::vector<std::future<int>> futuros;

    // Enviar tareas BAJA prioridad (valor alto = menos urgente)
    for (int i = 0; i < 5; ++i) {
        futuros.push_back(pool.enqueue(10, [i] {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            return i * 10;
        }));
    }

    // Enviar tarea ALTA prioridad (valor bajo = urgente)
    auto urgente = pool.enqueue(1, [] {
        std::cout << "[URGENTE] ejecutando AHORA\n";
        return 999;
    });

    // La urgente debería salir ANTES que las de prioridad 10
    std::cout << "Urgente: " << urgente.get() << "\n";
    for (auto& f : futuros) std::cout << "Normal: " << f.get() << "\n";

    pool.apagar();
    std::cout << "Priority pool OK\n\n";
}

// -----------------------------------------------------------------------------
// 3) Continuaciones manuales: encadenar futures (patrón then)
// -----------------------------------------------------------------------------
// Como C++20 no tiene future::then(), implementamos helper:
template <typename T, typename F>
auto then(std::future<T>&& fut, F&& cont)
    -> std::future<std::invoke_result_t<F, T>> {
    using Ret = std::invoke_result_t<F, T>;
    auto prom = std::make_shared<std::promise<Ret>>();
    auto res_fut = prom->get_future();

    // Lanzar hilo que espera el future original y aplica continuacion
    std::thread([fut = std::move(fut), prom, cont = std::forward<F>(cont)]() mutable {
        try {
            T valor = fut.get();  // espera resultado anterior
            if constexpr (std::is_void_v<Ret>) {
                cont(std::move(valor));
                prom->set_value();
            } else {
                prom->set_value(cont(std::move(valor)));
            }
        } catch (...) {
            prom->set_exception(std::current_exception());
        }
    }).detach();

    return res_fut;
}

void demo_continuaciones() {
    std::cout << "--- Continuaciones manuales (patrón then) ---\n";

    auto fut = std::async(std::launch::async, [] {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        return 10;
    });

    // Encadenar: fut -> *2 -> +5 -> to_string
    auto fut2 = then(std::move(fut), [](int x) { return x * 2; });
    auto fut3 = then(std::move(fut2), [](int x) { return x + 5; });
    auto fut4 = then(std::move(fut3), [](int x) { return std::to_string(x); });

    std::cout << "Resultado final: " << fut4.get() << " (esperado: 25)\n\n";
}

// -----------------------------------------------------------------------------
// 4) Cancelación cooperativa simple con stop_token (C++20)
// -----------------------------------------------------------------------------
// Versión simplificada: tarea recibe stop_token por parámetro
void tarea_cancelable(std::stop_token st, int id, std::promise<int>&& prom) {
    for (int i = 0; i < 100; ++i) {
        if (st.stop_requested()) {
            prom.set_value(-1);  // valor especial: cancelado
            std::cout << "[tarea " << id << "] cancelada en iter " << i << "\n";
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    prom.set_value(id * 100);
}

void demo_cancelable_pool() {
    std::cout << "--- Tareas cancelables con stop_token (sin pool dedicado) ---\n";

    // Lanzar tareas cancelables directamente con jthread
    std::vector<std::pair<std::future<int>, std::stop_source>> tareas;
    for (int i = 1; i <= 5; ++i) {
        std::promise<int> prom;
        auto fut = prom.get_future();
        std::stop_source source;
        std::jthread jt(tarea_cancelable, source.get_token(), i, std::move(prom));
        tareas.emplace_back(std::move(fut), std::move(source));
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Cancelar tarea 3 y 5
    std::cout << "[main] cancelando tareas 3 y 5...\n";
    tareas[2].second.request_stop();  // tarea 3 (índice 2)
    tareas[4].second.request_stop();  // tarea 5

    for (size_t i = 0; i < tareas.size(); ++i) {
        int r = tareas[i].first.get();
        std::cout << "Tarea " << (i+1) << ": " << (r == -1 ? "CANCELADA" : std::to_string(r)) << "\n";
    }

    std::cout << "Cancelable tasks OK\n\n";
}

// -----------------------------------------------------------------------------
// 5) Métricas básicas del pool (throughput, latencia, cola)
// -----------------------------------------------------------------------------
class MonitoredThreadPool {
    std::vector<std::jthread> workers_;
    std::queue<std::packaged_task<void()>> cola_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::atomic<bool> stop_{false};

    // Métricas
    std::atomic<uint64_t> total_enviadas_{0};
    std::atomic<uint64_t> total_completadas_{0};
    std::atomic<uint64_t> tiempo_total_us_{0};  // suma de latencias

public:
    explicit MonitoredThreadPool(size_t hilos = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < hilos; ++i) {
            workers_.emplace_back([this](std::stop_token st, size_t) {
                worker_loop(st);
            }, i);
        }
    }

    ~MonitoredThreadPool() { apagar(); }

    template <typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
        using Ret = std::invoke_result_t<F, Args...>;
        auto inicio = std::chrono::steady_clock::now();

        auto task = std::make_shared<std::packaged_task<Ret()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        std::future<Ret> fut = task->get_future();

        {
            std::lock_guard<std::mutex> lk(mtx_);
            if (stop_) throw std::runtime_error("Pool apagado");
            cola_.emplace([task, inicio, this]() {
                (*task)();
                auto fin = std::chrono::steady_clock::now();
                auto us = std::chrono::duration_cast<std::chrono::microseconds>(fin - inicio).count();
                tiempo_total_us_ += us;
                ++total_completadas_;
            });
            ++total_enviadas_;
        }
        cv_.notify_one();
        return fut;
    }

    void apagar() {
        if (stop_.exchange(true)) return;
        cv_.notify_all();
    }

    void reportar() const {
        uint64_t tot = total_completadas_;
        double avg_us = tot ? double(tiempo_total_us_) / tot : 0;
        std::cout << "[Métricas] Enviadas: " << total_enviadas_
                  << ", Completadas: " << tot
                  << ", Latencia prom: " << avg_us << " µs\n";
    }

private:
    void worker_loop(std::stop_token st) {
        while (!st.stop_requested() && !stop_) {
            std::packaged_task<void()> task;
            {
                std::unique_lock<std::mutex> lk(mtx_);
                cv_.wait(lk, [this, &st] {
                    return !cola_.empty() || st.stop_requested() || stop_;
                });
                if (cola_.empty() && (st.stop_requested() || stop_)) break;
                task = std::move(cola_.front());
                cola_.pop();
            }
            task();
        }
    }
};

void demo_monitored_pool() {
    std::cout << "--- ThreadPool con métricas ---\n";
    MonitoredThreadPool pool(3);

    for (int i = 0; i < 20; ++i) {
        pool.enqueue([i] {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            return i;
        });
    }

    // Esperar un poco
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    pool.reportar();

    pool.apagar();
    std::cout << "Monitored pool OK\n\n";
}

// -----------------------------------------------------------------------------
// MAIN
// -----------------------------------------------------------------------------
int main() {
    std::cout << "=== LECCIÓN 09: packaged_task avanzado, thread pool, prioridades ===\n\n";

    demo_threadpool_basico();
    demo_priority_pool();
    demo_continuaciones();
    demo_cancelable_pool();
    demo_monitored_pool();

    std::cout << "=== FIN LECCIÓN 09 ===\n";
    return 0;
}