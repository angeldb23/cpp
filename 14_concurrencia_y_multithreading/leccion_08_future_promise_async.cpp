/* =============================================================================
   MÓDULO 14 - LECCIÓN 08: future, promise, async y packaged_task
   =============================================================================
   TEMA 5 de TEMAS.md (2 lecciones): future/promise/async y packaged_task.

   std::future<T> — RESULTADO ASÍNCRONO:
   - Representa un valor que "llegará en el futuro" (quizás ya está).
   - get(): BLOQUEA hasta que el valor esté listo, luego lo devuelve (mueve).
     Solo se puede llamar UNA VEZ (después el future está "vacío").
   - wait(): bloquea hasta listo, no devuelve valor.
   - wait_for/wait_until: timeouts.
   - valid(): true si está asociado a un estado compartido (promise, async, etc.).

   std::promise<T> — PRODUCTOR DEL RESULTADO:
   - Crea el estado compartido y se lo pasa a un future (via get_future()).
   - set_value(val): cumple la promesa con valor (move si T no es ref).
   - set_exception(eptr): cumple con excepción (propaga a future.get()).
   - Solo UN set_value/set_exception permitido (segunda vez -> future_error).
   - Si promise se destruye SIN set_value -> future.get() lanza broken_promise.

   std::async(policy, fn, args...) — LANZAR TAREA ASÍNCRONA:
   - Devuelve future<result_of<fn(args...)>>.
   - Políticas de lanzamiento (bitmask):
     * std::launch::async:   NUEVO hilo garantizado (como thread + future).
     * std::launch::deferred: EJECUCIÓN PEREZOSA (sync): fn corre al llamar get()/wait().
     * std::launch::async | std::launch::deferred (DEFAULT): la implementación elige.
       En práctica (glibc/libstdc++): usa async (nuevo hilo) salvo que el sistema
       esté saturado. NO confíes en el default para lógica crítica.
   - RECOMENDACIÓN: usá std::launch::async EXPLÍCITO si querés paralelismo real.
     Usá deferred solo para evaluación perezosa explícita.

   std::packaged_task<Signature> — ENVAVOLVER CUALQUIER CALLABLE:
   - Wrapper que toma cualquier callable (función, lambda, bind, functor)
     y la convierte en algo que produce un future.
   - get_future(): devuelve el future asociado (solo UNA vez).
   - operator()(args...): ejecuta la tarea, setea valor/excepción en el future.
   - Movible, no copiable. Útil para colas de tareas type-erased (ej: thread pool).

   ESTADO COMPARTIDO (shared state):
   - future, promise, async, packaged_task comparten un "estado" interno
     (control block) con ref-count. El estado vive mientras haya al menos
     un future o promise referenciándolo.

   std::shared_future<T> — FUTURE COPIABLE (múltiples consumidores):
   - future es move-only; shared_future se puede COPIAR.
   - get() devuelve const T& (o T& si future<T&>), se puede llamar MÚLTIPLES veces.
   - Conversión: shared_future sf = std::move(fut);  // fut queda inválido

   EJEMPLOS: promise+future básico, async con políticas, packaged_task en cola,
   shared_future, manejo de excepciones via promise. */

#include <iostream>
#include <thread>
#include <future>
#include <chrono>
#include <vector>
#include <functional>
#include <queue>
#include <memory>
#include <exception>

// -----------------------------------------------------------------------------
// 1) promise + future básico: hilo productor -> hilo consumidor
// -----------------------------------------------------------------------------
void productor_promise(std::promise<int>&& prom) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "[productor] calculando resultado...\n";
    int resultado = 42;
    prom.set_value(resultado);  // cumple la promesa
    std::cout << "[productor] promise cumplida\n";
}

void consumidor_future(std::future<int>&& fut) {
    std::cout << "[consumidor] esperando future...\n";
    int valor = fut.get();  // BLOQUEA hasta que promise set_value
    std::cout << "[consumidor] recibió: " << valor << "\n";
    // fut.get() de nuevo -> UB (future vacío)
}

void demo_promise_future() {
    std::cout << "--- promise + future básico ---\n";
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();  // futuro asociado

    std::thread prod(productor_promise, std::move(prom));
    std::thread cons(consumidor_future, std::move(fut));

    prod.join(); cons.join();
    std::cout << "promise/future OK\n\n";
}

// -----------------------------------------------------------------------------
// 2) promise con excepción: propagar error a future.get()
// -----------------------------------------------------------------------------
void productor_con_error(std::promise<std::string>&& prom, bool fallar) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    if (fallar) {
        try {
            throw std::runtime_error("algo salió mal en el hilo");
        } catch (...) {
            prom.set_exception(std::current_exception());  // propagar excepción
        }
    } else {
        prom.set_value("éxito");
    }
}

void demo_promise_excepcion() {
    std::cout << "--- promise con set_exception ---\n";

    // Caso éxito
    {
        std::promise<std::string> prom;
        auto fut = prom.get_future();
        std::thread t(productor_con_error, std::move(prom), false);
        std::cout << "Resultado: " << fut.get() << "\n";
        t.join();
    }

    // Caso error
    {
        std::promise<std::string> prom;
        auto fut = prom.get_future();
        std::thread t(productor_con_error, std::move(prom), true);
        try {
            std::cout << "Resultado: " << fut.get() << "\n";
        } catch (const std::exception& e) {
            std::cout << "Excepción capturada en get(): " << e.what() << "\n";
        }
        t.join();
    }
    std::cout << "Excepciones via promise OK\n\n";
}

// -----------------------------------------------------------------------------
// 3) std::async con políticas de lanzamiento
// -----------------------------------------------------------------------------
int tarea_costosa(int x) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return x * x;
}

void demo_async_politicas() {
    std::cout << "--- std::async con launch::async vs launch::deferred ---\n";

    // async: NUEVO HILO garantizado
    auto fut_async = std::async(std::launch::async, tarea_costosa, 5);
    std::cout << "[main] async lanzado, haciendo otra cosa...\n";
    std::cout << "[main] get() -> " << fut_async.get() << " (bloqueó hasta fin)\n";

    // deferred: EJECUCIÓN PEREZOSA (sync en get())
    auto fut_deferred = std::async(std::launch::deferred, tarea_costosa, 7);
    std::cout << "[main] deferred creado, NO corrió aún\n";
    std::cout << "[main] get() -> " << fut_deferred.get() << " (corrió AHORA en main)\n";

    // Default: implementación elige (generalmente async)
    auto fut_default = std::async(tarea_costosa, 9);
    std::cout << "[main] default -> " << fut_default.get() << "\n\n";
}

// -----------------------------------------------------------------------------
// 4) packaged_task: envolver callable para cola de tareas
// -----------------------------------------------------------------------------
// Cola simple de packaged_task<void()> (type-erased)
class TaskQueue {
    std::mutex mtx_;
    std::condition_variable cv_;
    std::queue<std::packaged_task<void()>> cola_;
    bool terminado_ = false;

public:
    template <typename F>
    auto enqueue(F&& f) -> std::future<decltype(f())> {
        using R = decltype(f());
        std::packaged_task<R()> task(std::forward<F>(f));
        std::future<R> fut = task.get_future();
        {
            std::lock_guard<std::mutex> lk(mtx_);
            if (terminado_) throw std::runtime_error("cola terminada");
            cola_.emplace(std::move(task));
        }
        cv_.notify_one();
        return fut;
    }

    void worker_loop(int id) {
        while (true) {
            std::packaged_task<void()> task;
            {
                std::unique_lock<std::mutex> lk(mtx_);
                cv_.wait(lk, [this] { return !cola_.empty() || terminado_; });
                if (cola_.empty() && terminado_) break;
                task = std::move(cola_.front());
                cola_.pop();
            }
            task();  // ejecuta y setea resultado en future
            std::cout << "[worker " << id << "] tarea completada\n";
        }
    }

    void terminar() {
        std::lock_guard<std::mutex> lk(mtx_);
        terminado_ = true;
        cv_.notify_all();
    }
};

void demo_packaged_task() {
    std::cout << "--- packaged_task en cola de tareas ---\n";
    TaskQueue queue;

    std::thread w1([&] { queue.worker_loop(1); });
    std::thread w2([&] { queue.worker_loop(2); });

    // Enviar tareas con retorno
    auto f1 = queue.enqueue([] { return 10 + 20; });
    auto f2 = queue.enqueue([] { std::this_thread::sleep_for(std::chrono::milliseconds(50)); return 3.14; });
    auto f3 = queue.enqueue([] { return std::string("hola desde task"); });

    std::cout << "f1: " << f1.get() << "\n";
    std::cout << "f2: " << f2.get() << "\n";
    std::cout << "f3: " << f3.get() << "\n";

    queue.terminar();
    w1.join(); w2.join();
    std::cout << "packaged_task queue OK\n\n";
}

// -----------------------------------------------------------------------------
// 5) shared_future: múltiples consumidores del mismo resultado
// -----------------------------------------------------------------------------
void demo_shared_future() {
    std::cout << "--- std::shared_future (múltiples consumidores) ---\n";

    std::promise<int> prom;
    std::future<int> fut = prom.get_future();

    // Convertir a shared_future (move)
    std::shared_future<int> sf = fut.share();  // o std::shared_future<int> sf = std::move(fut);

    // Múltiples hilos pueden leer el MISMO resultado
    auto lector = [&](int id) {
        std::cout << "[lector " << id << "] esperando...\n";
        int v = sf.get();  // get() en shared_future: devuelve const int&, reentrant
        std::cout << "[lector " << id << "] leyó: " << v << "\n";
    };

    std::thread r1(lector, 1);
    std::thread r2(lector, 2);
    std::thread r3(lector, 3);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    prom.set_value(999);

    r1.join(); r2.join(); r3.join();
    std::cout << "shared_future OK: todos leyeron el mismo valor\n\n";
}

// -----------------------------------------------------------------------------
// 6) future::wait_for / wait_until: sondeo no bloqueante
// -----------------------------------------------------------------------------
void demo_wait_for() {
    std::cout << "--- future::wait_for (sondeo con timeout) ---\n";

    auto fut = std::async(std::launch::async, [] {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        return 123;
    });

    // Sondeo periódico sin bloquear indefinidamente
    while (fut.wait_for(std::chrono::milliseconds(50)) == std::future_status::timeout) {
        std::cout << "[main] aún no listo, haciendo otra cosa...\n";
    }
    std::cout << "[main] listo! resultado: " << fut.get() << "\n\n";
}

// -----------------------------------------------------------------------------
// 7) std::future_error: códigos de error comunes
// -----------------------------------------------------------------------------
void demo_future_error() {
    std::cout << "--- future_error códigos comunes ---\n";

    // 1. promise ya satisfecha (double set_value)
    try {
        std::promise<int> p;
        p.set_value(1);
        p.set_value(2);  // lanza future_error: promise_already_satisfied
    } catch (const std::future_error& e) {
        std::cout << "promise_already_satisfied: " << e.what() << " (code=" << e.code().value() << ")\n";
    }

    // 2. get_future() llamado dos veces
    try {
        std::promise<int> p;
        auto f1 = p.get_future();
        auto f2 = p.get_future();  // lanza: future_already_retrieved
    } catch (const std::future_error& e) {
        std::cout << "future_already_retrieved: " << e.what() << "\n";
    }

    // 3. get() en future movido (vacío)
    try {
        std::promise<int> p;
        auto f = p.get_future();
        auto f2 = std::move(f);
        f.get();  // lanza: no_state
    } catch (const std::future_error& e) {
        std::cout << "no_state (future movido): " << e.what() << "\n";
    }

    // 4. broken_promise: promise destruida sin set_value
    try {
        std::future<int> f;
        {
            std::promise<int> p;
            f = p.get_future();
        }  // p destruida sin set_value -> broken_promise
        f.get();
    } catch (const std::future_error& e) {
        std::cout << "broken_promise: " << e.what() << "\n";
    }
    std::cout << "\n";
}

// -----------------------------------------------------------------------------
// MAIN
// -----------------------------------------------------------------------------
int main() {
    std::cout << "=== LECCIÓN 08: future, promise, async, packaged_task ===\n\n";

    demo_promise_future();
    demo_promise_excepcion();
    demo_async_politicas();
    demo_packaged_task();
    demo_shared_future();
    demo_wait_for();
    demo_future_error();

    std::cout << "=== FIN LECCIÓN 08 ===\n";
    return 0;
}