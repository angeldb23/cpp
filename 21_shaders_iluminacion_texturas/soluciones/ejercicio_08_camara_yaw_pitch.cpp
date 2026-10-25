/* =============================================================================
   SOLUCION 08 (L15): camara_yaw_pitch
   ============================================================================ */

#include "../../include/test_framework.hpp"
#include "../../19_matematica_motores/include/mate3d.hpp"
#include <cmath>

namespace {

using mate::Vec3;
using mate::Mat4;
using mate::aRadianes;

inline float dot(Vec3 a, Vec3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline Vec3 cross(Vec3 a, Vec3 b) { return {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x}; }
inline float len(Vec3 v) { return std::sqrt(dot(v,v)); }
inline Vec3 normalizar(Vec3 v) { float l=len(v); return l>0 ? Vec3{v.x/l,v.y/l,v.z/l} : Vec3{0,0,0}; }

Vec3 forwardFromYawPitch(float yawDeg, float pitchDeg) {
    float yaw = aRadianes(yawDeg);
    float pitch = aRadianes(pitchDeg);
    float cp = std::cos(pitch);
    return {cp * std::sin(yaw), std::sin(pitch), -cp * std::cos(yaw)};
}

Vec3 rightFromYaw(float yawDeg) {
    float yaw = aRadianes(yawDeg);
    return {std::cos(yaw), 0.0f, std::sin(yaw)};
}

Mat4 lookAt(Vec3 ojo, Vec3 centro, Vec3 arriba) {
    Vec3 f = normalizar(centro - ojo);
    Vec3 s = normalizar(cross(f, arriba));
    Vec3 u = cross(s, f);
    Mat4 m;
    m.set(0, 0, s.x); m.set(1, 0, s.y); m.set(2, 0, s.z);
    m.set(0, 1, u.x); m.set(1, 1, u.y); m.set(2, 1, u.z);
    m.set(0, 2, -f.x); m.set(1, 2, -f.y); m.set(2, 2, -f.z);
    m.set(3, 0, -dot(s, ojo));
    m.set(3, 1, -dot(u, ojo));
    m.set(3, 2, dot(f, ojo));
    m.set(3, 3, 1.0f);
    return m;
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