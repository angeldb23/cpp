/* =============================================================================
   MÓDULO 17 - SOLUCIÓN 08: Colisiones — Overlap AABB, Círculos y Closest Point
   =============================================================================
   Resuelve ejercicio_08_colisiones.cpp. La convención inclusiva (bordes que
   se tocan cuentan como colisión) es la misma de la lección 06; en círculo
   el test se hace con dist² (sin sqrt como la lección); en el closest point,
   el clamp devuelve el propio punto si está adentro.

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_08_colisiones.cpp -o sol && ./sol
   ============================================================================= */
#include <algorithm>
#include <iostream>
#include <cmath>
#include "../../include/test_framework.hpp"

// ============================ STUBS (no tocar) ===============================
struct Vector2 { float x, y; };
struct Rectangle { float x, y, width, height; };
struct Circle { Vector2 center; float radius; };

// ============================ SOLUCIÓN =======================================
// TODO 1: overlap de AABBs — es el test de separación por ejes de la lección 06
// (con ">" estricto en la separación, el borde compartido CUENTA como
// colisión: convención inclusiva)
bool RectsOverlap(Rectangle a, Rectangle b) {
    // Separación en X: si uno de los max queda ANTES del min del otro, no se tocan
    if (a.x + a.width < b.x || b.x + b.width < a.x) return false;
    // Separación en Y: lo mismo para el eje vertical
    if (a.y + a.height < b.y || b.y + b.height < a.y) return false;
    return true;
}

// TODO 2: overlap de círculos, SIN sqrt
bool CirclesOverlap(Circle a, Circle b) {
    const float dx = b.center.x - a.center.x;
    const float dy = b.center.y - a.center.y;
    const float rSum = a.radius + b.radius;
    return dx*dx + dy*dy <= rSum * rSum;   // row-level: NO sqrt
}

// TODO 3: punto del rect más cercano, por clamp de ejes
Vector2 ClosestPointOnRect(Rectangle rect, Vector2 p) {
    return { std::clamp(p.x, rect.x, rect.x + rect.width),
             std::clamp(p.y, rect.y, rect.y + rect.height) };
}

// ============================ TESTS (iguales) ================================
TEST_CASE(RectsOverlapCasoDeTactoPorBorde) {
    const Rectangle a{100, 100, 10, 10};
    const Rectangle b{110, 100, 10, 10};
    ASSERT_TRUE(RectsOverlap(a, b));
}

TEST_CASE(RectsOverlapSeparadosYSuperpuestos) {
    const Rectangle a{100, 100, 10, 10};
    const Rectangle b{120, 100, 10, 10};
    ASSERT_FALSE(RectsOverlap(a, b));
    const Rectangle c{105, 100, 10, 10};
    ASSERT_TRUE(RectsOverlap(a, c));
    const Rectangle d{100, 115, 10, 10};
    ASSERT_FALSE(RectsOverlap(a, d));
}

TEST_CASE(CirclesOverlapTocaYNoToca) {
    ASSERT_TRUE(CirclesOverlap({{0, 0}, 10}, {{16, 0}, 6}));
    ASSERT_FALSE(CirclesOverlap({{0, 0}, 10}, {{17, 0}, 6}));
}

TEST_CASE(CirclesOverlapDiagonalEsSeparados) {
    ASSERT_FALSE(CirclesOverlap({{0, 0}, 10}, {{12, 12}, 6}));
}

TEST_CASE(ClampDentroDevuelveElMismoPunto) {
    Rectangle r{100, 100, 40, 40};
    Vector2 centro{120, 120};
    Vector2 c = ClosestPointOnRect(r, centro);
    ASSERT_NEAR(c.x, 120.0f, 0.001f);
    ASSERT_NEAR(c.y, 120.0f, 0.001f);
}

TEST_CASE(ClampParaParametroFueraRango) {
    Rectangle r{100, 100, 40, 40};
    Vector2 c = ClosestPointOnRect(r, {50, 200});
    ASSERT_NEAR(c.x, 100.0f, 0.001f);
    ASSERT_NEAR(c.y, 140.0f, 0.001f);
    Vector2 c2 = ClosestPointOnRect(r, {310, 30});
    ASSERT_NEAR(c2.x, 140.0f, 0.001f);
    ASSERT_NEAR(c2.y, 100.0f, 0.001f);
}

int main() {
    std::cout << "Solución 08: Colisiones\n";
    return RUN_ALL_TESTS();
}
