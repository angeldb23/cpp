/* =============================================================================
   SOLUCIÓN 01: vectores I — largo, normalizar, distancia, ángulo (lección 03)
   =============================================================================
   - largoCuadrado (sin sqrt) es la versión BARATA para comparar largos.
   - normalizar SIEMPRE con guardia del vector cero: es la diferencia entre
     un juego estable y un NaN que borra todo el frame.
   - anguloEntre: acos de los DOS vectores normalizados con clamp a [-1, 1]
     (el acos de un 1+ε de error de punto flotante da NaN: el clamp es obligatorio).
   ============================================================================= */
#include <cmath>
#include "../../include/test_framework.hpp"
#include "../include/mate3d.hpp"

using namespace mate;

// ===== SOLUCIÓN =====
// (el dot no viene en mate3d — es una lección del módulo, va local):
static float productoEscalarLibre(Vec3 a, Vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

float largoCuadrado(Vec3 a) {
    return a.x*a.x + a.y*a.y + a.z*a.z;
}

float largo(Vec3 a) {
    return std::sqrt(largoCuadrado(a));
}

Vec3 normalizar(Vec3 a) {
    float l = largo(a);
    if (l <= 1e-8f) return {0.0f, 0.0f, 0.0f};   // el cero no se divide
    return a / l;
}

float distancia(Vec3 p, Vec3 q) {
    return largo(q - p);
}

float anguloEntre(Vec3 a, Vec3 b) {
    float d = productoEscalarLibre(normalizar(a), normalizar(b));
    d = std::fmax(-1.0f, std::fmin(1.0f, d));    // acos SIN dominio inválido
    return std::acos(d);
}

// ============================= TESTS (del ejercicio — sin cambios) ==========
TEST_CASE(LargoBasico) {
    ASSERT_NEAR(largoCuadrado(Vec3{3.0f, 4.0f, 0.0f}), 25.0f, 1e-4f);
    ASSERT_NEAR(largo(Vec3{3.0f, 4.0f, 0.0f}), 5.0f, 1e-4f);
}

TEST_CASE(NormalizarDaUnitario) {
    Vec3 n = normalizar(Vec3{0.0f, 3.0f, 0.0f});
    ASSERT_NEAR(largo(n), 1.0f, 1e-4f);
    ASSERT_NEAR(n.y, 1.0f, 1e-4f);
    ASSERT_NEAR(n.x, 0.0f, 1e-4f);
    ASSERT_NEAR(n.z, 0.0f, 1e-4f);
}

TEST_CASE(NormalizarDelCeroEsSeguro) {
    Vec3 n = normalizar(Vec3{0.0f, 0.0f, 0.0f});
    ASSERT_TRUE(casiIgualVec3(n, Vec3{0.0f, 0.0f, 0.0f}));
}

TEST_CASE(DistanciaEuclidea) {
    ASSERT_NEAR(distancia(Vec3{0.0f, 0.0f, 0.0f}, Vec3{2.0f, 2.0f, 2.0f}),
                3.4641f, 1e-3f);
    ASSERT_NEAR(distancia(Vec3{5.0f, 0.0f, 0.0f}, Vec3{5.0f, 0.0f, 0.0f}),
                0.0f, 1e-5f);
}

TEST_CASE(Angulos90Y0) {
    ASSERT_NEAR(aGrados(anguloEntre(V3X, V3Y)), 90.0f, 1e-3f);
    ASSERT_NEAR(aGrados(anguloEntre(V3X, V3X)), 0.0f, 1e-3f);
    ASSERT_NEAR(aGrados(anguloEntre(V3X, -V3X)), 180.0f, 1e-3f);
}

int main() {
    std::cout << "SOLUCIÓN 01: vectores básicos (todos los tests DEBEN pasar)\n";
    int r = RUN_ALL_TESTS();
    if (r != 0) std::cout << "ERROR: la solución debe pasar todos los tests.\n";
    return r;
}
