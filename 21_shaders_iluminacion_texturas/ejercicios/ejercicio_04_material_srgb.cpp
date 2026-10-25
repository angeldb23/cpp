/* =============================================================================
   EJERCICIO 04 (L05-L06): material_srgb - PBR albedo/metal/roughness + sRGB
   ============================================================================ */

#include "../../include/test_framework.hpp"
#include <cmath>

namespace {

/* --- EJERCICIO ---
   1) srgbALineal(c) - igual que ejercicio 01
   2) linealAsRGB(c) - igual que ejercicio 01
   3) F0 = mix(0.04, albedo, metal)
   4) D_GGX(roughness, NdotH)
   5) G_Smith(roughness, NdotV, NdotL)
   6) Lambert: albedo / PI
*/

float srgbALineal(float c) { (void)c; return -1.0f; }
float linealAsRGB(float c) { (void)c; return -1.0f; }
float F0_mix(float albedo, float metal) { (void)albedo; (void)metal; return -1.0f; }
float D_GGX(float roughness, float NdotH) { (void)roughness; (void)NdotH; return -1.0f; }
float G_Smith(float roughness, float NdotV, float NdotL) { (void)roughness; (void)NdotV; (void)NdotL; return -1.0f; }
float BRDF_lambert(float albedo) { (void)albedo; return -1.0f; }

/* Referencias:
   F0_mix(1.0, 0.0) = 0.04
   F0_mix(1.0, 1.0) = 1.0
   D_GGX(0.5, 1.0) con roughness=0.5 -> alpha=0.25 -> 1/(pi*0.25^2*1^2) = 5.093
*/

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
    // NdotH=1, roughness=0.5 -> alpha=0.25 -> alpha2=0.0625
    // D = alpha2 / (pi * (NdotH^2 * (alpha2-1) + 1)^2) = 0.0625 / (pi * 0.0625^2) = 0.0625 / (pi * 0.00390625) = 5.0929
    float d = D_GGX(0.5f, 1.0f);
    ASSERT_NEAR(d, 5.093f, 0.01f);
}

TEST_CASE(test_G_Smith_neutral) {
    // NdotV=NdotL=1, roughness=0.5 -> k=(0.5+1)^2/8=0.28125
    // G1 = 1/(1*(1-k)+k) = 1/(0.71875+0.28125) = 1
    // G = 1*1 = 1
    float g = G_Smith(0.5f, 1.0f, 1.0f);
    ASSERT_NEAR(g, 1.0f, 1e-5f);
}

TEST_CASE(test_lambert_pi) {
    ASSERT_NEAR(BRDF_lambert(1.0f), 1.0f / 3.14159265f, 1e-5f);
}

int main() {
    return RUN_ALL_TESTS();
}