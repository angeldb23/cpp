/* =============================================================================
   MÓDULO 14 - LECCIÓN 04: Memory ordering avanzado, atomic_ref, patrones
   =============================================================================
   TEMA 2 (continuación): memory ordering completo, std::atomic_ref (C++20),
   patrones prácticos: double-checked locking, publicación inmutable, contadores.

   MEMORY ORDERING — RESUMEN PRÁCTICO:
   - relaxed:   solo atomicidad. Contadores, stats, IDs. NO sincroniza otras vars.
   - acquire:   load. "Nada antes de este load se mueve después". Para LEER datos
                publicados por un release.
   - release:   store. "Nada después de este store se mueve antes". Para PUBLICAR
                datos escritos antes.
   - acq_rel:   RMW (fetch_add, exchange, CAS). Combine acquire + release.
   - seq_cst:   default. Orden global único. Más seguro, más lento. Úsalo si
                no estás seguro; optimiza a acquire/release SOLO si medís y gana.

   FENCE (vallas de memoria) — std::atomic_thread_fence:
   - acquire fence:   nada ANTES del fence se mueve DESPUÉS.
   - release fence:   nada DESPUÉS del fence se mueve ANTES.
   - seq_cst fence:   ambos + orden global.
   Sirven para ordenar operaciones NO atómicas respecto a atómicas.

   std::atomic_ref<T> (C++20):
   - Referencia atómica a un objeto EXISTENTE (no dueño de la memoria).
   - Permite hacer atómico un campo de struct, elemento de array, variable global
     legacy, etc. SIN cambiar su tipo.
   - Requiere que la dirección esté alineada para T (alignof(T)).
   - Útil para: atomicidad en campos de structs compartidos, shmem, legacy code.

   PATRÓN: Publicación inmutable (immutable publication)
   - Construí el objeto COMPLETO en un hilo (sin sincronización, solo local).
   - Publicá el puntero/referencia con release store.
   - Otros hilos leen con acquire load y ven el objeto COMPLETO e inmutable.
   - Como es inmutable, NO hace falta más sincronización para leerlo.

   PATRÓN: Double-checked locking (DCL) — CUIDADO en C++!
   - El patrón clásico "check; lock; check; init; unlock" FALLA en C++ pre-C++11
     por reordenamiento de writes en el constructor.
   - En C++11+ con atomic<ptr> + release/acquire FUNCIONA, PERO:
     std::call_once / std::once_flag es MÁS SIMPLE, SEGURO y estándar.
   - Usá call_once salvo que midas y necesites el control manual.

   EJEMPLOS: atomic_ref, fences, publicación inmutable, call_once vs DCL. */

#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <chrono>
#include <mutex>
#include <memory>
#include <cassert>

// -----------------------------------------------------------------------------
// 1) std::atomic_ref (C++20): atomicidad sobre objeto EXISTENTE
// -----------------------------------------------------------------------------
struct ContadorLegacy {
    int valor = 0;  // campo NO atómico originalmente
};

void worker_atomic_ref(ContadorLegacy& c, int vueltas) {
    // atomic_ref al campo 'valor' del MISMO objeto
    std::atomic_ref<int> ref(c.valor);
    for (int i = 0; i < vueltas; ++i) {
        ref.fetch_add(1, std::memory_order_relaxed);
    }
}

void demo_atomic_ref() {
    std::cout << "--- std::atomic_ref (C++20) ---\n";
    ContadorLegacy c;
    constexpr int VUELTAS = 50000;

    std::thread t1(worker_atomic_ref, std::ref(c), VUELTAS);
    std::thread t2(worker_atomic_ref, std::ref(c), VUELTAS);
    t1.join(); t2.join();

    std::cout << "Esperado: " << 2 * VUELTAS << ", Obtenido: " << c.valor << "\n";
    std::cout << "atomic_ref permite atomicidad SIN cambiar el tipo del campo.\n\n";
}

// -----------------------------------------------------------------------------
// 2) Fences (vallas de memoria) para ordenar ops NO atómicas
// -----------------------------------------------------------------------------
int datos_no_atomicos = 0;
std::atomic<bool> bandera{false};

void productor_con_fence() {
    datos_no_atomicos = 42;                    // write no atómico
    std::atomic_thread_fence(std::memory_order_release);  // release fence
    bandera.store(true, std::memory_order_relaxed);       // store relaxed
    // El release fence garantiza que 'datos_no_atomicos = 42' sea visible
    // para cualquier hilo que haga acquire fence + load relaxed de 'bandera'.
}

void consumidor_con_fence() {
    while (!bandera.load(std::memory_order_relaxed)) {
        std::this_thread::yield();
    }
    std::atomic_thread_fence(std::memory_order_acquire);  // acquire fence
    // El acquire fence garantiza que veamos 'datos_no_atomicos == 42'
    assert(datos_no_atomicos == 42);
    std::cout << "[consumidor] vió datos_no_atomicos = " << datos_no_atomicos << "\n";
}

void demo_fences() {
    std::cout << "--- atomic_thread_fence (release/acquire) ---\n";
    bandera = false;
    std::thread t1(productor_con_fence);
    std::thread t2(consumidor_con_fence);
    t1.join(); t2.join();
    std::cout << "Fences OK: ops no atómicas ordenadas respecto a atómicas relaxed\n\n";
}

// -----------------------------------------------------------------------------
// 3) Publicación inmutable (immutable publication) — patrón seguro y rápido
// -----------------------------------------------------------------------------
struct ConfigInmutable {
    const int max_conexiones;
    const double timeout_seg;
    const std::string nombre_servidor;

    ConfigInmutable(int mc, double ts, std::string ns)
        : max_conexiones(mc), timeout_seg(ts), nombre_servidor(std::move(ns)) {}
};

// Puntero atómico a config inmutable (nullptr = no publicada aún)
std::atomic<const ConfigInmutable*> config_ptr{nullptr};

void publicador_config() {
    // 1. Construir COMPLETAMENTE el objeto en local (sin sincronización)
    auto config = new ConfigInmutable(100, 30.5, "servidor-prod");

    // 2. Publicar con RELEASE: todos los writes del constructor son visibles
    //    para quien haga ACQUIRE load de este puntero.
    config_ptr.store(config, std::memory_order_release);
}

void lector_config(int id) {
    // 3. Leer con ACQUIRE: ve el objeto COMPLETO e inmutable
    const ConfigInmutable* cfg = config_ptr.load(std::memory_order_acquire);
    while (!cfg) {
        std::this_thread::yield();
        cfg = config_ptr.load(std::memory_order_acquire);
    }
    // Como ConfigInmutable es TOTALMENTE inmutable (todos const), NO hace
    // falta más sincronización para leer sus campos. Seguro y rápido.
    std::cout << "[lector " << id << "] config: " << cfg->nombre_servidor
              << ", max=" << cfg->max_conexiones << ", timeout=" << cfg->timeout_seg << "\n";
}

void demo_publicacion_inmutable() {
    std::cout << "--- Publicación inmutable (immutable publication) ---\n";
    config_ptr = nullptr;

    std::thread pub(publicador_config);
    std::thread l1(lector_config, 1);
    std::thread l2(lector_config, 2);
    std::thread l3(lector_config, 3);

    pub.join(); l1.join(); l2.join(); l3.join();

    // Cleanup (en programa real: shared_ptr con custom deleter o epoch-based reclamation)
    delete config_ptr.load(std::memory_order_relaxed);
    std::cout << "Publicación inmutable OK: lectores ven objeto completo sin locks\n\n";
}

// -----------------------------------------------------------------------------
// 4) std::call_once / std::once_flag — inicialización thread-safe ÚNICA
//    ES LA FORMA CORRECTA de singleton/inicialización perezosa en C++ moderno.
// -----------------------------------------------------------------------------
std::once_flag flag_inicializacion;
std::shared_ptr<ConfigInmutable> singleton_config;

void inicializar_singleton() {
    std::cout << "[call_once] inicializando singleton...\n";
    singleton_config = std::make_shared<ConfigInmutable>(200, 15.0, "singleton");
}

void obtener_singleton(int id) {
    // call_once garantiza: UNA sola llamada a inicializar_singleton(),
    // aunque miles de hilos llamen concurrentemente a obtener_singleton().
    std::call_once(flag_inicializacion, inicializar_singleton);
    std::cout << "[hilo " << id << "] singleton: " << singleton_config->nombre_servidor << "\n";
}

void demo_call_once() {
    std::cout << "--- std::call_once / std::once_flag ---\n";
    std::vector<std::thread> hilos;
    for (int i = 0; i < 10; ++i) hilos.emplace_back(obtener_singleton, i);
    for (auto& t : hilos) t.join();
    std::cout << "call_once: inicialización única garantizada sin data race\n\n";
}

// -----------------------------------------------------------------------------
// 5) Double-checked locking (DCL) — SOLO para mostrar POR QUÉ call_once es mejor
//    En C++11+ FUNCIONA con atomic<ptr> + release/acquire, pero es frágil.
// -----------------------------------------------------------------------------
std::atomic<ConfigInmutable*> dcl_ptr{nullptr};
std::mutex dcl_mutex;

ConfigInmutable* obtener_dcl() {
    ConfigInmutable* p = dcl_ptr.load(std::memory_order_acquire);
    if (!p) {
        std::lock_guard<std::mutex> lk(dcl_mutex);
        p = dcl_ptr.load(std::memory_order_relaxed);  // re-check bajo lock
        if (!p) {
            auto nuevo = new ConfigInmutable(300, 10.0, "DCL");
            dcl_ptr.store(nuevo, std::memory_order_release);  // publish
            p = nuevo;
        }
    }
    return p;
}

void demo_dcl() {
    std::cout << "--- Double-checked locking (funciona en C++11+ pero frágil) ---\n";
    std::vector<std::thread> hilos;
    for (int i = 0; i < 5; ++i) {
        hilos.emplace_back([] {
            auto cfg = obtener_dcl();
            std::cout << "[DCL] " << cfg->nombre_servidor << "\n";
        });
    }
    for (auto& t : hilos) t.join();
    delete dcl_ptr.load();
    std::cout << "DCL OK pero... usá std::call_once (más simple, mismo costo)\n\n";
}

// -----------------------------------------------------------------------------
// 6) Contador con fetch_add relaxed + acquire fence para lectura consistente
//    Patrón: writers usan relaxed (rápido), reader usa acquire fence + load.
// -----------------------------------------------------------------------------
std::atomic<long long> contador_global{0};

void writer_relajado(int id, int vueltas) {
    for (int i = 0; i < vueltas; ++i) {
        contador_global.fetch_add(1, std::memory_order_relaxed);
    }
    std::cout << "[writer " << id << "] terminó\n";
}

long long leer_consistente() {
    // Acquire fence asegura que veamos TODOS los fetch_add previos
    std::atomic_thread_fence(std::memory_order_acquire);
    return contador_global.load(std::memory_order_relaxed);
}

void demo_contador_relajado() {
    std::cout << "--- Contador relaxed + acquire fence para lectura ---\n";
    constexpr int VUELTAS = 100000;
    contador_global = 0;

    std::thread w1(writer_relajado, 1, VUELTAS);
    std::thread w2(writer_relajado, 2, VUELTAS);

    // Leer mientras escriben (demo: snapshot consistente)
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        std::cout << "[snapshot " << i << "] " << leer_consistente() << "\n";
    }

    w1.join(); w2.join();
    std::cout << "Final: " << contador_global.load() << " (esperado " << 2*VUELTAS << ")\n\n";
}

// -----------------------------------------------------------------------------
// MAIN
// -----------------------------------------------------------------------------
int main() {
    std::cout << "=== LECCIÓN 04: Memory ordering avanzado, atomic_ref, patrones ===\n\n";

    demo_atomic_ref();
    demo_fences();
    demo_publicacion_inmutable();
    demo_call_once();
    demo_dcl();
    demo_contador_relajado();

    std::cout << "=== FIN LECCIÓN 04 ===\n";
    return 0;
}