/* =============================================================================
   MÓDULO 14 - LECCIÓN 10: Thread pool avanzado — work stealing, dependencias,
   paralelismo estructurado, integración con algoritmos paralelos (C++17/20).
   =============================================================================
   TEMA 6 de TEMAS.md (2 lecciones): Pool de hilos y cola de tareas.

   WORK STEALING (robo de trabajo):
   - Cada worker tiene su PROPIA cola local (deque).
   - Push: encola en SU cola local (rápido, sin contención global).
   - Pop: toma de SU cola (LIFO para cache locality).
   - Si cola propia vacía: ROBA de cola AJENA (FIFO para balancear).
   - Reduce drásticamente contención en cola global.
   - Usado en: Intel TBB, Microsoft PPL, .NET ThreadPool, Go scheduler, Java ForkJoinPool.

   DEPENDENCIAS ENTRE TAREAS (DAG de tareas):
   - Tarea B depende de A: B no empieza hasta que A termina.
   - Implementación: contador de dependencias (atomic) + notificación.
   - Cuando A termina, decrementa contadores de sus dependientes.
   - Si contador llega a 0, encola el dependiente.
   - Permite expresar pipelines, grafos de computación, etc.

   PARALELISMO ESTRUCTURADO (C++17/20/23):
   - std::execution::par / par_unseq / seq / unseq (políticas de ejecución).
   - std::for_each, transform, reduce, sort, etc. con política paralela.
   - C++23: std::execution::par_task (para tareas con bloqueo).
   - Requiere: iteradores random_access, operaciones sin data races.
   - El runtime (TBB, oneDPL, HPX) usa thread pool interno.

   std::jthread + stop_token + condition_variable = BLOQUES ESTRUCTURADOS:
   - C++23: std::sync_wait, std::when_all, std::when_any (senders/receivers).
   - Mientras tanto: patrones manuales con future/promise + join automático.

   EJEMPLOS: WorkStealingPool simplificado, TaskGraph con dependencias,
   algoritmos paralelos (std::transform_reduce), structured concurrency básico. */

#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <deque>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <functional>
#include <memory>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <random>
#include <stop_token>

// Helper: paralelismo manual (equivalente a std::execution::par)
template <typename Iterator, typename T, typename BinaryOp, typename UnaryOp>
T parallel_transform_reduce(Iterator begin, Iterator end, T init,
                            BinaryOp reduce_op, UnaryOp transform_op,
                            size_t num_threads = 0) {
    size_t n = std::distance(begin, end);
    if (n == 0) return init;
    if (num_threads == 0) num_threads = std::thread::hardware_concurrency();
    num_threads = std::min(num_threads, n);

    size_t chunk = (n + num_threads - 1) / num_threads;
    std::vector<std::future<T>> futures;
    futures.reserve(num_threads);

    Iterator it = begin;
    for (size_t t = 0; t < num_threads; ++t) {
        Iterator chunk_begin = it;
        Iterator chunk_end = (t == num_threads - 1) ? end : std::next(it, chunk);
        if (chunk_begin == chunk_end) break;
        futures.push_back(std::async(std::launch::async,
            [chunk_begin, chunk_end, init, reduce_op, transform_op]() mutable {
                T local = init;
                for (auto it2 = chunk_begin; it2 != chunk_end; ++it2) {
                    local = reduce_op(local, transform_op(*it2));
                }
                return local;
            }));
        it = chunk_end;
        if (it == end) break;
    }

    T result = init;
    for (auto& f : futures) result = reduce_op(result, f.get());
    return result;
}

void parallel_sort(std::vector<int>& v, size_t num_threads = 0) {
    if (num_threads == 0) num_threads = std::thread::hardware_concurrency();
    if (v.size() < 1000 || num_threads <= 1) {
        std::sort(v.begin(), v.end());
        return;
    }

    size_t n = v.size();
    size_t chunk = (n + num_threads - 1) / num_threads;
    std::vector<std::future<void>> futures;
    futures.reserve(num_threads);

    for (size_t t = 0; t < num_threads; ++t) {
        size_t begin = t * chunk;
        size_t end = (t == num_threads - 1) ? n : (t + 1) * chunk;
        if (begin >= end) break;
        futures.push_back(std::async(std::launch::async, [&v, begin, end]() {
            std::sort(v.begin() + begin, v.begin() + end);
        }));
    }
    for (auto& f : futures) f.get();

    size_t stride = chunk;
    while (stride < n) {
        for (size_t i = 0; i < n; i += 2 * stride) {
            size_t mid = std::min(i + stride, n);
            size_t end = std::min(i + 2 * stride, n);
            if (mid < end) std::inplace_merge(v.begin() + i, v.begin() + mid, v.begin() + end);
        }
        stride *= 2;
    }
}

// -----------------------------------------------------------------------------
// 1) WorkStealingPool simplificado (cada worker tiene deque local)
// -----------------------------------------------------------------------------
class WorkStealingPool {
    struct Worker {
        std::deque<std::packaged_task<void()>> local_queue;
        std::mutex mtx;  // protege local_queue para robo
    };

    std::vector<std::unique_ptr<Worker>> workers_;
    std::vector<std::jthread> threads_;
    std::atomic<bool> stop_{false};
    std::atomic<size_t> total_tasks_{0};

public:
    explicit WorkStealingPool(size_t n = std::thread::hardware_concurrency()) {
        workers_.reserve(n);
        for (size_t i = 0; i < n; ++i) workers_.push_back(std::make_unique<Worker>());

        for (size_t i = 0; i < n; ++i) {
            threads_.emplace_back([this, i](std::stop_token st) {
                worker_loop(st, i);
            });
        }
        std::cout << "[WorkStealingPool] " << n << " workers con deques locales\n";
    }

    ~WorkStealingPool() { apagar(); }

    template <typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
        using Ret = std::invoke_result_t<F, Args...>;
        auto task = std::make_shared<std::packaged_task<Ret()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        std::future<Ret> fut = task->get_future();

        // Encolar en cola local de un worker aleatorio (distribución inicial)
        size_t idx = total_tasks_++ % workers_.size();
        {
            std::lock_guard<std::mutex> lk(workers_[idx]->mtx);
            workers_[idx]->local_queue.emplace_back([task]() { (*task)(); });
        }
        return fut;
    }

    void apagar() {
        if (stop_.exchange(true)) return;
        for (auto& t : threads_) t.request_stop();
        // jthreads join automático
    }

    size_t total_tasks() const { return total_tasks_; }

private:
    // Robar tarea de otro worker (FIFO: pop_front)
    bool try_steal(size_t victim_idx, std::packaged_task<void()>& out) {
        auto& victim = workers_[victim_idx];
        std::lock_guard<std::mutex> lk(victim->mtx);
        if (victim->local_queue.empty()) return false;
        out = std::move(victim->local_queue.front());
        victim->local_queue.pop_front();
        return true;
    }

    void worker_loop(std::stop_token st, size_t my_idx) {
        std::mt19937 rng(my_idx + std::chrono::steady_clock::now().time_since_epoch().count());

        while (!st.stop_requested() && !stop_) {
            std::packaged_task<void()> task;
            bool got = false;

            // 1. Intentar tomar de PROPIA cola (LIFO: pop_back para cache locality)
            {
                std::lock_guard<std::mutex> lk(workers_[my_idx]->mtx);
                if (!workers_[my_idx]->local_queue.empty()) {
                    task = std::move(workers_[my_idx]->local_queue.back());
                    workers_[my_idx]->local_queue.pop_back();
                    got = true;
                }
            }

            // 2. Si propia vacía, ROBAR de victim aleatorio (FIFO: pop_front)
            if (!got) {
                for (int attempts = 0; attempts < 3 && !got; ++attempts) {
                    size_t victim = rng() % workers_.size();
                    if (victim != my_idx) got = try_steal(victim, task);
                }
            }

            // 3. Si nada, yield breve
            if (!got) {
                std::this_thread::yield();
                continue;
            }

            task();  // ejecutar
        }
    }
};

void demo_work_stealing() {
    std::cout << "--- WorkStealingPool ---\n";
    WorkStealingPool pool(4);

    std::vector<std::future<int>> futuros;
    for (int i = 0; i < 100; ++i) {
        futuros.push_back(pool.submit([i] {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
            return i * 2;
        }));
    }

    long long suma = 0;
    for (auto& f : futuros) suma += f.get();
    std::cout << "Suma: " << suma << " (esperado: " << (99*100) << ")\n";
    std::cout << "Total tasks submitted: " << pool.total_tasks() << "\n\n";

    pool.apagar();
}

// -----------------------------------------------------------------------------
// 2) TaskGraph: DAG de tareas con dependencias (contadores atómicos)
// -----------------------------------------------------------------------------
class TaskGraph {
    struct Nodo {
        std::packaged_task<void()> tarea;
        std::shared_ptr<std::atomic<int>> dependencias_restantes;
        std::vector<size_t> dependientes;
        bool encolado = false;

        Nodo() : dependencias_restantes(std::make_shared<std::atomic<int>>(0)) {}
        Nodo(Nodo&&) = default;
        Nodo& operator=(Nodo&&) = default;
        Nodo(const Nodo&) = delete;
        Nodo& operator=(const Nodo&) = delete;
        Nodo(std::packaged_task<void()>&& t, int deps, std::vector<size_t>&& dep, bool enc)
            : tarea(std::move(t)), dependencias_restantes(std::make_shared<std::atomic<int>>(deps)),
              dependientes(std::move(dep)), encolado(enc) {}
    };

    std::vector<Nodo> nodos_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::queue<size_t> listos_;
    std::atomic<bool> stop_{false};
    std::vector<std::jthread> workers_;

public:
    explicit TaskGraph(size_t hilos = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < hilos; ++i) {
            workers_.emplace_back([this](std::stop_token st) { worker_loop(st); });
        }
    }

    ~TaskGraph() { apagar(); }

    // Añadir tarea, devuelve índice del nodo
    template <typename F>
    size_t add_task(F&& f) {
        std::lock_guard<std::mutex> lk(mtx_);
        size_t idx = nodos_.size();
        nodos_.emplace_back(std::packaged_task<void()>(std::forward<F>(f)), 0, std::vector<size_t>{}, false);
        return idx;
    }

    // Añadir dependencia: 'dependiente' espera a 'requisito'
    void add_dependency(size_t requisito, size_t dependiente) {
        std::lock_guard<std::mutex> lk(mtx_);
        nodos_[requisito].dependientes.push_back(dependiente);
        ++(*nodos_[dependiente].dependencias_restantes);
    }

    // Ejecutar grafo completo
    void ejecutar() {
        // Encolar nodos sin dependencias inicialmente
        {
            std::lock_guard<std::mutex> lk(mtx_);
            for (size_t i = 0; i < nodos_.size(); ++i) {
                if (*nodos_[i].dependencias_restantes == 0 && !nodos_[i].encolado) {
                    listos_.push(i);
                    nodos_[i].encolado = true;
                }
            }
        }
        cv_.notify_all();

        // Esperar a que todos terminen (polling simple para demo)
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            {
                std::lock_guard<std::mutex> lk(mtx_);
                bool all_done = true;
                for (auto& n : nodos_) {
                    if (*n.dependencias_restantes >= 0 && !n.tarea.valid()) continue; // ya ejecutado
                    all_done = false;
                    break;
                }
                if (all_done) break;
            }
        }
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

            // Ejecutar tarea
            nodos_[idx].tarea();

            // Notificar dependientes
            {
                std::lock_guard<std::mutex> lk(mtx_);
                for (size_t dep : nodos_[idx].dependientes) {
                    if (--(*nodos_[dep].dependencias_restantes) == 0) {
                        listos_.push(dep);
                    }
                }
            }
            cv_.notify_one();
        }
    }
};

void demo_task_graph() {
    std::cout << "--- TaskGraph con dependencias (DAG) ---\n";
    TaskGraph g(3);

    // Grafo: A -> B, A -> C, B -> D, C -> D
    // A: init, B y C: procesan en paralelo, D: merge
    auto a = g.add_task([] { std::cout << "[A] init\n"; std::this_thread::sleep_for(std::chrono::milliseconds(50)); });
    auto b = g.add_task([] { std::cout << "[B] proceso 1\n"; std::this_thread::sleep_for(std::chrono::milliseconds(30)); });
    auto c = g.add_task([] { std::cout << "[C] proceso 2\n"; std::this_thread::sleep_for(std::chrono::milliseconds(40)); });
    auto d = g.add_task([] { std::cout << "[D] merge final\n"; });

    g.add_dependency(a, b);
    g.add_dependency(a, c);
    g.add_dependency(b, d);
    g.add_dependency(c, d);

    g.ejecutar();
    // Nota: demo simplificada; en versión real esperaríamos completación
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    g.apagar();
    std::cout << "TaskGraph OK\n\n";
}

// -----------------------------------------------------------------------------
// 3) Algoritmos paralelos manuales (equivalentes a std::execution::par)
// -----------------------------------------------------------------------------
void demo_algoritmos_paralelos() {
    std::cout << "--- Algoritmos paralelos manuales (equivalentes a std::execution::par) ---\n";

    const size_t N = 1'000'000;
    std::vector<double> datos(N);
    std::iota(datos.begin(), datos.end(), 1.0);  // 1, 2, 3, ...

    // transform_reduce paralelo manual: map + reduce en uno
    auto inicio = std::chrono::high_resolution_clock::now();
    double suma_par = parallel_transform_reduce(
        datos.begin(), datos.end(), 0.0,
        std::plus<>(), [](double x) { return x * x; });
    auto fin = std::chrono::high_resolution_clock::now();
    auto dur_par = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio);

    // Secuencial para comparar
    inicio = std::chrono::high_resolution_clock::now();
    double suma_seq = std::transform_reduce(
        datos.begin(), datos.end(),
        0.0,
        std::plus<>(),
        [](double x) { return x * x; }
    );
    fin = std::chrono::high_resolution_clock::now();
    auto dur_seq = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio);

    std::cout << "Paralelo (manual): " << suma_par << " en " << dur_par.count() << " ms\n";
    std::cout << "Secuencial (std): " << suma_seq << " en " << dur_seq.count() << " ms\n";
    std::cout << "Speedup: " << double(dur_seq.count()) / dur_par.count() << "x\n\n";

    // sort paralelo manual
    std::vector<int> v(10000);
    std::iota(v.begin(), v.end(), 0);
    std::shuffle(v.begin(), v.end(), std::mt19937{42});
    parallel_sort(v);
    std::cout << "parallel_sort (manual) OK: primero=" << v.front() << ", último=" << v.back() << "\n\n";
}

// -----------------------------------------------------------------------------
// 4) Paralelismo estructurado: when_all / when_any manual con futures
// -----------------------------------------------------------------------------
// Helper para extraer value_type de future
template <typename T>
struct future_value_type;

template <typename T>
struct future_value_type<std::future<T>> {
    using type = T;
};

template <typename... Futures>
auto when_all(Futures&&... futs) {
    using Tuple = std::tuple<typename future_value_type<std::decay_t<Futures>>::type...>;
    auto prom = std::make_shared<std::promise<Tuple>>();
    auto fut = prom->get_future();

    std::thread([prom, ... futs = std::forward<Futures>(futs)]() mutable {
        try {
            prom->set_value(Tuple{futs.get()...});
        } catch (...) {
            prom->set_exception(std::current_exception());
        }
    }).detach();

    return fut;
}

void demo_when_all() {
    std::cout << "--- when_all manual (paralelismo estructurado) ---\n";

    auto f1 = std::async(std::launch::async, [] {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        return 1;
    });
    auto f2 = std::async(std::launch::async, [] {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        return 2.5;
    });
    auto f3 = std::async(std::launch::async, [] {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        return std::string("tres");
    });

    auto all = when_all(std::move(f1), std::move(f2), std::move(f3));
    auto [r1, r2, r3] = all.get();  // structured binding (C++17)

    std::cout << "Resultados: " << r1 << ", " << r2 << ", " << r3 << "\n";
    std::cout << "when_all: espera TODOS, devuelve tupla\n\n";
}

// -----------------------------------------------------------------------------
// 5) Pipeline paralelo manual (equivalente a std::execution::par)
// -----------------------------------------------------------------------------
void demo_pipeline_paralelo() {
    std::cout << "--- Pipeline con algoritmos paralelos manuales ---\n";

    std::vector<int> entrada(10000);
    std::iota(entrada.begin(), entrada.end(), 1);

    // Paralelo: filter (pares) -> transform (cuadrado) -> reduce (suma)
    auto inicio = std::chrono::high_resolution_clock::now();

    long long suma = parallel_transform_reduce(
        entrada.begin(), entrada.end(), 0LL,
        std::plus<>(),
        [](int x) { return (x % 2 == 0) ? (long long)x * x : 0LL; }
    );

    auto fin = std::chrono::high_resolution_clock::now();
    std::cout << "Suma cuadrados de pares (par manual): " << suma
              << " en " << std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio).count() << " ms\n\n";
}

// -----------------------------------------------------------------------------
// MAIN
// -----------------------------------------------------------------------------
int main() {
    std::cout << "=== LECCIÓN 10: Thread pool avanzado — work stealing, DAG, alg. paralelos ===\n\n";

    demo_work_stealing();
    demo_task_graph();
    demo_algoritmos_paralelos();
    demo_when_all();
    demo_pipeline_paralelo();

    std::cout << "=== FIN LECCIÓN 10 ===\n";
    return 0;
}