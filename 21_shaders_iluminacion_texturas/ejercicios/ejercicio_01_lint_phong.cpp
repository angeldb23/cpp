/* =============================================================================
   EJERCICIO 01 (L01-L02): lint Phong vs Blinn-Phong + gamma/sRGB
   Lógica pura: NO abre ventana. Tests en test_framework.hpp.
   ============================================================================ */

#include "../../include/test_framework.hpp"
#include <cmath>

namespace {

/* --- EJERCICIO: completar estas funciones ---
   1) phongSpecular(N, L, V, shininess) -> float
   2) blinnPhongSpecular(N, L, V, shininess) -> float
   3) srgbALineal(c) -> float  (tramos exactos: <=0.04045 lineal/12.92, else pow((c+0.055)/1.055, 2.4))
   4) linealAsRGB(c) -> float  (<=0.0031308: c*12.92, else: 1.055*pow(c,1/2.4)-0.055)
*/

// STUB: TODOS devuelven -1.0 => los tests FALLAN (NO crash).
float phongSpecular(float NdotL, float NdotH, float shininess) {
    (void)NdotL; (void)NdotH; (void)shininess;
    return -1.0f;
}

float blinnPhongSpecular(float NdotL, float NdotH, float shininess) {
    (void)NdotL; (void)NdotH; (void)shininess;
    return -1.0f;
}

float srgbALineal(float c) {
    (void)c;
    return -1.0f;
}

float linealAsRGB(float c) {
    (void)c;
    return -1.0f;
}

/* Tabla de referencia (L02 verificada runtime):
   srgbALineal(0.5) = 0.214041
   linealAsRGB(0.5) = 0.735357
   pow(0.5, 1/2.2)  = 0.72974 (aprox sRGB simple)
*/

} // namespace

// =========== TESTS ===========

TEST_CASE(test_phong_specular_basico) {
    // N=(0,1,0), L=(0,1,0), V=(0,0,-1) => R=(0,1,0), R·V=0 => spec=0
    // con shininess=1
    float sp = phongSpecular(1.0f, 0.0f, 1.0f);
    ASSERT_NEAR(sp, 0.0f, 1e-5f);
}

TEST_CASE(test_phong_specular_frente) {
    // N=L=V=(0,0,1) => R=V, R·V=1 => spec=1^shininess=1
    float sp = phongSpecular(1.0f, 1.0f, 32.0f);
    ASSERT_NEAR(sp, 1.0f, 1e-5f);
}

TEST_CASE(test_blinn_vs_phong_mismo_exponente) {
    float ph = phongSpecular(0.99f, 0.99f, 100.0f);
    float bp = blinnPhongSpecular(0.99f, 0.99f, 100.0f);
    ASSERT_NEAR(ph, bp, 0.01f);
}

TEST_CASE(test_srgbALineal_tramos) {
    // c <= 0.04045 -> lineal: c/12.92
    ASSERT_NEAR(srgbALineal(0.04045f), 0.04045f / 12.92f, 1e-6f);
    // 0.5 -> rama gamma: ((0.5+0.055)/1.055)^2.4
    float esperado = std::pow((0.5f + 0.055f) / 1.055f, 2.4f);
    ASSERT_NEAR(srgbALineal(0.5f), esperado, 1e-5f);
    // 1.0 -> 1.0
    ASSERT_NEAR(srgbALineal(1.0f), 1.0f, 1e-5f);
}

TEST_CASE(test_linealAsRGB_tramos) {
    // c <= 0.0031308 -> lineal: c*12.92
    ASSERT_NEAR(linealAsRGB(0.0031308f), 0.0031308f * 12.92f, 1e-6f);
    // 0.5 -> 1.055*pow(0.5,1/2.4)-0.055
    float esperado = 1.055f * std::pow(0.5f, 1.0f / 2.4f) - 0.055f;
    ASSERT_NEAR(linealAsRGB(0.5f), esperado, 1e-5f);
    // 1.0 -> 1.0
    ASSERT_NEAR(linealAsRGB(1.0f), 1.0f, 1e-5f);
}

TEST_CASE(test_idempotencia_srgb) {
    // srgbALineal(linealAsRGB(x)) ≈ x  y  linealAsRGB(srgbALineal(x)) ≈ x
    for (float x = 0.0f; x <= 1.0f; x += 0.1f) {
        float lin = srgbALineal(linealAsRGB(x));
        float srgb = linealAsRGB(srgbALineal(x));
        ASSERT_NEAR(lin, x, 1e-3f);
        ASSERT_NEAR(srgb, x, 1e-3f);
    }
}

TEST_CASE(test_valores_referencia_l02) {
    // srgbALineal(0.5) = 0.214041
    ASSERT_NEAR(srgbALineal(0.5f), 0.214041f, 1e-5f);
    // linealAsRGB(0.5) = 0.735357
    ASSERT_NEAR(linealAsRGB(0.5f), 0.735357f, 1e-5f);
}

int main() {
    return RUN_ALL_TESTS();
}