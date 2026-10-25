/* =============================================================================
   MÓDULO 14 - LECCIÓN 11: thread_local, inmutabilidad, particionado de datos,
   diseño concurrente sin locks (lock-free by design).
   =============================================================================
   TEMA 7 de TEMAS.md (1 lección): Diseño concurrente: inmutabilidad,
   thread_local y particionado de datos.

   thread_local — VARIABLES POR HILO:
   - Cada hilo tiene su PROPIA instancia independiente de la variable.
   - Inicialización: primera vez que el hilo accede (thread-safe en C++11+).
   - Destrucción: al terminar el hilo (antes de thread_local de otros hilos).
   - NO hay data race: cada hilo toca SU copia.
   - USOS COMUNES:
     * Caches locales (allocators, pools, buffers).
     * Contadores/estadísticas por hilo (agregar al final).
     * Generadores aleatorios (mt19937 no es thread-safe).
     * Contextos de request (request_id, trace_id, locale).
     * Recursos TLS (OpenGL context, DB connection pool per thread).

   INMUTABILIDAD — ELIMINAR ESTADO COMPARTIDO MUTABLE:
   - Si no hay writes compartidos, NO HACE FALTA SINCRONIZACIÓN.
   - Patrones:
     * Datos inmutables + publicación atómica (release/acquire) = zero locks.
     * Copy-on-write (COW): shared_ptr<const T>, al modificar clonar + publicar.
     * Persistent data structures: structural sharing (Clojure style, Immer lib).
   - Ventajas: reasoning simple, composable, cero deadlocks, escalabilidad perfecta.

   PARTICIONADO DE DATOS (Data Partitioning / Sharding):
   - Dividir datos en particiones INDEPENDIENTES, una por hilo/owner.
   - Cada partición tiene SU mutex (o ninguna si inmutable).
   - Operaciones van a la partición correcta (hash, rango, key).
   - Ejemplos: sharded hash map, per-thread counters, per-CPU structures.
   - Trade-off: particiones fijas vs dinámicas (rebalanceo).

   REDUCCIÓN (Reduction / Map-Reduce local):
   - Cada hilo acumula en SU variable local (thread_local o local al loop).
   - Al final: combinan resultados (suma, merge, etc.) — una sola sincronización.
   - Evita contención en variable compartida durante el loop.

   EJEMPLOS: thread_local RNG/buffer, copy-on-write con shared_ptr<const>,
   contador particionado (sharded counter), map-reduce local, arena allocator. */

#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <memory>
#include <chrono>
#include <random>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <future>
#include <queue>

// -----------------------------------------------------------------------------
// 1) thread_local: RNG independiente por hilo (mt19937 NO es thread-safe)
// -----------------------------------------------------------------------------
thread_local std::mt19937 rng_thread_local([]{
    std::random_device rd;
    // Mezclar thread_id para semillas distintas por hilo
    return rd() ^ std::hash<std::thread::id>{}(std::this_thread::get_id());
}());

int random_int(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng_thread_local);  // cada hilo usa SU rng
}

void demo_thread_local_rng() {
    std::cout << "--- thread_local RNG ---\n";
    std::atomic<int> suma{0};

    auto worker = [&](int id) {
        int local_sum = 0;
        for (int i = 0; i < 10000; ++i) local_sum += random_int(1, 10);
        suma += local_sum;
        std::cout << "[hilo " << id << "] suma local: " << local_sum << "\n";
    };

    std::thread t1(worker, 1), t2(worker, 2), t3(worker, 3);
    t1.join(); t2.join(); t3.join();
    std::cout << "Total: " << suma << "\n\n";
}

// -----------------------------------------------------------------------------
// 2) thread_local buffer/arena: evitar allocaciones concurrentes
// -----------------------------------------------------------------------------
thread_local std::vector<char> thread_buffer;

char* get_thread_buffer(size_t size) {
    if (thread_buffer.size() < size) thread_buffer.resize(size);
    return thread_buffer.data();
}

void demo_thread_local_buffer() {
    std::cout << "--- thread_local buffer (arena simple) ---\n";

    auto worker = [](int id) {
        for (int i = 0; i < 100; ++i) {
            char* buf = get_thread_buffer(256);
            // usar buf...
            buf[0] = static_cast<char>(id);
        }
        std::cout << "[hilo " << id << "] buffer size: " << thread_buffer.size() << "\n";
    };

    std::thread t1(worker, 1), t2(worker, 2);
    t1.join(); t2.join();
    std::cout << "Buffers independientes, sin contención en allocador global\n\n";
}

// -----------------------------------------------------------------------------
// 3) Contador particionado (sharded counter) — reducción de contención
// -----------------------------------------------------------------------------
class ShardedCounter {
    static constexpr size_t SHARDS = 64;  // potencia de 2, >= num_cpus
    alignas(64) std::atomic<long long> shards_[SHARDS];  // 64 bytes = cache line

public:
    ShardedCounter() { for (auto& s : shards_) s = 0; }

    // Incrementar en shard basado en thread_id (distribución determinista)
    void increment() {
        size_t idx = std::hash<std::thread::id>{}(std::this_thread::get_id()) & (SHARDS - 1);
        shards_[idx].fetch_add(1, std::memory_order_relaxed);
    }

    // Incrementar en shard específico (para control manual)
    void increment_shard(size_t shard) {
        shards_[shard & (SHARDS - 1)].fetch_add(1, std::memory_order_relaxed);
    }

    // Leer total (snapshot consistente: acquire en cada shard)
    long long load() const {
        long long total = 0;
        for (size_t i = 0; i < SHARDS; ++i) {
            total += shards_[i].load(std::memory_order_acquire);
        }
        return total;
    }
};

void demo_sharded_counter() {
    std::cout << "--- Contador particionado (sharded counter) ---\n";
    ShardedCounter counter;
    const int VUELTAS = 100000;

    auto worker = [&](int id) {
        for (int i = 0; i < VUELTAS; ++i) counter.increment();
    };

    std::thread t1(worker, 1), t2(worker, 2), t3(worker, 3), t4(worker, 4);
    t1.join(); t2.join(); t3.join(); t4.join();

    std::cout << "Esperado: " << 4 * VUELTAS << ", Obtenido: " << counter.load() << "\n";
    std::cout << "Sin contención en cache line (cada hilo en su shard)\n\n";
}

// -----------------------------------------------------------------------------
// 4) Copy-on-Write con shared_ptr<const T> — inmutabilidad + publicación atómica
// -----------------------------------------------------------------------------
struct ConfigApp {
    int max_conexiones;
    double timeout;
    std::string servidor;

    ConfigApp(int mc, double t, std::string s) : max_conexiones(mc), timeout(t), servidor(std::move(s)) {}
};

// Puntero atómico a config inmutable (shared_ptr es thread-safe para ref-count)
std::atomic<std::shared_ptr<const ConfigApp>> config_global{nullptr};

void lector_config_cow(int id) {
    for (int i = 0; i < 10; ++i) {
        auto cfg = config_global.load(std::memory_order_acquire);
        if (cfg) {
            // Lectura SIN locks: config es INMUTABLE (const)
            // std::cout << "[lector " << id << "] " << cfg->servidor << "\n";
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void actualizar_config_cow(int max_con, double timeout, std::string servidor) {
    // 1. Crear NUEVA config inmutable (copy-on-write)
    auto nueva = std::make_shared<const ConfigApp>(max_con, timeout, std::move(servidor));
    // 2. Publicar con RELEASE (atomic store)
    config_global.store(nueva, std::memory_order_release);
    // 3. La antigua se destruye cuando todos los shared_ptr lectores terminen
}

void demo_cow() {
    std::cout << "--- Copy-on-Write con shared_ptr<const> ---\n";

    actualizar_config_cow(10, 1.0, "servidor-v1");

    std::thread l1(lector_config_cow, 1);
    std::thread l2(lector_config_cow, 2);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    actualizar_config_cow(20, 2.0, "servidor-v2");  // nueva versión
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    actualizar_config_cow(30, 3.0, "servidor-v3");  // otra versión

    l1.join(); l2.join();
    std::cout << "COW: lectores sin locks, writers publican atómicamente\n\n";
}

// -----------------------------------------------------------------------------
// 5) Map-Reduce local: cada hilo acumula en local, merge al final
// -----------------------------------------------------------------------------
using FreqMap = std::unordered_map<std::string, int>;

FreqMap contar_palabras_local(const std::vector<std::string>& palabras) {
    FreqMap local;
    for (const auto& p : palabras) ++local[p];
    return local;  // RVO / move
}

FreqMap merge_maps(FreqMap&& a, const FreqMap& b) {
    for (const auto& [k, v] : b) a[k] += v;
    return std::move(a);
}

void demo_map_reduce_local() {
    std::cout << "--- Map-Reduce local (acumular por hilo, merge final) ---\n";

    std::vector<std::string> datos = {
        "hola", "mundo", "hola", "cpp", "mundo", "hilo",
        "hola", "concurre", "cpp", "mundo", "hola", "hola"
    };

    // Dividir datos entre hilos
    size_t mid = datos.size() / 2;
    std::vector<std::string> parte1(datos.begin(), datos.begin() + mid);
    std::vector<std::string> parte2(datos.begin() + mid, datos.end());

    std::future<FreqMap> f1 = std::async(std::launch::async, contar_palabras_local, parte1);
    std::future<FreqMap> f2 = std::async(std::launch::async, contar_palabras_local, parte2);

    FreqMap total = merge_maps(f1.get(), f2.get());

    for (auto& [k, v] : total) std::cout << "  " << k << ": " << v << "\n";
    std::cout << "Map-Reduce: cero contención durante cuenta, merge final único\n\n";
}

// -----------------------------------------------------------------------------
// 6) Diseño sin locks: estado inmutable + mensaje passing (actor model simple)
// -----------------------------------------------------------------------------
struct Mensaje {
    enum Tipo { INCREMENTAR, OBTENER, PARAR } tipo;
    int valor = 0;
    std::promise<int> resp;  // para respuesta sincrónica
};

class ContadorActor {
    std::queue<Mensaje> cola_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::jthread worker_;
    int valor_ = 0;

    void loop(std::stop_token st) {
        while (!st.stop_requested()) {
            Mensaje msg;
            {
                std::unique_lock<std::mutex> lk(mtx_);
                cv_.wait(lk, [this, &st] { return !cola_.empty() || st.stop_requested(); });
                if (cola_.empty() && st.stop_requested()) break;
                msg = std::move(cola_.front());
                cola_.pop();
            }

            switch (msg.tipo) {
                case Mensaje::INCREMENTAR: valor_ += msg.valor; break;
                case Mensaje::OBTENER: msg.resp.set_value(valor_); break;
                case Mensaje::PARAR: return;  // salir del loop, jthread hace join
            }
        }
    }

public:
    ContadorActor() : worker_([this](std::stop_token st) { loop(st); }) {}

    void incrementar(int v = 1) {
        std::lock_guard<std::mutex> lk(mtx_);
        cola_.push({Mensaje::INCREMENTAR, v});
        cv_.notify_one();
    }

    int obtener() {
        Mensaje msg{ Mensaje::OBTENER };
        auto fut = msg.resp.get_future();
        {
            std::lock_guard<std::mutex> lk(mtx_);
            cola_.push(std::move(msg));
        }
        cv_.notify_one();
        return fut.get();  // bloquea hasta respuesta
    }

    ~ContadorActor() {
        {
            std::lock_guard<std::mutex> lk(mtx_);
            cola_.push({Mensaje::PARAR});
        }
        cv_.notify_one();
        // jthread join auto
    }
};

void demo_actor_model() {
    std::cout << "--- Actor model simple (estado encapsulado, message passing) ---\n";
    ContadorActor actor;

    auto worker = [&](int id) {
        for (int i = 0; i < 1000; ++i) actor.incrementar();
    };

    std::thread t1(worker, 1), t2(worker, 2), t3(worker, 3);
    t1.join(); t2.join(); t3.join();

    std::cout << "Valor final: " << actor.obtener() << " (esperado 3000)\n";
    std::cout << "Actor: estado mutable SOLO en hilo owner, acceso via mensajes\n\n";
}

// -----------------------------------------------------------------------------
// MAIN
// -----------------------------------------------------------------------------
int main() {
    std::cout << "=== LECCIÓN 11: thread_local, inmutabilidad, particionado, actores ===\n\n";

    demo_thread_local_rng();
    demo_thread_local_buffer();
    demo_sharded_counter();
    demo_cow();
    demo_map_reduce_local();
    demo_actor_model();

    std::cout << "=== FIN LECCIÓN 11 ===\n";
    return 0;
}