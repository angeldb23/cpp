/* =============================================================================
   MÓDULO 15 - LECCIÓN 10: Optimización guiada por datos — complejidad,
   localidad de caché y copias evitables
   =============================================================================
   TEMA 9 de TEMAS.md (2 lecciones): Optimización data-oriented.

   DATA-ORIENTED DESIGN (DOD):
   - Organiza datos por CÓMO se acceden, no por "objetos".
   - Estructuras de arrays (SoA) vs Array de estructuras (AoS).
   - Cache lines: 64 bytes. Aprovecha spatial locality.
   - Prefetching: hardware + software (__builtin_prefetch).

   AOS vs SOA:
   // AoS (Object-Oriented) - MAL para cache
   struct Particle { float x,y,z; float vx,vy,vz; float mass; };
   std::vector<Particle> particles;  // stride = 28 bytes, cache miss al iterar solo x

   // SoA (Data-Oriented) - BIEN para cache
   struct Particles {
       std::vector<float> x, y, z;
       std::vector<float> vx, vy, vz;
       std::vector<float> mass;
   };
   // Iterar solo x: acceso contiguo, cache line llena de x's

   CACHE LINES (64 bytes en x86):
   - Una línea = 64 bytes contiguos.
   - Leer 1 byte trae 64 bytes a L1.
   - False sharing: hilos escriben en vars distintas en misma línea -> ping-pong.
   - Solución: alignas(64) o padding.

   PREFETCHING:
   - Hardware: detector de patrones (strided access).
   - Software: __builtin_prefetch(&arr[i+64], 0, 3);  // read, high temporal
   - Prefetch distance: ~L2 latency cycles ahead.

   COPIAS EVITABLES:
   - Pass by value + move: void foo(std::string s) { use(std::move(s)); }
   - Return value optimization (RVO/NRVO): return local by value.
   - std::move en return: NO (inhibe NRVO).
   - std::string_view para read-only string params.
   - span<T> para arrays read-only.

   ALGORITMOS CACHE-FRIENDLY:
   - Loop tiling / blocking: procesar tiles que caben en L1.
   - Loop fusion: combinar loops que recorren mismo array.
   - Loop interchange: inner loop = stride-1 access.
   - SIMD: #pragma omp simd / std::simd (C++26) / intrinsics.

   MEMORY LAYOUT:
   - Struct padding: ordenar miembros por alineación (grande a pequeño).
   - Bitfields para flags compactos.
   - Compressed pointers (32-bit en 64-bit) si heap < 4GB.

   ALLOCADORES PERSONALIZADOS:
   - Pool allocator: objetos del mismo tamaño.
   - Arena allocator: región contigua, free masivo al final.
   - Bump allocator: puntero que avanza, free = reset pointer.
   - std::pmr::polymorphic_allocator (C++17).

   HOT/COLD DATA SEPARATION:
   - Separar campos hot (acceso frecuente) de cold (raro).
   - Struct splitting: HotFields + ColdFields*.

   ENTITY COMPONENT SYSTEM (ECS):
   - Arquetipos: contiguos arrays por componente.
   - Iteración: solo componentes necesarios.
   - Cache-friendly por diseño.

   PROFILING PARA DOD:
   - perf stat -e cache-misses,L1-dcache-load-misses
   - Cachegrind: --simulate-cache=yes
   - VTune: Memory Access Analysis
   - Métricas: cache miss rate, bandwidth utilization, IPC.

   REGLAS DE ORO DOD:
   1. Acceso lineal > acceso aleatorio
   2. SoA > AoS para procesamiento masivo
   3. Datos hot juntos, cold separados
   4. Evita punteros (indirección = cache miss)
   5. Prefetch manual si patrón conocido
   6. Alinea a 64 bytes para evitar false sharing
   7. Pool/arena allocators para objetos efímeros
   8. Mide con perf stat cache-misses antes de optimizar

   CASO REAL: PARTICLE SYSTEM
   - AoS: 1M particles, update() = 50ms
   - SoA: 1M particles, update() = 12ms (4x speedup)
   - + SIMD: 3ms (16x vs AoS) */

#include <iostream>
#include <vector>
#include <chrono>
#include <memory>
#include <cstring>
#include <atomic>
#include <thread>

// Demo: AoS vs SoA

struct ParticleAoS {
    float x, y, z;
    float vx, vy, vz;
    float mass;
};

struct ParticlesSoA {
    std::vector<float> x, y, z;
    std::vector<float> vx, vy, vz;
    std::vector<float> mass;

    void resize(size_t n) {
        x.resize(n); y.resize(n); z.resize(n);
        vx.resize(n); vy.resize(n); vz.resize(n);
        mass.resize(n);
    }
};

void update_aos(std::vector<ParticleAoS>& p, float dt) {
    for (auto& pt : p) {
        pt.x += pt.vx * dt;
        pt.y += pt.vy * dt;
        pt.z += pt.vz * dt;
    }
}

void update_soa(ParticlesSoA& p, float dt) {
    size_t n = p.x.size();
    for (size_t i = 0; i < n; ++i) {
        p.x[i] += p.vx[i] * dt;
        p.y[i] += p.vy[i] * dt;
        p.z[i] += p.vz[i] * dt;
    }
}

void demo_aos_vs_soa() {
    std::cout << "--- AoS vs SoA benchmark ---\n";
    constexpr size_t N = 1'000'000;
    constexpr int ITER = 10;

    std::vector<ParticleAoS> aos(N);
    ParticlesSoA soa;
    soa.resize(N);

    // Init
    for (size_t i = 0; i < N; ++i) {
        aos[i] = {float(i), float(i), float(i), 1.0f, 2.0f, 3.0f, 1.0f};
        soa.x[i] = float(i); soa.y[i] = float(i); soa.z[i] = float(i);
        soa.vx[i] = 1.0f; soa.vy[i] = 2.0f; soa.vz[i] = 3.0f;
        soa.mass[i] = 1.0f;
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITER; ++i) update_aos(aos, 0.016f);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto aos_time = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITER; ++i) update_soa(soa, 0.016f);
    t2 = std::chrono::high_resolution_clock::now();
    auto soa_time = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    std::cout << "AoS: " << aos_time << " ms\n";
    std::cout << "SoA: " << soa_time << " ms\n";
    std::cout << "Speedup: " << float(aos_time) / soa_time << "x\n\n";
}

// -----------------------------------------------------------------------------
// 2. False sharing demo
// -----------------------------------------------------------------------------
struct alignas(64) CounterPadded {
    std::atomic<long long> value{0};
};

struct CounterPlain {
    std::atomic<long long> value{0};
};

void demo_false_sharing() {
    std::cout << "--- False sharing ---\n";
    constexpr int THREADS = 4;
    constexpr int ITER = 1'000'000;

    CounterPlain plain[THREADS];
    std::vector<std::thread> threads;
    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < THREADS; ++i) {
        threads.emplace_back([&, i] {
            for (int j = 0; j < ITER; ++j) plain[i].value++;
        });
    }
    for (auto& t : threads) t.join();
    auto t_plain = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - t1).count();

    CounterPadded padded[THREADS];
    threads.clear();
    auto t2 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < THREADS; ++i) {
        threads.emplace_back([&, i] {
            for (int j = 0; j < ITER; ++j) padded[i].value++;
        });
    }
    for (auto& t : threads) t.join();
    auto t_padded = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - t2).count();

    std::cout << "Sin padding (false sharing): " << t_plain << " ms\n";
    std::cout << "Con padding (alignas 64):    " << t_padded << " ms\n";
    std::cout << "Mejora: " << float(t_plain) / t_padded << "x\n\n";
}

// -----------------------------------------------------------------------------
// 3. Pool allocator simple
// -----------------------------------------------------------------------------
template <typename T, size_t PoolSize = 1024>
class PoolAllocator {
    alignas(64) char pool_[PoolSize * sizeof(T)];
    std::atomic<size_t> next_{0};

public:
    T* allocate() {
        size_t idx = next_.fetch_add(1);
        if (idx >= PoolSize) return nullptr;
        return reinterpret_cast<T*>(pool_ + idx * sizeof(T));
    }
    void deallocate(T*) {}  // No-op, reset al final
    void reset() { next_ = 0; }
};

struct Particle {
    float x, y, z;
    float vx, vy, vz;
};

void demo_pool_allocator() {
    std::cout << "--- Pool allocator ---\n";
    PoolAllocator<Particle, 100000> pool;
    std::vector<Particle*> particles;
    particles.reserve(100000);

    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000; ++i) {
        Particle* p = pool.allocate();
        new (p) Particle{1,2,3, 0.1f, 0.2f, 0.3f};
        particles.push_back(p);
    }
    auto t_pool = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - t1).count();

    std::vector<Particle*> particles_new;
    particles_new.reserve(100000);
    auto t3 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000; ++i) {
        particles_new.push_back(new Particle{1,2,3, 0.1f, 0.2f, 0.3f});
    }
    auto t_new = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - t3).count();

    std::cout << "Pool allocator: " << t_pool << " us\n";
    std::cout << "new/delete:     " << t_new << " us\n";
    std::cout << "Speedup: " << float(t_new) / t_pool << "x\n";

    for (auto p : particles_new) delete p;
    std::cout << "\n";
}

// -----------------------------------------------------------------------------
// 4. string_view / span para evitar copias
// -----------------------------------------------------------------------------
#include <string_view>
#include <span>

void foo_copy(std::string s) { std::cout << s << "\n"; }
void foo_view(std::string_view s) { std::cout << s << "\n"; }
void process_span(std::span<const int> s) {
    for (int x : s) std::cout << x << " ";
    std::cout << "\n";
}

void demo_string_view() {
    std::cout << "--- string_view / span ---\n";
    std::string s = "hola mundo";

    // Mal: copia
    foo_copy(s);
    // Bien: string_view
    foo_view(s);

    // span para arrays
    std::vector<int> v = {1,2,3,4,5};
    process_span(v);
    std::cout << "string_view/span: zero-copy views\n\n";
}

// -----------------------------------------------------------------------------
// 5. Hot/Cold separation
// -----------------------------------------------------------------------------
struct EntityHot {
    float x, y, z;
    float vx, vy, vz;
    int health;
};

struct EntityCold {
    std::string name;
    std::string description;
    int level;
    int exp;
};

struct Entity {
    EntityHot hot;
    EntityCold* cold;  // Puntero a datos fríos (puede ser nullptr)
};

void demo_hot_cold() {
    std::cout << "--- Hot/Cold separation ---\n";
    std::vector<Entity> entities(1000);
    for (auto& e : entities) {
        e.hot = {0,0,0, 1,0,0, 100};
        e.cold = new EntityCold{"Enemy", "Big boss", 10, 1000};
    }

    // Loop hot: solo toca hot data (cache friendly)
    int alive = 0;
    for (const auto& e : entities) {
        if (e.hot.health > 0) alive++;
    }
    std::cout << "Alive: " << alive << " (solo tocó hot data)\n";

    for (auto& e : entities) delete e.cold;
    std::cout << "Cold data separado: mejor cache utilization\n\n";
}

int main() {
    std::cout << "=== LECCIÓN 10: Optimización guiada por datos ===\n\n";

    demo_aos_vs_soa();
    demo_false_sharing();
    demo_pool_allocator();
    demo_string_view();
    demo_hot_cold();

    std::cout << "=== FIN LECCIÓN 10 ===\n";
    return 0;
}