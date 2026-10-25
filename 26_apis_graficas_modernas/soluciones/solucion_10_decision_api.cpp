/* =============================================================================
   MÓDULO 26 - SOLUCIÓN 10: decisión de API (scorecard del cierre del módulo)
   =============================================================================
   La solución de ejercicio_10_decision_api.cpp. Es la tabla del
   cierre/DECISION_API.md en forma de código: el dueño modifica este
   archivo fácilmente si cambia la placa (una UHD 600 nueva NO cambia
   la decisión; una dGPU con familia de compute separada podría).
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
    bool iso_ready;   /* PipelineSpec ya extraída (paso 2 del plan) */
};

enum class Api { GL33, SDL_GPU, VulkanNativo };

Api decidir(const PerfilProyecto& p) {
    /* 1) prototipo rápido y escena chica: el costo fijo no se paga. */
    if (p.corto_plazo && p.draws_frame < 500 && !p.necesita_compute)
        return Api::GL33;
    /* 2) la migración estructural ya hecha sube el "techo GL" a 1200 (inclusivo):
          mientras el renderer corra sobre una spec declarativa, la
          API no es la parte difícil (código listo para SDL_GPU). */
    const int techo_gl = p.iso_ready ? 1200 : 500;
    if (p.iso_ready && p.draws_frame <= techo_gl && !p.necesita_compute &&
        !p.portar_windows)
        return Api::GL33;
    if (p.draws_frame >= techo_gl || p.necesita_compute ||
        p.portar_windows)
        return Api::SDL_GPU;
    /* 3) resto: escena chica, sin compute ni porte, migración lista:
          el interés pendiente es control fino de memoria/threads con
          tiempo de equipo (por eso NO es iso_ready => quiere decir
          que hay trabajo de migración pendiente: Vulkan nativo exige
          ese tiempo). */
    return Api::VulkanNativo;
}

} // namespace

/* ------------------------------ TESTS (no tocar)-------------------------- */
TEST_CASE(PrototipoRapidoQuedaEnGL) {
    const Api a = decidir({40, 6, false, true, false, false});
    ASSERT_TRUE(a == Api::GL33);
}

TEST_CASE(IsoReadySubeElTechoHasta1200) {
    ASSERT_TRUE(decidir({800, 10, false, false, false, true}) == Api::GL33);
    ASSERT_TRUE(decidir({1200, 10, false, false, false, true}) == Api::GL33);
    ASSERT_TRUE(decidir({1201, 10, false, false, false, true}) == Api::SDL_GPU);
}

TEST_CASE(EscenaGordaOComputeVaASdlGpu) {
    ASSERT_TRUE(decidir({2000, 4, true, false, false, false}) == Api::SDL_GPU);
    ASSERT_TRUE(decidir({2000, 4, false, false, false, false}) == Api::SDL_GPU);
    ASSERT_TRUE(decidir({300, 10, false, false, true, false}) == Api::SDL_GPU);
}

TEST_CASE(ControlFinoSinIsoEsVulkan) {
    ASSERT_TRUE(decidir({200, 8, false, false, false, false}) ==
                Api::VulkanNativo);
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
