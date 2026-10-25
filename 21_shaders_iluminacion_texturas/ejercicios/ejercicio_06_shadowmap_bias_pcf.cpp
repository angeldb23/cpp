/* =============================================================================
   EJERCICIO 06 (L09-L10): shadowmap_bias_pcf
   depth map sintético grid + PCF 3x3
   ============================================================================ */

#include "../../include/test_framework.hpp"
#include <cmath>
#include <vector>

namespace {

/* --- EJERCICIO ---
   1) depthMapSintetica(w,h) -> vector<float>: plano en z=0.5 con ruido
   2) biasSlope(float NdotL) -> float: max(0.05*(1-NdotL), 0.005)
   3) shadowTest(depthMap, uv, ref, bias) -> float: 1 o 0
   4) PCF3x3(depthMap, w, h, uv, ref, bias) -> float: promedio 9 samples
*/

std::vector<float> depthMapSintetica(int w, int h) { (void)w; (void)h; return {}; }
float biasSlope(float NdotL) { (void)NdotL; return -1.0f; }
float shadowTest(const std::vector<float>& map, int w, int h, float u, float v, float ref, float bias) { (void)map; (void)w; (void)h; (void)u; (void)v; (void)ref; (void)bias; return -1.0f; }
float PCF3x3(const std::vector<float>& map, int w, int h, float u, float v, float ref, float bias) { (void)map; (void)w; (void)h; (void)u; (void)v; (void)ref; (void)bias; return -1.0f; }

} // namespace

TEST_CASE(test_bias_slope_cara_frente) {
    // NdotL=1 -> bias = max(0, 0.005) = 0.005
    ASSERT_NEAR(biasSlope(1.0f), 0.005f, 1e-5f);
}

TEST_CASE(test_bias_slope_cara_rasante) {
    // NdotL=0 -> bias = max(0.05, 0.005) = 0.05
    ASSERT_NEAR(biasSlope(0.0f), 0.05f, 1e-5f);
}

TEST_CASE(test_bias_slope_45_grados) {
    // NdotL=0.707 -> bias = max(0.05*0.293, 0.005) = 0.01465
    ASSERT_NEAR(biasSlope(0.707f), 0.01465f, 1e-4f);
}

TEST_CASE(test_shadow_sin_oclusion) {
    std::vector<float> map = {0.5f, 0.5f, 0.5f, 0.5f};
    // ref=0.3 < depth=0.5 -> sin sombra (1.0)
    float s = shadowTest(map, 2, 2, 0.5f, 0.5f, 0.3f, 0.0f);
    ASSERT_NEAR(s, 1.0f, 1e-5f);
}

TEST_CASE(test_shadow_oclusion) {
    std::vector<float> map = {0.5f, 0.5f, 0.5f, 0.5f};
    // ref=0.7 > depth=0.5 -> en sombra (0.0)
    float s = shadowTest(map, 2, 2, 0.5f, 0.5f, 0.7f, 0.0f);
    ASSERT_NEAR(s, 0.0f, 1e-5f);
}

TEST_CASE(test_pcf3x3_uniforme) {
    std::vector<float> map(9, 0.5f); // 3x3 todo 0.5
    // ref=0.5, bias=0 -> promedio de 9 samples = 1.0
    float p = PCF3x3(map, 3, 3, 0.5f, 0.5f, 0.5f, 0.0f);
    ASSERT_NEAR(p, 1.0f, 1e-3f);
}

TEST_CASE(test_pcf3x3_media_sombra) {
    std::vector<float> map = {0.3f, 0.3f, 0.3f, 0.7f, 0.7f, 0.7f, 0.5f, 0.5f, 0.5f};
    // ref=0.5: 3 samples < 0.5 (1.0), 3 samples > 0.5 (0.0), 3 samples = 0.5 (1.0 con LEQUAL)
    // promedio = 6/9 = 0.666...
    float p = PCF3x3(map, 3, 3, 0.5f, 0.5f, 0.5f, 0.0f);
    ASSERT_NEAR(p, 0.666f, 0.02f);
}

int main() {
    return RUN_ALL_TESTS();
}