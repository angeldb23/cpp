/* =============================================================================
   SOLUCION 01 (L01-L02): lint Phong vs Blinn-Phong + gamma/sRGB
   Lógica pura: NO abre ventana. Tests en test_framework.hpp.
   ============================================================================ */

#include "../../include/test_framework.hpp"
#include <cmath>

namespace {

/* Implementación correcta */
float phongSpecular(float NdotL, float NdotH, float shininess) {
    // Phong: spec = max(dot(R, V), 0)^shininess
    // con R = reflect(-L, N) => R·V = 2*(N·L)*(N·V) - L·V (no usado aquí)
    // En este test, NdotH ES R·V
    return std::pow(std::max(NdotH, 0.0f), shininess);
}

float blinnPhongSpecular(float NdotL, float NdotH, float shininess) {
    // Blinn-Phong: spec = max(dot(N, H), 0)^shininess
    return std::pow(std::max(NdotH, 0.0f), shininess);
}

float srgbALineal(float c) {
    // c en [0,1] sRGB -> lineal
    // si c <= 0.04045: c/12.92  else: ((c+0.055)/1.055)^2.4
    if (c <= 0.04045f) return c / 12.92f;
    return std::pow((c + 0.055f) / 1.055f, 2.4f);
}

float linealAsRGB(float c) {
    // c en [0,1] lineal -> sRGB
    // si c <= 0.0031308: c*12.92  else: 1.055*c^(1/2.4)-0.055
    if (c <= 0.0031308f) return c * 12.92f;
    return 1.055f * std::pow(c, 1.0f / 2.4f) - 0.055f;
}

/* Tabla de referencia (L02 verificada runtime):
   srgbALineal(0.5) = 0.214041
   linealAsRGB(0.5) = 0.735357
   pow(0.5, 1/2.2)  = 0.72974 (aprox sRGB simple)
*/

} // namespace

// =========== TESTS ===========

TEST_CASE(test_phong_specular_basico) {
    float sp = phongSpecular(1.0f, 0.0f, 1.0f);
    ASSERT_NEAR(sp, 0.0f, 1e-5f);
}

TEST_CASE(test_phong_specular_frente) {
    float sp = phongSpecular(1.0f, 1.0f, 32.0f);
    ASSERT_NEAR(sp, 1.0f, 1e-5f);
}

TEST_CASE(test_blinn_vs_phong_mismo_exponente) {
    // Mismos argumentos -> mismo resultado (NdotH es el coseno en ambos)
    float ph = phongSpecular(0.99f, 0.99f, 100.0f);
    float bp = blinnPhongSpecular(0.99f, 0.99f, 100.0f);
    ASSERT_NEAR(ph, bp, 0.01f);
}

TEST_CASE(test_srgbALineal_tramos) {
    ASSERT_NEAR(srgbALineal(0.04045f), 0.04045f / 12.92f, 1e-6f);
    float esperado = std::pow((0.5f + 0.055f) / 1.055f, 2.4f);
    ASSERT_NEAR(srgbALineal(0.5f), esperado, 1e-5f);
    ASSERT_NEAR(srgbALineal(1.0f), 1.0f, 1e-5f);
}

TEST_CASE(test_linealAsRGB_tramos) {
    ASSERT_NEAR(linealAsRGB(0.0031308f), 0.0031308f * 12.92f, 1e-6f);
    float esperado = 1.055f * std::pow(0.5f, 1.0f / 2.4f) - 0.055f;
    ASSERT_NEAR(linealAsRGB(0.5f), esperado, 1e-5f);
    ASSERT_NEAR(linealAsRGB(1.0f), 1.0f, 1e-5f);
}

TEST_CASE(test_idempotencia_srgb) {
    for (float x = 0.0f; x <= 1.0f; x += 0.1f) {
        float lin = srgbALineal(linealAsRGB(x));
        float srgb = linealAsRGB(srgbALineal(x));
        ASSERT_NEAR(lin, x, 1e-3f);
        ASSERT_NEAR(srgb, x, 1e-3f);
    }
}

TEST_CASE(test_valores_referencia_l02) {
    ASSERT_NEAR(srgbALineal(0.5f), 0.214041f, 1e-5f);
    ASSERT_NEAR(linealAsRGB(0.5f), 0.735357f, 1e-5f);
}

int main() {
    return RUN_ALL_TESTS();
}