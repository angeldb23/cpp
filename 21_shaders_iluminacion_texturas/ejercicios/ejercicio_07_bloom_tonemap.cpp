/* =============================================================================
   EJERCICIO 07 (L11-L12): bloom_tonemap
   bright pass, kernel [1,4,6,4,1]/16, Reinhard/ACES
   ============================================================================ */

#include "../../include/test_framework.hpp"
#include <cmath>
#include <vector>

namespace {

/* --- EJERCICIO ---
   1) brightPass(c, umbral) -> float: max(luma - umbral, 0) / (1 + max(luma-umbral, 0))
      luma = dot(c, (0.2126, 0.7152, 0.0722))
   2) blur1D(src, w, horizontal) -> vector<float>: kernel [1,4,6,4,1]/16
   3) reinhard(c) -> float: c/(1+c)
   4) aces(c) -> float: clamp((c*(2.51c+0.03))/(c*(2.43c+0.59)+0.14), 0, 1)
*/

float lumaRGB(float r, float g, float b) { (void)r; (void)g; (void)b; return -1.0f; }
float brightPass(float r, float g, float b, float umbral) { (void)r; (void)g; (void)b; (void)umbral; return -1.0f; }
std::vector<float> blur1D(const std::vector<float>& src, int w, int h, bool horizontal) { (void)src; (void)w; (void)h; (void)horizontal; return std::vector<float>(w*h, 0.0f); }
float reinhard(float c) { (void)c; return -1.0f; }
float aces(float c) { (void)c; return -1.0f; }

} // namespace

TEST_CASE(test_luma_bt709) {
    // blanco -> 1.0
    ASSERT_NEAR(lumaRGB(1.0f, 1.0f, 1.0f), 1.0f, 1e-5f);
    // rojo -> 0.2126
    ASSERT_NEAR(lumaRGB(1.0f, 0.0f, 0.0f), 0.2126f, 1e-4f);
    // verde -> 0.7152
    ASSERT_NEAR(lumaRGB(0.0f, 1.0f, 0.0f), 0.7152f, 1e-4f);
}

TEST_CASE(test_bright_pass_debajo) {
    float b = brightPass(0.5f, 0.5f, 0.5f, 1.0f); // luma ~0.5 < umbral
    ASSERT_NEAR(b, 0.0f, 1e-5f);
}

TEST_CASE(test_bright_pass_encima) {
    // luma(2,2,2) = 2.0, umbral=1.0 -> diff=1.0 -> 1/(1+1)=0.5
    float b = brightPass(2.0f, 2.0f, 2.0f, 1.0f);
    ASSERT_NEAR(b, 0.5f, 1e-3f);
}

TEST_CASE(test_blur1D_centro) {
    std::vector<float> src(5, 1.0f);
    auto out = blur1D(src, 5, 1, true);
    // kernel suma = 1.0, entrada uniforme -> salida 1.0
    ASSERT_NEAR(out[2], 1.0f, 1e-5f);
}

TEST_CASE(test_reinhard_pequeno) {
    ASSERT_NEAR(reinhard(0.0f), 0.0f, 1e-5f);
    ASSERT_NEAR(reinhard(1.0f), 0.5f, 1e-5f);
    ASSERT_NEAR(reinhard(10.0f), 10.0f/11.0f, 1e-3f);
}

TEST_CASE(test_aces_clamp) {
    // ACES comprime a [0,1]
    ASSERT_NEAR(aces(0.0f), 0.0f, 1e-5f);
    ASSERT_NEAR(aces(1.0f), 1.0f, 1e-3f);
    ASSERT_NEAR(aces(100.0f), 1.0f, 1e-3f);
}

int main() {
    return RUN_ALL_TESTS();
}