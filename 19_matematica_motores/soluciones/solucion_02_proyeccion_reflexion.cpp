/* =============================================================================
   SOLUCIÓN 02: vectores II — cross, proyección y reflexión (lección 04)
   =============================================================================
   - cross: la "regla de la mano derecha" en 3 restas por componente; el
     ORDEN manda (cross(a,b) = -cross(b,a)) y el largo es el área del
     paralelogramo.
   - proyectarSobre: la versión GENERAL con dot(n, n) en el denominador
     funciona con n normalizado O NO (si n no es unitario, dot(n,n) lo
     corrige: es 1 cuando lo es, |n|² si no). Guardia del vector cero.
   - reflejar: v - 2*dot(v, n̂)*n̂ con n̂ normalizado: la componente
     NORMAL se invierte, la TANGENCIAL queda intacta.
   ============================================================================= */
#include <cmath>
#include "../../include/test_framework.hpp"
#include "../include/mate3d.hpp"

using namespace mate;

// Ayuda DADA (resuelta en el ejercicio 01):
float productoEscalar(Vec3 a, Vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
float largo(Vec3 a) { return std::sqrt(productoEscalar(a, a)); }
Vec3  normalizar(Vec3 a) {
    float l = largo(a);
    if (l <= 1e-8f) return {0.0f, 0.0f, 0.0f};
    return a / l;
}

// ===== SOLUCIÓN =====
Vec3 productoVectorial(Vec3 a, Vec3 b) {
    return { a.y*b.z - a.z*b.y,     // "yi * bz - zi * by"
             a.z*b.x - a.x*b.z,
             a.x*b.y - a.y*b.x };
}

Vec3 proyectarSobre(Vec3 v, Vec3 n) {
    float nn = productoEscalar(n, n);
    if (nn <= 1e-8f) return {0.0f, 0.0f, 0.0f};       // n cero: nada
    return n * (productoEscalar(v, n) / nn);
}

Vec3 perpendicularA(Vec3 v, Vec3 n) {
    return v - proyectarSobre(v, n);
}

Vec3 reflejar(Vec3 v, Vec3 n) {
    Vec3 nh = normalizar(n);
    return v - nh * (2.0f * productoEscalar(v, nh));
}

// ============================= TESTS (del ejercicio — sin cambios) ==========
TEST_CASE(CrossDesdeLaReglaDeLaMano) {
    ASSERT_TRUE(casiIgualVec3(productoVectorial(V3X, V3Y), V3Z));
    ASSERT_TRUE(casiIgualVec3(productoVectorial(V3Y, V3X), -V3Z));
    Vec3 c = productoVectorial(Vec3{2.0f, 0.0f, 0.0f}, Vec3{0.0f, 2.0f, 0.0f});
    ASSERT_NEAR(largo(c), 4.0f, 1e-4f);
}

TEST_CASE(ProyeccionASobrePiso) {
    Vec3 p = proyectarSobre(Vec3{1.0f, 1.0f, 0.0f}, Vec3{0.0f, 1.0f, 0.0f});
    ASSERT_TRUE(casiIgualVec3(p, Vec3{0.0f, 1.0f, 0.0f}));
    Vec3 p2 = proyectarSobre(Vec3{1.0f, 1.0f, 0.0f}, Vec3{0.0f, 2.0f, 0.0f});
    ASSERT_TRUE(casiIgualVec3(p2, Vec3{0.0f, 1.0f, 0.0f}, 1e-4f));
}

TEST_CASE(PerpendicularYDescomposicion) {
    Vec3 v{1.0f, 1.0f, 0.0f}, n{0.0f, 1.0f, 0.0f};
    Vec3 perp = perpendicularA(v, n);
    ASSERT_NEAR(perp.x, 1.0f, 1e-4f);
    ASSERT_NEAR(perp.y, 0.0f, 1e-4f);
    Vec3 suma = proyectarSobre(v, n) + perp;
    ASSERT_TRUE(casiIgualVec3(suma, v, 1e-4f));
}

TEST_CASE(ReboteConLaNormalDelPiso) {
    Vec3 r = reflejar(Vec3{1.0f, -1.0f, 0.0f}, Vec3{0.0f, 1.0f, 0.0f});
    ASSERT_NEAR(r.x, 1.0f, 1e-4f);
    ASSERT_NEAR(r.y, 1.0f, 1e-4f);
    ASSERT_NEAR(productoEscalar(r, Vec3{0.0f, 1.0f, 0.0f}), 1.0f, 1e-4f);
}

int main() {
    std::cout << "SOLUCIÓN 02: cross / proyección / reflexión (todos DEBEN pasar)\n";
    int r = RUN_ALL_TESTS();
    if (r != 0) std::cout << "ERROR: la solución debe pasar todos los tests.\n";
    return r;
}
