/* =============================================================================
   MÓDULO 23 - EJERCICIO 05: Círculo vs polígono convexo
   =============================================================================
   DIFICULTAD: Medio
   APOYO: Lección 05 (circle-poly, punto más cercano, Minkowski y GJK).

   CONSIGNA:
   Implementa `circuloVsPoligono`:
     - Buscar el punto del polígono más cercano al centro del círculo.
     - Si el centro está AFUERA: hay contacto si distancia <= radio.
       normal = (centro - punto) normalizada; profundidad = radio - distancia.
     - Si el centro está ADENTRO: profundidad = radio + distancia a la arista;
       normal = (punto - centro) normalizada (hacia afuera).
     - El contacto es estricto: profundidad 0 (justo tocando) NO es contacto.
   ============================================================================= */
#include "../include/fisica2d.hpp"
#include "../../include/test_framework.hpp"

#include <vector>

using f2d::Vec2;

struct Poly {
    std::vector<Vec2> pts;
};

struct Circle {
    Vec2 center;
    float radius = 0.0f;
};

struct CirclePolyResult {
    bool hit = false;
    Vec2 normal;
    float depth = 0.0f;
    Vec2 point;
};

CirclePolyResult circlePoly(const Circle& c, const Poly& p) {
    // TODO
    (void)c;
    (void)p;
    return {};
}

// ============================================================================
// TESTS (no tocar)
// ============================================================================
namespace {
Poly caja(float hx, float hy) {
    return {{{-hx, -hy}, {hx, -hy}, {hx, hy}, {-hx, hy}}};
}
} // namespace

TEST_CASE(CirculoFueraQueToca) {
    const auto r = circlePoly({{1.2f, 0.0f}, 0.5f}, caja(1, 1));
    ASSERT_TRUE(r.hit);
    ASSERT_NEAR(r.depth, 0.3f, 1e-4f);
    ASSERT_NEAR(r.normal.x, 1.0f, 1e-4f);
    ASSERT_NEAR(r.normal.y, 0.0f, 1e-4f);
    ASSERT_NEAR(r.point.x, 1.0f, 1e-4f);
}

TEST_CASE(CirculoAdentro) {
    const auto r = circlePoly({{0.0f, 0.0f}, 0.5f}, caja(1, 1));
    ASSERT_TRUE(r.hit);
    ASSERT_NEAR(r.depth, 1.5f, 1e-4f);  // radio + distancia a la arista
}

TEST_CASE(CirculoLejos) {
    const auto r = circlePoly({{3.0f, 0.0f}, 0.5f}, caja(1, 1));
    ASSERT_FALSE(r.hit);
}

TEST_CASE(JustoTocandoNoEsContacto) {
    const auto r = circlePoly({{1.5f, 0.0f}, 0.5f}, caja(1, 1));
    ASSERT_FALSE(r.hit);
}

TEST_CASE(Esquina) {
    const auto r = circlePoly({{1.3f, 1.3f}, 0.5f}, caja(1, 1));
    ASSERT_TRUE(r.hit);
    ASSERT_NEAR(r.point.x, 1.0f, 1e-4f);
    ASSERT_NEAR(r.point.y, 1.0f, 1e-4f);
    ASSERT_NEAR(r.depth, 0.5f - std::sqrt(0.18f), 1e-4f);
    ASSERT_NEAR(r.normal.x, 0.707107f, 1e-4f);
    ASSERT_NEAR(r.normal.y, 0.707107f, 1e-4f);
}

TEST_CASE(EsquinaLejana) {
    const auto r = circlePoly({{1.4f, 1.4f}, 0.5f}, caja(1, 1));
    ASSERT_FALSE(r.hit);
}

int main() {
    std::cout << "Ejercicio 05 (círculo vs polígono)...\n";
    return RUN_ALL_TESTS();
}
