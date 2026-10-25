/* =============================================================================
   MÓDULO 14 - LECCIÓN 03: Data races, std::atomic y memory ordering básico
   =============================================================================
   TEMA 2 de TEMAS.md (2 lecciones): Data races, std::atomic, memory ordering.

   DATA RACE — DEFINICIÓN FORMAL (C++ memory model):
   Dos accesos concurrentes a la misma ubicación de memoria, donde:
   - Al menos uno es ESCRITURA (no atómico).
   - No hay "happens-before" entre ellos (ninguna sincronización).
   Resultado: COMPORTAMIENTO INDEFINIDO (UB). El programa puede hacer cualquier
   cosa: crashear, dar resultados raros, parecer que funciona y fallar en prod.

   std::atomic<T> — LA HERRAMIENTA BÁSICA:
   - Garantiza atomicidad: la operación es indivisible (ningún otro hilo ve
     estado intermedio).
   - Establece ordenamiento de memoria (memory ordering) entre hilos.
   - NO usa mutex por debajo (lock-free en la mayoría de plataformas para
     tipos enteros/punteros hasta 64 bits).

   OPERACIONES BÁSICAS:
   - load(order): leer valor atómico.
   - store(val, order): escribir valor atómico.
   - exchange(val, order): escribir y devolver valor anterior.
   - compare_exchange_weak/strong(expected, desired, success_order, fail_order):
     CAS (Compare-And-Swap): "si == expected, pon desired; sino pon actual en expected".
     Es la base de algoritmos lock-free.

   MEMORY ORDERING (orden de memoria) — LOS 6 NIVELES (C++11):
   1. memory_order_relaxed:   Solo atomicidad. SIN ordenamiento. El compilador/CPU
                              puede reordenar alrededor. Úsalo SOLO para contadores
                              estadísticos, IDs únicos, etc. donde el orden no importa.
   2. memory_order_consume:   (Deprecated en C++20, evitado) Para dependencias de datos.
   3. memory_order_acquire:   Load: nada ANTES de este load se mueve DESPUÉS.
                              "Adquiere" visibilidad de writes previos con release.
   4. memory_order_release:   Store: nada DESPUÉS de este store se mueve ANTES.
                              "Libera" writes previos para que un acquire los vea.
   5. memory_order_acq_rel:   RMW (exchange, CAS): acquire + release junto.
   6. memory_order_seq_cst:   DEFAULT. Total order global: todos los hilos ven el
                              MISMO orden de operaciones seq_cst. Más caro, más seguro.

   RELACIÓN ACQUIRE-RELEASE (el patrón clásico):
   Hilo A (productor):                     Hilo B (consumidor):
     data = 42;                            // espera hasta que flag sea true
     flag.store(true, release);    <--->   if (flag.load(acquire)) usa(data);
   El release del store "sincroniza con" el acquire del load: B ve data=42.

   seq_cst (default) es más simple: todo tiene un orden global único.
   acquire/release es más fino (solo sincroniza pares específicos) y más rápido.

   EJEMPLOS: data race vs atomic, relaxed vs seq_cst, acquire-release pattern. */

#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <chrono>
#include <cassert>

// -----------------------------------------------------------------------------
// 1) DATA RACE: el problema (UB real, no teórico)
// -----------------------------------------------------------------------------
int contador_sin_atomic = 0;  // variable global compartida SIN protección

void incrementar_sin_atomic(int vueltas) {
    for (int i = 0; i < vueltas; ++i) {
        ++contador_sin_atomic;  // DATA RACE: read-modify-write no atómico
    }
}

void demo_data_race() {
    std::cout << "--- Data race (UB) ---\n";
    constexpr int VUELTAS = 100000;
    contador_sin_atomic = 0;

    std::thread t1(incrementar_sin_atomic, VUELTAS);
    std::thread t2(incrementar_sin_atomic, VUELTAS);
    t1.join(); t2.join();

    std::cout << "Esperado: " << 2 * VUELTAS << ", Obtenido: " << contador_sin_atomic;
    if (contador_sin_atomic != 2 * VUELTAS) std::cout << "  <- DATA RACE DETECTADO!";
    std::cout << "\n\n";
}

// -----------------------------------------------------------------------------
// 2) std::atomic<int> con seq_cst (default) — correcto y simple
// -----------------------------------------------------------------------------
std::atomic<int> contador_atomic{0};

void incrementar_atomic(int vueltas) {
    for (int i = 0; i < vueltas; ++i) {
        contador_atomic.fetch_add(1, std::memory_order_seq_cst);  // o solo ++
        // contador_atomic++;  // equivalente a fetch_add(1, seq_cst)
    }
}

void demo_atomic_seq_cst() {
    std::cout << "--- std::atomic con seq_cst (default) ---\n";
    constexpr int VUELTAS = 100000;
    contador_atomic = 0;

    std::thread t1(incrementar_atomic, VUELTAS);
    std::thread t2(incrementar_atomic, VUELTAS);
    t1.join(); t2.join();

    std::cout << "Esperado: " << 2 * VUELTAS << ", Obtenido: " << contador_atomic.load() << "\n\n";
}

// -----------------------------------------------------------------------------
// 3) memory_order_relaxed: solo atomicidad, SIN ordenamiento
//    Útil para contadores donde el orden de visibilidad no importa.
// -----------------------------------------------------------------------------
std::atomic<int> contador_relaxed{0};

void incrementar_relaxed(int vueltas) {
    for (int i = 0; i < vueltas; ++i) {
        contador_relaxed.fetch_add(1, std::memory_order_relaxed);
    }
}

void demo_relaxed() {
    std::cout << "--- memory_order_relaxed (solo atomicidad) ---\n";
    constexpr int VUELTAS = 100000;
    contador_relaxed = 0;

    std::thread t1(incrementar_relaxed, VUELTAS);
    std::thread t2(incrementar_relaxed, VUELTAS);
    t1.join(); t2.join();

    std::cout << "Esperado: " << 2 * VUELTAS << ", Obtenido: " << contador_relaxed.load() << "\n";
    std::cout << "NOTA: relaxed garantiza cuenta correcta, PERO el orden de visibilidad\n";
    std::cout << "      de otras variables NO atómicas respecto a este contador es indefinido.\n\n";
}

// -----------------------------------------------------------------------------
// 4) Patrón ACQUIRE-RELEASE: publicación segura de datos
// -----------------------------------------------------------------------------
struct Mensaje {
    int datos[100];
    bool listo = false;
};

Mensaje msg;
std::atomic<bool> flag{false};  // flag atómica para sincronizar

void productor_acq_rel() {
    // Preparamos datos (escrituras NO atómicas)
    for (int i = 0; i < 100; ++i) msg.datos[i] = i * 2;

    // PUBLICAMOS con release: garantiza que TODAS las escrituras anteriores
    // sean visibles para CUALQUIER hilo que haga acquire en esta misma atómica.
    flag.store(true, std::memory_order_release);
}

void consumidor_acq_rel() {
    // Esperamos con acquire: garantiza que TODAS las lecturas posteriores
    // vean las escrituras que el release "liberó".
    while (!flag.load(std::memory_order_acquire)) {
        std::this_thread::yield();  // busy wait simple (demo)
    }
    // Acá estamos seguros: msg.datos[] tiene los valores escritos por productor.
    int suma = 0;
    for (int i = 0; i < 100; ++i) suma += msg.datos[i];
    std::cout << "[consumidor] suma verificada: " << suma << " (esperado 9900)\n";
    assert(suma == 9900);
}

void demo_acquire_release() {
    std::cout << "--- Patrón Acquire-Release ---\n";
    flag = false;
    std::thread t1(productor_acq_rel);
    std::thread t2(consumidor_acq_rel);
    t1.join(); t2.join();
    std::cout << "Acquire-Release OK: datos publicados visibles para consumidor\n\n";
}

// -----------------------------------------------------------------------------
// 5) compare_exchange_weak (CAS) — base de algoritmos lock-free
//    "Si valor == esperado, pon nuevo; sino actualiza esperado con valor real"
// -----------------------------------------------------------------------------
std::atomic<int> cas_valor{0};

bool cas_incrementar(int& esperado, int nuevo) {
    // weak: puede fallar espuriamente (en arquitecturas LL/SC como ARM).
    // En loop: reintenta con el valor actualizado en 'esperado'.
    return cas_valor.compare_exchange_weak(esperado, nuevo,
                                           std::memory_order_acq_rel,
                                           std::memory_order_relaxed);
}

void worker_cas(int vueltas) {
    for (int i = 0; i < vueltas; ++i) {
        int esperado = cas_valor.load(std::memory_order_relaxed);
        while (!cas_incrementar(esperado, esperado + 1)) {
            // 'esperado' ya tiene el valor actual (actualizado por CAS fallido)
            // reintentamos con el nuevo valor
        }
    }
}

void demo_cas() {
    std::cout << "--- compare_exchange_weak (CAS loop) ---\n";
    constexpr int VUELTAS = 50000;
    cas_valor = 0;

    std::thread t1(worker_cas, VUELTAS);
    std::thread t2(worker_cas, VUELTAS);
    t1.join(); t2.join();

    std::cout << "Esperado: " << 2 * VUELTAS << ", Obtenido: " << cas_valor.load() << "\n\n";
}

// -----------------------------------------------------------------------------
// 6) atomic_flag: el tipo atómico MÁS SIMPLE (solo test_and_set/clear)
//    Garantizado lock-free siempre. Base de spinlocks.
// -----------------------------------------------------------------------------
std::atomic_flag lock_flag = ATOMIC_FLAG_INIT;

void spinlock_worker(int id, int vueltas) {
    for (int i = 0; i < vueltas; ++i) {
        // test_and_set: pone flag en true y devuelve valor ANTERIOR.
        // Si devuelve true -> otro hilo tenía el lock -> spin.
        while (lock_flag.test_and_set(std::memory_order_acquire)) {
            std::this_thread::yield();  // cede CPU (buena práctica en spinlock)
        }
        // SECCIÓN CRÍTICA
        static int compartido = 0;
        ++compartido;
        // FIN SECCIÓN CRÍTICA
        lock_flag.clear(std::memory_order_release);  // libera lock
    }
}

void demo_atomic_flag() {
    std::cout << "--- atomic_flag (spinlock simple) ---\n";
    constexpr int VUELTAS = 10000;
    lock_flag.clear();

    std::thread t1(spinlock_worker, 1, VUELTAS);
    std::thread t2(spinlock_worker, 2, VUELTAS);
    t1.join(); t2.join();

    std::cout << "Contador protegido por spinlock: " << 2 * VUELTAS << " (ok si ==)\n\n";
}

// -----------------------------------------------------------------------------
// MAIN
// -----------------------------------------------------------------------------
int main() {
    std::cout << "=== LECCIÓN 03: Data races, std::atomic, memory ordering básico ===\n\n";

    demo_data_race();
    demo_atomic_seq_cst();
    demo_relaxed();
    demo_acquire_release();
    demo_cas();
    demo_atomic_flag();

    std::cout << "=== FIN LECCIÓN 03 ===\n";
    return 0;
}