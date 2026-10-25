/* =============================================================================
   MÓDULO 19 - EJERCICIO 07: Cuaterniones I — eje-ángulo y rotar vectores
   =============================================================================
   DIFICULTAD: Avanzado
   APOYO: Lección 09 (de dónde sale el half-angle sin/cos, por qué el producto
   de Hamilton evita el gimbal lock y cómo aplicar q a un vector; compará con
   la matriz de rotación del ejercicio 03: MISMA rotación, NINGUNA esquina).

   CONSIGNA:

     Quat quatDesdeEjeAngulo(Vec3 ejeUnit, float rad)
        -> q = (eje * sin(rad/2), cos(rad/2))
           (¡la mitad del ángulo! el quat representa "la MITAD del giro",
            y aplicarlo con q * v * q* compone un giro COMPLETO)

     Vec3 rotarVectorQuat(Quat const& q, Vec3 v)
        -> la rotación de v por q: la fórmula directa
              t = 2 * cross(q.xyz, v)
              v' = v + q.w * t + cross(q.xyz, t)
           (sin construir matrices: mejor para "roto mucho, comparto poco")

     Quat quatAcumular(Quat const& rotActual, Quat const& rotNueva)
        -> el producto de Hamilton: rotNueva * rotActual
           (el factor de la DERECHA se aplica primero, igual que matrices;
            es el "rotar frame a frame" del joystick/cámara del módulo 17)

   TESTS (no tocar): fallan hasta las 3 funciones. casiIgualQuat (de mate3d)
   respeta la doble cobertura: q y -q son LA MISMA rotación.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_07_... -o ej07 && ./ej07
   ============================================================================= */
#include <cmath>
#include "../../include/test_framework.hpp"
#include "../include/mate3d.hpp"

using namespace mate;

// Ayudas DADA (del ejercicio 01 / lección 03):
static float productoEscalar(Vec3 a, Vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
static Vec3 productoVectorial(Vec3 a, Vec3 b) {
    return { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
}

// ===== TU CÓDIGO (mirá la LECCIÓN 09) =====
Quat quatDesdeEjeAngulo(Vec3 ejeUnit, float rad) {
    (void)ejeUnit; (void)rad;
    return quatIdentidad();      // TODO 1: eje * sin(rad/2)  y  cos(rad/2)
}

Vec3 rotarVectorQuat(Quat const& q, Vec3 v) {
    (void)q; (void)v;
    return v;                    // TODO 2: v + q.w*t + cross(q.xyz, t)
                                 //        (con t = 2 * cross(q.xyz, v))
}

Quat quatAcumular(Quat const& rotActual, Quat const& rotNueva) {
    (void)rotActual; (void)rotNueva;
    return quatIdentidad();      // TODO 3: el producto de Hamilton EN ORDEN
                                 //        (¿cuál factor va a la derecha?)
}

// ============================= TESTS (no tocar) =============================
TEST_CASE(MitadDeAnguloYZ90) {
    Quat q = quatDesdeEjeAngulo(V3Z, aRadianes(90.0f));
    ASSERT_TRUE(casiIgual(q.x, 0.0f, 1e-5f));
    ASSERT_TRUE(casiIgual(q.y, 0.0f, 1e-5f));
    ASSERT_TRUE(casiIgual(q.z, 0.70710678f, 1e-4f));   // sin(45°)
    ASSERT_TRUE(casiIgual(q.w, 0.70710678f, 1e-4f));   // cos(45°)
}

TEST_CASE(Rotar90ZLlevaXAY) {
    Quat q = quatDesdeEjeAngulo(V3Z, aRadianes(90.0f));
    Vec3 r = rotarVectorQuat(q, V3X);                  // (1,0,0) -> (0,1,0)
    ASSERT_TRUE(casiIgualVec3(r, Vec3{0.0f, 1.0f, 0.0f}, 1e-3f));
    // y con el MISMO quat, la Y va hacia -X (rotación antihoraria RH):
    Vec3 r2 = rotarVectorQuat(q, V3Y);
    ASSERT_TRUE(casiIgualVec3(r2, Vec3{-1.0f, 0.0f, 0.0f}, 1e-3f));
    // (la IDENTIDAD del modulo: rotar el eje del giro no lo mueve)
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
    // 360° sobre z: el quat es -identidad (doble cobertura) pero LA ROTACIÓN
    // es la misma: casiIgualQuat lo sabe, y el vector no se mueve:
    Quat q = quatDesdeEjeAngulo(V3Z, TAU);
    ASSERT_TRUE(casiIgualQuat(q, quatIdentidad()));
    Vec3 r = rotarVectorQuat(q, Vec3{0.3f, -1.2f, 4.5f});
    ASSERT_TRUE(casiIgualVec3(r, Vec3{0.3f, -1.2f, 4.5f}, 1e-3f));
}

TEST_CASE(AcumularRotaciones) {
    // 3 frames de rotación de 30° en z: el acumulado es UNA rotación de 90°
    // (la componen por Hamilton, NINGUNA traza de euler):
    Quat rot = quatIdentidad();
    Quat paso = quatDesdeEjeAngulo(V3Z, aRadianes(30.0f));
    rot = quatAcumular(rot, paso);
    rot = quatAcumular(rot, paso);
    rot = quatAcumular(rot, paso);
    ASSERT_TRUE(casiIgualQuat(rot, quatDesdeEjeAngulo(V3Z, aRadianes(90.0f))));
    // y el vector rotado por el acumulado confirma los 90°:
    Vec3 r = rotarVectorQuat(rot, V3X);
    ASSERT_TRUE(casiIgualVec3(r, Vec3{0.0f, 1.0f, 0.0f}, 1e-3f));
}

TEST_CASE(AcumularRespetaElOrden) {
    // dos GIROS DISTINTOS: el orden de la composición sí importa
    // (igual que con matrices). giroZ = 90° en z, giroX = 90° en x:
    Quat giroZ = quatDesdeEjeAngulo(V3Z, aRadianes(90.0f));
    Quat giroX = quatDesdeEjeAngulo(V3X, aRadianes(90.0f));
    // actual=giroZ, nueva=giroX => el acumulado es giroX*giroZ: primero Z.
    // El vector (0,0,1) no es tocado por la Z, y la X lo lleva a -Y:
    Quat acumulado = quatAcumular(giroZ, giroX);
    Vec3 r = rotarVectorQuat(acumulado, V3Z);
    ASSERT_TRUE(casiIgualVec3(r, Vec3{0.0f, -1.0f, 0.0f}, 1e-3f));
    // el orden INVERSO (actual=giroX, nueva=giroZ) da OTRO punto: la X lo
    // baja a -Y primero y la Z lo gira a +X. Dos acumulados, dos puntos:
    Quat alReves = quatAcumular(giroX, giroZ);
    Vec3 r2 = rotarVectorQuat(alReves, V3Z);
    ASSERT_TRUE(casiIgualVec3(r2, Vec3{1.0f, 0.0f, 0.0f}, 1e-3f));
    ASSERT_FALSE(casiIgualVec3(r, r2, 1e-3f));   // el orden ES la matemática
}

int main() {
    std::cout << "EJERCICIO 07: cuaterniones eje-ángulo (lección 09)\n";
    int r = RUN_ALL_TESTS();
    if (r != 0) std::cout << "Sin resolver: los tests están fallando (OK).\n";
    return r;
}
