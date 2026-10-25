/* =============================================================================
   SOLUCION 04 (L05-L06): material_srgb - PBR + sRGB
   ============================================================================ */

#include "../../include/test_framework.hpp"
#include <cmath>
#include <algorithm>

namespace {

const float PI = 3.14159265359f;

float srgbALineal(float c) {
    if (c <= 0.04045f) return c / 12.92f;
    return std::pow((c + 0.055f) / 1.055f, 2.4f);
}

float linealAsRGB(float c) {
    if (c <= 0.0031308f) return c * 12.92f;
    return 1.055f * std::pow(c, 1.0f / 2.4f) - 0.055f;
}

float F0_mix(float albedo, float metal) {
    return 0.04f * (1.0f - metal) + albedo * metal;
}

float D_GGX(float roughness, float NdotH) {
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = NdotH * NdotH * (alpha2 - 1.0f) + 1.0f;
    return alpha2 / (PI * denom * denom);
}

float G_Smith(float roughness, float NdotV, float NdotL) {
    float k = (roughness + 1.0f) * (roughness + 1.0f) / 8.0f;
    float gv = NdotV / (NdotV * (1.0f - k) + k);
    float gl = NdotL / (NdotL * (1.0f - k) + k);
    return gv * gl;
}

float BRDF_lambert(float albedo) {
    return albedo / PI;
}

} // namespace

TEST_CASE(test_F0_dielectrico) {
    ASSERT_NEAR(F0_mix(1.0f, 0.0f), 0.04f, 1e-5f);
}

TEST_CASE(test_F0_metalico) {
    ASSERT_NEAR(F0_mix(0.8f, 1.0f), 0.8f, 1e-5f);
}

TEST_CASE(test_F0_mix_medio) {
    ASSERT_NEAR(F0_mix(0.5f, 0.5f), 0.04f * 0.5f + 0.5f * 0.5f, 1e-5f);
}

TEST_CASE(test_D_GGX_cara_frente) {
    float d = D_GGX(0.5f, 1.0f);
    ASSERT_NEAR(d, 5.093f, 0.01f);
}

TEST_CASE(test_G_Smith_neutral) {
    float g = G_Smith(0.5f, 1.0f, 1.0f);
    ASSERT_NEAR(g, 1.0f, 1e-5f);
}

TEST_CASE(test_lambert_pi) {
    ASSERT_NEAR(BRDF_lambert(1.0f), 1.0f / 3.14159265f, 1e-5f);
}

int main() {
    return RUN_ALL_TESTS();
}