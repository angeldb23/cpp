/* =============================================================================
   MÓDULO 17 - EJERCICIO 06: Camera2D — Lerp, Límites de Mundo y Culling Base
   =============================================================================
   DIFICULTAD: Intermedio
   APOYO: Lección 04 (Cámaras 2D: lerp, clamp de mundo, zoom)

   CONSIGNA:
   1) lerpTowards(current, target, factor): mueve current hacia target sin
      saltar (el "seguimiento suave" de la lección 04). Factor en [0,1]:
      0 => no se mueve, 1 => llega exacto al target.
   2) clampCameraTarget(desired, worldSize, viewport, zoom): el target de la
      cámara no puede dejar de ver mundo. El target mínimo/máximo por eje es:
      target >= viewport/2 /zoom       (no mostrar de más a la izquierda)
      target <= worldSize - viewport/2/zoom   (ídem al final)
      Si el mundo es MÁS CHICO que el viewport (en su eje), el target queda
      centrado en la mitad del mundo para ese eje (no hay margen: se centra).
   3) UpdateCamera(camera): aplica (1) hacia el jugador y después (2).

   TESTS (no tocar): lerp a mitad, clamp en bordes, mundo chico centrado.

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_06_camara_follow.cpp -o ej && ./ej
   ============================================================================= */
#include <algorithm>
#include <iostream>
#include <cmath>
#include "../../include/test_framework.hpp"

// ============================ STUBS (no tocar) ===============================
struct Vector2 { float x, y; };
struct Camera2D { Vector2 offset; Vector2 target; float rotation, zoom = 1.0f; };

const Vector2 WORLD{960, 640};
const Vector2 VIEWPORT{640, 320};

// ============================ TU CÓDIGO AQUÍ ================================
// TODO 1: lerp suave (factor en [0,1])
Vector2 lerpTowards(Vector2 current, Vector2 target, float factor) {
    // Pista: current + (target - current) * factor, por eje
    return current;   // placeholder
}

// TODO 2: clamp del target según zona visible
Vector2 clampCameraTarget(Vector2 desired, Vector2 worldSize,
                          Vector2 viewport, float zoom) {
    return desired;   // placeholder
}

// TODO 3: la cámara real sigue al player con lerp y luego se clampea
void updateCamera(Camera2D& cam, Vector2 playerPos, float lerpFactor) {
    (void)playerPos; (void)lerpFactor;
}

// ============================ TESTS (no tocar) ==============================
TEST_CASE(LerpAMitad) {
    Vector2 r = lerpTowards({0, 100}, {100, 0}, 0.5f);
    ASSERT_NEAR(r.x, 50.0f, 0.01f);
    ASSERT_NEAR(r.y, 50.0f, 0.01f);
}

TEST_CASE(LerpFactorUnoYZero) {
    // factor 1: llega exacto
    Vector2 r1 = lerpTowards({20, 20}, {80, 40}, 1.0f);
    ASSERT_NEAR(r1.x, 80.0f, 0.001f);
    ASSERT_NEAR(r1.y, 40.0f, 0.001f);
    // factor 0: no se mueve
    Vector2 r2 = lerpTowards({20, 20}, {80, 40}, 0.0f);
    ASSERT_NEAR(r2.x, 20.0f, 0.001f);
    ASSERT_NEAR(r2.y, 20.0f, 0.001f);
}

TEST_CASE(LerpRepetidoConverge) {
    // 10 lerp de 0.2 NO llegan exacto pero si dejan de acercarse (mov cap)
    Vector2 pos{0, 0}, target{100, 0};
    for (int i = 0; i < 50; ++i) pos = lerpTowards(pos, target, 0.2f);
    ASSERT_TRUE(pos.x > 99.5f);       // casi llegó
    ASSERT_TRUE(pos.x <= 100.0f);     // nunca se pasa
}

TEST_CASE(ClapCentroPerfectoNoMueve) {
    // centro del mundo: máx margen, fuera de clamp
    Vector2 r = clampCameraTarget({480, 320}, WORLD, VIEWPORT, 1.0f);
    ASSERT_NEAR(r.x, 480.0f, 0.001f);
    ASSERT_NEAR(r.y, 320.0f, 0.001f);
}

TEST_CASE(ClampNoSaleALosBordes) {
    // queremos ver más allá del borde izquierdo: clamp a viewport/2
    Vector2 r = clampCameraTarget({100, 100}, WORLD, VIEWPORT, 1.0f);
    ASSERT_NEAR(r.x, 320.0f, 0.001f);   // 640/2
    ASSERT_NEAR(r.y, 160.0f, 0.001f);   // 320/2

    // cerca del borde derecho: max = 960 - 320 = 640
    Vector2 r2 = clampCameraTarget({900, 500}, WORLD, VIEWPORT, 1.0f);
    ASSERT_NEAR(r2.x, 640.0f, 0.001f);
    ASSERT_NEAR(r2.y, 480.0f, 0.001f);  // 640 - 160
}

TEST_CASE(ClampConZoomHaceVisibleMenosArea) {
    // zoom 2: el viewport se ve la mitad => margen de tile menos
    // viewport/2/zoom = 640/2/2 = 160; mundo 960 => x max 800
    Vector2 r = clampCameraTarget({900, 100}, WORLD, VIEWPORT, 2.0f);
    ASSERT_NEAR(r.x, 800.0f, 0.001f);
    // y: 320/2/2 = 80; mundo 640 => max 560; pedimos 100 => clamp a 80
    ASSERT_NEAR(r.y, 100.0f, 0.001f);   // 100 > 80, rango válido sin clamp
}

TEST_CASE(MundoChicoVistaSeCentra) {
    // mundo 600x400 con viewport 640x320: x mundo CHICO (600 < 640) -> centrar
    // y mundo suficientemente grande -> clamp normal
    Vector2 r = clampCameraTarget({0, 0}, {600, 400}, VIEWPORT, 1.0f);
    ASSERT_NEAR(r.x, 300.0f, 0.001f);   // 600/2
    ASSERT_NEAR(r.y, 160.0f, 0.001f);   // min = 320/2 = 160 (y=0 clampea)
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 06 (Cámara Follow)...\n";
    return RUN_ALL_TESTS();
}
