/* =============================================================================
   EJERCICIO 05 (L07-L08): TBN Gram-Schmidt + handedness + unpack 2x-1
   ============================================================================ */

#include "../../include/test_framework.hpp"
#include <cmath>

namespace {

/* --- EJERCICIO ---
   1) tangentFromNormal(Vec3 N) -> Vec3 T (Gram-Schmidt: T = normalize(any - N*dot))
   2) bitangent(N, T) -> Vec3 B = cross(N, T)
   3) handedness(N, T, B) -> float (+1 o -1): sign(dot(cross(N,T), B))
   4) unpackNormalMap(Vec3 rgb) -> Vec3: rgb*2-1
   5) perturbNormal(N, T, B, nm) -> Vec3: N + nm.x*T + nm.y*B + nm.z*N (simplificado)
*/

struct Vec3 { float x,y,z; };

Vec3 tangentFromNormal(Vec3 N) { (void)N; return {0,0,0}; }
Vec3 bitangent(Vec3 N, Vec3 T) { (void)N; (void)T; return {0,0,0}; }
float handedness(Vec3 N, Vec3 T, Vec3 B) { (void)N; (void)T; (void)B; return 0.0f; }
Vec3 unpackNormalMap(Vec3 rgb) { (void)rgb; return {0,0,0}; }
Vec3 perturbNormal(Vec3 N, Vec3 T, Vec3 B, Vec3 nm) { (void)N; (void)T; (void)B; (void)nm; return {0,0,0}; }

inline float dot(Vec3 a, Vec3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
inline Vec3 cross(Vec3 a, Vec3 b) { return {a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x}; }
inline float len(Vec3 v) { return std::sqrt(dot(v,v)); }
inline Vec3 norm(Vec3 v) { float l=len(v); return l>0 ? Vec3{v.x/l,v.y/l,v.z/l} : Vec3{0,0,0}; }

/* Test data:
   N=(0,1,0), T=(1,0,0) => B=(0,0,1), handedness=+1
   nm=(0.5,0.5,1.0) => unpack=(0,0,1) => perturb=N
*/

} // namespace

TEST_CASE(test_tangent_ortogonal) {
    Vec3 N{0,1,0};
    Vec3 T = tangentFromNormal(N);
    float d = dot(N, T);
    ASSERT_NEAR(d, 0.0f, 1e-5f);
}

TEST_CASE(test_tangent_unitario) {
    Vec3 N{0,1,0};
    Vec3 T = tangentFromNormal(N);
    ASSERT_NEAR(len(T), 1.0f, 1e-5f);
}

TEST_CASE(test_bitangent_ortogonal) {
    Vec3 N{0,1,0}, T{1,0,0};
    Vec3 B = bitangent(N, T);
    ASSERT_NEAR(dot(N,B), 0.0f, 1e-5f);
    ASSERT_NEAR(dot(T,B), 0.0f, 1e-5f);
}

TEST_CASE(test_handedness_derecha) {
    Vec3 N{0,1,0}, T{1,0,0}, B{0,0,1};
    ASSERT_NEAR(handedness(N,T,B), 1.0f, 1e-5f);
}

TEST_CASE(test_handedness_izquierda) {
    Vec3 N{0,1,0}, T{1,0,0}, B{0,0,-1};
    ASSERT_NEAR(handedness(N,T,B), -1.0f, 1e-5f);
}

TEST_CASE(test_unpack_neutral) {
    Vec3 nm = unpackNormalMap({0.5f, 0.5f, 1.0f});
    ASSERT_NEAR(nm.x, 0.0f, 1e-5f);
    ASSERT_NEAR(nm.y, 0.0f, 1e-5f);
    ASSERT_NEAR(nm.z, 1.0f, 1e-5f);
}

TEST_CASE(test_unpack_rojo) {
    Vec3 nm = unpackNormalMap({1.0f, 0.5f, 0.5f});
    ASSERT_NEAR(nm.x, 1.0f, 1e-5f);
    ASSERT_NEAR(nm.y, 0.0f, 1e-5f);
    ASSERT_NEAR(nm.z, 0.0f, 1e-5f);
}

TEST_CASE(test_perturb_neutral) {
    Vec3 N{0,1,0}, T{1,0,0}, B{0,0,1};
    Vec3 nm{0,0,1};
    Vec3 n = perturbNormal(N,T,B,nm);
    ASSERT_NEAR(n.x, 0.0f, 1e-5f);
    ASSERT_NEAR(n.y, 1.0f, 1e-5f);
    ASSERT_NEAR(n.z, 0.0f, 1e-5f);
}

int main() {
    return RUN_ALL_TESTS();
}