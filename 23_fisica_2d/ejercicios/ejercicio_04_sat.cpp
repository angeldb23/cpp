/* =============================================================================
   MÓDULO 23 - EJERCICIO 04: SAT para polígonos convexos
   =============================================================================
   DIFICULTAD: Medio-alto
   APOYO: Lección 04 (SAT, proyecciones, MTV y orientación de la normal).

   CONSIGNA:
   Implementa `sat(a, b)`:
     - Probar los ejes normales de las aristas de A y de B.
     - Proyectar ambos polígonos y calcular el solape.
     - Si algún solape es <= 0, no hay colisión.
     - Devolver el MENOR solape (profundidad) y su eje (normal).
     - Orientar la normal de A hacia B: dot(normal, centroB - centroA) > 0.
   `Poly` guarda los vértices en orden (horario o antihorario).
   ============================================================================= */
#include "../include/fisica2d.hpp"
#include "../../include/test_framework.hpp"

#include <vector>

using f2d::Vec2;

struct Poly {
    std::vector<Vec2> pts;
};

struct SatResult {
    bool overlap = false;
    Vec2 normal;
    float depth = 0.0f;
};

SatResult satPolygons(const Poly& a, const Poly& b) {
    // TODO
    (void)a;
    (void)b;
    return {};
}

// ============================================================================
// TESTS (no tocar)
// ============================================================================
namespace {
Poly caja(float hx, float hy, Vec2 c = {}) {
    return {{{c.x - hx, c.y - hy}, {c.x + hx, c.y - hy},
             {c.x + hx, c.y + hy}, {c.x - hx, c.y + hy}}};
}
} // namespace

TEST_CASE(CajasSolapadas) {
    const auto r = satPolygons(caja(1, 1), caja(1, 1, {1.5f, 0.0f}));
    ASSERT_TRUE(r.overlap);
    ASSERT_NEAR(r.depth, 0.5f, 1e-4f);
    ASSERT_NEAR(r.normal.x, 1.0f, 1e-4f);
    ASSERT_NEAR(r.normal.y, 0.0f, 1e-4f);
}

TEST_CASE(CajasSeparadas) {
    const auto r = satPolygons(caja(1, 1), caja(1, 1, {4.0f, 0.0f}));
    ASSERT_FALSE(r.overlap);
}

TEST_CASE(CajasQueSeTocanNoSolapan) {
    const auto r = satPolygons(caja(1, 1), caja(1, 1, {2.0f, 0.0f}));
    ASSERT_FALSE(r.overlap);
}

TEST_CASE(Contencion) {
    const auto r = satPolygons(caja(2, 2), caja(0.5f, 0.5f));
    ASSERT_TRUE(r.overlap);
    ASSERT_NEAR(r.depth, 1.0f, 1e-4f);
}

TEST_CASE(RomboContraCaja) {
    // Rombo de "radio" 1 centrado en (1.2, 1.2) contra una caja de 2x2.
    const Poly rombo = {{{1.2f, 0.2f}, {2.2f, 1.2f}, {1.2f, 2.2f}, {0.2f, 1.2f}}};
    const auto r = satPolygons(caja(1, 1), rombo);
    ASSERT_TRUE(r.overlap);
    ASSERT_NEAR(r.depth, 0.424264f, 1e-4f);
    ASSERT_NEAR(std::fabs(r.normal.x), 0.707107f, 1e-4f);
    ASSERT_NEAR(std::fabs(r.normal.y), 0.707107f, 1e-4f);
}

TEST_CASE(NormalApuntaDeAHaciaB) {
    const auto r1 = satPolygons(caja(1, 1), caja(1, 1, {1.5f, 0.0f}));
    const auto r2 = satPolygons(caja(1, 1), caja(1, 1, {-1.5f, 0.0f}));
    ASSERT_TRUE(r1.overlap && r2.overlap);
    ASSERT_TRUE(r1.normal.x > 0.0f);
    ASSERT_TRUE(r2.normal.x < 0.0f);
}

int main() {
    std::cout << "Ejercicio 04 (SAT)...\n";
    return RUN_ALL_TESTS();
}
