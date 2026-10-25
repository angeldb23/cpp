/* =============================================================================
   MÓDULO 16 - LECCIÓN 10: Cache Locality Avanzada — Prefetch, False Sharing, Alignment
   =============================================================================
   TEMA 6 de TEMAS.md (2da lección): Cache locality, prefetch, alignment.

   CACHE LINE (64 bytes en x86/ARM):
   ---------------------------------
   - Unidad mínima de transferencia entre RAM y L1/L2/L3.
   - Leer 1 byte trae 64 bytes contiguos.
   - Spatial locality: acceder datos adyacentes = gratis (ya en cache line).
   - Temporal locality: re-acceder mismo dato pronto = hit en L1.

   FALSE SHARING:
   --------------
   - Dos hilos escriben en variables DISTINTAS pero en MISMA cache line.
   - Hardware ve "escritura en line X" -> invalida line en otros cores -> ping-pong.
   - Síntoma: escalado negativo al añadir hilos (más lento que 1 hilo).

   SOLUCIÓN: alignas(64) / padding:
   --------------------------------
   struct Counter { alignas(64) std::atomic<int> value; };  // 1 por cache line
   // O manual:
   struct Counter { std::atomic<int> value; char pad[64 - sizeof(std::atomic<int>)]; };

   PREFETCH SOFTWARE (__builtin_prefetch):
   ---------------------------------------
   - Indica al HW que cargue datos ANTES de necesitarlos.
   - __builtin_prefetch(&array[i + DISTANCE], 0, 3);
     - rw=0: lectura, 1: escritura
     - locality=3: alta (mantener en L1), 0: baja (streaming)
   - Distancia óptima: ~L2 latency cycles ahead (empírico: 16-64 elementos).
   - Cuidado: prefetch innecesario contamina cache y gasta ancho de banda.

   ALIGNMENT PARA SIMD:
   --------------------
   - AVX (256-bit = 32 bytes): alignas(32)
   - AVX-512 (512-bit = 64 bytes): alignas(64)
   - std::vector no garantiza alineación > alignof(max_align_t) (16/32 bytes).
   - Para arrays SIMD: alignas(32) float arr[N] o aligned_alloc(32, size).

   HOT/COLD SPLITTING:
   -------------------
   - Separar campos accedidos CADA FRAME (hot) de campos RAROS (cold).
   - Hot: pos, vel, health (update physics).
   - Cold: name, description, level, questData (UI, save/load).
   - Struct splitting:
     struct EntityHot { Vec2 pos, vel; float health; };
     struct EntityCold { std::string name; int level; ... };
     struct Entity { EntityHot hot; EntityCold* cold; };  // Puntero a cold

   MEMORY LAYOUT OPTIMIZATION:
   ---------------------------
   // MAL: padding desperdiciado
   struct Bad { char c; double d; int i; char c2; };  // 24 bytes (11 padding)

   // BIEN: ordenado por alineación
   struct Good { double d; int i; char c; char c2; };  // 16 bytes (2 padding)

   BITFIELDS PARA FLAGS:
   ---------------------
   struct Flags { uint32_t active:1; visible:1; dirty:1; reserved:29; };  // 4 bytes

   EJEMPLO: ENTITY CON HOT/COLD + ALIGNMENT:
   -----------------------------------------
   struct alignas(64) EntityHot {
       Vec2 pos, vel;
       float health;
       uint32_t flags;  // bitfields
   };

   struct EntityCold {
       std::string name;
       std::string description;
       int level, exp;
   };

   struct Entity {
       EntityHot hot;
       EntityCold* cold = nullptr;  // nullptr = sin datos cold
   };

   // Update loop: SOLO toca hot data (cache friendly)
   void UpdateEntities(Entity* entities, size_t count, float dt) {
       for (size_t i = 0; i < count; ++i) {
           // Prefetch siguiente entity hot data
           if (i + 1 < count) __builtin_prefetch(&entities[i + 1].hot, 0, 3);
           entities[i].hot.pos.x += entities[i].hot.vel.x * dt;
           entities[i].hot.pos.y += entities[i].hot.vel.y * dt;
       }
   }

   ============================================================================= */

#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>

struct Vec2 { float x, y; };

// False sharing demo
struct CounterPlain { std::atomic<long long> value{0}; };
struct CounterPadded { alignas(64) std::atomic<long long> value{0}; };

void FalseSharingDemo() {
    std::cout << "--- False Sharing Demo ---\n";
    constexpr int THREADS = 4;
    constexpr int ITER = 1'000'000;

    CounterPlain plain[THREADS];
    std::vector<std::thread> threads;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < THREADS; ++i) {
        threads.emplace_back([&, i] {
            for (int j = 0; j < ITER; ++j) plain[i].value++;
        });
    }
    for (auto& t : threads) t.join();
    auto t_plain = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start).count();

    CounterPadded padded[THREADS];
    threads.clear();
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < THREADS; ++i) {
        threads.emplace_back([&, i] {
            for (int j = 0; j < ITER; ++j) padded[i].value++;
        });
    }
    for (auto& t : threads) t.join();
    auto t_padded = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start).count();

    std::cout << "Sin padding (false sharing): " << t_plain << " ms\n";
    std::cout << "Con alignas(64):             " << t_padded << " ms\n";
    std::cout << "Speedup: " << float(t_plain) / t_padded << "x\n\n";
}

// Hot/Cold + Prefetch demo
struct EntityHot {
    Vec2 pos, vel;
    float health;
};
struct EntityCold { std::string name; int level; };
struct Entity {
    EntityHot hot;
    EntityCold* cold = nullptr;
};

void HotColdPrefetchDemo() {
    std::cout << "--- Hot/Cold + Prefetch Demo ---\n";
    constexpr size_t N = 100'000;
    std::vector<Entity> entities(N);
    for (auto& e : entities) {
        e.hot = {{0,0}, {1,0}, 100};
        e.cold = new EntityCold{"Enemy", 1};
    }

    // Con prefetch
    auto start = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < 100; ++iter) {
        for (size_t i = 0; i < N; ++i) {
            if (i + 1 < N) __builtin_prefetch(&entities[i + 1].hot, 0, 3);
            if (entities[i].hot.health > 0) {
                entities[i].hot.pos.x += entities[i].hot.vel.x * 0.016f;
            }
        }
    }
    auto t_prefetch = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start).count();

    // Sin prefetch
    start = std::chrono::high_resolution_clock::now();
    for (int iter = 0; iter < 100; ++iter) {
        for (size_t i = 0; i < N; ++i) {
            if (entities[i].hot.health > 0) {
                entities[i].hot.pos.x += entities[i].hot.vel.x * 0.016f;
            }
        }
    }
    auto t_noprefetch = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start).count();

    std::cout << "Con __builtin_prefetch: " << t_prefetch << " ms\n";
    std::cout << "Sin prefetch:           " << t_noprefetch << " ms\n";
    std::cout << "Diferencia: " << (float(t_noprefetch) - t_prefetch) / t_noprefetch * 100 << "%\n\n";

    for (auto& e : entities) delete e.cold;
}

// Memory layout
struct BadLayout { char c; double d; int i; char c2; };
struct GoodLayout { double d; int i; char c; char c2; };

void LayoutDemo() {
    std::cout << "--- Memory Layout ---\n";
    std::cout << "BadLayout:  " << sizeof(BadLayout) << " bytes (char, double, int, char)\n";
    std::cout << "GoodLayout: " << sizeof(GoodLayout) << " bytes (double, int, char, char)\n";
    std::cout << "Ahorro: " << sizeof(BadLayout) - sizeof(GoodLayout) << " bytes por instancia\n\n";
}

int main() {
    std::cout << "=== LECCIÓN 10: Cache Locality Avanzada ===\n\n";
    FalseSharingDemo();
    HotColdPrefetchDemo();
    LayoutDemo();
    std::cout << "=== FIN LECCIÓN 10 ===\n";
    return 0;
}