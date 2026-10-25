/* =============================================================================
   SOLUCION 02 (L03-L04): atenuación puntual/spot + smoothstep cono
   ============================================================================ */

#include "../../include/test_framework.hpp"
#include <cmath>
#include <algorithm>

namespace {

float atenuacion(float d, float kc, float kl, float kq) {
    return 1.0f / (kc + kl * d + kq * d * d);
}

float smoothstep(float edge0, float edge1, float x) {
    float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

float spotFactor(float cosTheta, float cosInner, float cosOuter) {
    // cosInner > cosOuter (conos: inner más angosto)
    return smoothstep(cosOuter, cosInner, cosTheta);
}

float spotFactorConAtenuacion(float d, float kc, float kl, float kq,
                               float cosTheta, float cosInner, float cosOuter) {
    return atenuacion(d, kc, kl, kq) * spotFactor(cosTheta, cosInner, cosOuter);
}

/* Referencias:
   ~7m  : 1/(1+0.7*7+1.8*49) = 1/92.4 = 0.010822
   ~13m : 1/(1+0.35*13+0.44*169) = 1/80.81 = 0.012375
   ~50m : 1/(1+0.09*50+0.032*2500) = 1/91.5 = 0.010929
*/

} // namespace

TEST_CASE(test_atenuacion_distancia_cero) {
    ASSERT_NEAR(atenuacion(0.0f, 1.0f, 0.7f, 1.8f), 1.0f, 1e-5f);
}

TEST_CASE(test_atenuacion_7m) {
    float a = atenuacion(7.0f, 1.0f, 0.70f, 1.8f);
    ASSERT_NEAR(a, 0.0108f, 1e-3f);
}

TEST_CASE(test_atenuacion_13m) {
    float a = atenuacion(13.0f, 1.0f, 0.35f, 0.44f);
    ASSERT_NEAR(a, 0.0124f, 1e-3f);
}

TEST_CASE(test_atenuacion_50m) {
    float a = atenuacion(50.0f, 1.0f, 0.09f, 0.032f);
    ASSERT_NEAR(a, 0.0109f, 1e-3f);
}

TEST_CASE(test_spot_dentro_cono_interno) {
    ASSERT_NEAR(spotFactor(0.99f, 0.95f, 0.85f), 1.0f, 1e-5f);
}

TEST_CASE(test_spot_fuera_cono_externo) {
    ASSERT_NEAR(spotFactor(0.80f, 0.95f, 0.85f), 0.0f, 1e-5f);
}

TEST_CASE(test_spot_transicion) {
    float f = spotFactor(0.90f, 0.95f, 0.85f);
    ASSERT_NEAR(f, 0.5f, 1e-3f);
}

TEST_CASE(test_spot_con_atenuacion) {
    float f = spotFactorConAtenuacion(7.0f, 1.0f, 0.70f, 1.8f,
                                       0.90f, 0.95f, 0.85f);
    float a = atenuacion(7.0f, 1.0f, 0.70f, 1.8f);
    ASSERT_NEAR(f, a * 0.5f, 1e-3f);
}

int main() {
    return RUN_ALL_TESTS();
}