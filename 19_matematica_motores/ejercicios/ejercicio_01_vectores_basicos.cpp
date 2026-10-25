/* =============================================================================
   MÓDULO 19 - EJERCICIO 01: Vectores I — largo, normalizar, distancia, ángulo
   =============================================================================
   DIFICULTAD: Básico
   APOYO: Lección 03 (vectores básicos: largo, dot, normalizar, distancia).

   CONSIGNA: implementá (1 a 1) las funciones que la lección 03 enseña:

     float largoCuadrado(Vec3 a)      -> dot(a,a): sin sqrt
     float largo(Vec3 a)              -> sqrt de lo anterior
     Vec3  normalizar(Vec3 a)         -> unitario; vector CERO -> (0,0,0)
                                            (seguro: sin dividir por 0, sin NaN)
     float distancia(Vec3 p, Vec3 q)  -> largo(q - p)
     float anguloEntre(Vec3 a, Vec3 b)-> acos de dot de LOS DOS NORMALIZADOS,
                                            con clamp del cos a [-1, 1]

   TESTS (no tocar): fallan hasta que las 5 funciones estén bien.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_01_... -o ej01 && ./ej01
   ============================================================================= */
#include <cmath>
#include "../../include/test_framework.hpp"
#include "../include/mate3d.hpp"

using namespace mate;

// ===== TU CÓDIGO (mirá la LECCIÓN 03; acá vienen los TODOs) =====
float largoCuadrado(Vec3 a) {
    (void)a;
    return 0.0f;  // TODO 1: la suma de los cuadrados de las componentes
}

float largo(Vec3 a) {
    (void)a;
    return 0.0f;  // TODO 2: raíz del largoCuadrado
}

Vec3 normalizar(Vec3 a) {
    (void)a;
    return {0.0f, 0.0f, 0.0f};  // TODO 3: a / largo(a), con caso especial
                                // cuando largo <= 1e-8: devolver (0,0,0)
}

float distancia(Vec3 p, Vec3 q) {
    (void)p; (void)q;
    return 0.0f;  // TODO 4: largo del VECTOR q - p
}

float anguloEntre(Vec3 a, Vec3 b) {
    (void)a; (void)b;
    return 0.0f;  // TODO 5: clamp(dot(norm_a, norm_b), -1, 1) y acos
}

// ============================= TESTS (no tocar) =============================
TEST_CASE(LargoBasico) {
    ASSERT_NEAR(largoCuadrado(Vec3{3.0f, 4.0f, 0.0f}), 25.0f, 1e-4f);
    ASSERT_NEAR(largo(Vec3{3.0f, 4.0f, 0.0f}), 5.0f, 1e-4f);
}

TEST_CASE(NormalizarDaUnitario) {
    Vec3 n = normalizar(Vec3{0.0f, 3.0f, 0.0f});
    ASSERT_NEAR(largo(n), 1.0f, 1e-4f);          // (acá podés usar TU largo)
    ASSERT_NEAR(n.y, 1.0f, 1e-4f);
    ASSERT_NEAR(n.x, 0.0f, 1e-4f);
    ASSERT_NEAR(n.z, 0.0f, 1e-4f);
}

TEST_CASE(NormalizarDelCeroEsSeguro) {
    Vec3 n = normalizar(Vec3{0.0f, 0.0f, 0.0f});
    ASSERT_TRUE(casiIgualVec3(n, Vec3{0.0f, 0.0f, 0.0f}));   // sin NaN
}

TEST_CASE(DistanciaEuclidea) {
    ASSERT_NEAR(distancia(Vec3{0.0f, 0.0f, 0.0f}, Vec3{2.0f, 2.0f, 2.0f}),
                3.4641f, 1e-3f);
    ASSERT_NEAR(distancia(Vec3{5.0f, 0.0f, 0.0f}, Vec3{5.0f, 0.0f, 0.0f}),
                0.0f, 1e-5f);
}

TEST_CASE(Angulos90Y0) {
    ASSERT_NEAR(aGrados(anguloEntre(V3X, V3Y)), 90.0f, 1e-3f);   // perpendiculares
    ASSERT_NEAR(aGrados(anguloEntre(V3X, V3X)), 0.0f, 1e-3f);    // igual dir
    ASSERT_NEAR(aGrados(anguloEntre(V3X, -V3X)), 180.0f, 1e-3f); // opuestos
}

int main() {
    std::cout << "EJERCICIO 01: vectores básicos (lección 03)\n";
    int r = RUN_ALL_TESTS();
    if (r != 0) std::cout << "Sin resolver: los tests están fallando (OK).\n";
    return r;
}
