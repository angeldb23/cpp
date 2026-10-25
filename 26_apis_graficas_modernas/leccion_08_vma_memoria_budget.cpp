/* =============================================================================
   MÓDULO 26 - LECCIÓN 08: Vulkan Memory Allocator (VMA) — tipos de memoria,
   heaps, budget y persistent mapping
   =============================================================================
   TEMA 6: "Memory management: Vma (Vulkan Memory Allocator), budget, memory
   types y persistent mapping" — 1 lección.

   POR QUÉ EXISTE VMA
   -------------------
   En Vulkan la memoria NO es transparente: cada VkBuffer/VkImage hay
   que BINDEARLO a un VkDeviceMemory elegido de una lista de
   memoryTypeIndex que el device físico reporta
   (vkGetPhysicalDeviceMemoryProperties). Hacerlo a mano termina en
   errores típicos: elegir "HOST_VISIBLE sin DEVICE_LOCAL" para todo
   (lento) o quedarnos 32 allocs VMA-S (los maxMemoryAllocationCount se
   agotan). VMA es la librería de referencia AMD (repo
   GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator, tag v3.4.0 verificado
   con git ls-remote en esta sesión) y encapsula:
     * suballocation: pide BLOQUES grandes y reparte adentro;
     * selección de tipo por flags (DEVICE_LOCAL, HOST_ACCESS_*...);
     * budget: vmaGetBudget lee el uso real del driver + el heap total;
     * defragmentación opcional para consolidar espacios sueltos.

   LOS TIPOS DE MEMORIA (conceptos que NO cambian)
   -----------------------------------------------
   vkGetPhysicalDeviceMemoryProperties te da dos listas: HEAPS (qué
   memoria hay, en bytes y con flags como DEVICE_LOCAL=0x1) y TYPES
   (qué propiedades ese heap cumple: HOST_VISIBLE=0x2, HOST_COHERENT=
   0x4, HOST_CACHED=0x8, LAZILY_ALLOCATED=0x10). En las iGPU el heap
   DEVICE_LOCAL es la RAM compartida (no hay VRAM separada y aun así
   es la memoria "rápida" del GPU).

   REGLAS DE ASIGNACIÓN (las que testea el ejercicio 08):
   1. Vertex/index buffers grandes: DEVICE_LOCAL, subido desde un
      staging HOST_VISIBLE (el camino del transfer buffer de la L05).
   2. Uniform per-mesh actualizable cada frame: decidir si conviene
      DEVICE_LOCAL + staging, o persistent mapping directo (iGPU: la
      RAM compartida ya es DEVICE_LOCAL, se saltea el staging).
   3. HOST_VISIBLE + HOST_CACHED: RAM del CPU-side "no hay coherencia
      SI el buffers de write complementario": el staging común.
   4. Persistent mapping (HOST_VISIBLE, mapped UNA vez y olvidado):
      el modo estándar para uniforms/vertex dinámicos con HOST_COHERENT
      (o HOST_CACHED + flush manual): la "double buffered" del ring
      in-flight de la L03 evita que el GPU lea lo que la CPU escribe.

   VMA EN LA PRÁCTICA (target real del módulo):
     #include <vk_mem_alloc.h>      // una TU: #define VMA_IMPLEMENTATION
     VmaAllocatorCreateInfo ci{...physicalDevice, device, instance...};
     vmaCreateAllocator(&ci, &alloc);            // una vez
     vmaCreateBuffer(alloc, &bufferInfo, &allocInfo,   // flags de hints,
                     &buffer, &memoria, &info_out);
   El allocInfo.usages es donde VMA mete sus hints bajo la mesa:
     VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT (staging),
     VMA_ALLOCATION_CREATE_DEVICE_LOCAL_BIT, ...
   El budget (vmaGetBudget) reporta, por heap, usage+allocados ONEL
   vs budget recomendado: es lo que el EJERCICIO 08 simula con el
   tracker "¿sigue en la rampa segura?".
   ============================================================================= */
#include "include/f26.hpp"

#include <bit>
#include <cstdint>
#include <cstdio>
#include <vector>

namespace {

/* Bits de properties (valores reales de vulkan_core.h). */
constexpr std::uint32_t M_DEVICE_LOCAL = 0x1, M_HOST_VISIBLE = 0x2,
                        M_HOST_COHERENT = 0x4, M_HOST_CACHED = 0x8,
                        M_LAZY = 0x10;

struct MemoryHeap { std::uint64_t size_mb; std::uint32_t flags; };
struct MemoryType { std::uint32_t heap_index; std::uint32_t flags; };

/* El picker de VMA en miniatura:
     requerido: el type TIENE que tener TODOS esos bits;
     preferido: puntúa los bits extra que matchean (el mejor gana);
     devuelve -1 si ningún type cumple (error real de Vulkan). */
int elegirMemoryType(const std::vector<MemoryType>& types,
                     const std::uint32_t requerido,
                     const std::uint32_t preferido) {
    int mejor = -1;
    int puntaje_mejor = -1;
    for (std::size_t i = 0; i < types.size(); ++i) {
        if ((types[i].flags & requerido) != requerido) continue;
        const int puntaje = std::popcount(types[i].flags & preferido);
        if (puntaje > puntaje_mejor) { puntaje_mejor = puntaje; mejor = int(i); }
    }
    return mejor;
}

/* Tracker de budget de juguete: usage / budget->  recomendación VMA
   (no gastes más del ~75% del heap para "reservar" para el driver). */
struct BudgetHeap {
    std::uint64_t used_mb = 0;
    std::uint64_t budget_mb = 0;
    bool dentro() const { return used_mb * 4 <= budget_mb * 3; }
};

} // namespace

int main() {
    /* iGPU tipo UHD 600: un heap DEVICE_LOCAL (la RAM compartida) de
       4 GB; los types ordenados cómo los ordering vk: */
    const std::vector<MemoryHeap> heaps = {{4096, M_DEVICE_LOCAL}};
    (void)heaps;
    const std::vector<MemoryType> types = {
        {0, M_DEVICE_LOCAL},                        /* VRAM exclusiva   */
        {0, M_DEVICE_LOCAL | M_HOST_VISIBLE},       /* BAR (mappable)   */
        {0, M_HOST_VISIBLE | M_HOST_CACHED},        /* RAM staging      */
    };

    std::printf("vertex    (DEVICE_LOCAL):                type=%d\n",
                elegirMemoryType(types, M_DEVICE_LOCAL, 0));
    std::printf("uniform   (HOST_VISIBLE + pref LOCAL):  type=%d\n",
                elegirMemoryType(types, M_HOST_VISIBLE, M_DEVICE_LOCAL));
    std::printf("staging   (HOST_VISIBLE + pref CACHED): type=%d\n",
                elegirMemoryType(types, M_HOST_VISIBLE, M_HOST_CACHED));
    std::printf("imposible (LAZY|HOST_VISIBLE):          type=%d\n",
                elegirMemoryType(types, M_LAZY | M_HOST_VISIBLE, 0));

    BudgetHeap b{2500, 4096};
    std::printf("budget: %llu/%llu MB -> %s\n",
                static_cast<unsigned long long>(b.used_mb),
                static_cast<unsigned long long>(b.budget_mb),
                b.dentro() ? "DENTRO" : "SOBRE (se frena el alloc)");
    return 0;
}

