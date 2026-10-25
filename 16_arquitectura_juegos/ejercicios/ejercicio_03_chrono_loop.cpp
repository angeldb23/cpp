/* =============================================================================
   MÓDULO 16 - EJERCICIO 03: std::chrono — Game Loop con steady_clock y duration<float>
   =============================================================================
   DIFICULTAD: Intermedio
   APOYO: Lección 06 (std::chrono para timestep)

   CONSIGNA:
   Implementa un game loop manual usando std::chrono::steady_clock:
   - TimePoint, duration<float>, literales (16ms, 1s, etc.)
   - Fixed timestep con duration<float>
   - Clamp de frame time excesivo
   - Acumulador con duration<float>
   - Alpha como float (ratio de durations)

   ESTRUCTURA:
   - using Clock = steady_clock; using Seconds = duration<float>;
   - Función RunChronoLoop(frameDurations, fixedDt, maxSteps)
     frameDurations: vector<Seconds> con duración de cada frame simulado
   - Devuelve steps totales, alpha final, tiempo simulado total

   TESTS:
   - Duraciones en seconds, milliseconds, microseconds (literales)
   - duration_cast para convertir
   - Aritmética type-safe: tp + dt, tp - tp = dt

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_03_chrono_loop.cpp -o ej03 && ./ej03
   ============================================================================= */
#include <chrono>
#include <vector>
#include <algorithm>
#include "../../include/test_framework.hpp"

using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;
using Seconds = std::chrono::duration<float>;
using Milliseconds = std::chrono::duration<float, std::milli>;

struct SimState {
    float x = 0.0f;
    float velocity = 100.0f;  // unidades/segundo
};

// TODO 1: Update usando Seconds (duration<float>)
// TU CÓDIGO AQUÍ:
void UpdateChrono(SimState& s, Seconds dt) {
    // Pista: s.x += s.velocity * dt.count();
}

// TODO 2: Game loop con std::chrono types
// TU CÓDIGO AQUÍ:
struct ChronoResult {
    int steps;
    float alpha;        // [0, 1)
    float totalSimSec;  // tiempo simulado total (acumulador real)
};

ChronoResult RunChronoLoop(const std::vector<Seconds>& frameDurations,
                           Seconds fixedDt,
                           int maxSteps) {
    SimState s;
    Seconds accumulator{0.0f};
    int totalSteps = 0;
    float finalAlpha = 0.0f;
    float totalSim = 0.0f;

    // Pista:
    // for (auto frameDt : frameDurations) {
    //     frameDt = std::min(frameDt, Seconds(0.25f));  // clamp
    //     accumulator += frameDt;
    //     int steps = 0;
    //     while (accumulator >= fixedDt && steps < maxSteps) {
    //         UpdateChrono(s, fixedDt);
    //         accumulator -= fixedDt;
    //         steps++; totalSteps++;
    //     }
    //     finalAlpha = (accumulator / fixedDt).count();  // ratio float
    //     totalSim += frameDt.count();
    // }
    // return {totalSteps, finalAlpha, totalSim};

    return {0, 0.0f, 0.0f};  // placeholder
}

// ============= TESTS =============
TEST_CASE(DurationLiterals) {
    using namespace std::chrono_literals;
    Seconds dt = 16.666ms;
    ASSERT_NEAR(dt.count(), 0.016666f, 0.0001f);
    Seconds dt2 = 1s;
    ASSERT_EQ(dt2.count(), 1.0f);
}

TEST_CASE(DurationArithmetic) {
    using namespace std::chrono_literals;
    TimePoint start = Clock::now();
    TimePoint end = start + 100ms;
    Seconds diff = end - start;
    ASSERT_NEAR(diff.count(), 0.1f, 0.01f);
}

TEST_CASE(DurationCast) {
    using namespace std::chrono_literals;
    Seconds s = 1.5s;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(s);
    ASSERT_EQ(ms.count(), 1500);
}

TEST_CASE(UpdateChronoAdvances) {
    SimState s;
    UpdateChrono(s, Seconds(1.0f));  // 1 segundo
    ASSERT_NEAR(s.x, 100.0f, 0.001f);
}

TEST_CASE(ChronoLoopFixedSteps) {
    using namespace std::chrono_literals;
    std::vector<Seconds> frames = {16.666ms, 16.666ms, 16.666ms};  // 3 frames @ 60Hz
    auto result = RunChronoLoop(frames, 16.666ms, 10);
    ASSERT_EQ(result.steps, 3);
    ASSERT_NEAR(result.alpha, 0.0f, 0.1f);  // casi exacto
    ASSERT_NEAR(result.totalSimSec, 0.05f, 0.01f);  // ~50ms
}

TEST_CASE(ChronoLoopClampAndLimit) {
    using namespace std::chrono_literals;
    // Frame de 500ms -> clamp a 250ms -> max 3 steps de 16.666ms
    std::vector<Seconds> frames = {500ms};
    auto result = RunChronoLoop(frames, 16.666ms, 3);
    ASSERT_EQ(result.steps, 3);  // limitado por maxSteps
}

TEST_CASE(AlphaCalculation) {
    using namespace std::chrono_literals;
    // Frame de 20ms, fixedDt = 16.666ms -> 1 step, accumulator = 3.334ms
    // alpha = 3.334 / 16.666 ≈ 0.2
    std::vector<Seconds> frames = {20ms};
    auto result = RunChronoLoop(frames, 16.666ms, 10);
    ASSERT_EQ(result.steps, 1);
    ASSERT_TRUE(result.alpha >= 0.15f);
    ASSERT_TRUE(result.alpha <= 0.25f);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 03 (std::chrono Game Loop)...\n";
    return RUN_ALL_TESTS();
}