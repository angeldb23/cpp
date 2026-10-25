/* =============================================================================
   EJERCICIO 08 (L15): camara_yaw_pitch
   Puede usar ../../19_matematica_motores/include/mate3d.hpp
   ============================================================================ */

#include "../../include/test_framework.hpp"
#include "../../19_matematica_motores/include/mate3d.hpp"
#include <cmath>

namespace {

/* --- EJERCICIO ---
   1) forwardFromYawPitch(yaw, pitch) -> Vec3
        x = cos(pitch)*sin(yaw)
        y = sin(pitch)
        z = -cos(pitch)*cos(yaw)
   2) rightFromYaw(yaw) -> Vec3: (cos(yaw), 0, sin(yaw))
   3) lookAt(ojo, centro, arriba) -> Mat4 (usar mate3d o propia)
*/

using mate::Vec3;
using mate::Mat4;
using mate::aRadianes;

Vec3 forwardFromYawPitch(float yawDeg, float pitchDeg) {
    (void)yawDeg; (void)pitchDeg;
    return {0,0,0};
}

Vec3 rightFromYaw(float yawDeg) {
    (void)yawDeg;
    return {0,0,0};
}

Mat4 lookAt(Vec3 ojo, Vec3 centro, Vec3 arriba) {
    (void)ojo; (void)centro; (void)arriba;
    return Mat4{};
}

} // namespace

TEST_CASE(test_forward_yaw0_pitch0) {
    Vec3 f = forwardFromYawPitch(0.0f, 0.0f);
    ASSERT_NEAR(f.x, 0.0f, 1e-5f);
    ASSERT_NEAR(f.y, 0.0f, 1e-5f);
    ASSERT_NEAR(f.z, -1.0f, 1e-5f);
}

TEST_CASE(test_forward_yaw90_pitch0) {
    Vec3 f = forwardFromYawPitch(90.0f, 0.0f);
    ASSERT_NEAR(f.x, 1.0f, 1e-5f);
    ASSERT_NEAR(f.y, 0.0f, 1e-5f);
    ASSERT_NEAR(f.z, 0.0f, 1e-5f);
}

TEST_CASE(test_forward_pitch90) {
    Vec3 f = forwardFromYawPitch(0.0f, 90.0f);
    ASSERT_NEAR(f.x, 0.0f, 1e-5f);
    ASSERT_NEAR(f.y, 1.0f, 1e-5f);
    ASSERT_NEAR(f.z, 0.0f, 1e-5f);
}

TEST_CASE(test_forward_unitario) {
    for (float yaw = 0; yaw < 360; yaw += 45)
        for (float pitch = -80; pitch <= 80; pitch += 20) {
            Vec3 f = forwardFromYawPitch(yaw, pitch);
            float l = std::sqrt(f.x*f.x + f.y*f.y + f.z*f.z);
            ASSERT_NEAR(l, 1.0f, 1e-4f);
        }
}

TEST_CASE(test_right_ortogonal_forward) {
    for (float yaw = 0; yaw < 360; yaw += 30) {
        Vec3 r = rightFromYaw(yaw);
        Vec3 f = forwardFromYawPitch(yaw, 0.0f);
        // right dot forward = 0 (en pitch=0)
        float d = r.x * f.x + r.y * f.y + r.z * f.z;
        ASSERT_NEAR(d, 0.0f, 1e-4f);
    }
}

TEST_CASE(test_right_unitario) {
    for (float yaw = 0; yaw < 360; yaw += 15) {
        Vec3 r = rightFromYaw(yaw);
        float l = std::sqrt(r.x*r.x + r.y*r.y + r.z*r.z);
        ASSERT_NEAR(l, 1.0f, 1e-5f);
    }
}

int main() {
    return RUN_ALL_TESTS();
}