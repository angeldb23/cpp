/* =============================================================================
   MÓDULO 16 - LECCIÓN 09: Entidades SoA vs AoS — Cache Locality y Data-Oriented
   =============================================================================
   TEMA 6 de TEMAS.md (1ra lección): Struct of Arrays vs Array of Structs.

   AOS (Array of Structures) — OOP TRADICIONAL:
   --------------------------------------------
   struct Entity {
       Vector2 pos, vel;
       float health, maxHealth;
       int type;          // 0=player, 1=enemy, 2=bullet...
       bool active;
       // ... más campos
   };
   std::vector<Entity> entities;

   PROBLEMAS:
   - Stride = sizeof(Entity) (ej: 48 bytes).
   - Update physics: solo lee pos, vel (16 bytes) pero trae 48 bytes a cache.
   - Cache line (64 bytes) = 1.3 entities. Baja utilización.
   - Branch misprediction en `if (e.active && e.type == ENEMY)`.

   SOA (Structure of Arrays) — DATA-ORIENTED:
   ------------------------------------------
   struct Entities {
       std::vector<Vector2> pos;
       std::vector<Vector2> vel;
       std::vector<float> health;
       std::vector<float> maxHealth;
       std::vector<int> type;
       std::vector<bool> active;
       size_t count = 0;
   };

   VENTAJAS:
   - Update physics: itera SOLO pos y vel (contiguas, 100% cache utilization).
   - Cache line (64 bytes) = 8 Vector2 = 8 entities por miss.
   - SIMD friendly: _mm256_loadu_ps en pos.x, pos.y, vel.x, vel.y.
   - Filtrado por tipo: itera type[] y salta indices (branchless con máscaras).

   ENTITY ID (Handle):
   -------------------
   // En SoA, entidad = índice en los arrays.
   using EntityID = uint32_t;  // Índice [0, count)
   // Para soportar eliminación: free list o swap-with-last + update handles.

   SWAP-WITH-LAST (Eliminación O(1) en SoA):
   -----------------------------------------
   void RemoveEntity(Entities& e, EntityID id) {
       size_t last = e.count - 1;
       if (id != last) {
           // Mover último a posición id en TODOS los arrays
           e.pos[id] = e.pos[last];
           e.vel[id] = e.vel[last];
           e.health[id] = e.health[last];
           // ...
           // Actualizar cualquier handle externo que apunte a 'last'
       }
       e.count--;
       // Opcional: pop_back en vectors para liberar memoria
   }

   ARQUETIPOS (ECS AVANZADO - ver Módulo 22):
   ------------------------------------------
   - Agrupar entidades por conjunto de componentes (Position+Velocity vs Position+Health).
   - Cada arquetipo = su propio SoA.
   - Query itera solo arquetipos relevantes.

   EJEMPLO: PARTICLE SYSTEM SoA (pseudocódigo):
   --------------------------------------------
   struct Particles {
       std::vector<float> x, y;
       std::vector<float> vx, vy;
       std::vector<float> life, maxLife;
       std::vector<uint32_t> color;  // RGBA packed
       size_t count = 0, capacity = 0;

       void Reserve(size_t n) { ... }
       size_t Add(float px, float py, float pvx, float pvy, float plife, uint32_t c) { ... }
       void Update(float dt) { ... }  // swap-with-last para partículas muertas
   };

   BENCHMARK ESPERADO (1M entities, update simple):
   ------------------------------------------------
   - AoS:  ~50-80 ms/frame (cache misses, branch mispredict)
   - SoA:  ~12-20 ms/frame (cache friendly, vectorizable)
   - SoA + SIMD: ~3-5 ms/frame

   CUÁNDO USAR CADA UNO:
   ---------------------
   - AoS: Pocas entidades (<1000), lógica compleja por entidad, código simple.
   - SoA: Muchas entidades (>1000), update homogéneo, performance crítico.
   - Híbrido: AoS para gameplay (player, bosses), SoA para partículas, balas, enemigos simples.

   ============================================================================= */

#include <iostream>
#include <vector>
#include <chrono>
#include <algorithm>

struct Vec2 { float x, y; };

// AoS versión
struct EntityAoS {
    Vec2 pos, vel;
    float health;
    int type;
    bool active;
};

// SoA versión
struct EntitiesSoA {
    std::vector<Vec2> pos, vel;
    std::vector<float> health;
    std::vector<int> type;
    std::vector<bool> active;
    size_t count = 0;

    void Reserve(size_t n) {
        pos.resize(n); vel.resize(n);
        health.resize(n); type.resize(n); active.resize(n);
    }

    size_t Add(Vec2 p, Vec2 v, float h, int t) {
        size_t id = count++;
        if (id >= pos.size()) Reserve(pos.size() * 2);
        pos[id] = p; vel[id] = v; health[id] = h; type[id] = t; active[id] = true;
        return id;
    }

    void UpdatePhysics(float dt) {
        for (size_t i = 0; i < count; ++i) {
            if (active[i]) {
                pos[i].x += vel[i].x * dt;
                pos[i].y += vel[i].y * dt;
            }
        }
    }

    void UpdatePhysicsEnemyOnly(float dt) {
        for (size_t i = 0; i < count; ++i) {
            if (active[i] && type[i] == 1) {  // 1 = enemy
                pos[i].x += vel[i].x * dt;
                pos[i].y += vel[i].y * dt;
            }
        }
    }
};

void BenchmarkAoS(std::vector<EntityAoS>& entities, float dt, int iterations) {
    auto start = std::chrono::high_resolution_clock::now();
    for (int it = 0; it < iterations; ++it) {
        for (auto& e : entities) {
            if (e.active) {
                e.pos.x += e.vel.x * dt;
                e.pos.y += e.vel.y * dt;
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "AoS: " << ms << " ms (" << iterations << " iteraciones)\n";
}

void BenchmarkSoA(EntitiesSoA& entities, float dt, int iterations) {
    auto start = std::chrono::high_resolution_clock::now();
    for (int it = 0; it < iterations; ++it) {
        entities.UpdatePhysics(dt);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "SoA: " << ms << " ms (" << iterations << " iteraciones)\n";
}

int main() {
    std::cout << "=== LECCIÓN 09: SoA vs AoS - Cache Locality ===\n";

    const size_t N = 100'000;
    const int ITER = 100;
    const float DT = 0.016f;

    // AoS setup
    std::vector<EntityAoS> aos(N);
    for (size_t i = 0; i < N; ++i) {
        aos[i] = {{float(i), 0}, {1.0f, 0.5f}, 100.0f, int(i % 3), true};
    }

    // SoA setup
    EntitiesSoA soa;
    soa.Reserve(N);
    for (size_t i = 0; i < N; ++i) {
        soa.Add({float(i), 0}, {1.0f, 0.5f}, 100.0f, int(i % 3));
    }
    soa.count = N;

    std::cout << "\nBenchmark: " << N << " entidades, " << ITER << " iteraciones\n";
    BenchmarkAoS(aos, DT, ITER);
    BenchmarkSoA(soa, DT, ITER);

    std::cout << "\nSoA itera solo pos/vel (contiguas). AoS trae toda la struct.\n";
    std::cout << "Cache line 64B: SoA = 8 Vec2/line, AoS = 1.3 Entity/line.\n";
    std::cout << "=== FIN LECCIÓN 09 ===\n";
    return 0;
}