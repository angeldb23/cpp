/* =============================================================================
   MÓDULO 14 - LECCIÓN 14: Thread pool avanzado — dependencias entre tareas,
   ejecución diferida, C++23 senders/receivers (P2300) preview, async patterns.
   =============================================================================
   TEMA 6 (continuación, lección 2 de 2): Pool de hilos y cola de tareas.

   TAREAS CON DEPENDENCIAS (DAG - Directed Acyclic Graph):
   - Cada tarea tiene contador de prerequisitos (in-degree).
   - Cuando tarea termina, decrementa contadores de sus dependientes.
   - Si contador llega a 0, tarea se encola (ready).
   - Permite expresar: paralelismo de datos, pipelines, fork-join, map-reduce.

   EJECUCIÓN DIFERIDA (Lazy execution):
   - std::execution::par_task (C++23): tareas que pueden bloquear (I/O, locks).
   - Sender/Receiver (P2300): modelo unificado para async:
     * Sender: describe trabajo (puede ser eager o lazy).
     * Receiver: consume resultado (set_value, set_error, set_stopped).
     * Algoritmos: then, let_value, let_error, let_stopped, when_all, transfer.
   - Ejecución: eager (inmediata) vs lazy (bajo demanda al conectar receiver).

   PATRONES ASÍNCRONOS AVANZADOS:
   - Fan-out / Fan-in: una tarea -> muchas paralelas -> una merge.
   - Pipeline streaming: datos fluyen entre stages (backpressure).
   - Timeout/cancelación por tarea (stop_token por tarea).
   - Prioridades y deadlines (real-time soft).

   C++23 SENDERS/RECEIVERS (std::execution, <execution>):
   - std::execution::just(val) -> sender que emite val.
   - std::execution::just_error(err) -> sender que emite error.
   - snd | std::execution::then(fn) -> encadena transformación.
   - snd | std::execution::let_value(fn) -> fn devuelve nuevo sender.
   - std::execution::sync_wait(snd) -> bloquea hasta completar.
   - std::execution::schedule(sch) -> sender que agenda en scheduler.
   - Scheduler: thread_pool, inline_scheduler, new_thread_scheduler.

   NOTA: P2300 (senders/receivers) está en C++23 pero implementaciones
   completas están en librerías externas (stdexec, HPX, folly, oneAPI).
   En estándar: partes en <execution> (C++26 target). Aquí mostramos concepto. */

#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <functional>
#include <memory>
#include <chrono>
#include <algorithm>
#include <optional>
#include <variant>
#include <stop_token>
#include <execution>  // C++17 policies; C++23 senders (parcial)

// -----------------------------------------------------------------------------
// 1) Task DAG con dependencias (versión completa y usable)
// -----------------------------------------------------------------------------
class TaskDAG {
    struct Nodo {
        std::function<void()> trabajo;
        std::shared_ptr<std::atomic<int>> deps_restantes;
        std::vector<size_t> dependientes;
        std::shared_ptr<std::promise<void>> completado;
        bool encolado = false;

        Nodo() : deps_restantes(std::make_shared<std::atomic<int>>(0)),
                 completado(std::make_shared<std::promise<void>>()) {}
        Nodo(Nodo&&) noexcept = default;
        Nodo& operator=(Nodo&&) noexcept = default;
        Nodo(const Nodo&) = delete;
        Nodo& operator=(const Nodo&) = delete;
        Nodo(std::function<void()>&& t, int deps, std::vector<size_t>&& dep, bool enc)
            : trabajo(std::move(t)), deps_restantes(std::make_shared<std::atomic<int>>(deps)),
              dependientes(std::move(dep)), completado(std::make_shared<std::promise<void>>()), encolado(enc) {}
    };

    std::vector<Nodo> nodos_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::queue<size_t> listos_;
    std::atomic<bool> stop_{false};
    std::vector<std::jthread> workers_;

public:
    explicit TaskDAG(size_t hilos = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < hilos; ++i) {
            workers_.emplace_back([this](std::stop_token st) { worker_loop(st); });
        }
    }

    ~TaskDAG() { apagar(); }

    // Añadir tarea, devuelve handle (índice)
    size_t add(std::function<void()> trabajo) {
        std::lock_guard<std::mutex> lk(mtx_);
        size_t idx = nodos_.size();
        nodos_.emplace_back(std::move(trabajo), 0, std::vector<size_t>{}, false);
        return idx;
    }

    // Añadir dependencia: 'despues' depende de 'antes'
    void depends_on(size_t antes, size_t despues) {
        std::lock_guard<std::mutex> lk(mtx_);
        nodos_[antes].dependientes.push_back(despues);
        ++(*nodos_[despues].deps_restantes);
    }

    // Ejecutar todo el DAG
    void run() {
        // Encolar nodos sin deps
        {
            std::lock_guard<std::mutex> lk(mtx_);
            for (size_t i = 0; i < nodos_.size(); ++i) {
                if (nodos_[i].deps_restantes == 0 && !nodos_[i].encolado) {
                    listos_.push(i);
                    nodos_[i].encolado = true;
                }
            }
        }
        cv_.notify_all();

        // Esperar completación de TODOS
        std::vector<std::future<void>> futuros;
        {
            std::lock_guard<std::mutex> lk(mtx_);
            for (auto& n : nodos_) {
                futuros.push_back(n.completado->get_future());
            }
        }
        for (auto& f : futuros) f.wait();
    }

    void apagar() {
        if (stop_.exchange(true)) return;
        cv_.notify_all();
    }

private:
    void worker_loop(std::stop_token st) {
        while (!st.stop_requested() && !stop_) {
            size_t idx;
            {
                std::unique_lock<std::mutex> lk(mtx_);
                cv_.wait(lk, [this, &st] {
                    return !listos_.empty() || st.stop_requested() || stop_;
                });
                if (listos_.empty() && (st.stop_requested() || stop_)) break;
                idx = listos_.front();
                listos_.pop();
            }

            // Ejecutar
            try {
                nodos_[idx].trabajo();
            } catch (...) {
                // En versión real: propagar error a dependientes
            }
            nodos_[idx].completado->set_value();

            // Notificar dependientes
            {
                std::lock_guard<std::mutex> lk(mtx_);
                for (size_t dep : nodos_[idx].dependientes) {
                    if (--(*nodos_[dep].deps_restantes) == 0 && !nodos_[dep].encolado) {
                        listos_.push(dep);
                        nodos_[dep].encolado = true;
                    }
                }
            }
            cv_.notify_one();
        }
    }
};

void demo_task_dag() {
    std::cout << "--- TaskDAG: dependencias entre tareas ---\n";

    TaskDAG dag(4);

    // Grafo:  A
    //        / \
    //       B   C
    //        \ /
    //         D
    auto a = dag.add([] { std::cout << "[A] init\n"; std::this_thread::sleep_for(std::chrono::milliseconds(50)); });
    auto b = dag.add([] { std::cout << "[B] trabajo 1\n"; std::this_thread::sleep_for(std::chrono::milliseconds(30)); });
    auto c = dag.add([] { std::cout << "[C] trabajo 2\n"; std::this_thread::sleep_for(std::chrono::milliseconds(40)); });
    auto d = dag.add([] { std::cout << "[D] merge final\n"; });

    dag.depends_on(a, b);
    dag.depends_on(a, c);
    dag.depends_on(b, d);
    dag.depends_on(c, d);

    auto start = std::chrono::high_resolution_clock::now();
    dag.run();
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "DAG completado en "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms\n\n";
}

// -----------------------------------------------------------------------------
// 2) Fan-out / Fan-in: una tarea lanza N paralelas, espera todas, merge
// -----------------------------------------------------------------------------
void demo_fanout_fanin() {
    std::cout << "--- Fan-out / Fan-in con TaskDAG ---\n";

    TaskDAG dag(4);

    auto inicio = dag.add([] { std::cout << "[inicio] generando items\n"; });

    std::vector<size_t> workers;
    for (int i = 0; i < 5; ++i) {
        workers.push_back(dag.add([i] {
            std::cout << "[worker " << i << "] procesando\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }));
        dag.depends_on(inicio, workers.back());
    }

    auto merge = dag.add([] { std::cout << "[merge] combinando resultados\n"; });
    for (auto w : workers) dag.depends_on(w, merge);

    dag.run();
    std::cout << "Fan-out/fan-in OK\n\n";
}

// -----------------------------------------------------------------------------
// 3) Pipeline streaming con backpressure (canales acotados)
// -----------------------------------------------------------------------------
template <typename T>
class BoundedChannel {
    std::queue<T> q_;
    size_t cap_;
    std::mutex mtx_;
    std::condition_variable cv_push_, cv_pop_;
    bool closed_ = false;

public:
    explicit BoundedChannel(size_t cap) : cap_(cap) {}

    bool send(T val) {
        std::unique_lock<std::mutex> lk(mtx_);
        cv_push_.wait(lk, [this] { return q_.size() < cap_ || closed_; });
        if (closed_) return false;
        q_.push(std::move(val));
        lk.unlock();
        cv_pop_.notify_one();
        return true;
    }

    std::optional<T> recv() {
        std::unique_lock<std::mutex> lk(mtx_);
        cv_pop_.wait(lk, [this] { return !q_.empty() || closed_; });
        if (q_.empty() && closed_) return std::nullopt;
        T val = std::move(q_.front());
        q_.pop();
        lk.unlock();
        cv_push_.notify_one();
        return val;
    }

    void close() {
        std::lock_guard<std::mutex> lk(mtx_);
        closed_ = true;
        cv_push_.notify_all();
        cv_pop_.notify_all();
    }
};

void demo_pipeline_streaming() {
    std::cout << "--- Pipeline streaming con backpressure ---\n";

    BoundedChannel<int> ch1(5), ch2(5), ch3(5);

    // Stage 1: genera números
    std::thread s1([&] {
        for (int i = 1; i <= 20; ++i) {
            if (!ch1.send(i)) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        ch1.close();
    });

    // Stage 2: x2
    std::thread s2([&] {
        while (auto opt = ch1.recv()) {
            ch2.send(*opt * 2);
        }
        ch2.close();
    });

    // Stage 3: +10
    std::thread s3([&] {
        while (auto opt = ch2.recv()) {
            ch3.send(*opt + 10);
        }
        ch3.close();
    });

    // Consumer
    std::thread cons([&] {
        while (auto opt = ch3.recv()) {
            std::cout << "[out] " << *opt << "\n";
        }
    });

    s1.join(); s2.join(); s3.join(); cons.join();
    std::cout << "Pipeline streaming OK\n\n";
}

// -----------------------------------------------------------------------------
// 4) C++23 Senders/Receivers CONCEPTUAL (preview - requiere librería externa)
//    Aquí simulamos la API con tipos propios para mostrar el patrón.
// -----------------------------------------------------------------------------
namespace async {

// Receiver concept: set_value, set_error, set_stopped
template <typename... Values>
struct Receiver {
    virtual void set_value(Values&&...) = 0;
    virtual void set_error(std::exception_ptr) = 0;
    virtual void set_stopped() = 0;
    virtual ~Receiver() = default;
};

// Sender concept: connect(receiver) -> operation_state (conceptual, sin std::execution::connect)
template <typename Sender, typename Receiver>
concept SenderTo = requires(Sender&& s, Receiver&& r) {
    { s.connect(std::forward<Receiver>(r)) };
};

// Operación simple: just_value(sender que emite un valor)
template <typename... Values>
struct JustSender {
    std::tuple<Values...> values_;

    template <typename R>
    struct OpState {
        R receiver_;
        std::tuple<Values...> values_;
        void start() {
            std::apply([&](auto&&... v) {
                receiver_.set_value(std::forward<decltype(v)>(v)...);
            }, values_);
        }
    };

    template <typename R>
    auto connect(R&& r) {
        return OpState<std::remove_cvref_t<R>>{std::forward<R>(r), values_};
    }
};

template <typename... Values>
auto just(Values&&... v) {
    return JustSender<std::decay_t<Values>...>{std::tuple<Values...>(std::forward<Values>(v)...)};
}

// then: encadena transformación
template <typename PrevSender, typename Fn>
struct ThenSender {
    PrevSender prev_;
    Fn fn_;

    template <typename R>
    struct OpState {
        R receiver_;
        // En versión real: conecta prev_ con receiver intermedio que aplica fn
        void start() {
            // Simplificación: ejecutamos prev_ y aplicamos fn
            // Real: usa receiver adapter
        }
    };

    template <typename R>
    auto connect(R&& r) { return OpState<std::remove_cvref_t<R>>{std::forward<R>(r)}; }
};

template <typename Sender, typename Fn>
auto then(Sender&& s, Fn&& fn) {
    return ThenSender<std::decay_t<Sender>, std::decay_t<Fn>>{std::forward<Sender>(s), std::forward<Fn>(fn)};
}

// sync_wait: bloquea hasta completación (simplificado)
template <typename Sender>
auto sync_wait(Sender&& s) {
    // En versión real: crea promise/future, conecta receiver que setea promise
    // Aquí solo demo conceptual
    std::cout << "[sync_wait] ejecutando sender...\n";
    return std::optional<std::tuple<>>{};  // placeholder
}

} // namespace async

void demo_senders_receivers_conceptual() {
    std::cout << "--- C++23 Senders/Receivers (conceptual preview) ---\n";
    std::cout << "P2300: unified async model (sender -> receiver -> operation_state)\n";
    std::cout << "  just(42) | then([](int x){ return x*2; }) | sync_wait()\n";
    std::cout << "Implementaciones completas: stdexec (NVIDIA), HPX, folly, oneAPI\n";
    std::cout << "Estándar C++26 target para <execution> completo\n\n";
}

// -----------------------------------------------------------------------------
// 5) Task con timeout y cancelación por stop_token individual
// -----------------------------------------------------------------------------
struct CancelableTask {
    std::stop_source source;
    std::future<void> future;

    template <typename F>
    CancelableTask(F&& f) {
        auto prom = std::make_shared<std::promise<void>>();
        future = prom->get_future();
        // jthread pasa stop_token como primer argumento al callable
        std::jthread([prom, f = std::forward<F>(f)](std::stop_token token) mutable {
            try {
                // Inyectar token en la tarea si acepta stop_token
                if constexpr (std::invocable<F, std::stop_token>) {
                    f(token);
                } else {
                    f();
                }
                prom->set_value();
            } catch (...) {
                prom->set_exception(std::current_exception());
            }
        }, source.get_token()).detach();
    }

    void cancel() { source.request_stop(); }
    bool wait_for(std::chrono::milliseconds ms) {
        return future.wait_for(ms) == std::future_status::ready;
    }
    void wait() { future.wait(); }
};

void demo_cancelable_tasks() {
    std::cout << "--- Tareas cancelables con stop_token individual ---\n";

    auto tarea_larga = [](std::stop_token st) {
        for (int i = 0; i < 100; ++i) {
            if (st.stop_requested()) {
                std::cout << "[tarea] cancelada en iter " << i << "\n";
                return;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        std::cout << "[tarea] completada normalmente\n";
    };

    CancelableTask t1(tarea_larga);
    CancelableTask t2(tarea_larga);

    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    t1.cancel();  // cancelar solo t1
    std::cout << "[main] t1 cancelada, esperando t2...\n";
    t2.wait();

    std::cout << "Cancelable tasks OK\n\n";
}

// -----------------------------------------------------------------------------
// 6) Scheduler abstraction (concepto C++23)
// -----------------------------------------------------------------------------
class ThreadPoolScheduler {
    // Thread pool simple interno
    std::vector<std::jthread> workers_;
    std::queue<std::function<void()>> q_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::atomic<bool> stop_{false};

public:
    explicit ThreadPoolScheduler(size_t n = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < n; ++i) {
            workers_.emplace_back([this](std::stop_token st) {
                while (!st.stop_requested() && !stop_) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lk(mtx_);
                        cv_.wait(lk, [this, &st] {
                            return !q_.empty() || st.stop_requested() || stop_;
                        });
                        if (q_.empty() && (st.stop_requested() || stop_)) break;
                        task = std::move(q_.front());
                        q_.pop();
                    }
                    task();
                }
            });
        }
    }

    ~ThreadPoolScheduler() {
        stop_ = true;
        cv_.notify_all();
    }

    // schedule() -> sender que ejecuta en este pool
    // (versión simplificada: retorna future)
    template <typename F>
    auto schedule(F&& f) {
        auto prom = std::make_shared<std::promise<void>>();
        auto fut = prom->get_future();
        {
            std::lock_guard<std::mutex> lk(mtx_);
            q_.emplace([prom, f = std::forward<F>(f)]() mutable {
                try { f(); prom->set_value(); }
                catch (...) { prom->set_exception(std::current_exception()); }
            });
        }
        cv_.notify_one();
        return fut;
    }
};

void demo_scheduler() {
    std::cout << "--- Scheduler abstraction (concepto C++23) ---\n";

    ThreadPoolScheduler sched(2);

    auto f1 = sched.schedule([] {
        std::cout << "[sched] tarea 1 en pool\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    });
    auto f2 = sched.schedule([] {
        std::cout << "[sched] tarea 2 en pool\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    });

    f1.wait(); f2.wait();
    std::cout << "Scheduler OK\n\n";
}

// -----------------------------------------------------------------------------
// MAIN
// -----------------------------------------------------------------------------
int main() {
    std::cout << "=== LECCIÓN 14: Pool avanzado, DAG, pipeline, C++23 senders preview ===\n\n";

    demo_task_dag();
    demo_fanout_fanin();
    demo_pipeline_streaming();
    demo_senders_receivers_conceptual();
    demo_cancelable_tasks();
    demo_scheduler();

    std::cout << "=== FIN LECCIÓN 14 ===\n";
    return 0;
}