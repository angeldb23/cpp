/* =============================================================================
   MÓDULO 26 - EJERCICIO 01: modelo de costos GL vs API explícita
   =============================================================================
   DIFICULTAD: Fácil
   APOYO: Lección 01 (por qué Vulkan/WebGPU: explicit control, overhead,
   comparación con OpenGL).

   CONSIGNA
   --------
   Completá el modelo de costos que la lección usa para comparar APIs:

   1. `tiempoGL(c, draws, frames)`:
        draws * frames * c.overhead_gl_por_call_us
      + frames * c.costo_crear_pass_gl_us
   2. `tiempoVK(c, draws, frames, pipelines)`:
        draws * frames * c.overhead_vk_por_call_us
      + frames * c.costo_crear_pass_vk_us
      + pipelines * c.costo_crear_pipeline_us
   3. `excesoGL(us/frame)`: (tiempoGL - tiempoVK) / frames. Positivo =
      OpenGL paga más CPU por frame; negativo = el costo fijo de crear
      pipelines todavía no se amortizó.
   4. `decideApi(exceso, umbral)`: exceso > umbral -> "explicita",
      si no -> "gl".

   Valores de f26.hpp (CostosDraw por defecto): overhead_gl=1.5 us,
   overhead_vk=0.3 us, pass gl=60 us, pass vk=40 us, pipeline=1200 us.
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

double tiempoGL(const Costos& c, int draws, int frames) {
    // TODO
    (void)c; (void)draws; (void)frames;
    return -1.0;   // CODIGO de error: los tests lo detectan
}

double tiempoVK(const Costos& c, int draws, int frames, int pipelines) {
    // TODO
    (void)c; (void)draws; (void)frames; (void)pipelines;
    return -1.0;
}

double excesoGL(const Costos& c, int draws, int frames, int pipelines) {
    // TODO: (tiempoGL - tiempoVK) / frames
    return -1.0;
}

const char* decideApi(double exceso_por_frame, double umbral) {
    // TODO: "explicita" si exceso > umbral, "gl" si no
    return "";
}

} // namespace

/* ------------------------------ TESTS (no tocar)-------------------------- */
TEST_CASE(FormulasPorEscena) {
    const Costos c;
    /* escena 1: 40 draws, 6 pipelines, 300 frames */
    ASSERT_NEAR(tiempoGL(c, 40, 300), 120.0 * 300.0, 0.001);
    ASSERT_NEAR(tiempoVK(c, 40, 300, 6), 76.0 * 300.0, 0.001);
    ASSERT_NEAR(excesoGL(c, 40, 300, 6), 44.0, 0.001);
    /* escena 3 (bullet hell): 2000 draws, 4 pipelines */
    ASSERT_NEAR(tiempoGL(c, 2000, 300), 3060.0 * 300.0, 0.001);
    ASSERT_NEAR(tiempoVK(c, 2000, 300, 4), 656.0 * 300.0, 0.001);
    ASSERT_NEAR(excesoGL(c, 2000, 300, 4), 2404.0, 0.001);
}

TEST_CASE(CostoFijoSinAmortizar) {
    /* 1 draw, 1 pipeline, 2 frames: el costo fijo (1200) pesa MÁS que
       todo el trabajo: el exceso queda NEGATIVO. */
    const Costos c;
    const double ex = excesoGL(c, 1, 2, 1);
    ASSERT_TRUE(ex < 0.0);
    ASSERT_NEAR(ex, -578.8, 0.001);
}

TEST_CASE(RiesgoDecisiones) {
    ASSERT_EQ(decideApi(2404.0, 500.0), "explicita");
    ASSERT_EQ(decideApi(44.0, 500.0), "gl");
    ASSERT_EQ(decideApi(500.0, 500.0), "gl");   /* umbral no inclusivo */
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
