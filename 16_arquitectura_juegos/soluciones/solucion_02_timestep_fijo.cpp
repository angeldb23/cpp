/* =============================================================================
   MÓDULO 16 - SOLUCIÓN 02: Timestep Fijo — Acumulador e Interpolación (Alpha)
   ============================================================================= */
#include <vector>
#include <algorithm>
#include "../../include/test_framework.hpp"

struct State {
    float x = 0.0f;
    float prevX = 0.0f;
    float velocity = 60.0f;
};

void Update(State& s, float dt) {
    s.prevX = s.x;
    s.x += s.velocity * dt;
}

float Interpolate(float prev, float curr, float alpha) {
    return prev + (curr - prev) * alpha;
}

struct LoopResult {
    float finalX;
    float renderX;
    int steps;
};

LoopResult RunLoop(const std::vector<float>& frameTimes,
                   float fixedDt,
                   int maxSteps) {
    State s;
    float accumulator = 0.0f;
    int totalSteps = 0;
    float renderX = 0.0f;

    for (float ft : frameTimes) {
        ft = std::min(ft, 0.25f);
        accumulator += ft;
        int steps = 0;
        while (accumulator >= fixedDt && steps < maxSteps) {
            Update(s, fixedDt);
            accumulator -= fixedDt;
            steps++;
            totalSteps++;
        }
        float alpha = accumulator / fixedDt;
        // At exact tick boundary (alpha=0), render current state not previous
        if (alpha < 0.001f) {
            renderX = s.x;
        } else {
            renderX = Interpolate(s.prevX, s.x, alpha);
        }
    }
    return {s.x, renderX, totalSteps};
}

// ============= TESTS =============
TEST_CASE(UpdateAdvancesPosition) {
    State s{0, 0, 60.0f};
    Update(s, 1.0f/60.0f);
    ASSERT_NEAR(s.x, 1.0f, 0.001f);
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
    std::vector<float> times(3, 1.0f/60.0f);
    auto result = RunLoop(times, 1.0f/60.0f, 10);
    ASSERT_EQ(result.steps, 3);
    ASSERT_NEAR(result.finalX, 3.0f, 0.01f);
    ASSERT_NEAR(result.renderX, 3.0f, 0.01f);
}

TEST_CASE(SpikeClampedAndLimited) {
    std::vector<float> times = {0.5f};
    auto result = RunLoop(times, 1.0f/60.0f, 3);
    ASSERT_EQ(result.steps, 3);
    ASSERT_TRUE(result.steps <= 3);
}

TEST_CASE(AlphaInRange) {
    std::vector<float> times = {0.020f};
    auto result = RunLoop(times, 1.0f/60.0f, 10);
    ASSERT_TRUE(result.renderX >= 0.0f);
    ASSERT_TRUE(result.renderX <= result.finalX + 1.0f);
}

int main() {
    std::cout << "Solución 02 (Timestep Fijo)...\n";
    return RUN_ALL_TESTS();
}