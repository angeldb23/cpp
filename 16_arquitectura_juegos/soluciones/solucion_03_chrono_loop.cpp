/* =============================================================================
   MÓDULO 16 - SOLUCIÓN 03: std::chrono — Game Loop con steady_clock
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
    float velocity = 100.0f;
};

void UpdateChrono(SimState& s, Seconds dt) {
    s.x += s.velocity * dt.count();
}

struct ChronoResult {
    int steps;
    float alpha;
    float totalSimSec;
};

ChronoResult RunChronoLoop(const std::vector<Seconds>& frameDurations,
                           Seconds fixedDt,
                           int maxSteps) {
    SimState s;
    Seconds accumulator{0.0f};
    int totalSteps = 0;
    float finalAlpha = 0.0f;
    float totalSim = 0.0f;

    for (auto frameDt : frameDurations) {
        frameDt = std::min(frameDt, Seconds(0.25f));
        accumulator += frameDt;
        int steps = 0;
        while (accumulator >= fixedDt && steps < maxSteps) {
            UpdateChrono(s, fixedDt);
            accumulator -= fixedDt;
            steps++;
            totalSteps++;
        }
        finalAlpha = (accumulator / fixedDt);  // ratio float
        totalSim += frameDt.count();
    }
    return {totalSteps, finalAlpha, totalSim};
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
    UpdateChrono(s, Seconds(1.0f));
    ASSERT_NEAR(s.x, 100.0f, 0.001f);
}

TEST_CASE(ChronoLoopFixedSteps) {
    using namespace std::chrono_literals;
    std::vector<Seconds> frames = {16.666ms, 16.666ms, 16.666ms};
    auto result = RunChronoLoop(frames, 16.666ms, 10);
    ASSERT_EQ(result.steps, 3);
    ASSERT_NEAR(result.alpha, 0.0f, 0.1f);
    ASSERT_NEAR(result.totalSimSec, 0.05f, 0.01f);
}

TEST_CASE(ChronoLoopClampAndLimit) {
    using namespace std::chrono_literals;
    std::vector<Seconds> frames = {500ms};
    auto result = RunChronoLoop(frames, 16.666ms, 3);
    ASSERT_EQ(result.steps, 3);
}

TEST_CASE(AlphaCalculation) {
    using namespace std::chrono_literals;
    std::vector<Seconds> frames = {20ms};
    auto result = RunChronoLoop(frames, 16.666ms, 10);
    ASSERT_EQ(result.steps, 1);
    ASSERT_TRUE(result.alpha >= 0.15f);
    ASSERT_TRUE(result.alpha <= 0.25f);
}

int main() {
    std::cout << "Solución 03 (std::chrono Game Loop)...\n";
    return RUN_ALL_TESTS();
}