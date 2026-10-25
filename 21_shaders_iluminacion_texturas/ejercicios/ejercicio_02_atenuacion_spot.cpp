/* =============================================================================
   EJERCICIO 02 (L03-L04): atenuación puntual/spot + smoothstep cono
   Lógica pura: NO abre ventana.
   ============================================================================ */

#include "../../include/test_framework.hpp"
#include <cmath>

namespace {

/* --- EJERCICIO ---
   1) atenuacion(float d, float kc, float kl, float kq) -> float
        1.0 / (kc + kl*d + kq*d*d)
   2) spotFactor(float cosTheta, float cosInner, float cosOuter) -> float
        smoothstep(cosOuter, cosInner, cosTheta)
        (si cosTheta >= cosInner -> 1.0; <= cosOuter -> 0.0; entre: interp)
   3) spotFactorConAtenuacion(...) -> float: combinar ambas
*/

// STUB
float atenuacion(float d, float kc, float kl, float kq) {
    (void)d; (void)kc; (void)kl; (void)kq;
    return -1.0f;
}

float spotFactor(float cosTheta, float cosInner, float cosOuter) {
    (void)cosTheta; (void)cosInner; (void)cosOuter;
    return -1.0f;
}

float spotFactorConAtenuacion(float d, float kc, float kl, float kq,
                               float cosTheta, float cosInner, float cosOuter) {
    (void)d; (void)kc; (void)kl; (void)kq;
    (void)cosTheta; (void)cosInner; (void)cosOuter;
    return -1.0f;
}

/* Referencias atenuación (tabla clásica, distancia en metros):
   ~7m  : kc=1.0, kl=0.70, kq=1.8  => 1/(1+0.7*7+1.8*49) = 1/92.4 = 0.0108
   ~13m : kc=1.0, kl=0.35, kq=0.44 => 1/(1+0.35*13+0.44*169) = 1/80.81 = 0.0124
   ~50m : kc=1.0, kl=0.09, kq=0.032=> 1/(1+0.09*50+0.032*2500)=1/91.5 = 0.0109
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
    // cosTheta > cosInner -> 1.0
    ASSERT_NEAR(spotFactor(0.99f, 0.95f, 0.85f), 1.0f, 1e-5f);
}

TEST_CASE(test_spot_fuera_cono_externo) {
    // cosTheta < cosOuter -> 0.0
    ASSERT_NEAR(spotFactor(0.80f, 0.95f, 0.85f), 0.0f, 1e-5f);
}

TEST_CASE(test_spot_transicion) {
    // cosTheta en medio -> smoothstep
    // smoothstep(0.85, 0.95, 0.90) -> t=(0.90-0.85)/(0.95-0.85)=0.5
    // smoothstep(0.5) = 0.5*0.5*(3-2*0.5) = 0.25*2 = 0.5
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