/* =============================================================================
   MÓDULO 26 - SOLUCIÓN 02: Vulkan — queue families, swapchain y present mode
   =============================================================================
   La solución de ejercicio_02_vulkan_swapchain.cpp. Estas tres reglas
   (elegir familia por capacidad, clampear imageCount con las surface
   capabilities, FIFO como fallback garantizado) son las que el
   demo_03_vulkan_instancia_minima.cpp usa con la API real: acá las
   testea la lógica SIN driver para poder fallar/afinar rápido.
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <algorithm>
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
    SeleccionColas sel;
    for (std::size_t i = 0; i < fams.size(); ++i) {
        const auto& f = fams[i];
        if (f.conteo_queues == 0) continue;
        const bool gfx = (f.flags & Q_GRAPHICS) != 0;
        /* graphics: la primera familia con el bit (independiente de */
        /* poder presentar: eso lo marca "present").                */
        if (gfx && sel.grafica < 0) sel.grafica = int(i);
        /* compute dedicado (async): familia CON compute SIN graphics. */
        if ((f.flags & Q_COMPUTE) != 0 && !gfx && sel.compute < 0)
            sel.compute = int(i);
        /* present: lo dice vkGetPhysicalDeviceSurfaceSupportKHR. */
        if (f.puede_present && sel.present < 0) sel.present = int(i);
    }
    /* sin familia dedicada: el compute igual corre en la gráfica. */
    if (sel.compute < 0) sel.compute = sel.grafica;
    return sel;
}

struct SurfaceCaps { std::uint32_t min_images, max_images; };

/* Clampeo del swapchain: los caps de la surface son leyes. */
std::uint32_t clampImageCount(const SurfaceCaps& c, std::uint32_t pedido) {
    const std::uint32_t max = c.max_images != 0 ? c.max_images : pedido;
    return std::clamp(pedido, c.min_images, max);
}

enum class PresentMode { IMMEDIATE, FIFO, FIFO_RELAXED, MAILBOX };

PresentMode elegirPresent(bool quiere_lag_bajo, bool soporta_mailbox) {
    if (quiere_lag_bajo && soporta_mailbox) return PresentMode::MAILBOX;
    return PresentMode::FIFO;   /* garantizado por la spec en todo driver */
}

} // namespace

/* ------------------------------ TESTS (no tocar)-------------------------- */
TEST_CASE(SeleccionDeColasIgpu) {
    const std::vector<QueueFamily> fams = {
        {Q_GRAPHICS | Q_COMPUTE | Q_TRANSFER, 1, true},
        {Q_TRANSFER,                          2, false},
    };
    const auto sel = elegirColas(fams);
    ASSERT_EQ(sel.grafica, 0);
    ASSERT_EQ(sel.compute, 0);
    ASSERT_EQ(sel.present, 0);
}

TEST_CASE(SeleccionDeColasParticionada) {
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
    ASSERT_EQ(clampImageCount(c, 2u), 3u);
    ASSERT_EQ(clampImageCount(c, 4u), 4u);
    ASSERT_EQ(clampImageCount(c, 16u), 8u);
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
