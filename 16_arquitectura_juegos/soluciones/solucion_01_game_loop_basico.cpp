/* =============================================================================
   MÓDULO 16 - SOLUCIÓN 01: Game Loop Básico — Estructura y Timestep Variable
   ============================================================================= */
#include <vector>
#include <cmath>
#include "../../include/test_framework.hpp"

struct Player {
    float x = 400.0f;
    float y = 300.0f;
    float speed = 200.0f;
};

struct InputState {
    bool right = false;
    bool left = false;
    bool up = false;
    bool down = false;
};

void Update(Player& player, float dt, const InputState& input) {
    if (input.right) player.x += player.speed * dt;
    if (input.left)  player.x -= player.speed * dt;
    if (input.up)    player.y -= player.speed * dt;
    if (input.down)  player.y += player.speed * dt;
}

Player SimulateLoop(const std::vector<float>& frameTimes, const InputState& input) {
    Player p{0, 0, 100.0f};  // Start at origin for predictable tests
    for (float dt : frameTimes) {
        Update(p, dt, input);
    }
    return p;
}

// ============= TESTS =============
TEST_CASE(UpdateMovesRight) {
    Player p{0, 0, 100.0f};
    InputState input{true, false, false, false};
    Update(p, 1.0f, input);
    ASSERT_NEAR(p.x, 100.0f, 0.001f);
    ASSERT_NEAR(p.y, 0.0f, 0.001f);
}

TEST_CASE(UpdateMovesDiagonal) {
    Player p{0, 0, 100.0f};
    InputState input{true, false, true, false};
    Update(p, 1.0f, input);
    ASSERT_NEAR(p.x, 100.0f, 0.001f);
    ASSERT_NEAR(p.y, -100.0f, 0.001f);
}

TEST_CASE(Simulate60FramesAt60FPS) {
    std::vector<float> times(60, 1.0f / 60.0f);
    InputState input{true, false, false, false};
    Player p = SimulateLoop(times, input);
    ASSERT_NEAR(p.x, 100.0f, 0.5f);
    ASSERT_NEAR(p.y, 0.0f, 0.001f);
}

TEST_CASE(SimulateVariableDt) {
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
    ASSERT_NEAR(p.x, 100.0f * totalTime, 1.0f);
}

int main() {
    std::cout << "Solución 01 (Game Loop Básico)...\n";
    return RUN_ALL_TESTS();
}