/* =============================================================================
   MÓDULO 17 - EJERCICIO 08: Colisiones — Overlap AABB, Círculos y Closest Point
   =============================================================================
   DIFICULTAD: Intermedio
   APOYO: Lección 06 (Colisiones 2D: AABB, círculo, clamp)

   CONSIGNA:
   1) RectsOverlap(a, b): true si los AABBs se superponen (BORDES QUE TOCAN
     CUENTAN como colisión, es la convención inclusiva de la lección 06).
   2) CirclesOverlap(a, b): true si la distancia² <= (r1+r2)². SIN sqrt.
   3) ClosestPointOnRect(rect, point): el punto del rect más cercano (clamp
      por eje). Si point está dentro del rect, devuelve el MISMO point.

   TESTS (no tocar): casos de límites (toca por borde), separados, círculos
   exactos y el clamp en los 3 casos (fuera por x, por y, y adentro).

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_08_colisiones.cpp -o ej && ./ej
   ============================================================================= */
#include <algorithm>
#include <iostream>
#include <cmath>
#include "../../include/test_framework.hpp"

// ============================ STUBS (no tocar) ===============================
struct Vector2 { float x, y; };
struct Rectangle { float x, y, width, height; };
struct Circle { Vector2 center; float radius; };

// ============================ TU CÓDIGO AQUÍ ================================
// TODO 1: overlap de AABBs (bordes tocando = true)
bool RectsOverlap(Rectangle a, Rectangle b) {
    // Pista: comparar max/min por eje con early exit
    return false;   // placeholder
}

// TODO 2: overlap de círculos (dist² contra (r1+r2)², SIN sqrt)
bool CirclesOverlap(Circle a, Circle b) {
    return false;   // placeholder
}

// TODO 3: punto del rect más cercano (clamp por eje)
Vector2 ClosestPointOnRect(Rectangle rect, Vector2 p) {
    return p;   // placeholder (devuelve el mismo punto)
}

// ============================ TESTS (no tocar) ==============================
TEST_CASE(RectsOverlapCasoDeTactoPorBorde) {
    // A termina en x=110; B empieza en x=110: borde compartido (toca)
    const Rectangle a{100, 100, 10, 10};
    const Rectangle b{110, 100, 10, 10};
    ASSERT_TRUE(RectsOverlap(a, b));
}

TEST_CASE(RectsOverlapSeparadosYSuperpuestos) {
    const Rectangle a{100, 100, 10, 10};
    const Rectangle b{120, 100, 10, 10};   // hueco de 10 px
    ASSERT_FALSE(RectsOverlap(a, b));
    const Rectangle c{105, 100, 10, 10};   // solapa
    ASSERT_TRUE(RectsOverlap(a, c));
    // solo se tocan en Y, no en X: separados
    const Rectangle d{100, 115, 10, 10};
    ASSERT_FALSE(RectsOverlap(a, d));
}

TEST_CASE(CirclesOverlapTocaYNoToca) {
    // r1+r2 = 16; dist 16.0 => toca (<=)
    ASSERT_TRUE(CirclesOverlap({{0, 0}, 10}, {{16, 0}, 6}));
    ASSERT_FALSE(CirclesOverlap({{0, 0}, 10}, {{17, 0}, 6}));
}

// La diagonal ES separados: dist² = 288 > (10+6)² = 256
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
    ASSERT_NEAR(c.x, 100.0f, 0.001f);   // clamp al borde izquierdo
    ASSERT_NEAR(c.y, 140.0f, 0.001f);   // clamp al borde inferior

    Vector2 c2 = ClosestPointOnRect(r, {310, 30});
    ASSERT_NEAR(c2.x, 140.0f, 0.001f);  // clamp al borde derecho
    ASSERT_NEAR(c2.y, 100.0f, 0.001f);  // clamp al borde superior
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 08 (Colisiones)...\n";
    return RUN_ALL_TESTS();
}
