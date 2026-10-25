/* =============================================================================
   MÓDULO 19 - EJERCICIO 02: Vectores II — cross, proyección y reflexión
   =============================================================================
   DIFICULTAD: Básico-Intermedio
   APOYO: Lección 04 (cross, proyección sobre una dirección, componente
   perpendicular y reflexión).

   CONSIGNA:

     Vec3 productoVectorial(Vec3 a, Vec3 b) -> el cross QUE SIRVE:
          (a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x)
          (right-handed: cross(X, Y) = +Z)

     Vec3 proyectarSobre(Vec3 v, Vec3 n)
          -> la parte de v que "va en dirección a n". SI n no viene
             normalizada, dividís por dot(n, n) (versión general).
             portes: proyectar(v, n) = n * (dot(v, n) / dot(n, n)).

     Vec3 perpendicularA(Vec3 v, Vec3 n)
          -> la parte de v que NO va en dirección a n: v - proyección.

     Vec3 reflejar(Vec3 v, Vec3 n)
          -> v - 2*dot(v, n̂) * n̂  (n̂ NORMALIZADO: el "espejo" del rayo,
             típico del rebote del juego).

   TESTS (no tocar): fallan hasta que las 4 funciones estén bien.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_02_... -o ej02 && ./ej02
   ============================================================================= */
#include <cmath>
#include "../../include/test_framework.hpp"
#include "../include/mate3d.hpp"

using namespace mate;

// Ayuda DADA (ya la resolviste en el ejercicio 01): producto escalar y largo.
float productoEscalar(Vec3 a, Vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
float largo(Vec3 a) { return std::sqrt(productoEscalar(a, a)); }
Vec3  normalizar(Vec3 a) {
    float l = largo(a);
    if (l <= 1e-8f) return {0.0f, 0.0f, 0.0f};
    return a / l;
}

// ===== TU CÓDIGO (mirá la LECCIÓN 04) =====
Vec3 productoVectorial(Vec3 a, Vec3 b) {
    (void)a; (void)b;
    return {0.0f, 0.0f, 0.0f};  // TODO 1: la fórmula del cross
}

Vec3 proyectarSobre(Vec3 v, Vec3 n) {
    (void)v; (void)n;
    return {0.0f, 0.0f, 0.0f};  // TODO 2: n * (dot(v,n) / dot(n,n))
                                //        (si dot(n,n) == 0: devolver (0,0,0))
}

Vec3 perpendicularA(Vec3 v, Vec3 n) {
    (void)v; (void)n;
    return {0.0f, 0.0f, 0.0f};  // TODO 3: v - proyectarSobre(v, n)
}

Vec3 reflejar(Vec3 v, Vec3 n) {
    (void)v; (void)n;
    return {0.0f, 0.0f, 0.0f};  // TODO 4: v - normalizar(n) * (2*dot(v, n̂))
}

// ============================= TESTS (no tocar) =============================
TEST_CASE(CrossDesdeLaReglaDeLaMano) {
    ASSERT_TRUE(casiIgualVec3(productoVectorial(V3X, V3Y), V3Z));   // (X,Y,Z)
    ASSERT_TRUE(casiIgualVec3(productoVectorial(V3Y, V3X), -V3Z)); // orden importa
    Vec3 c = productoVectorial(Vec3{2.0f, 0.0f, 0.0f}, Vec3{0.0f, 2.0f, 0.0f});
    ASSERT_NEAR(largo(c), 4.0f, 1e-4f);        // el largo ES el área del paralelogramo
}

TEST_CASE(ProyeccionASobrePiso) {
    // v=(1,1,0) sobre el piso n=(0,1,0): la componente Y sola
    Vec3 p = proyectarSobre(Vec3{1.0f, 1.0f, 0.0f}, Vec3{0.0f, 1.0f, 0.0f});
    ASSERT_TRUE(casiIgualVec3(p, Vec3{0.0f, 1.0f, 0.0f}));
    // y si n NO era unitario (0,2,0), el resultado es el mismo:
    Vec3 p2 = proyectarSobre(Vec3{1.0f, 1.0f, 0.0f}, Vec3{0.0f, 2.0f, 0.0f});
    ASSERT_TRUE(casiIgualVec3(p2, Vec3{0.0f, 1.0f, 0.0f}, 1e-4f));
}

TEST_CASE(PerpendicularYDescomposicion) {
    Vec3 v{1.0f, 1.0f, 0.0f}, n{0.0f, 1.0f, 0.0f};
    Vec3 perp = perpendicularA(v, n);
    ASSERT_NEAR(perp.x, 1.0f, 1e-4f);     // el eje X queda perpendicular
    ASSERT_NEAR(perp.y, 0.0f, 1e-4f);     // "no hay Y en elperpendiculares"
    // Y la descomposición es EXACTA (v = proyección + perpendicular):
    Vec3 suma = proyectarSobre(v, n) + perp;
    ASSERT_TRUE(casiIgualVec3(suma, v, 1e-4f));
}

TEST_CASE(ReboteConLaNormalDelPiso) {
    // la bola viene hacia abajo-derecha (1, -1, 0) y golpea el piso y=1?
    Vec3 r = reflejar(Vec3{1.0f, -1.0f, 0.0f}, Vec3{0.0f, 1.0f, 0.0f});
    ASSERT_NEAR(r.x, 1.0f, 1e-4f);   // la dirección X no cambia
    ASSERT_NEAR(r.y, 1.0f, 1e-4f);   // Y SE INVIERTA (sube)
    // y el ancho RADIO está FUERA del espejo: reflejar * n == 0
    ASSERT_NEAR(productoEscalar(r, Vec3{0.0f, 1.0f, 0.0f}), 1.0f, 1e-4f);
}

int main() {
    std::cout << "EJERCICIO 02: cross, proyección y reflexión (lección 04)\n";
    int r = RUN_ALL_TESTS();
    if (r != 0) std::cout << "Sin resolver: los tests están fallando (OK).\n";
    return r;
}
