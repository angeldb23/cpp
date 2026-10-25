/* =============================================================================
   MÓDULO 26 - LECCIÓN 02: Vulkan parte 1 — instance, physical/logical device,
   queue families y swapchain
   =============================================================================
   TEMA 2 (parte 1): "Vulkan básico: instance, device, queue families,
   swapchain" — 2 lecciones (esta es la 1°).

   Vulkan es una API C donde nada está defaulteado. El arranque clásico:

     1. VkApplicationInfo    -> quién sos (apiVersion, engine, juego).
     2. VkInstanceCreateInfo -> capas (validation layers) + extensiones de
        instance. En desktop siempre: VK_KHR_surface más la del sistema de
        ventanas (VK_KHR_wayland_surface / VK_KHR_xcb_surface /
        VK_KHR_win32_surface).
     3. vkCreateInstance     -> el handle raíz de tu programa.
     4. vkEnumeratePhysicalDevices -> GPUs físicas; elegís una (en la
        máquina del dueño hay exactamente una: la UHD 600).
     5. vkGetPhysicalDeviceQueueFamilyProperties -> cada GPU trae FAMILIAS
        de colas con capacidades: GRAPHICS (0x1), COMPUTE (0x2),
        TRANSFER (0x4), SPARSE_BINDING (0x8; esos bits son los valores
        reales de vk_queue_flags de vulkan_core.h).
     6. vkCreateDevice (lógico) -> prendés features y extensiones por
        device, y pedís cuántas colas de cada familia vas a usar.
     7. Superficie (SDL_CreateVulkanSurface en SDL3) y swapchain con las
        surface capabilities: minImageCount/maxImageCount, formatos y
        present modes.

   PARA QUÉ SIRVE CADA ETAPA EN EL MOTOR
   -------------------------------------
   * instance: "quién es el proceso y qué extensión de ventanas pide".
   * physical device: "qué GPU elijo" (si hay híbrida: criterio tipo).
   * queue families: "qué trabajo puedo encolar en paralelo". Si render y
     compute comparten familia, el async compute igual tiene que
     sincronizarse con barreras/semáforos entre uno y otro; si hay una
     familia de solo TRANSFER, la usás como "DMA" para subir texturas
     sin pisar al render (LECCIÓN 03 y 09).
   * logical device: "qué features activo" (anisotropy, timeline
     semaphore, etc.).
   * swapchain: la fila de imágenes que presentás. Su minImageCount dicta
     si podés hacer triple buffering; el formato típico es B8G8R8A8 SRGB
     (si el surface lo soporta; el SRGB aplica el gamma al presentar).

   EN ESTA LECCIÓN: simulador HEADLESS de la secuencia. Elegir queue
   family, clampear imageCount y elegir present mode es exactamente la
   lógica que después vas a escribir contra la API real.
   ============================================================================= */
#include "include/f26.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <vector>

namespace {

/* Bits de capacidades (misma semántica que VK_QUEUE_*_BIT). */
constexpr std::uint32_t Q_GRAPHICS = 1u << 0;
constexpr std::uint32_t Q_COMPUTE  = 1u << 1;
constexpr std::uint32_t Q_TRANSFER = 1u << 2;

struct QueueFamily {
    std::uint32_t flags;          /* combinación de Q_* */
    std::uint32_t conteo_queues;  /* colas de ESTA familia */
    bool          puede_present;  /* lo reporta vkGetPhysicalDeviceSurfaceSupportKHR */
};

/* Regla real de selección: la familia de GRAPHICS se elige por sí sola
   (aunque no presente); la de COMPUTE ASYNC busca familia sin GRAPHICS
   (dedicada), con fallback a la gráfica; la de PRESENT es la primera
   que reporta surface support. En las iGPU con todo en la familia 0
   quedan los tres índices iguales. */
struct SeleccionColas { int grafica = -1; int compute = -1; int present = -1; };

SeleccionColas elegirColas(const std::vector<QueueFamily>& fams) {
    SeleccionColas sel;
    for (std::size_t i = 0; i < fams.size(); ++i) {
        const auto& f = fams[i];
        if (f.conteo_queues == 0) continue;
        const bool gfx = (f.flags & Q_GRAPHICS) != 0;
        if (gfx && sel.grafica < 0) sel.grafica = int(i);
        if ((f.flags & Q_COMPUTE) != 0 && !gfx && sel.compute < 0)
            sel.compute = int(i);          /* familia dedicada async */
        if (f.puede_present && sel.present < 0) sel.present = int(i);
    }
    if (sel.compute < 0) sel.compute = sel.grafica;
    return sel;
}

/* Swapchain: la surface impone [min, max] de imágenes y hay que clampear
   (min==max en algunas placas viejas; max==0 significa "sin límite"). */
struct SurfaceCaps { std::uint32_t min_images = 0, max_images = 0; };

std::uint32_t clampImageCount(const SurfaceCaps& c, std::uint32_t pedido) {
    return std::clamp(pedido, c.min_images,
                      c.max_images != 0 ? c.max_images : pedido);
}

/* Present modes: IMMEDIATE (tearing), FIFO (vsync, GARANTIZADO por la
   spec en todo driver), FIFO_RELAXED (vsync que saltera si vas atras),
   MAILBOX (triple buffering con descarte del frame viejo: el "low lag"
   de desktop). Regla obligatoria: MAILBOX NO está garantizado, o sea
   SIEMPRE tu segundo modo de fallback es FIFO. */
enum class PresentMode { IMMEDIATE, FIFO, FIFO_RELAXED, MAILBOX };

PresentMode elegirPresent(bool quiere_lag_bajo, bool soporta_mailbox) {
    if (quiere_lag_bajo && soporta_mailbox) return PresentMode::MAILBOX;
    return PresentMode::FIFO;
}

} // namespace

int main() {
    /* GPU integrada moderna (como la UHD 600 de Mesa ANV): la familia 0
       comparte graphics+compute+present; la familia 1 es solo transfer
       (el "DMA engine" para subir recursos). */
    const std::vector<QueueFamily> fams = {
        {Q_GRAPHICS | Q_COMPUTE | Q_TRANSFER, 1, true }, // family 0
        {Q_TRANSFER,                          2, false}, // family 1
    };
    const auto sel = elegirColas(fams);
    std::printf("familias: grafica=%d compute=%d present=%d\n",
                sel.grafica, sel.compute, sel.present);
    /* Ojo: como el compute "dedicado" pedía familia sin GRAPHICS y no
       existe ACA, el fallback devuelve la 0: en esta placa el async
       compute real no va a brillar (LECCIÓN 09 lo estudia igual). */

    const SurfaceCaps caps{3u, 8u};
    std::printf("imageCount: pedido=2 -> %u ; pedido=16 -> %u\n",
                clampImageCount(caps, 2u), clampImageCount(caps, 16u));
    std::printf("present: lag=si,mailbox=si -> %d | lag=si,mailbox=no -> %d\n",
                int(elegirPresent(true, true)), int(elegirPresent(true, false)));
    return 0;
}

/* ---------------- API REAL: extracto verificado -------------------------
   La sesión compiló y linkeó (NO ejecutó) la instancia mínima real contra
   Vulkan-Headers v1.4.366 por FetchContent: demos/
   demo_03_vulkan_instancia_minima.cpp. El resumen del arranque real:

     VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO};
     app.apiVersion = VK_API_VERSION_1_3;          // pedido mínimo al driver
     VkInstanceCreateInfo ci{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
     ci.pApplicationInfo = &app;
     ci.enabledExtensionCount   = 2;               // VK_KHR_surface + ventana
     ci.ppEnabledExtensionNames = extensiones;     // (wayland en el dueño)
     vkCreateInstance(&ci, nullptr, &instance);

   Las extensiones de instance de la MÁQUINA ya las listó vulkaninfo:
   VK_KHR_surface + VK_KHR_wayland_surface + VK_EXT_debug_utils (todas
   disponibles en el Mesa del dueño). Validation layers:
   "VK_LAYER_KHRONOS_validation" (no activa en vulkaninfo del dueño, que
   lista capas de Mesa; el dueño puede instalar vulkan-validation-layers).
   ------------------------------------------------------------------------ */
