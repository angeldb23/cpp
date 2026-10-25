/* =============================================================================
   MÓDULO 14 - LECCIÓN 13: Primitive de sincronización C++20 — semaphore,
   latch, barrier. Patrones de coordinación estructurada.
   =============================================================================
   TEMA 4 (continuación, lección 3 de 3): condition_variable avanzado +
   NUEVAS primitivas C++20: counting_semaphore, binary_semaphore, latch, barrier.

   std::counting_semaphore<LeastMaxValue> (C++20):
   - Contador atómico con acquire (wait/P) y release (signal/V).
   - acquire(): decrementa; si < 0, bloquea hasta que otro release() lo suba.
   - release(n=1): incrementa en n; despierta hilos esperando.
   - try_acquire(): no bloquea, devuelve bool.
   - try_acquire_for/until: timeouts.
   - binary_semaphore = counting_semaphore<1> (mutex-like pero sin owner).
   - USOS: pooling de recursos (conexiones, hilos), rate limiting, señalización.

   std::latch (C++20):
   - Contador descendente DE UN SOLO USO (single-shot).
   - count_down(): decrementa; arrive_and_wait() = count_down + wait.
   - wait(): bloquea hasta que contador llega a 0.
   - NO se puede resetear ni incrementar.
   - USOS: esperar a que N tareas terminen (join barrier simple), inicio sincronizado.

   std::barrier (C++20) / std::flex_barrier:
   - REUTILIZABLE: múltiples fases (ciclos).
   - arrive_and_wait(): llega, decrementa, espera a que todos lleguen, pasa a fase siguiente.
   - arrive_and_drop(): llega y se retira (reduce cuenta esperada para próximas fases).
   - flex_barrier: callback en cada fase (para trabajo de coordinación).
   - USOS: algoritmos iterativos paralelos, simulaciones paso a paso, pipelines.

   COMPARACIÓN:
   | Primitiva      | Reutilizable | Contador | Callback | Caso de uso típico          |
   |----------------|--------------|----------|----------|----------------------------|
   | semaphore      | Sí           | Arriba   | No       | Pool recursos, rate limit  |
   | latch          | No (1 uso)   | Abajo    | No       | Join N tareas, inicio sync |
   | barrier        | Sí (fases)   | Abajo    | No       | Algoritmos iterativos      |
   | flex_barrier   | Sí (fases)   | Abajo    | Sí       | Coordinación compleja      |

   EJEMPLOS: semaphore para pool de conexiones, latch para espera de workers,
   barrier para algoritmo iterativo (Jacobi), flex_barrier para logging por fase. */

#include <iostream>
#include <thread>
#include <vector>
#include <semaphore>    // C++20: counting_semaphore, binary_semaphore
#include <latch>        // C++20: latch
#include <barrier>      // C++20: barrier, flex_barrier
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>

// -----------------------------------------------------------------------------
// 1) counting_semaphore: pool de recursos limitados (ej: conexiones DB)
// -----------------------------------------------------------------------------
class ConnectionPool {
    std::counting_semaphore<10> sem_{10};  // máx 10 conexiones simultáneas
    mutable std::mutex mtx_;  // mutable para poder lockear en métodos const
    std::vector<int> conexiones_disponibles_;  // IDs de conexiones libres
    std::vector<int> conexiones_en_uso_;

public:
    ConnectionPool() {
        for (int i = 1; i <= 10; ++i) conexiones_disponibles_.push_back(i);
    }

    // Adquirir conexión (bloquea si pool agotado)
    int adquirir() {
        sem_.acquire();  // espera si contador == 0
        std::lock_guard<std::mutex> lk(mtx_);
        int id = conexiones_disponibles_.back();
        conexiones_disponibles_.pop_back();
        conexiones_en_uso_.push_back(id);
        return id;
    }

    // Intentar adquirir con timeout
    std::optional<int> try_adquirir_for(std::chrono::milliseconds timeout) {
        if (!sem_.try_acquire_for(timeout)) return std::nullopt;
        std::lock_guard<std::mutex> lk(mtx_);
        int id = conexiones_disponibles_.back();
        conexiones_disponibles_.pop_back();
        conexiones_en_uso_.push_back(id);
        return id;
    }

    // Liberar conexión
    void liberar(int id) {
        {
            std::lock_guard<std::mutex> lk(mtx_);
            auto it = std::find(conexiones_en_uso_.begin(), conexiones_en_uso_.end(), id);
            if (it != conexiones_en_uso_.end()) {
                conexiones_en_uso_.erase(it);
                conexiones_disponibles_.push_back(id);
            }
        }
        sem_.release();  // incrementa contador, despierta un waiter
    }

    size_t disponibles() const {
        std::lock_guard<std::mutex> lk(mtx_);
        return conexiones_disponibles_.size();
    }
};

void demo_semaphore_pool() {
    std::cout << "--- counting_semaphore: pool de conexiones ---\n";
    ConnectionPool pool;

    auto worker = [&](int id) {
        for (int i = 0; i < 3; ++i) {
            int conn = pool.adquirir();
            std::cout << "[worker " << id << "] conn " << conn << " (disp: " << pool.disponibles() << ")\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            pool.liberar(conn);
            std::cout << "[worker " << id << "] liberó " << conn << " (disp: " << pool.disponibles() << ")\n";
        }
    };

    std::thread t1(worker, 1), t2(worker, 2), t3(worker, 3), t4(worker, 4);
    t1.join(); t2.join(); t3.join(); t4.join();
    std::cout << "Semaphore pool OK\n\n";
}

// -----------------------------------------------------------------------------
// 2) binary_semaphore: señalización simple (como condition_variable pero más simple)
// -----------------------------------------------------------------------------
std::binary_semaphore sem_signal{0};  // inicia en 0 (no señal)
int dato_compartido = 0;

void productor_sem() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    dato_compartido = 42;
    std::cout << "[productor] dato listo, signal\n";
    sem_signal.release();  // signal
}

void consumidor_sem() {
    std::cout << "[consumidor] esperando...\n";
    sem_signal.acquire();  // espera signal
    std::cout << "[consumidor] recibido: " << dato_compartido << "\n";
}

void demo_binary_semaphore() {
    std::cout << "--- binary_semaphore: señalización simple ---\n";
    std::thread p(productor_sem), c(consumidor_sem);
    p.join(); c.join();
    std::cout << "binary_semaphore OK\n\n";
}

// -----------------------------------------------------------------------------
// 3) latch: esperar a que N tareas terminen (join colectivo)
// -----------------------------------------------------------------------------
void worker_latch(std::latch& l, int id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50 + id * 20));
    std::cout << "[worker " << id << "] terminó, count_down\n";
    l.count_down();  // decrementa latch
}

void demo_latch() {
    std::cout << "--- latch: join colectivo de N tareas ---\n";

    const int N = 5;
    std::latch l(N);  // contador inicial = N

    std::vector<std::thread> workers;
    for (int i = 1; i <= N; ++i) workers.emplace_back(worker_latch, std::ref(l), i);

    std::cout << "[main] esperando a " << N << " workers...\n";
    l.wait();  // bloquea hasta que contador == 0
    std::cout << "[main] todos terminaron\n";

    for (auto& t : workers) t.join();
    std::cout << "latch OK\n\n";
}

// -----------------------------------------------------------------------------
// 4) latch: inicio sincronizado (todos empiezan juntos)
// -----------------------------------------------------------------------------
void worker_inicio_sync(std::latch& inicio, std::latch& fin, int id) {
    std::cout << "[worker " << id << "] listo, esperando señal de inicio...\n";
    inicio.wait();  // TODOS esperan aquí
    std::cout << "[worker " << id << "] ¡EMPEZANDO JUNTOS!\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::cout << "[worker " << id << "] terminó trabajo\n";
    fin.count_down();
}

void demo_latch_inicio_sync() {
    std::cout << "--- latch: inicio sincronizado ---\n";

    const int N = 4;
    std::latch inicio(N);  // main hace count_down N veces
    std::latch fin(N);     // workers hacen count_down al terminar

    std::vector<std::thread> workers;
    for (int i = 1; i <= N; ++i) workers.emplace_back(worker_inicio_sync, std::ref(inicio), std::ref(fin), i);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::cout << "[main] señal de inicio (count_down x" << N << ")\n";
    for (int i = 0; i < N; ++i) inicio.count_down();

    fin.wait();  // esperar a que todos terminen
    for (auto& t : workers) t.join();
    std::cout << "Inicio sincronizado OK\n\n";
}

// -----------------------------------------------------------------------------
// 5) barrier: algoritmo iterativo (método de Jacobi para Laplace)
// -----------------------------------------------------------------------------
void demo_barrier_jacobi() {
    std::cout << "--- barrier: algoritmo iterativo (Jacobi) ---\n";

    const int N = 10;  // grid 10x10
    const int ITER = 5;
    std::vector<std::vector<double>> grid(N, std::vector<double>(N, 0.0));
    std::vector<std::vector<double>> nuevo(N, std::vector<double>(N, 0.0));

    // Condiciones de borde
    for (int i = 0; i < N; ++i) {
        grid[0][i] = 100.0;      // arriba caliente
        grid[N-1][i] = 0.0;      // abajo frío
        grid[i][0] = 50.0;       // izq tibio
        grid[i][N-1] = 50.0;     // der tibio
        nuevo[0][i] = 100.0;
        nuevo[N-1][i] = 0.0;
        nuevo[i][0] = 50.0;
        nuevo[i][N-1] = 50.0;
    }

    // Barrier para sincronizar cada iteración
    std::barrier barr(N - 2, [&]() noexcept {
        // Callback al final de cada fase: swap grids
        grid.swap(nuevo);
        // std::cout << "[barrier] fase completada, grids swapped\n";
    });

    auto worker_fila = [&](int fila) {
        for (int iter = 0; iter < ITER; ++iter) {
            // Calcular nueva temperatura (promedio 4 vecinos)
            for (int j = 1; j < N-1; ++j) {
                nuevo[fila][j] = 0.25 * (grid[fila-1][j] + grid[fila+1][j] +
                                         grid[fila][j-1] + grid[fila][j+1]);
            }
            barr.arrive_and_wait();  // esperar a que todas las filas terminen
        }
    };

    std::vector<std::thread> hilos;
    for (int i = 1; i < N-1; ++i) hilos.emplace_back(worker_fila, i);
    for (auto& t : hilos) t.join();

    std::cout << "Centro grid[5][5] = " << grid[5][5] << " (converge a ~50)\n";
    std::cout << "barrier OK: sincronización por fases reutilizable\n\n";
}

// -----------------------------------------------------------------------------
// 6) Barrier con callback manual (flex_barrier no implementado en GCC aún)
// -----------------------------------------------------------------------------
void demo_barrier_con_callback() {
    std::cout << "--- Barrier con callback manual (simula flex_barrier) ---\n";

    int fase = 0;
    const int HILOS = 3;
    std::barrier barr(HILOS);
    std::atomic<int> llegados{0};

    auto worker = [&](int id) {
        for (int f = 0; f < 3; ++f) {
            std::cout << "[worker " << id << "] fase " << f+1 << " trabajando\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            
            // Llegar al barrier y esperar (arrive_and_wait = arrive + wait)
            auto token = barr.arrive();
            // Uno de los hilos ejecuta el callback (el último en llegar)
            if (++llegados == HILOS) {
                ++fase;
                std::cout << "[barrier callback] fase " << fase << " completada por todos\n";
                llegados = 0;
            }
            barr.wait(std::move(token));  // esperar a que todos lleguen (token es rvalue)
        }
    };

    std::thread t1(worker, 1), t2(worker, 2), t3(worker, 3);
    t1.join(); t2.join(); t3.join();
    std::cout << "Barrier con callback OK: 3 fases completadas\n\n";
}

// -----------------------------------------------------------------------------
// 7) Combinando latch + barrier: startup -> iteraciones -> shutdown
// -----------------------------------------------------------------------------
void demo_pipeline_completo() {
    std::cout << "--- Pipeline: latch (startup) -> barrier (iter) -> latch (shutdown) ---\n";

    const int WORKERS = 4;
    std::latch inicio(WORKERS);   // workers señalan listos
    std::latch arranque(1);       // main señala inicio
    std::barrier iter(WORKERS);   // sincronizar cada iteración
    std::latch fin(WORKERS);      // workers señalan fin

    auto worker = [&](int id) {
        std::cout << "[w" << id << "] iniciando...\n";
        inicio.count_down();      // "estoy listo"
        arranque.wait();          // esperar orden de main

        for (int i = 0; i < 3; ++i) {
            std::cout << "[w" << id << "] iter " << i+1 << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
            iter.arrive_and_wait();
        }

        std::cout << "[w" << id << "] terminando\n";
        fin.count_down();
    };

    std::vector<std::thread> hs;
    for (int i = 1; i <= WORKERS; ++i) hs.emplace_back(worker, i);

    inicio.wait();        // esperar a que todos estén listos
    std::cout << "[main] todos listos, arrancando...\n";
    arranque.count_down();  // señal de inicio

    fin.wait();           // esperar fin
    for (auto& t : hs) t.join();
    std::cout << "Pipeline completo OK\n\n";
}

// -----------------------------------------------------------------------------
// MAIN
// -----------------------------------------------------------------------------
int main() {
    std::cout << "=== LECCIÓN 13: C++20 sync primitives — semaphore, latch, barrier ===\n\n";

    demo_semaphore_pool();
    demo_binary_semaphore();
    demo_latch();
    demo_latch_inicio_sync();
    demo_barrier_jacobi();
    demo_barrier_con_callback();
    demo_pipeline_completo();

    std::cout << "=== FIN LECCIÓN 13 ===\n";
    return 0;
}