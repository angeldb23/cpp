/* =============================================================================
   MÓDULO 15 - LECCIÓN 11: Data-Oriented Design avanzado — ECS, arquetipos,
   SIMD, memory layout, custom allocators, casos reales
   =============================================================================
   TEMA 9 de TEMAS.md (2da lección): Data-Oriented Design avanzado.

   ENTITY COMPONENT SYSTEM (ECS) — ARQUETIPOS:
   - Arquetipo = conjunto de componentes (tipo + layout)
   - Cada arquetipo = arrays contiguos por componente (SoA)
   - Entidad = índice en arrays del arquetipo
   - Query: iterar arquetipos que contienen componentes requeridos
   - Ventaja: cache-friendly, sin branch misprediction en loops

   ECS IMPLEMENTACIÓN:
   - ComponentPool<T>: array contiguo + free list
   - Archetype: vector<ComponentPool*> + entity_count
   - Entity: (archetype_id, index_in_archetype)
   - Query: filter archetypes by component mask
   - Iteration: for archetype in query { for i in 0..count: process(pools) }

   SIMD / VECTORIZACIÓN:
   - #pragma omp simd (OpenMP 4+)
   - std::simd (C++26, experimental en libc++)
   - Intrinsics: _mm256_add_ps, _mm256_fmadd_ps (AVX2/AVX-512)
   - Auto-vectorization: -O3 -march=native -ftree-vectorize
   - Alignment: alignas(32) para AVX2, alignas(64) para AVX-512

   MEMORY LAYOUT OPTIMIZATION:
   - Struct padding: ordenar miembros por alineación (grande→pequeño)
   - Bitfields para flags compactos
   - Compressed pointers (32-bit en 64-bit si heap < 4GB)
   - Hot/Cold splitting: struct Hot { ... }; struct Cold*;

   ALLOCADORES PERSONALIZADOS (C++17 std::pmr):
   - std::pmr::memory_resource (polimórfico)
   - std::pmr::polymorphic_allocator<T>
   - monotonic_buffer_resource (arena, bump allocator)
   - pool_options + unsynchronized_pool_resource
   - synchronized_pool_resource (thread-safe)
   - Custom memory_resource para tracking/debugging

   HOT/COLD DATA SEPARATION:
   - Separar campos hot (frame) de cold (config, metadata)
   - Struct splitting: HotData + ColdData*
   - Cold data: pointer + allocation separada
   - Acceso hot: stride = sizeof(HotData), cache-friendly

   ENTITY COMPONENT SYSTEM (ECS) PATTERNS:
   - Archetype-based (EnTT, flecs): arrays por arquetipo
   - Sparse Set (EnTT): sparse + dense arrays, O(1) add/remove
   - Bitset/ComponentMask: filter rápido de queries
   - Deferred operations: add/remove en frame boundary

   CASOS REALES:
   1. Particle System: AoS 50ms → SoA 12ms → +SIMD 3ms
   2. Physics Engine: AABB broadphase → SIMD 4x
   3. Renderer: frustum culling → SoA + SIMD
   4. AI/Pathfinding: cache-friendly grids + prefetch
   4. Audio DSP: SoA buffers + SIMD FIR/IIR

   PROFILING PARA DOD:
   - perf stat -e cache-misses,L1-dcache-load-misses
   - Cachegrind: --simulate-cache=yes
   - VTune: Memory Access Analysis
   - Métricas: cache miss rate, bandwidth utilization, IPC

   MEMORY POOLS ESPECIALIZADOS:
   - Object Pool: T fijo, free list
   - Arena: bump allocator, reset frame
   - Stack Allocator: LIFO, frame
   - Free List: variable size, coalescing
   - Buddy Allocator: potencias de 2, coalescing

   FUTURO DOD:
   - MTE (Memory Tagging Extension) ARMv8.5+
   - CHERI (Capability Hardware)
   - std::simd (C++26)
   - P0900 (std::execution::simd)
   - P2300 (senders/receivers para pipelines DOD) */

#include <iostream>
#include <vector>
#include <chrono>
#include <memory>
#include <cstring>
#include <atomic>
#include <immintrin.h>  // AVX intrinsics (requiere -mavx2 -mfma al compilar)

/*
 * NOTA IMPORTANTE SOBRE AVX2 Y EL CELERON N4020:
 * ------------------------------------------------
 * El CPU Intel Celeron N4020 (Gemini Lake) SOLO soporta SSE4.1/SSE4.2, NO AVX/AVX2.
 * Este código usa #if defined(__AVX2__) && defined(__FMA__) que es DETECCIÓN EN TIEMPO DE COMPILACIÓN
 * basada en los flags del compilador (-mavx2 -mfma), NO en tiempo de ejecución.
 *
 * - Si compiles SIN -mavx2 (lo que hace el curso: -std=c++20 -Wall -Wextra), __AVX2__ no se define
 *   y se usa el fallback escalar (rama #else). El código compila y corre seguro en el N4020.
 * - Si compiles CON -mavx2 -mfma en una máquina CON AVX2, se usa el camino vectorizado.
 * - Los EJERCICIOS y SOLUCIONES del módulo 15 son STRING-BASED (devuelven código como texto),
 *   no ejecutan SIMD real; solo verifican que el estudiante conozca los intrinsics.
 *
 * Para detección en TIEMPO DE EJECUCIÓN real, usar __builtin_cpu_supports("avx2") (GCC/Clang)
 * o CPUID manual. Eso está fuera del alcance de este módulo.
 */
#include <memory_resource>

// Demo: ECS Arquetipos

struct Position { float x, y, z; };
struct Velocity { float vx, vy, vz; };
struct Health { int hp; };

// Component Pool simple
template <typename T>
struct ComponentPool {
    std::vector<T> data;
    std::vector<size_t> free_list;

    T* add() {
        if (!free_list.empty()) {
            size_t idx = free_list.back(); free_list.pop_back();
            return &data[idx];
        }
        data.emplace_back();
        return &data.back();
    }
    void remove(size_t idx) { free_list.push_back(idx); }
    T* get(size_t idx) { return &data[idx]; }
    size_t size() const { return data.size() - free_list.size(); }
};

// Arquetipo: conjunto de component pools
struct Archetype {
    ComponentPool<Position>* positions = nullptr;
    ComponentPool<Velocity>* velocities = nullptr;
    ComponentPool<Health>* health = nullptr;
    size_t count = 0;

    void add_entity() {
        // En implementación real: agregar a pools, manejar capacity
        count++;
    }

    // Stub methods para demo del Query template
    template <typename... Components>
    bool has_components() const { return true; }  // Simplificado para demo

    template <typename T>
    T* get(size_t idx) {
        if constexpr (std::is_same_v<T, Position>) return positions ? positions->get(idx) : nullptr;
        if constexpr (std::is_same_v<T, Velocity>) return velocities ? velocities->get(idx) : nullptr;
        if constexpr (std::is_same_v<T, Health>) return health ? health->get(idx) : nullptr;
        return nullptr;
    }
};

// ECS World simple
struct World {
    std::vector<Archetype> archetypes;

    Archetype* get_or_create_archetype(bool has_pos, bool has_vel, bool has_hp) {
        // Buscar o crear arquetipo con esos componentes
        (void)has_pos; (void)has_vel; (void)has_hp;  // unused en demo
        return &archetypes.emplace_back();
    }
};

// Query: iterar arquetipos con componentes requeridos (simplificado para demo)
template <typename... Components>
struct Query {
    World* world;
    void each(auto&& func) {
        for (auto& arch : world->archetypes) {
            if (arch.has_components<Components...>()) {
                for (size_t i = 0; i < arch.count; ++i) {
                    func(arch.get<Components>(i)...);
                }
            }
        }
    }
};

void demo_ecs_archetype() {
    std::cout << "--- ECS Arquetipos ---\n";
    std::cout << R"(
# Arquetipo = conjunto de componentes con mismo layout
# Cada arquetipo = arrays contiguos (SoA) por componente

# Estructura:
# Archetype[Position, Velocity]:
#   positions:  [x, y, z, x, y, z, ...]
#   velocities: [vx, vy, vz, vx, vy, vz, ...]
#   entities:   [eid1, eid2, ...]

# Archetype[Position, Velocity, Health]:
#   positions:  [...]
#   velocities: [...]
#   health:     [hp, hp, ...]

# Query: itera arquetipos que contienen componentes requeridos
# for (auto& arch : world.archetypes) {
#     if (arch.has<Position, Velocity>()) {
#         for (size_t i = 0; i < arch.count; ++i) {
#             pos = arch.positions[i];
#             vel = arch.velocities[i];
#             update(pos, vel);
#         }
#     }
# }

# Ventajas:
# - Cache-friendly: acceso contiguo
# - Sin branch misprediction (mismo loop para todos)
# - Fácil vectorización (SIMD)
)" << "\n\n";
}

// -----------------------------------------------------------------------------
// 2. SIMD / Vectorización
// -----------------------------------------------------------------------------
void demo_simd() {
    std::cout << "--- SIMD / Vectorización ---\n";
    std::cout << R"(
# Auto-vectorization:
# -O3 -march=native -ftree-vectorize
# #pragma omp simd
for (size_t i = 0; i < n; ++i) c[i] = a[i] + b[i];

# Intrinsics AVX2 (manual):
#include <immintrin.h>
void add_avx2(float* a, float* b, float* c, size_t n) {
    for (size_t i = 0; i < n; i += 8) {
        __m256 va = _mm256_loadu_ps(a + i);
        __m256 vb = _mm256_loadu_ps(b + i);
        __m256 vc = _mm256_add_ps(va, vb);
        _mm256_storeu_ps(c + i, vc);
    }
}

# AVX-512:
__m512 va = _mm512_loadu_ps(a + i);
__m512 vb = _mm512_loadu_ps(b + i);
__m512 vc = _mm512_add_ps(va, vb);
_mm512_storeu_ps(c + i, vc);

# Alignment para SIMD:
alignas(32) float arr[1024];  // AVX2 (32 bytes)
alignas(64) float arr2[1024]; // AVX-512 (64 bytes)

# std::simd (C++26 experimental):
#include <simd>
std::simd<float, std::simd_abi::native> va, vb, vc;
vc = va + vb;  // Auto-vectorizado
)" << "\n\n";
}

// -----------------------------------------------------------------------------
// 3. Memory Layout Optimization
// -----------------------------------------------------------------------------
struct BadLayout {
    char c;       // 1 byte
    double d;     // 8 bytes (padding 7)
    int i;        // 4 bytes
    char c2;      // 1 byte (padding 3)
    // Total: 24 bytes (padding = 11 bytes)
};

struct GoodLayout {
    double d;     // 8 bytes
    int i;        // 4 bytes
    char c;       // 1 byte
    char c2;      // 1 byte
    // Total: 16 bytes (padding = 2 bytes)
};

struct BitfieldExample {
    uint32_t flags : 4;
    uint32_t id : 28;
    // 4 bytes total vs 8 bytes con bool/int separados
};

void demo_memory_layout() {
    std::cout << "--- Memory Layout Optimization ---\n";
    std::cout << "BadLayout size: " << sizeof(BadLayout) << " bytes\n";
    std::cout << "GoodLayout size: " << sizeof(GoodLayout) << " bytes\n";
    std::cout << "BitfieldExample size: " << sizeof(BitfieldExample) << " bytes\n\n";

    std::cout << R"(
# Reglas:
# 1. Ordenar miembros: grande → pequeño (double → int → char)
# 2. Agrupar bool/flags en bitfields
# 3. alignas(64) para evitar false sharing
# 3. Hot/Cold splitting:
struct Hot { float x, y, z, vx, vy, vz; };
struct Cold { std::string name; std::string desc; int level; };
struct Entity { Hot hot; Cold* cold; };  // Cold separado
)" << "\n\n";
}

// -----------------------------------------------------------------------------
// 4. std::pmr / Custom Allocators
// -----------------------------------------------------------------------------
#include <memory_resource>

void demo_pmr() {
    std::cout << "--- std::pmr / Polymorphic Allocators ---\n";
    std::cout << R"(
# std::pmr::memory_resource (base polimórfica)
# std::pmr::polymorphic_allocator<T> (type-erased allocator)

# Recursos predefinidos:
std::pmr::monotonic_buffer_resource arena(1024 * 1024);  // 1MB arena
std::pmr::polymorphic_allocator<int> alloc(&arena);

std::pmr::vector<int> vec(&arena);
vec.push_back(42);  # Alloca del arena (bump pointer)

# Pool resources:
std::pmr::unsynchronized_pool_resource pool;
std::pmr::vector<int> pooled_vec(&pool);

# Synchronized (thread-safe):
std::pmr::synchronized_pool_resource sync_pool;
std::pmr::vector<int> sync_vec(&sync_pool);

# Custom memory_resource (tracking, debugging):
class TrackingResource : public std::pmr::memory_resource {
    std::pmr::memory_resource* upstream;
    size_t allocated = 0;
    void* do_allocate(size_t bytes, size_t align) override {
        allocated += bytes;
        return upstream->allocate(bytes, align);
    }
    void do_deallocate(void* p, size_t bytes, size_t align) override {
        upstream->deallocate(p, bytes, align);
    }
    bool do_is_equal(const memory_resource& other) const noexcept override {
        return this == &other;
    }
};
)" << "\n\n";
}

// -----------------------------------------------------------------------------
// 4. Custom Allocators especializados
// -----------------------------------------------------------------------------
template <typename T, size_t PoolSize = 1024>
class PoolAllocator {
    alignas(64) char pool_[PoolSize * sizeof(T)];
    std::atomic<size_t> next_{0};

public:
    using value_type = T;

    PoolAllocator() = default;

    T* allocate(size_t n) {
        if (n != 1) throw std::bad_alloc();
        size_t idx = next_.fetch_add(1);
        if (idx >= PoolSize) throw std::bad_alloc();
        return reinterpret_cast<T*>(pool_ + idx * sizeof(T));
    }
    void deallocate(T*, size_t) {}  // No-op, reset al final

    void reset() { next_ = 0; }
};

template <size_t Size = 1024 * 1024>
class ArenaAllocator {
    alignas(64) char buffer_[Size];
    size_t offset_ = 0;

public:
    void* allocate(size_t bytes, size_t align = alignof(std::max_align_t)) {
        size_t aligned = (offset_ + align - 1) & ~(align - 1);
        if (aligned + bytes > Size) return nullptr;
        void* ptr = buffer_ + aligned;
        offset_ = aligned + bytes;
        return ptr;
    }
    void deallocate(void*, size_t) {}
    void reset() { offset_ = 0; }
};

void demo_custom_allocators() {
    std::cout << "--- Custom Allocators ---\n";

    PoolAllocator<int, 1000> pool;
    int* p1 = pool.allocate(1);
    *p1 = 42;
    std::cout << "Pool allocated: " << *p1 << "\n";

    ArenaAllocator<> arena;
    int* arr = static_cast<int*>(arena.allocate(100 * sizeof(int), alignof(int)));
    for (int i = 0; i < 100; ++i) arr[i] = i;
    std::cout << "Arena allocated 100 ints\n";
    arena.reset();
    std::cout << "Arena reset\n\n";
}

// -----------------------------------------------------------------------------
// 5. Hot/Cold Separation + Prefetch
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

void demo_hot_cold_prefetch() {
    std::cout << "--- Hot/Cold + Prefetch ---\n";

    std::vector<Entity> entities(10000);
    for (auto& e : entities) {
        e.hot = {0,0,0, 1,0,0, 100};
        e.cold = new EntityCold{"Enemy", "Boss", 10, 1000};
    }

    // Loop HOT: solo toca hot data
    int alive = 0;
    for (const auto& e : entities) {
        // Prefetch next entity's hot data
        // __builtin_prefetch(&entities[i+1].hot, 0, 3);
        if (e.hot.health > 0) alive++;
    }
    std::cout << "Alive: " << alive << " (solo tocó hot data)\n";

    for (auto& e : entities) delete e.cold;
    std::cout << "Hot/Cold separation: mejor cache utilization\n\n";
}

// -----------------------------------------------------------------------------
// 6. SIMD + SoA combinado (caso real)
// -----------------------------------------------------------------------------
struct ParticleSoA {
    std::vector<float> x, y, z;
    std::vector<float> vx, vy, vz;
    std::vector<float> mass;
    size_t count = 0;

    void resize(size_t n) {
        x.resize(n); y.resize(n); z.resize(n);
        vx.resize(n); vy.resize(n); vz.resize(n);
        mass.resize(n);
        count = n;
    }
};

void update_particles_simd(ParticleSoA& p, float dt) {
    size_t n = p.count;

#if defined(__AVX2__) && defined(__FMA__)
    // AVX2 + FMA: procesa 8 floats por instrucción
    for (size_t i = 0; i + 7 < n; i += 8) {
        __m256 vx = _mm256_loadu_ps(p.vx.data() + i);
        __m256 vy = _mm256_loadu_ps(p.vy.data() + i);
        __m256 vz = _mm256_loadu_ps(p.vz.data() + i);

        __m256 dt_vec = _mm256_set1_ps(dt);

        __m256 x = _mm256_loadu_ps(p.x.data() + i);
        __m256 y = _mm256_loadu_ps(p.y.data() + i);
        __m256 z = _mm256_loadu_ps(p.z.data() + i);

        x = _mm256_fmadd_ps(vx, dt_vec, x);
        y = _mm256_fmadd_ps(vy, dt_vec, y);
        z = _mm256_fmadd_ps(vz, dt_vec, z);

        _mm256_storeu_ps(p.x.data() + i, x);
        _mm256_storeu_ps(p.y.data() + i, y);
        _mm256_storeu_ps(p.z.data() + i, z);
    }
#else
    // Fallback scalar implementation when AVX2/FMA not available
    for (size_t i = 0; i + 7 < n; i += 8) {
        for (size_t j = 0; j < 8; ++j) {
            p.x[i+j] += p.vx[i+j] * dt;
            p.y[i+j] += p.vy[i+j] * dt;
            p.z[i+j] += p.vz[i+j] * dt;
        }
    }
#endif
    // Remainder loop para n % 8
    for (size_t i = n & ~7; i < n; ++i) {
        p.x[i] += p.vx[i] * dt;
        p.y[i] += p.vy[i] * dt;
        p.z[i] += p.vz[i] * dt;
    }
}

void demo_simd_soa() {
    std::cout << "--- SIMD + SoA (Particle System) ---\n";

    constexpr size_t N = 1'000'000;
    ParticleSoA soa;
    soa.resize(N);

    // Init
    for (size_t i = 0; i < N; ++i) {
        soa.x[i] = float(i); soa.y[i] = float(i); soa.z[i] = float(i);
        soa.vx[i] = 1.0f; soa.vy[i] = 2.0f; soa.vz[i] = 3.0f;
        soa.mass[i] = 1.0f;
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < 10; ++iter) update_particles_simd(soa, 0.016f);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto simd_time = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    // Scalar para comparar
    struct ParticleAoS { float x,y,z, vx,vy,vz, mass; };
    std::vector<ParticleAoS> aos(N);
    for (size_t i = 0; i < N; ++i) aos[i] = {float(i),float(i),float(i), 1,2,3, 1};

    t1 = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < 10; ++iter) {
        for (auto& p : aos) {
            p.x += p.vx * 0.016f;
            p.y += p.vy * 0.016f;
            p.z += p.vz * 0.016f;
        }
    }
    t2 = std::chrono::high_resolution_clock::now();
    auto scalar_time = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    std::cout << "Scalar (AoS): " << scalar_time << " ms\n";
    std::cout << "SIMD + SoA:   " << simd_time << " ms\n";
    std::cout << "Speedup: " << float(scalar_time) / simd_time << "x\n\n";
}

// -----------------------------------------------------------------------------
// 7. Profiling para DOD
// -----------------------------------------------------------------------------
void demo_profiling_dod() {
    std::cout << "--- Profiling para DOD ---\n";
    std::cout << R"(
# perf stat:
perf stat -e cache-misses,cache-references,L1-dcache-load-misses ./exe

# Cachegrind:
valgrind --tool=cachegrind --cache-sim=yes ./exe
cg_annotate cachegrind.out.xxx

# VTune:
vtune -collect memory-access ./exe

# Métricas clave:
# - Cache miss rate = cache-misses / cache-references
# - L1/L2/L3 miss rate
# - Bandwidth utilization (achieved / peak)
# - IPC = instructions / cycles

# Métricas DOD:
# - Stride access pattern (1 = ideal)
# - Cache line utilization (bytes used / 64)
# - Prefetch effectiveness
)" << "\n\n";
}

int main() {
    std::cout << "=== LECCIÓN 11: Data-Oriented Design avanzado (parte 2) ===\n\n";

    demo_ecs_archetype();
    demo_simd();
    demo_memory_layout();
    demo_pmr();
    demo_custom_allocators();
    demo_hot_cold_prefetch();
    demo_simd_soa();
    demo_profiling_dod();

    std::cout << "=== FIN LECCIÓN 11 (DOD parte 2) ===\n";
    return 0;
}