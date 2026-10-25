/* =============================================================================
   MÓDULO 16 - SOLUCIÓN 05: Transiciones — Fade In/Out con Callback Mid
   ============================================================================= */
#include <functional>
#include "../../include/test_framework.hpp"

enum class FadePhase { None, FadeOut, FadeIn };

inline std::ostream& operator<<(std::ostream& os, FadePhase p) {
    switch (p) {
        case FadePhase::None: return os << "None";
        case FadePhase::FadeOut: return os << "FadeOut";
        case FadePhase::FadeIn: return os << "FadeIn";
    }
    return os << "Unknown";
}

struct FadeTransition {
    FadePhase phase = FadePhase::None;
    float alpha = 0.0f;
    float speed = 1.0f;
    std::function<void()> onMid;

    void Start(std::function<void()> callback) {
        phase = FadePhase::FadeOut;
        alpha = 0.0f;
        onMid = std::move(callback);
    }

    void Update(float dt) {
        if (phase == FadePhase::None) return;
        float remaining = dt;
        while (remaining > 0.0f && phase != FadePhase::None) {
            float delta = remaining / speed;
            if (phase == FadePhase::FadeOut) {
                alpha += delta;
                if (alpha >= 1.0f) {
                    float overshoot = alpha - 1.0f;
                    alpha = 1.0f;
                    phase = FadePhase::FadeIn;
                    if (onMid) { onMid(); onMid = nullptr; }
                    remaining = overshoot * speed;
                } else {
                    remaining = 0.0f;
                }
            } else { // FadeIn
                alpha -= delta;
                if (alpha <= 0.0f) {
                    float overshoot = -alpha;
                    alpha = 0.0f;
                    phase = FadePhase::None;
                    remaining = overshoot * speed;
                } else {
                    remaining = 0.0f;
                }
            }
        }
    }

    bool IsActive() const {
        return phase != FadePhase::None;
    }
};

// ============= TESTS =============
TEST_CASE(StartSetsFadeOut) {
    FadeTransition f;
    f.Start([](){});
    ASSERT_EQ(f.phase, FadePhase::FadeOut);
    ASSERT_EQ(f.alpha, 0.0f);
    ASSERT_TRUE(f.IsActive());
}

TEST_CASE(UpdateAdvancesAlphaInFadeOut) {
    FadeTransition f;
    f.speed = 2.0f;
    f.Start([](){});
    f.Update(0.5f);
    ASSERT_EQ(f.phase, FadePhase::FadeOut);
    ASSERT_NEAR(f.alpha, 0.25f, 0.001f);
}

TEST_CASE(MidpointCallsCallbackOnce) {
    FadeTransition f;
    f.speed = 1.0f;
    int called = 0;
    f.Start([&](){ called++; });
    f.Update(1.0f);
    ASSERT_EQ(called, 1);
    ASSERT_EQ(f.phase, FadePhase::FadeIn);
    ASSERT_EQ(f.alpha, 1.0f);
}

TEST_CASE(SecondCallDoesNotRecall) {
    FadeTransition f;
    f.speed = 1.0f;
    int called = 0;
    f.Start([&](){ called++; });
    f.Update(1.0f);
    f.Update(1.0f);
    ASSERT_EQ(called, 1);
}

TEST_CASE(FadeInCompletesToNone) {
    FadeTransition f;
    f.speed = 1.0f;
    f.Start([](){});
    f.Update(1.0f);
    f.Update(1.0f);
    ASSERT_EQ(f.phase, FadePhase::None);
    ASSERT_EQ(f.alpha, 0.0f);
    ASSERT_FALSE(f.IsActive());
}

TEST_CASE(SpeedControlsDuration) {
    FadeTransition f;
    f.speed = 0.5f;
    f.Start([](){});
    f.Update(0.5f);
    ASSERT_EQ(f.phase, FadePhase::FadeIn);
}

TEST_CASE(AlphaClampedAtOneAndZero) {
    FadeTransition f;
    f.speed = 1.0f;
    f.Start([](){});
    f.Update(10.0f);
    ASSERT_EQ(f.alpha, 0.0f);
    ASSERT_EQ(f.phase, FadePhase::None);
}

int main() {
    std::cout << "Solución 05 (Transiciones Fade)...\n";
    return RUN_ALL_TESTS();
}