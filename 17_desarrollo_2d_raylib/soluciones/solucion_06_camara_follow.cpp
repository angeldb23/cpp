/* =============================================================================
   MÓDULO 17 - SOLUCIÓN 06: Camera2D — Lerp, Límites de Mundo y Culling Base
   =============================================================================
   Resuelve ejercicio_06_camara_follow.cpp. Dos detalles del clamp:
   (1) el margen visible es viewport/2/zoom (con zoom 2 ves la mitad de mundo
   => margen menor); (2) si el mundo es chico para el viewport, no hay margen
   posible: el target se fija centrado en ese eje.

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_06_camara_follow.cpp -o sol && ./sol
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

// ============================ SOLUCIÓN =======================================
// TODO 1: lerp suave
Vector2 lerpTowards(Vector2 current, Vector2 target, float factor) {
    const float t = std::clamp(factor, 0.0f, 1.0f);
    return { current.x + (target.x - current.x) * t,
             current.y + (target.y - current.y) * t };
}

// TODO 2: clamp del target según el área visible
Vector2 clampCameraTarget(Vector2 desired, Vector2 worldSize,
                          Vector2 viewport, float zoom) {
    // La mitad de la pantalla visible en MUNDO es viewport/2. Con zoom, esa
    // mitad se multiplica por 1/zoom (zoom 2 => ves la MITAD de world-space)
    const float halfX = (viewport.x * 0.5f) / zoom;
    const float halfY = (viewport.y * 0.5f) / zoom;

    // Eje X: si el mundo es más chico que la vista, NO hay rango: centramos
    float minX = halfX, maxX = worldSize.x - halfX;
    float minXY = halfY, maxY = worldSize.y - halfY;
    if (minX > maxX) minX = maxX = worldSize.x * 0.5f;
    if (minXY > maxY) minXY = maxY = worldSize.y * 0.5f;

    return { std::clamp(desired.x, minX, maxX),
             std::clamp(desired.y, minXY, maxY) };
}

// TODO 3: sigue y clampea
void updateCamera(Camera2D& cam, Vector2 playerPos, float lerpFactor) {
    cam.target = lerpTowards(cam.target, playerPos, lerpFactor);
    cam.target = clampCameraTarget(cam.target, WORLD, VIEWPORT, cam.zoom);
}

// ============================ TESTS (iguales) ================================
TEST_CASE(LerpAMitad) {
    Vector2 r = lerpTowards({0, 100}, {100, 0}, 0.5f);
    ASSERT_NEAR(r.x, 50.0f, 0.01f);
    ASSERT_NEAR(r.y, 50.0f, 0.01f);
}

TEST_CASE(LerpFactorUnoYZero) {
    Vector2 r1 = lerpTowards({20, 20}, {80, 40}, 1.0f);
    ASSERT_NEAR(r1.x, 80.0f, 0.001f);
    ASSERT_NEAR(r1.y, 40.0f, 0.001f);
    Vector2 r2 = lerpTowards({20, 20}, {80, 40}, 0.0f);
    ASSERT_NEAR(r2.x, 20.0f, 0.001f);
    ASSERT_NEAR(r2.y, 20.0f, 0.001f);
}

TEST_CASE(LerpRepetidoConverge) {
    Vector2 pos{0, 0}, target{100, 0};
    for (int i = 0; i < 50; ++i) pos = lerpTowards(pos, target, 0.2f);
    ASSERT_TRUE(pos.x > 99.5f);
    ASSERT_TRUE(pos.x <= 100.0f);
}

TEST_CASE(ClapCentroPerfectoNoMueve) {
    Vector2 r = clampCameraTarget({480, 320}, WORLD, VIEWPORT, 1.0f);
    ASSERT_NEAR(r.x, 480.0f, 0.001f);
    ASSERT_NEAR(r.y, 320.0f, 0.001f);
}

TEST_CASE(ClampNoSaleALosBordes) {
    Vector2 r = clampCameraTarget({100, 100}, WORLD, VIEWPORT, 1.0f);
    ASSERT_NEAR(r.x, 320.0f, 0.001f);
    ASSERT_NEAR(r.y, 160.0f, 0.001f);
    Vector2 r2 = clampCameraTarget({900, 500}, WORLD, VIEWPORT, 1.0f);
    ASSERT_NEAR(r2.x, 640.0f, 0.001f);
    ASSERT_NEAR(r2.y, 480.0f, 0.001f);
}

TEST_CASE(ClampConZoomHaceVisibleMenosArea) {
    Vector2 r = clampCameraTarget({900, 100}, WORLD, VIEWPORT, 2.0f);
    ASSERT_NEAR(r.x, 800.0f, 0.001f);
    ASSERT_NEAR(r.y, 100.0f, 0.001f);
}

TEST_CASE(MundoChicoVistaSeCentra) {
    Vector2 r = clampCameraTarget({0, 0}, {600, 400}, VIEWPORT, 1.0f);
    ASSERT_NEAR(r.x, 300.0f, 0.001f);
    ASSERT_NEAR(r.y, 160.0f, 0.001f);
}

int main() {
    std::cout << "Solución 06: Camera2D Follow\n";
    return RUN_ALL_TESTS();
}
