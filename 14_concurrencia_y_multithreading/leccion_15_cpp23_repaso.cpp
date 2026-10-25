/* =============================================================================
   MÓDULO 14 - LECCIÓN 15: C++23/26 preview — structured concurrency,
   senders/receivers (P2300), exec::static_thread_pool, repaso y mejores prácticas.
   =============================================================================
   TEMA 8 (continuación) + REPASO GENERAL: Políticas de ejecución STL,
   medición de escalado, y visión de futuro (C++23/26).

   ESTRUCTURED CONCURRENCY (concurrencia estructurada):
   - Principio: "el scope del padre vive más que sus hijos".
   - std::jthread (C++20): join automático en dtor = structured concurrency básico.
   - C++23/26: std::sync_wait, std::when_all, std::when_any, std::ensure_started.
   - Task scopes: nurseries donde TODAS las tareas hijas deben terminar antes
     de que el scope padre salga (cancelación automática si una falla).

   SENDERS/RECEIVERS (P2300) — MODELO UNIFICADO ASÍNCRONO:
   - Sender: describe trabajo asíncrono (lazy o eager).
   - Receiver: 3 callbacks: set_value(T...), set_error(E), set_stopped().
   - OperationState: connect(sender, receiver) -> start() -> ejecuta.
   - Scheduler: agenda trabajo (thread_pool, inline, new_thread, gpu, etc.).
   - Algoritmos composables: then, let_value, let_error, let_stopped,
     when_all, when_any, transfer, continue_on, bulk, split, ensure_started.
   - Ventaja: composabilidad, cancelación propagada, backpressure, tipos.

   std::execution::static_thread_pool (C++26 propuesto):
   - Pool de hilos con scheduler integrado.
   - schedule() -> sender que corre en el pool.
   - Permite: exec::on(pool, work) | then(...) | sync_wait().

   MEJORES PRÁCTICAS RESUMEN:
   1. PREFERIR jthread sobre thread (join auto + stop_token).
   2. USAR RAII para locks: lock_guard (simple), unique_lock (flexible),
      scoped_lock (múltiples), shared_lock (lectura compartida).
   3. EVITAR data races: atomic para contadores, mutex para invariantes complejos.
   4. PREDICADOS en cv.wait(lk, pred) SIEMPRE (spurious wakeups).
   5. NOTIFICAR FUERA del lock (menos contención).
   6. PARTICIONAR datos (sharding, thread_local) antes que lockear.
   7. INMUTABILIDAD + publicación atómica (release/acquire) = zero locks.
   8. FUTURE/PROMISE para resultados asíncronos; packaged_task para colas.
   8. THREAD POOL fijo para evitar overhead de crear hilos.
   9. ALGORITMOS PARALELOS STL (std::execution::par) para loops de datos.
   10. MEDIR con warm-up, mediana, wall-clock; evitar false sharing (alignas 64).
   11. CANCELACIÓN cooperativa con stop_token (C++20) en tareas largas.
   12. LATCH/BARRIER/SEMAPHORE (C++20) para coordinación estructurada.

   ANTIPATRONES A EVITAR:
   - lock()/unlock() manual (excepciones -> leak).
   - double-checked locking sin atomic + acquire/release (usar call_once).
   - spinlocks en user-space salvo casos muy específicos (kernel, real-time).
   - busy-wait sin yield (quema CPU).
   - detach() sin plan de limpieza (recursos, destructores).
   - shared_ptr en ciclos sin weak_ptr (memory leak).
   - atomic con memory_order_relaxed cuando se necesita sincronizar otras vars.
   - std::async default (launch policy indeterminada) -> usar launch::async.
   - condition_variable sin predicado (spurious wakeups = bugs Heisenberg).

   HERRAMIENTAS DE DIAGNÓSTICO:
   - ThreadSanitizer (TSan): -fsanitize=thread (detecta data races).
   - Helgrind (Valgrind): data races, deadlocks.
   - perf / VTune / Instruments: contención, cache misses, escalado.
   - std::thread::hardware_concurrency(): guía para sizing de pools.

   EJEMPLOS: structured concurrency con jthread nursery, resumen de decisiones,
   checklist de revisión de código concurrente. */

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <chrono>
#include <algorithm>
#include <memory>
#include <functional>
#include <stop_token>
#include <semaphore>
#include <latch>
#include <barrier>

// -----------------------------------------------------------------------------
// 1) Structured Concurrency: Nursery (scope que espera a todos los hijos)
// -----------------------------------------------------------------------------
class Nursery {
    std::vector<std::jthread> hijos_;
    std::stop_source stop_source_;

public:
    Nursery() = default;

    // Añadir tarea hija (se ejecuta en jthread con stop_token del nursery)
    // jthread pasa stop_token como primer argumento al callable
    template <typename F>
    void spawn(F&& f) {
        hijos_.emplace_back([f = std::forward<F>(f)](std::stop_token token) mutable {
            if constexpr (std::invocable<F, std::stop_token>) {
                f(token);
            } else {
                f();
            }
        }, stop_source_.get_token());
    }

    // Solicitar cancelación a TODOS los hijos
    void cancel_all() { stop_source_.request_stop(); }

    // Destructor: espera a TODOS (join automático de jthread)
    ~Nursery() = default;
};

void demo_nursery() {
    std::cout << "--- Structured Concurrency: Nursery ---\n";

    {
        Nursery nursery;

        nursery.spawn([](std::stop_token st) {
            for (int i = 0; !st.stop_requested() && i < 10; ++i) {
                std::cout << "[hijo 1] " << i << "\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        });

        nursery.spawn([](std::stop_token st) {
            for (int i = 0; !st.stop_requested() && i < 5; ++i) {
                std::cout << "[hijo 2] " << i << "\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(80));
            }
        });

        nursery.spawn([] {
            std::cout << "[hijo 3] sin token, trabajo rápido\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        });

        // Al salir del scope: ~Nursery() -> join de todos los jthreads
        std::cout << "[main] scope ending, esperando hijos...\n";
    }  // <-- AQUÍ se hace join de todos (structured concurrency)

    std::cout << "Nursery OK: todos los hijos terminaron antes de continuar\n\n";
}

// -----------------------------------------------------------------------------
// 2) Error handling en nursery: propagar excepción
// -----------------------------------------------------------------------------
class NurseryConError {
    std::vector<std::jthread> hijos_;
    std::stop_source stop_source_;
    std::exception_ptr primera_excepcion_;
    std::mutex mtx_exc_;

public:
    template <typename F>
    void spawn(F&& f) {
        hijos_.emplace_back([f = std::forward<F>(f), this](std::stop_token token) mutable {
            try {
                if constexpr (std::invocable<F, std::stop_token>) {
                    f(token);
                } else {
                    f();
                }
            } catch (...) {
                std::lock_guard<std::mutex> lk(mtx_exc_);
                if (!primera_excepcion_) primera_excepcion_ = std::current_exception();
                stop_source_.request_stop();  // cancelar hermanos
            }
        }, stop_source_.get_token());
    }

    void join_all() {
        for (auto& t : hijos_) if (t.joinable()) t.join();
        if (primera_excepcion_) std::rethrow_exception(primera_excepcion_);
    }
};

void demo_nursery_error() {
    std::cout << "--- Nursery con propagación de error ---\n";

    try {
        NurseryConError nursery;

        nursery.spawn([](std::stop_token) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            throw std::runtime_error("fallo en hijo 1");
        });

        nursery.spawn([](std::stop_token st) {
            while (!st.stop_requested()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            std::cout << "[hijo 2] cancelado por error del hermano\n";
        });

        nursery.join_all();  // lanza excepción si hubo
    } catch (const std::exception& e) {
        std::cout << "[main] excepción capturada: " << e.what() << "\n";
    }
    std::cout << "Error handling OK\n\n";
}

// -----------------------------------------------------------------------------
// 3) Checklist de revisión de código concurrente (mental model)
// -----------------------------------------------------------------------------
void imprimir_checklist() {
    std::cout << "=== CHECKLIST REVISIÓN CÓDIGO CONCURRENTE ===\n\n";

    std::cout << "☐ DATA RACES:\n";
    std::cout << "   - ¿Toda variable compartida mutable tiene protección (mutex/atomic)?\n";
    std::cout << "   - ¿Atomics usan memory_order correcto (acq/rel para sincronizar)?\n";
    std::cout << "   - ¿No hay relaxed para publicar datos que otros leen?\n\n";

    std::cout << "☐ LOCKS:\n";
    std::cout << "   - ¿Siempre RAII (lock_guard/unique_lock/scoped_lock)?\n";
    std::cout << "   - ¿Sección crítica MÍNIMA (unlock temprano si posible)?\n";
    std::cout << "   - ¿Orden global de locks para evitar deadlock?\n";
    std::cout << "   - ¿scoped_lock para múltiples mutexes?\n\n";

    std::cout << "☐ CONDITION VARIABLES:\n";
    std::cout << "   - ¿SIEMPRE wait(lk, predicado)?\n";
    std::cout << "   - ¿notify_one para colas, notify_all para broadcast/terminación?\n";
    std::cout << "   - ¿Notificar FUERA del lock?\n\n";

    std::cout << "☐ THREAD LIFECYCLE:\n";
    std::cout << "   - ¿jthread en lugar de thread (join auto + stop_token)?\n";
    std::cout << "   - ¿No detach() sin plan de limpieza?\n";
    std::cout << "   - ¿Tareas largas chequean stop_token?\n\n";

    std::cout << "☐ DISEÑO:\n";
    std::cout << "   - ¿Inmutabilidad + publicación atómica en vez de locks?\n";
    std::cout << "   - ¿Datos particionados (sharding/thread_local) antes que compartir?\n";
    std::cout << "   - ¿Thread pool fijo en lugar de crear hilos por tarea?\n";
    std::cout << "   - ¿Algoritmos STL paralelos (execution::par) para loops de datos?\n\n";

    std::cout << "☐ RENDIMIENTO:\n";
    std::cout << "   - ¿alignas(64) en contadores por hilo (evitar false sharing)?\n";
    std::cout << "   - ¿Benchmark con warm-up, mediana, wall-clock?\n";
    std::cout << "   - ¿Speedup medido vs Amdahl/Gustafson?\n\n";

    std::cout << "☐ HERRAMIENTAS:\n";
    std::cout << "   - ¿Compilado con -fsanitize=thread (TSan)?\n";
    std::cout << "   - ¿Probado con Helgrind/Valgrind?\n";
    std::cout << "   - ¿Perfilado contención (perf/VTune)?\n\n";
}

// -----------------------------------------------------------------------------
// 4) Resumen de decisiones de diseño por escenario
// -----------------------------------------------------------------------------
void imprimir_guia_decisiones() {
    std::cout << "=== GUÍA RÁPIDA: QUÉ USAR CUÁNDO ===\n\n";

    std::cout << "┌─────────────────────────────────────────────────────────────┐\n";
    std::cout << "│ ESCENARIO                          │ HERRAMIENTA RECOMENDADA │\n";
    std::cout << "├─────────────────────────────────────────────────────────────┤\n";
    std::cout << "│ Contador simple compartido         │ atomic<long long>       │\n";
    std::cout << "│ Contador alta contención           │ ShardedCounter (64 shards)│\n";
    std::cout << "│ Invariante compleja (varias vars)  │ mutex + lock_guard      │\n";
    std::cout << "│ Múltiples mutexes a la vez         │ scoped_lock             │\n";
    std::cout << "│ Lectores muchos, escritor poco     │ shared_mutex + shared_lock│\n";
    std::cout << "│ Esperar condición (cola, flag)     │ condition_variable + pred│\n";
    std::cout << "│ Pool de recursos (N conexiones)    │ counting_semaphore      │\n";
    std::cout << "│ Esperar N tareas (join colectivo)  │ latch                   │\n";
    std::cout << "│ Sincronizar fases iterativas       │ barrier / flex_barrier  │\n";
    std::cout << "│ Resultado asíncrono único          │ promise + future        │\n";
    std::cout << "│ Cola de tareas type-erased         │ packaged_task<void()>   │\n";
    std::cout << "│ Paralelismo de datos (loops)       │ std::execution::par     │\n";
    std::cout << "│ Tareas con dependencias (DAG)      │ TaskDAG / senders P2300 │\n";
    std::cout << "│ Cancelación cooperativa            │ stop_token + stop_source│\n";
    std::cout << "│ Estado por hilo (RNG, buffer)      │ thread_local            │\n";
    std::cout << "│ Config inmutable compartida        │ shared_ptr<const> + atomic│\n";
    std::cout << "│ Actor / message passing            │ cola + hilo owner       │\n";
    std::cout << "└─────────────────────────────────────────────────────────────┘\n\n";
}

// -----------------------------------------------------------------------------
// 5) Resumen del módulo 14 (temas cubiertos)
// -----------------------------------------------------------------------------
void imprimir_resumen_modulo() {
    std::cout << "=== RESUMEN MÓDULO 14: CONCURRENCIA Y MULTITHREADING MODERNO ===\n\n";

    std::cout << "LECCIONES:\n";
    std::cout << "  01. std::thread, join, detach, jthread básico\n";
    std::cout << "  02. jthread, stop_token, stop_source, stop_callback\n";
    std::cout << "  03. Data races, atomic, memory_order (relaxed, acq, rel, seq_cst)\n";
    std::cout << "  04. atomic_ref, fences, publicación inmutable, call_once, DCL\n";
    std::cout << "  05. Mutex, lock_guard, unique_lock, scoped_lock, deadlocks, shared_mutex\n";
    std::cout << "  06. unique_lock features, condition_variable básico, wait/notify\n";
    std::cout << "  07. condition_variable avanzado: bounded queue, multi-CV, graceful shutdown\n";
    std::cout << "  08. future, promise, async (launch policies), packaged_task, shared_future\n";
    std::cout << "  09. ThreadPool con packaged_task, prioridades, continuaciones, cancelación\n";
    std::cout << "  10. Work stealing, TaskGraph (DAG), algoritmos paralelos STL, when_all\n";
    std::cout << "  11. thread_local, inmutabilidad, sharded counter, COW, map-reduce, actors\n";
    std::cout << "  12. Políticas ejecución (par/par_unseq), false sharing, benchmarking\n";
    std::cout << "  13. C++20 sync primitives: semaphore, latch, barrier, flex_barrier\n";
    std::cout << "  14. Pool avanzado: DAG, fan-out/in, pipeline streaming, C++23 senders preview\n";
    std::cout << "  15. Structured concurrency (nursery), C++23/26 preview, checklist, guía\n\n";

    std::cout << "TEMAS CUBIERTOS (TEMAS.md):\n";
    std::cout << "  1. std::thread, join/detach, jthread, stop_token ✓\n";
    std::cout << "  2. Data races, atomic, memory ordering ✓\n";
    std::cout << "  3. Mutex, RAII locks, deadlocks, orden bloqueo ✓\n";
    std::cout << "  4. condition_variable, predicados, productor-consumidor ✓\n";
    std::cout << "  5. future/promise/async, packaged_task ✓\n";
    std::cout << "  6. Pool de hilos, cola de tareas ✓\n";
    std::cout << "  7. Inmutabilidad, thread_local, particionado ✓\n";
    std::cout << "  8. Políticas ejecución STL, medición escalado ✓\n\n";
}

// -----------------------------------------------------------------------------
// MAIN
// -----------------------------------------------------------------------------
int main() {
    std::cout << "=== LECCIÓN 15: C++23/26 preview, structured concurrency, repaso ===\n\n";

    demo_nursery();
    demo_nursery_error();
    imprimir_checklist();
    imprimir_guia_decisiones();
    imprimir_resumen_modulo();

    std::cout << "=== FIN LECCIÓN 15 ===\n";
    std::cout << "=== FIN MÓDULO 14 ===\n";
    return 0;
}