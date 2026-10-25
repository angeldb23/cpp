/* =============================================================================
   MÓDULO 26 - EJERCICIO 10: decisión de API (scorecard del cierre del módulo)
   =============================================================================
   DIFICULTAD: Fácil-media
   APOYO: Lección 10 (cuándo GL 3.3, cuándo SDL_GPU, cuándo Vulkan nativo
   y migración incremental) + cierre/DECISION_API.md.

   CONSIGNA
   --------
   `decidir(p)` con las reglas de la lección 10 en orden:

   1. prototipo corto (< 3 meses) + menos de 500 draws/frame + sin
      compute -> Api::GL33 (el costo fijo no se amortiza).
   2. REFINAMIENTO de migración (novedad de este ejercicio): si el
      proyecto ya tiene la capa extraída en PipelineSpec/registro
      declarativo (iso_ready == true, el paso 2 del plan), el GL válido
      llega HASTA 1200 draws/frame INCLUSIVE: la migración estructural
      que ya hiciste compra margen a OpenGL.
   3. escena >= 500 draws (>= 1200 si iso_ready), compute necesario o
      porte a Windows -> Api::SDL_GPU.
   4. resto (escena chica, sin compute, sin porte, migración lista):
      control fino de memoria/threading con tiempo de equipo ->
      Api::VulkanNativo.
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <cstdint>

namespace {

struct PerfilProyecto {
    int  draws_frame;
    int  pipelines;
    bool necesita_compute;
    bool corto_plazo;
    bool portar_windows;
    bool iso_ready;   /* PipelineSpec ya extraída (migración 2.2) */
};

enum class Api { GL33, SDL_GPU, VulkanNativo };

Api decidir(const PerfilProyecto& p) {
    // TODO
    return Api::GL33;   // stub: siempre GL (los tests lo infieren)
}

} // namespace

/* ------------------------------ TESTS (no tocar)-------------------------- */
TEST_CASE(PrototipoRapidoQuedaEnGL) {
    const Api a = decidir({40, 6, false, true, false, false});
    ASSERT_TRUE(a == Api::GL33);
}

TEST_CASE(IsoReadySubeElTechoHasta1200) {
    /* con iso_ready el piso en el que GL es válido sube de 500 a 1200 */
    const Api a = decidir({800, 10, false, false, false, true});
    ASSERT_TRUE(a == Api::GL33);
    /* 1200 inclusive sigue en GL con iso_ready */
    ASSERT_TRUE(decidir({1200, 10, false, false, false, true}) == Api::GL33);
    /* 1201 ya pasa a SDL_GPU */
    ASSERT_TRUE(decidir({1201, 10, false, false, false, true}) ==
                Api::SDL_GPU);
}

TEST_CASE(EscenaGordaOComputeVaASdlGpu) {
    ASSERT_TRUE(decidir({2000, 4, true, false, false, false}) ==
                Api::SDL_GPU);
    ASSERT_TRUE(decidir({2000, 4, false, false, false, false}) ==
                Api::SDL_GPU);
    ASSERT_TRUE(decidir({300, 10, false, false, true, false}) ==
                Api::SDL_GPU);   /* porte a Windows */
}

TEST_CASE(ControlFinoSinIsoEsVulkan) {
    /* escena chica, sin compute, sin porte, no prototipo, iso pendiente:
       queda para motor con tiempo de equipo (típico caso "inbetween").
       Con iso_ready NO llega acá (cae en regla 2): por eso es
       indispensable el diseño del módulo 22. */
    ASSERT_TRUE(decidir({200, 8, false, false, false, false}) ==
                Api::VulkanNativo);
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
