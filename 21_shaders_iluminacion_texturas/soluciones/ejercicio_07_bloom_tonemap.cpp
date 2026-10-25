/* =============================================================================
   SOLUCION 07 (L11-L12): bloom_tonemap
   ============================================================================ */

#include "../../include/test_framework.hpp"
#include <cmath>
#include <vector>
#include <algorithm>

namespace {

float lumaRGB(float r, float g, float b) {
    return r * 0.2126f + g * 0.7152f + b * 0.0722f;
}

float brightPass(float r, float g, float b, float umbral) {
    float l = lumaRGB(r, g, b);
    float diff = std::max(l - umbral, 0.0f);
    return diff / (1.0f + diff);
}

std::vector<float> blur1D(const std::vector<float>& src, int w, int h, bool horizontal) {
    std::vector<float> dst(w * h);
    const float k[5] = {1.0f/16.0f, 4.0f/16.0f, 6.0f/16.0f, 4.0f/16.0f, 1.0f/16.0f};
    if (horizontal) {
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                float sum = 0.0f;
                for (int i = 0; i < 5; ++i) {
                    int xi = std::clamp(x + i - 2, 0, w - 1);
                    sum += src[y * w + xi] * k[i];
                }
                dst[y * w + x] = sum;
            }
        }
    } else {
        for (int x = 0; x < w; ++x) {
            for (int y = 0; y < h; ++y) {
                float sum = 0.0f;
                for (int i = 0; i < 5; ++i) {
                    int yi = std::clamp(y + i - 2, 0, h - 1);
                    sum += src[yi * w + x] * k[i];
                }
                dst[y * w + x] = sum;
            }
        }
    }
    return dst;
}

float reinhard(float c) {
    return c / (1.0f + c);
}

float aces(float c) {
    float num = c * (2.51f * c + 0.03f);
    float den = c * (2.43f * c + 0.59f) + 0.14f;
    return std::clamp(num / den, 0.0f, 1.0f);
}

} // namespace

TEST_CASE(test_luma_bt709) {
    ASSERT_NEAR(lumaRGB(1.0f, 1.0f, 1.0f), 1.0f, 1e-5f);
    ASSERT_NEAR(lumaRGB(1.0f, 0.0f, 0.0f), 0.2126f, 1e-4f);
    ASSERT_NEAR(lumaRGB(0.0f, 1.0f, 0.0f), 0.7152f, 1e-4f);
}

TEST_CASE(test_bright_pass_debajo) {
    float b = brightPass(0.5f, 0.5f, 0.5f, 1.0f);
    ASSERT_NEAR(b, 0.0f, 1e-5f);
}

TEST_CASE(test_bright_pass_encima) {
    float b = brightPass(2.0f, 2.0f, 2.0f, 1.0f);
    ASSERT_NEAR(b, 0.5f, 1e-3f);
}

TEST_CASE(test_blur1D_centro) {
    std::vector<float> src(5, 1.0f);
    auto out = blur1D(src, 5, 1, true);
    ASSERT_NEAR(out[2], 1.0f, 1e-5f);
}

TEST_CASE(test_reinhard_pequeno) {
    ASSERT_NEAR(reinhard(0.0f), 0.0f, 1e-5f);
    ASSERT_NEAR(reinhard(1.0f), 0.5f, 1e-5f);
    ASSERT_NEAR(reinhard(10.0f), 10.0f/11.0f, 1e-3f);
}

TEST_CASE(test_aces_clamp) {
    ASSERT_NEAR(aces(0.0f), 0.0f, 1e-5f);
    // aces(1.0) = (1*(2.51+0.03))/(1*(2.43+0.59)+0.14) = 2.54/3.16 = 0.8038
    ASSERT_NEAR(aces(1.0f), 0.804f, 1e-3f);
    ASSERT_NEAR(aces(100.0f), 1.0f, 1e-3f);
}

int main() {
    return RUN_ALL_TESTS();
}