/* =============================================================================
   SOLUCIÓN 08: cuaterniones II — nlerp y slerp entre rotaciones (lección 10)
   =============================================================================
   - dotQuat: escalar 4D; |q|=1 => cos del ángulo entre quats en la esfera.
   - El detalle CRÍTICO de ambas mezclas: si dot < 0, NEGO q1 antes de
     caminar. q1 y -q1 son LA MISMA rotación pero caminando del lado LARGO
     de la esfera: sin el flip, la cámara gira "por el otro lado".
   - nlerp: lerp + renormalizar (la cuerda corta la esfera). Barato; error
     acotado; por ángulos chicos es prácticamente un slerp.
   - slerp: pesos SENOIDales sobre el arco geodésico:
        w0 = sin((1-t)θ)/sin(θ), w1 = sin(tθ)/sin(θ)
     con θ = acos(clamp(dot, -1, 1)); θ chico => llamo nlerp (sin(θ) ≈ 0
     explota el cociente).
   ============================================================================= */
#include <cmath>
#include "../../include/test_framework.hpp"
#include "../include/mate3d.hpp"

using namespace mate;

static Quat quatDesdeEjeAngulo(Vec3 ejeUnit, float rad) {
    float mitad = std::sin(rad * 0.5f);
    float cosMitad = std::cos(rad * 0.5f);
    return { ejeUnit.x * mitad, ejeUnit.y * mitad,
             ejeUnit.z * mitad, cosMitad };
}

// ===== SOLUCIÓN =====
float dotQuat(Quat a, Quat b) {
    return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}

Quat nlerpQuat(Quat const& q0, Quat const& q1, float t) {
    Quat a = q0, b = q1;
    if (dotQuat(a, b) < 0.0f) b = -b;          // camino más corto
    Quat mezcla = a + (b - a) * t;
    float l = std::sqrt(dotQuat(mezcla, mezcla));
    if (l <= 1e-8f) return quatIdentidad();    // degenerado (t=0.5, dot=-1)
    return mezcla / l;
}

Quat slerpQuat(Quat const& q0, Quat const& q1, float t) {
    Quat a = q0, b = q1;
    if (dotQuat(a, b) < 0.0f) b = -b;          // doble cobertura: camino corto
    float d = std::fmax(-1.0f, std::fmin(1.0f, dotQuat(a, b)));
    float theta = std::acos(d);
    if (theta < 1e-5f) return nlerpQuat(q0, q1, t);   // casi iguales: nlerp
    float seno = std::sin(theta);
    float w0 = std::sin((1.0f - t) * theta) / seno;
    float w1 = std::sin(t * theta) / seno;
    return a * w0 + b * w1;
}

// ============================= TESTS (del ejercicio — sin cambios) ==========
TEST_CASE(DotBasico) {
    ASSERT_TRUE(casiIgual(dotQuat(quatIdentidad(), quatIdentidad()), 1.0f, 1e-6f));
    Quat z90 = quatDesdeEjeAngulo(V3Z, aRadianes(90.0f));
    ASSERT_TRUE(casiIgual(dotQuat(quatIdentidad(), z90), 0.70710678f, 1e-4f));
    ASSERT_TRUE(casiIgual(dotQuat(z90, z90), 1.0f, 1e-6f));
}

TEST_CASE(ExtremosDelCamino) {
    Quat z60 = quatDesdeEjeAngulo(V3Z, aRadianes(60.0f));
    ASSERT_TRUE(casiIgualQuat(slerpQuat(quatIdentidad(), z60, 0.0f),
                              quatIdentidad()));
    ASSERT_TRUE(casiIgualQuat(slerpQuat(quatIdentidad(), z60, 1.0f), z60));
    ASSERT_TRUE(casiIgualQuat(nlerpQuat(quatIdentidad(), z60, 0.0f),
                              quatIdentidad()));
    ASSERT_TRUE(casiIgualQuat(nlerpQuat(quatIdentidad(), z60, 1.0f), z60));
}

TEST_CASE(MitadDeCaminoEsTreinta) {
    Quat z60  = quatDesdeEjeAngulo(V3Z, aRadianes(60.0f));
    Quat z30e = quatDesdeEjeAngulo(V3Z, aRadianes(30.0f));
    Quat s = slerpQuat(quatIdentidad(), z60, 0.5f);
    ASSERT_TRUE(casiIgualQuat(s, z30e));
    ASSERT_TRUE(casiIgualQuat(nlerpQuat(quatIdentidad(), z60, 0.5f),
                              z30e, 0.05f));
}

TEST_CASE(CaminoCortoDobleCobertura) {
    Quat z60  = quatDesdeEjeAngulo(V3Z, aRadianes(60.0f));
    Quat z60Negada = {-z60.x, -z60.y, -z60.z, -z60.w};
    Quat z30e = quatDesdeEjeAngulo(V3Z, aRadianes(30.0f));
    ASSERT_TRUE(casiIgualQuat(slerpQuat(quatIdentidad(), z60Negada, 0.5f),
                              z30e));
    ASSERT_TRUE(casiIgualQuat(nlerpQuat(quatIdentidad(), z60Negada, 0.5f),
                              z30e, 0.05f));
}

TEST_CASE(NlerpYSlerpCoincidenEnAnguloChico) {
    Quat z10 = quatDesdeEjeAngulo(V3Z, aRadianes(10.0f));
    Quat z5e = quatDesdeEjeAngulo(V3Z, aRadianes(5.0f));
    ASSERT_TRUE(casiIgualQuat(slerpQuat(quatIdentidad(), z10, 0.5f), z5e, 1e-4f));
    ASSERT_TRUE(casiIgualQuat(nlerpQuat(quatIdentidad(), z10, 0.5f), z5e, 1e-3f));
}

TEST_CASE(MitadMinusUnoMaximoQueUno) {
    Quat z360 = quatDesdeEjeAngulo(V3Z, TAU);      // doble cobertura: -ident
    Quat q = slerpQuat(quatIdentidad(), z360, 0.5f);
    ASSERT_TRUE(casiIgualQuat(q, quatIdentidad(), 1e-2f));
}

int main() {
    std::cout << "SOLUCIÓN 08: slerp / nlerp (todos los tests DEBEN pasar)\n";
    int r = RUN_ALL_TESTS();
    if (r != 0) std::cout << "ERROR: la solución debe pasar todos los tests.\n";
    return r;
}
