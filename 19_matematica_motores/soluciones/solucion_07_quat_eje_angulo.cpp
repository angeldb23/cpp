/* =============================================================================
   SOLUCIÓN 07: cuaterniones I — eje-ángulo, rotar vector y acumular (L09)
   =============================================================================
   - quatDesdeEjeAngulo: (eje*sin(θ/2), cos(θ/2)) — la MITAD del ángulo,
     porque el operador q v q* aplica el giro dos veces (ida y vuelta).
   - rotarVectorQuat: fórmula directa con un solo cross "extra":
        t = 2 * cross(q.xyz, v)
        v' = v + q.w * t + cross(q.xyz, t)
     (equivale a v' = q * v * q* — el conjugado es la vuelta).
   - quatAcumular: el producto de Hamilton rotNueva * rotActual: el factor
     de la DERECHA se aplica primero (posición del mouse: giro nuevo sobre
     el estado ya acumulado).
   ============================================================================= */
#include <cmath>
#include "../../include/test_framework.hpp"
#include "../include/mate3d.hpp"

using namespace mate;

static Vec3 productoVectorial(Vec3 a, Vec3 b) {
    return { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
}

// ===== SOLUCIÓN =====
Quat quatDesdeEjeAngulo(Vec3 ejeUnit, float rad) {
    float mitad = std::sin(rad * 0.5f);
    float cosMitad = std::cos(rad * 0.5f);
    return { ejeUnit.x * mitad, ejeUnit.y * mitad,
             ejeUnit.z * mitad, cosMitad };
}

Vec3 rotarVectorQuat(Quat const& q, Vec3 v) {
    Vec3 eje = {q.x, q.y, q.z};
    Vec3 t   = productoVectorial(eje, v) * 2.0f;
    // v' = v + w*t + cross(eje, t)   ( == q * v * q* )
    return v + productoVectorial(eje, t)
         + t * q.w;
}

Quat quatAcumular(Quat const& rotActual, Quat const& rotNueva) {
    return rotNueva * rotActual;
}

// ============================= TESTS (del ejercicio — sin cambios) ==========
TEST_CASE(MitadDeAnguloYZ90) {
    Quat q = quatDesdeEjeAngulo(V3Z, aRadianes(90.0f));
    ASSERT_TRUE(casiIgual(q.x, 0.0f, 1e-5f));
    ASSERT_TRUE(casiIgual(q.y, 0.0f, 1e-5f));
    ASSERT_TRUE(casiIgual(q.z, 0.70710678f, 1e-4f));
    ASSERT_TRUE(casiIgual(q.w, 0.70710678f, 1e-4f));
}

TEST_CASE(Rotar90ZLlevaXAY) {
    Quat q = quatDesdeEjeAngulo(V3Z, aRadianes(90.0f));
    Vec3 r = rotarVectorQuat(q, V3X);
    ASSERT_TRUE(casiIgualVec3(r, Vec3{0.0f, 1.0f, 0.0f}, 1e-3f));
    Vec3 r2 = rotarVectorQuat(q, V3Y);
    ASSERT_TRUE(casiIgualVec3(r2, Vec3{-1.0f, 0.0f, 0.0f}, 1e-3f));
    Vec3 r3 = rotarVectorQuat(q, V3Z);
    ASSERT_TRUE(casiIgualVec3(r3, V3Z, 1e-3f));
}

TEST_CASE(AnguloCeroNoMueve) {
    Quat q = quatDesdeEjeAngulo(V3X, 0.0f);
    ASSERT_TRUE(casiIgualQuat(q, quatIdentidad()));
    Vec3 r = rotarVectorQuat(q, Vec3{1.0f, 2.0f, 3.0f});
    ASSERT_TRUE(casiIgualVec3(r, Vec3{1.0f, 2.0f, 3.0f}, 1e-4f));
}

TEST_CASE(VueltaCompletaEsLaIdentidad) {
    Quat q = quatDesdeEjeAngulo(V3Z, TAU);
    ASSERT_TRUE(casiIgualQuat(q, quatIdentidad()));
    Vec3 r = rotarVectorQuat(q, Vec3{0.3f, -1.2f, 4.5f});
    ASSERT_TRUE(casiIgualVec3(r, Vec3{0.3f, -1.2f, 4.5f}, 1e-3f));
}

TEST_CASE(AcumularRotaciones) {
    Quat rot = quatIdentidad();
    Quat paso = quatDesdeEjeAngulo(V3Z, aRadianes(30.0f));
    rot = quatAcumular(rot, paso);
    rot = quatAcumular(rot, paso);
    rot = quatAcumular(rot, paso);
    ASSERT_TRUE(casiIgualQuat(rot, quatDesdeEjeAngulo(V3Z, aRadianes(90.0f))));
    Vec3 r = rotarVectorQuat(rot, V3X);
    ASSERT_TRUE(casiIgualVec3(r, Vec3{0.0f, 1.0f, 0.0f}, 1e-3f));
}

TEST_CASE(AcumularRespetaElOrden) {
    Quat giroZ = quatDesdeEjeAngulo(V3Z, aRadianes(90.0f));
    Quat giroX = quatDesdeEjeAngulo(V3X, aRadianes(90.0f));
    Quat acumulado = quatAcumular(giroZ, giroX);   // giroX * giroZ: primero Z
    Vec3 r = rotarVectorQuat(acumulado, V3Z);
    ASSERT_TRUE(casiIgualVec3(r, Vec3{0.0f, -1.0f, 0.0f}, 1e-3f));
    Quat alReves = quatAcumular(giroX, giroZ);     // giroZ * giroX: primero X
    Vec3 r2 = rotarVectorQuat(alReves, V3Z);
    ASSERT_TRUE(casiIgualVec3(r2, Vec3{1.0f, 0.0f, 0.0f}, 1e-3f));
    ASSERT_FALSE(casiIgualVec3(r, r2, 1e-3f));
}

int main() {
    std::cout << "SOLUCIÓN 07: cuaterniones eje-ángulo (todos DEBEN pasar)\n";
    int r = RUN_ALL_TESTS();
    if (r != 0) std::cout << "ERROR: la solución debe pasar todos los tests.\n";
    return r;
}
