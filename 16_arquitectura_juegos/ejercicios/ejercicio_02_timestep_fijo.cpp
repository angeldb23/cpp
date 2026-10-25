/* =============================================================================
   MÓDULO 16 - EJERCICIO 02: Timestep Fijo — Acumulador e Interpolación (Alpha)
   =============================================================================
   DIFICULTAD: Intermedio
   APOYO: Lección 05 (Timestep fijo con acumulador)

   CONSIGNA:
   Implementa el game loop clásico con timestep fijo (Gaffer on Games):
   - Acumulador que suma frameTime
   - While (accumulator >= fixedDt) { Update(fixedDt); accumulator -= fixedDt; }
   - Alpha = accumulator / fixedDt para interpolación de render
   - Clamp de frameTime grande (spike protection)
   - Límite máximo de pasos por frame (spiral of death protection)

   ESTRUCTURA:
   - Estado: struct State { float x, prevX; }
   - Update(State&, float dt): x += velocity * dt; prevX = x antes de update
   - Interpolate(prev, curr, alpha): lerp lineal
   - RunLoop(frameTimes, fixedDt, maxSteps): devuelve (finalX, renderX, stepsCount)

   TESTS:
   - fixedDt = 1/60, frameTimes = [1/60, 1/60, 1/60] -> 3 steps, alpha ≈ 0
   - frameTime = 0.1 (spike) -> clamp a 0.25, maxSteps limita updates
   - Alpha en [0, 1) siempre

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_02_timestep_fijo.cpp -o ej02 && ./ej02
   ============================================================================= */
#include <vector>
#include <algorithm>
#include "../../include/test_framework.hpp"

struct State {
    float x = 0.0f;
    float prevX = 0.0f;
    float velocity = 60.0f;  // 60 unidades/segundo
};

// TODO 1: Update con dt FIJO
// TU CÓDIGO AQUÍ:
void Update(State& s, float dt) {
    // Pista: s.prevX = s.x; s.x += s.velocity * dt;
}

// TODO 2: Interpolación lineal para render
// TU CÓDIGO AQUÍ:
float Interpolate(float prev, float curr, float alpha) {
    // Pista: prev + (curr - prev) * alpha
    return 0.0f;  // placeholder
}

// TODO 3: Game loop con acumulador, clamp, maxSteps
// TU CÓDIGO AQUÍ:
struct LoopResult {
    float finalX;      // Estado final tras updates
    float renderX;     // Posición interpolada para render
    int steps;         // Cuántos steps de física se ejecutaron
};

LoopResult RunLoop(const std::vector<float>& frameTimes,
                   float fixedDt,
                   int maxSteps) {
    State s;
    float accumulator = 0.0f;
    int totalSteps = 0;

    // Pista:
    // for (float ft : frameTimes) {
    //     ft = std::min(ft, 0.25f);  // clamp
    //     accumulator += ft;
    //     int steps = 0;
    //     while (accumulator >= fixedDt && steps < maxSteps) {
    //         Update(s, fixedDt);
    //         accumulator -= fixedDt;
    //         steps++; totalSteps++;
    //     }
    //     float alpha = accumulator / fixedDt;
    //     float renderX = Interpolate(s.prevX, s.x, alpha);
    // }
    // return {s.x, renderX, totalSteps};

    return {0.0f, 0.0f, 0};  // placeholder
}

// ============= TESTS =============
TEST_CASE(UpdateAdvancesPosition) {
    State s{0, 0, 60.0f};
    Update(s, 1.0f/60.0f);
    ASSERT_NEAR(s.x, 1.0f, 0.001f);  // 60 * 1/60 = 1
    ASSERT_NEAR(s.prevX, 0.0f, 0.001f);
}

TEST_CASE(InterpolateAtZeroIsPrev) {
    ASSERT_NEAR(Interpolate(10.0f, 20.0f, 0.0f), 10.0f, 0.001f);
}

TEST_CASE(InterpolateAtOneIsCurr) {
    ASSERT_NEAR(Interpolate(10.0f, 20.0f, 1.0f), 20.0f, 0.001f);
}

TEST_CASE(InterpolateMidpoint) {
    ASSERT_NEAR(Interpolate(10.0f, 20.0f, 0.5f), 15.0f, 0.001f);
}

TEST_CASE(FixedStepThreeFrames) {
    std::vector<float> times(3, 1.0f/60.0f);  // 3 frames exactos
    auto result = RunLoop(times, 1.0f/60.0f, 10);
    ASSERT_EQ(result.steps, 3);
    ASSERT_NEAR(result.finalX, 3.0f, 0.01f);  // 3 steps * 1 unidad
    ASSERT_NEAR(result.renderX, 3.0f, 0.01f); // alpha ≈ 0
}

TEST_CASE(SpikeClampedAndLimited) {
    // Un frame grande de 0.5s (spike) -> clamp a 0.25s
    // fixedDt = 1/60 ≈ 0.0167, maxSteps = 3
    // 0.25 / 0.0167 ≈ 14 steps posibles, pero maxSteps=3 limita a 3
    std::vector<float> times = {0.5f};
    auto result = RunLoop(times, 1.0f/60.0f, 3);
    ASSERT_EQ(result.steps, 3);  // Limitado por maxSteps
    ASSERT_TRUE(result.steps <= 3);
}

TEST_CASE(AlphaInRange) {
    std::vector<float> times = {0.020f};  // Un frame de 20ms
    auto result = RunLoop(times, 1.0f/60.0f, 10);
    // 20ms = 1.2 steps -> 1 step ejecutado, accumulator = 0.0033, alpha ≈ 0.2
    ASSERT_TRUE(result.renderX >= 0.0f);
    ASSERT_TRUE(result.renderX <= result.finalX + 1.0f);  // interpolado entre prev y curr
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 02 (Timestep Fijo)...\n";
    return RUN_ALL_TESTS();
}