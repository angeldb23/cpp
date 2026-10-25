/* =============================================================================
   MÓDULO 23 - SOLUCIÓN 01: Acumulador de timestep fijo
   ============================================================================= */
#include "../include/fisica2d.hpp"
#include "../../include/test_framework.hpp"

struct StepConfig {
    float fixedDt = 0.01f;
    float maxFrame = 0.05f;
};

struct StepState {
    float accumulator = 0.0f;
    int steps = 0;
};

StepState avanzar(StepState estado, float frameTime, StepConfig cfg) {
    if (cfg.fixedDt <= f2d::kEps) return estado;
    frameTime = f2d::clampf(frameTime, 0.0f, cfg.maxFrame);
    estado.accumulator += frameTime;
    while (estado.accumulator >= cfg.fixedDt) {
        estado.accumulator -= cfg.fixedDt;
        ++estado.steps;
    }
    return estado;
}

float alpha(const StepState& estado, StepConfig cfg) {
    if (cfg.fixedDt <= f2d::kEps) return 0.0f;
    return f2d::clampf(estado.accumulator / cfg.fixedDt, 0.0f, 1.0f);
}

TEST_CASE(UnFrameCortoNoAvanza) {
    StepState s;
    s = avanzar(s, 0.005f, {});
    ASSERT_EQ(s.steps, 0);
    ASSERT_NEAR(s.accumulator, 0.005f, 1e-5f);
    ASSERT_NEAR(alpha(s, {}), 0.5f, 1e-5f);
}

TEST_CASE(AcumulaHastaElPasoFijo) {
    StepState s;
    s = avanzar(s, 0.035f, {});
    ASSERT_EQ(s.steps, 3);
    ASSERT_NEAR(s.accumulator, 0.005f, 1e-5f);
    ASSERT_NEAR(alpha(s, {}), 0.5f, 1e-5f);
}

TEST_CASE(ClampeaFramesEnormes) {
    StepState s;
    s = avanzar(s, 5.0f, {});
    ASSERT_EQ(s.steps, 5);
    ASSERT_NEAR(s.accumulator, 0.0f, 1e-5f);
}

TEST_CASE(ConservaElRestoEntreFrames) {
    StepState s;
    s = avanzar(s, 0.035f, {});
    s = avanzar(s, 0.008f, {});
    ASSERT_EQ(s.steps, 4);
    ASSERT_NEAR(s.accumulator, 0.003f, 1e-5f);
    ASSERT_NEAR(alpha(s, {}), 0.3f, 1e-5f);
}

TEST_CASE(AlphaSiempreEnRango) {
    StepConfig cfg;
    for (float ft : {0.0f, 0.001f, 0.0099f, 0.01f, 0.049f, 0.05f, 3.0f}) {
        StepState s;
        s = avanzar(s, ft, cfg);
        const float a = alpha(s, cfg);
        ASSERT_TRUE(a >= 0.0f && a <= 1.0f);
    }
}

int main() {
    std::cout << "Solución 01 (timestep fijo)...\n";
    return RUN_ALL_TESTS();
}
