/* =============================================================================
   MÓDULO 23 - EJERCICIO 03: Sweep-and-prune (1D)
   =============================================================================
   DIFICULTAD: Medio
   APOYO: Lección 03 (sweep-and-prune y quadtree).

   CONSIGNA:
   Implementa `sweepAndPrune` para intervalos en X:
     1. Ordená por minX (desempate por id).
     2. Barré con una lista activa: sacá los que terminan ANTES del minX
        actual (maxX < minX). Si maxX == minX se tocan y se quedan.
     3. Cada intervalo de la lista activa forma un par candidato.
     4. Devolvé los pares únicos (a < b) ordenados.
   ============================================================================= */
#include "../include/fisica2d.hpp"
#include "../../include/test_framework.hpp"

#include <algorithm>
#include <set>
#include <vector>

using f2d::Pair;

struct Intervalo {
    int id = 0;
    float minX = 0.0f;
    float maxX = 0.0f;
};

std::vector<Pair> sweepAndPrune(std::vector<Intervalo> intervalos) {
    // TODO
    (void)intervalos;
    return {};
}

// ============================================================================
// TESTS (no tocar)
// ============================================================================
TEST_CASE(DosQueSeSolapan) {
    const auto p = sweepAndPrune({{0, 0.0f, 2.0f}, {1, 1.0f, 3.0f}});
    ASSERT_EQ(p.size(), 1u);
    ASSERT_EQ(p[0].a, 0);
    ASSERT_EQ(p[0].b, 1);
}

TEST_CASE(DosSeparados) {
    const auto p = sweepAndPrune({{0, 0.0f, 1.0f}, {1, 2.0f, 3.0f}});
    ASSERT_EQ(p.size(), 0u);
}

TEST_CASE(TocarseCuenta) {
    const auto p = sweepAndPrune({{0, 0.0f, 1.0f}, {1, 1.0f, 2.0f}});
    ASSERT_EQ(p.size(), 1u);
}

TEST_CASE(TresSolapados) {
    const auto p = sweepAndPrune({{0, 0.0f, 3.0f}, {1, 1.0f, 4.0f}, {2, 2.0f, 5.0f}});
    ASSERT_EQ(p.size(), 3u);
    ASSERT_EQ(p[0].a, 0); ASSERT_EQ(p[0].b, 1);
    ASSERT_EQ(p[1].a, 0); ASSERT_EQ(p[1].b, 2);
    ASSERT_EQ(p[2].a, 1); ASSERT_EQ(p[2].b, 2);
}

TEST_CASE(EntradaDesordenada) {
    const auto p = sweepAndPrune({{2, 5.0f, 7.0f}, {0, 0.0f, 2.0f}, {1, 1.0f, 3.0f}});
    ASSERT_EQ(p.size(), 1u);
    ASSERT_EQ(p[0].a, 0);
    ASSERT_EQ(p[0].b, 1);
}

TEST_CASE(IntervalosQueSoloSeTocanEnCadena) {
    const auto p = sweepAndPrune({{0, 0.0f, 1.0f}, {1, 1.0f, 2.0f}, {2, 2.0f, 3.0f}});
    // (0,1) y (1,2) se tocan; (0,2) no.
    ASSERT_EQ(p.size(), 2u);
}

int main() {
    std::cout << "Ejercicio 03 (sweep-and-prune)...\n";
    return RUN_ALL_TESTS();
}
