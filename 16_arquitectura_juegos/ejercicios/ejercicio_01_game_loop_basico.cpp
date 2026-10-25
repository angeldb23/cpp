/* =============================================================================
   MÓDULO 16 - EJERCICIO 01: Game Loop Básico — Estructura y Timestep Variable
   =============================================================================
   DIFICULTAD: Básico
   APOYO: Lecciones 02, 04 (Game loop básico, timestep variable)

   CONSIGNA:
   Implementa un game loop mínimo que simule movimiento con timestep variable.

   ESTRUCTURA REQUERIDA:
   - Struct Player { float x, y, speed; }
   - Función Update(Player&, float dt): actualiza posición según teclas "presionadas"
   - Función GetFrameTimeSim(): devuelve dt simulado (para test determinista)
   - Loop principal que corra N frames y acumule posición final

   TESTS (no tocar):
   - Con dt=0.016f (60 FPS) y teclas RIGHT+UP durante 60 frames:
     x ≈ 60 * speed * 0.016, y ≈ -60 * speed * 0.016
   - Con dt variable [0.016, 0.017, 0.015...] mismo resultado aprox.

   NOTA: Este ejercicio usa SIMULACIÓN de input/tiempo (sin raylib real).
   La lógica es portable y testeable.

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_01_game_loop_basico.cpp -o ej01 && ./ej01
   ============================================================================= */
#include <vector>
#include <cmath>
#include "../../include/test_framework.hpp"

struct Player {
    float x = 400.0f;
    float y = 300.0f;
    float speed = 200.0f;  // pixels/segundo
};

// Simulación de estado de teclas (para tests deterministas)
struct InputState {
    bool right = false;
    bool left = false;
    bool up = false;
    bool down = false;
};

// TODO 1: Update player position based on input and dt
// TU CÓDIGO AQUÍ:
void Update(Player& player, float dt, const InputState& input) {
    // Pista: player.x += player.speed * dt si input.right
}

// TODO 2: Simulate game loop for N frames with given dt sequence
// TU CÓDIGO AQUÍ:
Player SimulateLoop(const std::vector<float>& frameTimes, const InputState& input) {
    Player p;
    // Pista: for each dt in frameTimes: Update(p, dt, input);
    return p;  // placeholder
}

// ============= TESTS =============
TEST_CASE(UpdateMovesRight) {
    Player p{0, 0, 100.0f};
    InputState input{true, false, false, false};
    Update(p, 1.0f, input);  // 1 segundo a 100 px/s
    ASSERT_NEAR(p.x, 100.0f, 0.001f);
    ASSERT_NEAR(p.y, 0.0f, 0.001f);
}

TEST_CASE(UpdateMovesDiagonal) {
    Player p{0, 0, 100.0f};
    InputState input{true, false, true, false};  // right + up
    Update(p, 1.0f, input);
    // Diagonal: cada eje 100 px/s independiente
    ASSERT_NEAR(p.x, 100.0f, 0.001f);
    ASSERT_NEAR(p.y, -100.0f, 0.001f);
}

TEST_CASE(Simulate60FramesAt60FPS) {
    std::vector<float> times(60, 1.0f / 60.0f);  // 60 frames @ 60 FPS = 1 segundo
    InputState input{true, false, false, false};
    Player p = SimulateLoop(times, input);
    // 100 px/s * 1s = 100
    ASSERT_NEAR(p.x, 100.0f, 0.5f);
    ASSERT_NEAR(p.y, 0.0f, 0.001f);
}

TEST_CASE(SimulateVariableDt) {
    // Frame times variables que suman ~1 segundo
    std::vector<float> times = {0.016f, 0.017f, 0.015f, 0.016f, 0.018f,
                                0.016f, 0.017f, 0.015f, 0.016f, 0.018f,
                                0.016f, 0.017f, 0.015f, 0.016f, 0.018f,
                                0.016f, 0.017f, 0.015f, 0.016f, 0.018f,
                                0.016f, 0.017f, 0.015f, 0.016f, 0.018f,
                                0.016f, 0.017f, 0.015f, 0.016f, 0.018f};
    InputState input{true, false, false, false};
    Player p = SimulateLoop(times, input);
    float totalTime = 0;
    for (float t : times) totalTime += t;
    ASSERT_NEAR(p.x, 100.0f * totalTime, 1.0f);  // ~100 * 0.5 = 50
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 01 (Game Loop Básico)...\n";
    return RUN_ALL_TESTS();
}