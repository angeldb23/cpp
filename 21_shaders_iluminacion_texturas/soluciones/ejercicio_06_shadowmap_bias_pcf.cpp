/* =============================================================================
   SOLUCION 06 (L09-L10): shadowmap_bias_pcf
   ============================================================================ */

#include "../../include/test_framework.hpp"
#include <cmath>
#include <vector>
#include <algorithm>

namespace {

std::vector<float> depthMapSintetica(int w, int h) {
    std::vector<float> map(w * h);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            map[y * w + x] = 0.5f + 0.01f * std::sin(x * 0.5f) * std::cos(y * 0.5f);
    return map;
}

float biasSlope(float NdotL) {
    return std::max(0.05f * (1.0f - NdotL), 0.005f);
}

float sampleDepth(const std::vector<float>& map, int w, int h, float u, float v) {
    int x = int(std::clamp(u, 0.0f, 1.0f) * (w - 1));
    int y = int(std::clamp(v, 0.0f, 1.0f) * (h - 1));
    return map[y * w + x];
}

float shadowTest(const std::vector<float>& map, int w, int h, float u, float v, float ref, float bias) {
    float d = sampleDepth(map, w, h, u, v);
    return (d >= ref - bias) ? 1.0f : 0.0f;
}

float PCF3x3(const std::vector<float>& map, int w, int h, float u, float v, float ref, float bias) {
    float sum = 0.0f;
    float du = 1.0f / w, dv = 1.0f / h;
    for (int dx = -1; dx <= 1; ++dx)
        for (int dy = -1; dy <= 1; ++dy)
            sum += shadowTest(map, w, h, u + dx * du, v + dy * dv, ref, bias);
    return sum / 9.0f;
}

} // namespace

TEST_CASE(test_bias_slope_cara_frente) {
    ASSERT_NEAR(biasSlope(1.0f), 0.005f, 1e-5f);
}

TEST_CASE(test_bias_slope_cara_rasante) {
    ASSERT_NEAR(biasSlope(0.0f), 0.05f, 1e-5f);
}

TEST_CASE(test_bias_slope_45_grados) {
    ASSERT_NEAR(biasSlope(0.707f), 0.01465f, 1e-4f);
}

TEST_CASE(test_shadow_sin_oclusion) {
    std::vector<float> map = {0.5f, 0.5f, 0.5f, 0.5f};
    float s = shadowTest(map, 2, 2, 0.5f, 0.5f, 0.3f, 0.0f);
    ASSERT_NEAR(s, 1.0f, 1e-5f);
}

TEST_CASE(test_shadow_oclusion) {
    std::vector<float> map = {0.5f, 0.5f, 0.5f, 0.5f};
    float s = shadowTest(map, 2, 2, 0.5f, 0.5f, 0.7f, 0.0f);
    ASSERT_NEAR(s, 0.0f, 1e-5f);
}

TEST_CASE(test_pcf3x3_uniforme) {
    std::vector<float> map(9, 0.5f);
    float p = PCF3x3(map, 3, 3, 0.5f, 0.5f, 0.5f, 0.0f);
    ASSERT_NEAR(p, 1.0f, 1e-3f);
}

TEST_CASE(test_pcf3x3_media_sombra) {
    std::vector<float> map = {0.3f, 0.3f, 0.3f, 0.7f, 0.7f, 0.7f, 0.5f, 0.5f, 0.5f};
    float p = PCF3x3(map, 3, 3, 0.5f, 0.5f, 0.5f, 0.0f);
    ASSERT_NEAR(p, 0.666f, 0.02f);
}

int main() {
    return RUN_ALL_TESTS();
}