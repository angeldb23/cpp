/* =============================================================================
   MÓDULO 16 - EJERCICIO 06: Entidades SoA — Struct of Arrays con Swap-Remove
   =============================================================================
   DIFICULTAD: Intermedio-Avanzado
   APOYO: Lecciones 09, 10 (SoA vs AoS, cache locality, swap-with-last)

   CONSIGNA:
   Implementa un sistema de entidades SoA para partículas:
   - Arrays separados: x, y, vx, vy, life, active
   - Add(): añade partícula, retorna ID (índice)
   - Update(dt): integra posición, reduce life, elimina muertas (swap-with-last)
   - Count(): entidades vivas
   - GetPos(id): retorna posición de entidad (para render)

   SWAP-WITH-LAST:
   - Al eliminar índice i: mover último elemento a i en TODOS los arrays
   - Decrementar count
   - NO iterar i++ si se eliminó (revisar mismo índice)

   TESTS:
   - Add retorna IDs secuenciales
   - Update mueve partículas según velocidad
   - Life decrece y elimina al llegar a 0
   - Count correcto tras eliminaciones
   - GetPos retorna datos correctos

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_06_entidades_soa.cpp -o ej06 && ./ej06
   ============================================================================= */
#include <vector>
#include <cstddef>
#include "../../include/test_framework.hpp"

struct ParticleSoA {
    std::vector<float> x, y;
    std::vector<float> vx, vy;
    std::vector<float> life;
    std::vector<bool> active;
    size_t count = 0;

    // TODO 1: Ensure capacity
    // TU CÓDIGO AQUÍ:
    void EnsureCapacity(size_t n) {
        // Pista: if (n >= x.size()) { resize all vectors to n*2 or similar }
    }

    // TODO 2: Add particle
    // TU CÓDIGO AQUÍ:
    size_t Add(float px, float py, float pvx, float pvy, float plife) {
        // Pista: EnsureCapacity(count+1); set arrays[count] = values; return count++;
        return 0;  // placeholder
    }

    // TODO 3: Swap-remove helper
    // TU CÓDIGO AQUÍ:
    void RemoveAt(size_t i) {
        // Pista: size_t last = count - 1; if (i != last) swap all arrays[i] with arrays[last]; count--;
    }

    // TODO 4: Update all particles
    // TU CÓDIGO AQUÍ:
    void Update(float dt) {
        // Pista: for (size_t i = 0; i < count; ) { life[i] -= dt; if (life[i] <= 0) RemoveAt(i); else { x[i] += vx[i]*dt; y[i] += vy[i]*dt; ++i; } }
    }

    // TODO 5: Get position
    // TU CÓDIGO AQUÍ:
    std::pair<float, float> GetPos(size_t id) const {
        // Pista: if (id < count && active[id]) return {x[id], y[id]}; return {0,0};
        return {0.0f, 0.0f};
    }

    size_t Count() const { return count; }
};

// ============= TESTS =============
TEST_CASE(AddReturnsSequentialIds) {
    ParticleSoA p;
    size_t id1 = p.Add(0, 0, 1, 0, 1.0f);
    size_t id2 = p.Add(10, 10, 0, 1, 2.0f);
    ASSERT_EQ(id1, 0u);
    ASSERT_EQ(id2, 1u);
    ASSERT_EQ(p.Count(), 2u);
}

TEST_CASE(UpdateMovesParticles) {
    ParticleSoA p;
    size_t id = p.Add(0, 0, 100, 50, 10.0f);  // 100 px/s x, 50 px/s y
    p.Update(0.01f);  // 10ms
    auto [x, y] = p.GetPos(id);
    ASSERT_NEAR(x, 1.0f, 0.01f);  // 100 * 0.01
    ASSERT_NEAR(y, 0.5f, 0.01f);  // 50 * 0.01
}

TEST_CASE(LifeDecreases) {
    ParticleSoA p;
    size_t id = p.Add(0, 0, 0, 0, 0.05f);  // life 50ms
    p.Update(0.01f);  // 10ms
    // life should be ~0.04
    ASSERT_EQ(p.Count(), 1u);
    p.Update(0.05f);  // 50ms more -> dead
    ASSERT_EQ(p.Count(), 0u);
}

TEST_CASE(SwapRemoveMaintainsCount) {
    ParticleSoA p;
    size_t a = p.Add(0, 0, 0, 0, 0.1f);   // live longer
    size_t b = p.Add(10, 10, 0, 0, 0.01f); // dies fast
    size_t c = p.Add(20, 20, 0, 0, 0.1f);  // live longer
    ASSERT_EQ(p.Count(), 3u);

    p.Update(0.02f);  // b should die
    ASSERT_EQ(p.Count(), 2u);  // a and c remain

    // Verify remaining particles still accessible
    // Note: IDs may have shifted due to swap, but Count is correct
}

TEST_CASE(GetPosReturnsCorrectData) {
    ParticleSoA p;
    size_t id = p.Add(123.5f, 456.7f, 0, 0, 1.0f);
    auto [x, y] = p.GetPos(id);
    ASSERT_NEAR(x, 123.5f, 0.01f);
    ASSERT_NEAR(y, 456.7f, 0.01f);
}

TEST_CASE(UpdateDoesNotCrashOnEmpty) {
    ParticleSoA p;
    p.Update(0.016f);  // Should not crash
    ASSERT_EQ(p.Count(), 0u);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 06 (Entidades SoA)...\n";
    return RUN_ALL_TESTS();
}