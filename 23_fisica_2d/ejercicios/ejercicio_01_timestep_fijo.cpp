/* =============================================================================
   MÓDULO 23 - EJERCICIO 01: Acumulador de timestep fijo
   =============================================================================
   DIFICULTAD: Básico
   APOYO: Lección 01 (física en timestep fijo, clamp y alpha).

   CONSIGNA:
   Implementa `avanzar()` y `alpha()` con la regla del acumulador:
     1. Clampear frameTime a [0, maxFrame].
     2. Sumarlo al acumulador.
     3. Ejecutar pasos mientras accumulator >= fixedDt, restándolo.
     4. `alpha()` devuelve accumulator / fixedDt, clampeado a [0, 1].

   Un frame de 0.035 s con fixedDt=0.01 debe ejecutar 3 pasos y dejar
   accumulator=0.005. Un frame de 5 s con maxFrame=0.05 ejecuta 5 pasos.
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

// TODO: implementar el acumulador.
StepState avanzar(StepState estado, float frameTime, StepConfig cfg) {
    (void)frameTime;
    (void)cfg;
    return estado;
}

// TODO: devolver el resto normalizado entre 0 y 1.
float alpha(const StepState& estado, StepConfig cfg) {
    (void)estado;
    (void)cfg;
    return 0.0f;
}

// ============================================================================
// TESTS (no tocar)
// ============================================================================
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
    ASSERT_EQ(s.steps, 5);  // maxFrame 0.05 / fixedDt 0.01
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
    std::cout << "Ejercicio 01 (timestep fijo)...\n";
    return RUN_ALL_TESTS();
}
