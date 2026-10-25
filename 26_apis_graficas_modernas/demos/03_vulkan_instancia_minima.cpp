/* =============================================================================
   MÓDULO 26 - DEMO 3: Instancia mínima de Vulkan (cierre TEMA 8, inciso c)
   =============================================================================
   DEMO DE LECTURA: se COMPILO contra los Vulkan-Headers reales
   (tag v1.4.366, FetchContent en CMakeLists.txt del módulo) y se LINKEA
   contra libvulkan.so del sistema. NUNCA se ejecuta en verificar.sh.

   Es la parte del arranque que SDL_GPU le ahorra al desarrollador
   (lección 04 dice "no hay instance/physical device que armar a mano"):
   este demo muestra TODO lo que está detrás de esa abstracción.
   En esta máquina (vulkaninfo --summary, DECISIONES.md):
     - GPU 0: Intel UHD Graphics 600 (GLK 2), driver Mesa ANV,
       apiVersion Vulkan 1.4.354.
     - extensiones de surface presentes (VK_KHR_wayland_surface).
   ============================================================================= */
#include <vulkan/vulkan.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

namespace {

VKAPI_ATTR VkBool32 VKAPI_CALL
debug_cb(VkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT, std::uint64_t,
         std::size_t, std::int32_t, const char*, const char* mensaje,
         void*) {
    std::fprintf(stderr, "[vulkan] %s\n", mensaje);
    return VK_FALSE;
}
/* busco la queue family que soporta graphics (VK_QUEUE_GRAPHICS_BIT) */
std::uint32_t familia_grafica(const std::vector<VkQueueFamilyProperties>& q) {
    for (std::uint32_t i = 0; i < q.size(); ++i) {
        if ((q[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) return i;
    }
    return UINT32_MAX;
}

}  // namespace

int main() {
    std::puts("=== M26 demo 3: instancia minima de Vulkan ===");

    /* 1. app info + instance. La capa de validación ES SOLO DEBUG:
          en release no va (lección 02: instance es el "puente" al driver) */
    VkApplicationInfo app{};
    app.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app.pApplicationName   = "M26 demo 3";
    app.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app.pEngineName        = "motor_curso";
    app.apiVersion         = VK_API_VERSION_1_3;

    const char* capas[]        = {"VK_LAYER_KHRONOS_validation"};
    const char* extensiones[]  = {VK_EXT_DEBUG_REPORT_EXTENSION_NAME};

    VkInstanceCreateInfo ici{};
    ici.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ici.pApplicationInfo = &app;
    ici.enabledLayerCount = 1; ici.ppEnabledLayerNames = capas;
    ici.enabledExtensionCount = 1; ici.ppEnabledExtensionNames = extensiones;

    VkInstance instancia = VK_NULL_HANDLE;
    if (vkCreateInstance(&ici, nullptr, &instancia) != VK_SUCCESS) {
        std::fprintf(stderr, "vkCreateInstance fallo\n");
        return 1;
    }

    /* 1b. el callback de debug es una extension instance-level: se pide
       con vkGetInstanceProcAddr (el simbolo NO esta en libvulkan.so
       directamente, leccion 02). */
    auto vkCreateDebugReportCallback =
        reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
            vkGetInstanceProcAddr(instancia, "vkCreateDebugReportCallbackEXT"));
    if (vkCreateDebugReportCallback != nullptr) {
        VkDebugReportCallbackCreateInfoEXT dcbi{};
        dcbi.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        dcbi.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
                     VK_DEBUG_REPORT_WARNING_BIT_EXT;
        dcbi.pfnCallback = &debug_cb;
        VkDebugReportCallbackEXT cbDebug = VK_NULL_HANDLE;
        vkCreateDebugReportCallback(instancia, &dcbi, nullptr, &cbDebug);
    }

    /* 2. physical device: NO se crea, se consulta (el GPU ya existe). */
    std::uint32_t n_gpu = 0;
    vkEnumeratePhysicalDevices(instancia, &n_gpu, nullptr);
    if (n_gpu == 0) {
        std::fprintf(stderr, "sin GPUs con Vulkan\n");
        vkDestroyInstance(instancia, nullptr);
        return 1;
    }
    std::vector<VkPhysicalDevice> gpus(n_gpu);
    vkEnumeratePhysicalDevices(instancia, &n_gpu, gpus.data());

    VkPhysicalDevice elegido = VK_NULL_HANDLE;
    for (VkPhysicalDevice gpu : gpus) {
        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(gpu, &props);
        std::printf("GPU: %s (driver Vulkan 1.%u.%u)\n", props.deviceName,
                    VK_VERSION_MAJOR(props.apiVersion),
                    VK_VERSION_MINOR(props.apiVersion));
        if (elegido == VK_NULL_HANDLE) elegido = gpu;  /* el primero sirve */
    }

    /* 3. queue families: las colas son propiedad del GPU, se piden
          por familia y DESPUES se pide el device con esas familias. */
    std::uint32_t n_fam = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(elegido, &n_fam, nullptr);
    std::vector<VkQueueFamilyProperties> familias(n_fam);
    vkGetPhysicalDeviceQueueFamilyProperties(elegido, &n_fam, familias.data());

    const std::uint32_t graficas = familia_grafica(familias);
    if (graficas == UINT32_MAX) {
        std::fprintf(stderr, "sin familia con graphics\n");
        vkDestroyInstance(instancia, nullptr);
        return 1;
    }
    std::printf("familia graphics: %u (de %u)\n", graficas, n_fam);

    /* 4. device: la cola que pidamos AQUI es la misma que en el demo 1
          SDL esconde (SDL_GetGPUDeviceProperties la resume). */
    float prioridad = 1.0f;
    VkDeviceQueueCreateInfo qci{};
    qci.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qci.queueFamilyIndex = graficas;
    qci.queueCount       = 1;
    qci.pQueuePriorities = &prioridad;

    VkDeviceCreateInfo dci{};
    dci.sType              = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dci.queueCreateInfoCount = 1;
    dci.pQueueCreateInfos    = &qci;

    VkDevice device = VK_NULL_HANDLE;
    if (vkCreateDevice(elegido, &dci, nullptr, &device) != VK_SUCCESS) {
        std::fprintf(stderr, "vkCreateDevice fallo\n");
        vkDestroyInstance(instancia, nullptr);
        return 1;
    }

    /* 5. cola grafica real (y ya con esto se podrian crear command
          pools, swapchain, y armar el frame de la leccion 03) */
    VkQueue graphics_queue = VK_NULL_HANDLE;
    vkGetDeviceQueue(device, graficas, 0, &graphics_queue);
    std::printf("device listo, cola graphics: %p\n",
                static_cast<void*>(graphics_queue));

    vkDestroyDevice(device, nullptr);
    vkDestroyInstance(instancia, nullptr);
    return 0;
}
