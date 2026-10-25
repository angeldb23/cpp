/* =============================================================================
   MÓDULO 26 - SOLUCIÓN 01: modelo de costos GL vs API explícita
   =============================================================================
   La solución de ejercicio_01_costo_api.cpp. La idea central de la
   lección 01: el costo de la API explícita es FIJO (crear pipelines una
   vez) y el de OpenGL es VARIABLE (por draw call). Cuando el variable
   supera al amortizado, conviene la API moderna; con escenas chicas el
   costo fijo no se amortiza y GL 3.3 queda bien (el dueño con 40 draws
   tiene exceso de 44 us/frame, lejos del umbral de 500).
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <string>

namespace {

struct Costos {
    double overhead_gl_por_call_us = 1.5;
    double overhead_vk_por_call_us = 0.3;
    double costo_crear_pipeline_us = 1200.0;
    double costo_crear_pass_gl_us  = 60.0;
    double costo_crear_pass_vk_us  = 40.0;
};

/* La CPU paga 1.5 us por draw call validando estado (driver GL). */
double tiempoGL(const Costos& c, int draws, int frames) {
    return double(draws) * double(frames) * c.overhead_gl_por_call_us
         + double(frames) * c.costo_crear_pass_gl_us;
}

/* Cada draw es casi "emitir comandos"; se paga además la creación de
   las pipelines (una vez por PSO, no por frame). */
double tiempoVK(const Costos& c, int draws, int frames, int pipelines) {
    return double(draws) * double(frames) * c.overhead_vk_por_call_us
         + double(frames) * c.costo_crear_pass_vk_us
         + double(pipelines) * c.costo_crear_pipeline_us;
}

/* (GL - VK) por frame: positivo = GL paga más por frame. */
double excesoGL(const Costos& c, int draws, int frames, int pipelines) {
    return (tiempoGL(c, draws, frames) - tiempoVK(c, draws, frames, pipelines))
         / double(frames);
}

const char* decideApi(double exceso_por_frame, double umbral) {
    return exceso_por_frame > umbral ? "explicita" : "gl";
}

} // namespace

/* ------------------------------ TESTS (no tocar)-------------------------- */
TEST_CASE(FormulasPorEscena) {
    const Costos c;
    ASSERT_NEAR(tiempoGL(c, 40, 300), 120.0 * 300.0, 0.001);
    ASSERT_NEAR(tiempoVK(c, 40, 300, 6), 76.0 * 300.0, 0.001);
    ASSERT_NEAR(excesoGL(c, 40, 300, 6), 44.0, 0.001);
    ASSERT_NEAR(tiempoGL(c, 2000, 300), 3060.0 * 300.0, 0.001);
    ASSERT_NEAR(tiempoVK(c, 2000, 300, 4), 656.0 * 300.0, 0.001);
    ASSERT_NEAR(excesoGL(c, 2000, 300, 4), 2404.0, 0.001);
}

TEST_CASE(CostoFijoSinAmortizar) {
    const Costos c;
    const double ex = excesoGL(c, 1, 2, 1);
    ASSERT_TRUE(ex < 0.0);
    ASSERT_NEAR(ex, -578.8, 0.001);
}

TEST_CASE(RiesgoDecisiones) {
    ASSERT_EQ(decideApi(2404.0, 500.0), "explicita");
    ASSERT_EQ(decideApi(44.0, 500.0), "gl");
    ASSERT_EQ(decideApi(500.0, 500.0), "gl");
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
