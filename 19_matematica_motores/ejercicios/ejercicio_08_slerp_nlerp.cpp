/* =============================================================================
   MÓDULO 19 - EJERCICIO 08: Cuaterniones II — nlerp y slerp entre rotaciones
   =============================================================================
   DIFICULTAD: Avanzado
   APOYO: Lección 10 (por qué el CAMINO ENTRE dos quats no puede ser lerp
   crudo — ¡el quat vive EN LA ESFERA 4D! —, la doble cobertura y la trampa
   del "camino corto"). Estado del módulo 17: con estas dos funciones el
   gimbal lock deja de existir.

   CONSIGNA:

     float dotQuat(Quat a, Quat b)
        -> el producto escalar 4D: la "distancia" entre quats
           (|a|=|b|=1 => dot = cos del ángulo entre ellos en la esfera)

     Quat nlerpQuat(Quat const& q0, Quat const& q1, float t)
        -> 1) si dot(q0, q1) < 0: desvilá q1 por -q1 (camino MAS CORTO:
              son la misma rotación, pero caminando por el otro lado)
           2) mezcla LINEAL: mezcl = q0 + (q1 - q0) * t
           3) renormalizar (la mezcla lineal SE SALIÓ de la esfera)

     Quat slerpQuat(Quat const& q0, Quat const& q1, float t)
        -> el ARCO GEODÉSICO exacto:
           1) ídem camino corto (dot < 0 => q1 = -q1)
           2) theta = acos(clamp(dot, -1, 1))
              (si theta es CHICO, llamá nlerpQuat: slerp con el seno casi
               0 es causa directriz de NaN — y el ítem es el mismo)
           3) pesa cada lado: w0 = sin((1-t)*theta) / sin(theta)
                               w1 = sin(t*theta)     / sin(theta)
           4) devolvé q0*w0 + q1*w1

   TESTS (no tocar): la rotación Rz(60°) es el quat (0, 0, sin30°, cos30°):
   a mitad del camino el slerp DEBE dar exactamente la rotación de 30°.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_08_... -o ej08 && ./ej08
   ============================================================================= */
#include <cmath>
#include "../../include/test_framework.hpp"
#include "../include/mate3d.hpp"

using namespace mate;

// Ayuda DADA (del ejercicio 07 / lección 09):
static Quat quatDesdeEjeAngulo(Vec3 ejeUnit, float rad) {
    float mitad = std::sin(rad * 0.5f);
    float cosMitad = std::cos(rad * 0.5f);
    return { ejeUnit.x * mitad, ejeUnit.y * mitad,
             ejeUnit.z * mitad, cosMitad };
}

// ===== TU CÓDIGO (mirá la LECCIÓN 10) =====
float dotQuat(Quat a, Quat b) {
    (void)a; (void)b;
    return 0.0f;                 // TODO 1: el escalar 4D (x,y,z,w)
}

Quat nlerpQuat(Quat const& q0, Quat const& q1, float t) {
    (void)q0; (void)q1; (void)t;
    return quatIdentidad();      // TODO 2: signo + lerp + renormalizar
}

Quat slerpQuat(Quat const& q0, Quat const& q1, float t) {
    (void)q0; (void)q1; (void)t;
    return quatIdentidad();      // TODO 3: signo + acos + sinoidal
}

// ============================= TESTS (no tocar) =============================
TEST_CASE(DotBasico) {
    ASSERT_TRUE(casiIgual(dotQuat(quatIdentidad(), quatIdentidad()), 1.0f, 1e-6f));
    // quats ORTOGONALES en la esfera = rotaciones de 90° entre sí:
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
    Quat z60 = quatDesdeEjeAngulo(V3Z, aRadianes(60.0f));
    Quat z30e = quatDesdeEjeAngulo(V3Z, aRadianes(30.0f));   // el ESPERADO
    // SLERP: exacto (el arco es geodésico):
    Quat s = slerpQuat(quatIdentidad(), z60, 0.5f);
    ASSERT_TRUE(casiIgualQuat(s, z30e));
    // NLERP: cerca del exacto (¡por ángulo chico son prácticamente iguales!).
    // a 60° la desviación del nlerp es ~1.5%: tolerancia holgada:
    ASSERT_TRUE(casiIgualQuat(nlerpQuat(quatIdentidad(), z60, 0.5f),
                              z30e, 0.05f));
}

TEST_CASE(CaminoCortoDobleCobertura) {
    Quat z60 = quatDesdeEjeAngulo(V3Z, aRadianes(60.0f));
    Quat z60Negada = {-z60.x, -z60.y, -z60.z, -z60.w};   // MISMA rotación
    Quat z30e = quatDesdeEjeAngulo(V3Z, aRadianes(30.0f));
    // sin el "si dot < 0, q1 = -q1" el camino sería de 300° (¡el LARGO!)
    // y la mitad caminaría hacia -150°: un quat que NO es la rotación 30°:
   ASSERT_TRUE(casiIgualQuat(slerpQuat(quatIdentidad(), z60Negada, 0.5f),
                              z30e));
    ASSERT_TRUE(casiIgualQuat(nlerpQuat(quatIdentidad(), z60Negada, 0.5f),
                              z30e, 0.05f));
}

TEST_CASE(NlerpYSlerpCoincidenEnAnguloChico) {
    // separación de 10°: el acorta la cuerda vs arco, y el error es de
    // menos de 0.001 rad — para CÁMARAS y TWEENS usá nlerp si el ángulo
    // es chico (el slerp con acos/sin por frame es más caro):
    Quat z10 = quatDesdeEjeAngulo(V3Z, aRadianes(10.0f));
    Quat z5e = quatDesdeEjeAngulo(V3Z, aRadianes(5.0f));
    ASSERT_TRUE(casiIgualQuat(slerpQuat(quatIdentidad(), z10, 0.5f), z5e, 1e-4f));
    ASSERT_TRUE(casiIgualQuat(nlerpQuat(quatIdentidad(), z10, 0.5f), z5e, 1e-3f));
}

TEST_CASE(MitadMinusUnoMaximoQueUno) {
    // el slerp NO tiene que saltar NaN por acos de un dot con error de punto
    // flotante: clamp(-1, 1) cubre el "acos dominio":
    Quat z360 = quatDesdeEjeAngulo(V3Z, TAU);      // DOBLE COBERTURA: -ident
    Quat q = slerpQuat(quatIdentidad(), z360, 0.5f);  // dot = -1 exacto
    ASSERT_TRUE(casiIgualQuat(q, quatIdentidad(), 1e-2f));   // camino corto
}

int main() {
    std::cout << "EJERCICIO 08: slerp / nlerp entre orientaciones (lección 10)\n";
    int r = RUN_ALL_TESTS();
    if (r != 0) std::cout << "Sin resolver: los tests están fallando (OK).\n";
    return r;
}
