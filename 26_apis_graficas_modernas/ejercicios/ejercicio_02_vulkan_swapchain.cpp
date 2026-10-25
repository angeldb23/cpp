/* =============================================================================
   MÓDULO 26 - EJERCICIO 02: Vulkan — queue families, swapchain y present mode
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 02 (instance, physical/logical device, queue families,
   swapchain).

   CONSIGNA
   --------
   Implementá la lógica de arranque de Vulkan en miniatura:

   1. `elegirColas(fams)`: recorre las familias y devuelve la primera con
      GRAPHICS que pueda PRESENT; una familia de COMPUTE sin GRAPHICS
      para async (si no hay, fallback a la gráfica); y la primera con
      present. Los índices sin match quedan en -1.
   2. `clampImageCount(caps, pedido)`: pedido fuera de [min, max] se
      clampea (max==0 significa "sin límite": solo min applies).
   3. `elegirPresent(quiere_lag_bajo, soporta_mailbox)`: MAILBOX si
      quiere lag bajo Y el driver lo soporta; SIEMPRE fallback FIFO
      (es el único garantizado por la spec).

   Bits de capacidades (misma semántica que VK_QUEUE_*_BIT):
     Q_GRAPHICS = 1, Q_COMPUTE = 2, Q_TRANSFER = 4.
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <cstdint>
#include <vector>

namespace {

constexpr std::uint32_t Q_GRAPHICS = 1u << 0;
constexpr std::uint32_t Q_COMPUTE  = 1u << 1;
constexpr std::uint32_t Q_TRANSFER = 1u << 2;

struct QueueFamily {
    std::uint32_t flags;
    std::uint32_t conteo_queues;
    bool          puede_present;
};

struct SeleccionColas { int grafica = -1; int compute = -1; int present = -1; };

SeleccionColas elegirColas(const std::vector<QueueFamily>& fams) {
    // TODO
    return {};   // stub: todo en -1 (los tests lo detectan)
}

struct SurfaceCaps { std::uint32_t min_images, max_images; };

std::uint32_t clampImageCount(const SurfaceCaps& c, std::uint32_t pedido) {
    // TODO
    return pedido;   // stub: no clampea
}

enum class PresentMode { IMMEDIATE, FIFO, FIFO_RELAXED, MAILBOX };

PresentMode elegirPresent(bool quiere_lag_bajo, bool soporta_mailbox) {
    // TODO
    return PresentMode::FIFO;   // stub: nunca MAILBOX
}

} // namespace

/* ------------------------------ TESTS (no tocar)-------------------------- */
TEST_CASE(SeleccionDeColasIgpu) {
    /* UHD 600 tipo: family 0 hace todo (gfx+compute+present); family 1
       es solo transfer (DMA). */
    const std::vector<QueueFamily> fams = {
        {Q_GRAPHICS | Q_COMPUTE | Q_TRANSFER, 1, true},
        {Q_TRANSFER,                          2, false},
    };
    const auto sel = elegirColas(fams);
    ASSERT_EQ(sel.grafica, 0);
    ASSERT_EQ(sel.compute, 0);   /* no hay familia dedicada: fallback */
    ASSERT_EQ(sel.present, 0);
}

TEST_CASE(SeleccionDeColasParticionada) {
    /* dGPU hipotética: gfx en 0, compute dedicado en 1, ninguna present
       (quiere decir: la surface no soporta present, caso raro pero
       posible); present queda en -1. */
    const std::vector<QueueFamily> fams = {
        {Q_GRAPHICS,            1, false},
        {Q_COMPUTE,             2, false},
        {Q_GRAPHICS|Q_TRANSFER, 1, false},
    };
    const auto sel = elegirColas(fams);
    ASSERT_EQ(sel.grafica, 0);
    ASSERT_EQ(sel.compute, 1);
    ASSERT_EQ(sel.present, -1);
}

TEST_CASE(ClampImageCount) {
    const SurfaceCaps c{3u, 8u};
    ASSERT_EQ(clampImageCount(c, 2u), 3u);    /* min aplica */
    ASSERT_EQ(clampImageCount(c, 4u), 4u);
    ASSERT_EQ(clampImageCount(c, 16u), 8u);   /* max aplica */
    /* max==0 = sin límite: 64 imágenes (¿triples en 21 monitores? ok). */
    ASSERT_EQ(clampImageCount(SurfaceCaps{1u, 0u}, 64u), 64u);
}

TEST_CASE(PresentModeFallback) {
    ASSERT_EQ(int(elegirPresent(true, true)), int(PresentMode::MAILBOX));
    ASSERT_EQ(int(elegirPresent(true, false)), int(PresentMode::FIFO));
    ASSERT_EQ(int(elegirPresent(false, true)), int(PresentMode::FIFO));
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
