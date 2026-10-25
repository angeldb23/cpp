/* =============================================================================
   MÓDULO 14 - LECCIÓN 05: Mutex, RAII de locks (lock_guard, unique_lock,
   scoped_lock), deadlocks y orden de bloqueo.
   =============================================================================
   TEMA 3 de TEMAS.md (2 lecciones): Mutex y RAII de locks.

   std::mutex — EXCLUSIÓN MUTUA BÁSICA:
   - lock(): bloquea hasta poder entrar. Si ya está bloqueado, el hilo duerme.
   - unlock(): libera. SOLO el hilo que hizo lock() puede hacer unlock().
   - try_lock(): intento no bloqueante; devuelve true si logró, false si no.
   - NO es copiable ni movible.

   RAII DE LOCKS — NUNCA LLAMÉS lock()/unlock() MANUAL:
   1. std::lock_guard<Mutex>: MÁS SIMPLE. Bloquea en ctor, libera en dtor.
      No se puede mover, no se puede unlock() antes, no admite defer_lock.
      Úsalo SIEMPRE que el lock dure TODO el scope.
   2. std::unique_lock<Mutex>: MÁS FLEXIBLE.
      - defer_lock: no bloquea en ctor; llamás lock() después.
      - unlock() manual: liberás antes del dtor (útil para reducir sección crítica).
      - transfer ownership: movable (útil para devolver lock desde función).
      - try_lock_for / try_lock_until: timeouts.
      - condition_variable::wait() REQUIERE unique_lock.
   3. std::scoped_lock<Mutex...> (C++17): lock_guard PERO para MÚLTIPLES mutexes.
      Hace lock de TODOS atómicamente (evita deadlock por orden).
      Libera todos en dtor en orden inverso. ES LA FORMA SEGURA DE LOCK MÚLTIPLE.

   DEADLOCK — CUANDO DOS HILOS ESPERAN RECURSOS QUE EL OTRO TIENE:
   Clásico: Hilo A toma mutex1, quiere mutex2. Hilo B toma mutex2, quiere mutex1.
   Ambos se bloquean para siempre.

   PREVENCIÓN DE DEADLOCK:
   1. ORDEN GLOBAL DE BLOQUEO: siempre tomá mutexes en el MISMO ORDEN
      (por dirección, por ID, por nombre). scoped_lock lo hace por vos.
   2. std::lock(m1, m2, ...): función libre que toma varios mutexes sin deadlock
      (usa algoritmo de backoff). Después adoptalos con unique_lock(adopt_lock).
   3. EVITÁ MÚLTIPLES MUTEXES: rediseñá para tener uno solo por invariante.
   4. TIMEOUTS: try_lock_for / try_lock_until para detectar y recuperarse.
   5. LOCK HIERARCHY: asigná "niveles" a mutexes; solo se puede tomar de
      nivel más alto a más bajo (nunca al revé).

   EJEMPLOS: lock_guard, unique_lock (defer, unlock temprano, move), scoped_lock
   para múltiples mutexes, deadlock demo y prevención con orden global. */

#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <algorithm>
#include <shared_mutex>  // C++17: shared_mutex para lector-escritor

// -----------------------------------------------------------------------------
// 1) lock_guard: lo básico,(scope completo)
// -----------------------------------------------------------------------------
std::mutex mtx_basico;
int contador_guard = 0;

void worker_lock_guard(int vueltas) {
    for (int i = 0; i < vueltas; ++i) {
        std::lock_guard<std::mutex> lk(mtx_basico);  // lock en ctor
        ++contador_guard;                             // sección crítica
    }  // unlock en dtor (scope sale)
}

void demo_lock_guard() {
    std::cout << "--- std::lock_guard ---\n";
    constexpr int VUELTAS = 100000;
    contador_guard = 0;

    std::thread t1(worker_lock_guard, VUELTAS);
    std::thread t2(worker_lock_guard, VUELTAS);
    t1.join(); t2.join();

    std::cout << "Esperado: " << 2 * VUELTAS << ", Obtenido: " << contador_guard << "\n\n";
}

// -----------------------------------------------------------------------------
// 2) unique_lock: defer_lock, unlock temprano, move
// -----------------------------------------------------------------------------
std::mutex mtx_unique;
std::vector<int> cola_unique;

void productor_unique(int id, int items) {
    for (int i = 0; i < items; ++i) {
        std::unique_lock<std::mutex> lk(mtx_unique, std::defer_lock);  // NO lock yet
        // ... trabajo prep que NO necesita lock ...
        std::this_thread::sleep_for(std::chrono::microseconds(10));

        lk.lock();  // AHORA sí: sección crítica mínima
        cola_unique.push_back(id * 1000 + i);
        lk.unlock();  // LIBERAMOS ANTES del dtor (reduce contención)

        // ... trabajo post que NO necesita lock ...
        std::this_thread::sleep_for(std::chrono::microseconds(5));
    }
}

void demo_unique_lock() {
    std::cout << "--- std::unique_lock (defer_lock, unlock temprano) ---\n";
    cola_unique.clear();

    std::thread p1(productor_unique, 1, 5);
    std::thread p2(productor_unique, 2, 5);
    p1.join(); p2.join();

    std::cout << "Items en cola: " << cola_unique.size() << " (esperado 10)\n";
    for (int v : cola_unique) std::cout << "  " << v << "\n";
    std::cout << "\n";
}

// -----------------------------------------------------------------------------
// 3) unique_lock movable: función que devuelve lock (patrón avanzado)
// -----------------------------------------------------------------------------
std::mutex mtx_movable;
int dato_protegido = 0;

// Adquiere lock, hace algo, y DEVUELVE el unique_lock (move) para que
// el caller decida cuándo liberar. Útil en APIs de "lock y callback".
std::unique_lock<std::mutex> adquirir_y_hacer_algo() {
    std::unique_lock<std::mutex> lk(mtx_movable);
    dato_protegido = 42;
    std::cout << "[adquirir] dato puesto a 42, devolviendo lock\n";
    return lk;  // move: lk se vacía, el caller recibe el lock activo
}

void demo_unique_lock_movable() {
    std::cout << "--- unique_lock movable (devolver lock) ---\n";
    auto lk = adquirir_y_hacer_algo();  // lk tiene el lock
    std::cout << "[main] tengo el lock, dato = " << dato_protegido << "\n";
    // lk se destruye acá -> unlock
    std::cout << "[main] lock liberado al salir del scope\n\n";
}

// -----------------------------------------------------------------------------
// 4) scoped_lock (C++17): MÚLTIPLES mutexes SIN deadlock
// -----------------------------------------------------------------------------
std::mutex mtx_a, mtx_b;
int recurso_a = 0, recurso_b = 0;

// FORMA SEGURA: scoped_lock toma AMBOS atómicamente (orden interno por dirección)
void transferir_seguro(int desde, int hasta, int cantidad) {
    // std::scoped_lock bloquea mtx_a Y mtx_b SIN deadlock (ordena por addr)
    std::scoped_lock lock(mtx_a, mtx_b);
    if (desde == 1) { recurso_a -= cantidad; recurso_b += cantidad; }
    else            { recurso_b -= cantidad; recurso_a += cantidad; }
}

// FORMA PELIGROSA: lock manual en orden fijo (si el orden falla -> deadlock)
void transferir_peligroso(int desde, int hasta, int cantidad) {
    // Si un hilo llama (1,2) y otro (2,1) al mismo tiempo -> DEADLOCK
    std::lock_guard<std::mutex> lk1(mtx_a);
    std::this_thread::sleep_for(std::chrono::microseconds(100));  // fuerza carrera
    std::lock_guard<std::mutex> lk2(mtx_b);
    if (desde == 1) { recurso_a -= cantidad; recurso_b += cantidad; }
    else            { recurso_b -= cantidad; recurso_a += cantidad; }
}

void demo_scoped_lock() {
    std::cout << "--- std::scoped_lock (múltiples mutexes seguro) ---\n";
    recurso_a = 100; recurso_b = 0;

    std::thread t1(transferir_seguro, 1, 2, 30);
    std::thread t2(transferir_seguro, 2, 1, 20);
    t1.join(); t2.join();

    std::cout << "Resultado seguro: A=" << recurso_a << ", B=" << recurso_b
              << " (total=" << recurso_a + recurso_b << ")\n\n";
}

// -----------------------------------------------------------------------------
// 5) DEADLOCK real y prevención con std::lock + adopt_lock
// -----------------------------------------------------------------------------
std::mutex mtx_1, mtx_2;
int dato_1 = 0, dato_2 = 0;

// FORMA CON std::lock (C++11): toma ambos sin deadlock, después adopt_lock
void transferir_con_std_lock(int desde, int hasta, int cantidad) {
    // std::lock bloquea AMBOS sin deadlock (algoritmo de backoff interno)
    std::lock(mtx_1, mtx_2);

    // adopt_lock: "ya está bloqueado, yo me hago cargo del unlock en dtor"
    std::unique_lock<std::mutex> lk1(mtx_1, std::adopt_lock);
    std::unique_lock<std::mutex> lk2(mtx_2, std::adopt_lock);

    if (desde == 1) { dato_1 -= cantidad; dato_2 += cantidad; }
    else            { dato_2 -= cantidad; dato_1 += cantidad; }
    // lk1, lk2 hacen unlock en dtor
}

void demo_std_lock_adopt() {
    std::cout << "--- std::lock + adopt_lock (pre-C++17) ---\n";
    dato_1 = 100; dato_2 = 0;

    std::thread t1(transferir_con_std_lock, 1, 2, 30);
    std::thread t2(transferir_con_std_lock, 2, 1, 20);
    t1.join(); t2.join();

    std::cout << "Resultado: dato_1=" << dato_1 << ", dato_2=" << dato_2
              << " (total=" << dato_1 + dato_2 << ")\n\n";
}

// -----------------------------------------------------------------------------
// 6) Jerarquía de locks (lock hierarchy) — prevención a nivel diseño
// -----------------------------------------------------------------------------
class MutexJerarquico {
    std::mutex mtx_;
    const int nivel_;  // nivel más ALTO = se toma PRIMERO
    static thread_local int nivel_actual_;  // nivel del lock que YA tenemos

public:
    explicit MutexJerarquico(int nivel) : nivel_(nivel) {}

    void lock() {
        if (nivel_actual_ != 0 && nivel_ >= nivel_actual_) {
            throw std::logic_error("Violación de jerarquía de locks: "
                "intentás tomar nivel " + std::to_string(nivel_) +
                " teniendo nivel " + std::to_string(nivel_actual_));
        }
        mtx_.lock();
        nivel_actual_ = nivel_;
    }

    void unlock() {
        nivel_actual_ = 0;
        mtx_.unlock();
    }

    bool try_lock() {
        if (nivel_actual_ != 0 && nivel_ >= nivel_actual_) return false;
        if (mtx_.try_lock()) { nivel_actual_ = nivel_; return true; }
        return false;
    }

    // RAII guard para este mutex jerárquico
    class Guard {
        MutexJerarquico& m_;
    public:
        explicit Guard(MutexJerarquico& m) : m_(m) { m_.lock(); }
        ~Guard() { m_.unlock(); }
        Guard(Guard&&) = delete; Guard& operator=(Guard&&) = delete;
    };
};

thread_local int MutexJerarquico::nivel_actual_ = 0;

MutexJerarquico mutex_nivel_alto(10);   // se toma PRIMERO
MutexJerarquico mutex_nivel_bajo(5);    // se toma DESPUÉS

void worker_jerarquia_correcto() {
    MutexJerarquico::Guard g1(mutex_nivel_alto);   // nivel 10
    MutexJerarquico::Guard g2(mutex_nivel_bajo);   // nivel 5 (OK: 5 < 10)
    std::cout << "[jerarquía] locks tomados en orden correcto\n";
}

void worker_jerarquia_incorrecto() {
    MutexJerarquico::Guard g1(mutex_nivel_bajo);   // nivel 5
    // MutexJerarquico::Guard g2(mutex_nivel_alto); // ¡EXCEPCIÓN! 10 > 5
    std::cout << "[jerarquía] solo lock bajo tomado\n";
}

void demo_jerarquia_locks() {
    std::cout << "--- Jerarquía de locks (prevención en tiempo de ejecución) ---\n";
    std::cout << "Correcto:\n";
    worker_jerarquia_correcto();
    std::cout << "Incorrecto (comentado para no tirar excepción):\n";
    // worker_jerarquia_incorrecto();  // descomentar para ver excepción
    std::cout << "\n";
}

// -----------------------------------------------------------------------------
// 7) shared_mutex (C++17): lectores múltiples, escritor exclusivo
// -----------------------------------------------------------------------------
std::shared_mutex mtx_rw;
std::vector<int> datos_compartidos;

void lector_rw(int id) {
    std::shared_lock<std::shared_mutex> lk(mtx_rw);  // lock compartido (lectura)
    std::cout << "[lector " << id << "] tamaño: " << datos_compartidos.size() << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}  // unlock compartido

void escritor_rw(int id, int valor) {
    std::unique_lock<std::shared_mutex> lk(mtx_rw);  // lock exclusivo (escritura)
    datos_compartidos.push_back(valor);
    std::cout << "[escritor " << id << "] agregó " << valor << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
}  // unlock exclusivo

void demo_shared_mutex() {
    std::cout << "--- std::shared_mutex (lectores múltiples, escritor exclusivo) ---\n";
    datos_compartidos.clear();

    std::thread e1(escritor_rw, 1, 100);
    std::thread l1(lector_rw, 1);
    std::thread l2(lector_rw, 2);
    std::thread e2(escritor_rw, 2, 200);
    std::thread l3(lector_rw, 3);

    e1.join(); l1.join(); l2.join(); e2.join(); l3.join();
    std::cout << "Final size: " << datos_compartidos.size() << "\n\n";
}

// -----------------------------------------------------------------------------
// MAIN
// -----------------------------------------------------------------------------
int main() {
    std::cout << "=== LECCIÓN 05: Mutex, RAII locks, deadlocks, orden ===\n\n";

    demo_lock_guard();
    demo_unique_lock();
    demo_unique_lock_movable();
    demo_scoped_lock();
    demo_std_lock_adopt();
    demo_jerarquia_locks();
    demo_shared_mutex();

    std::cout << "=== FIN LECCIÓN 05 ===\n";
    return 0;
}