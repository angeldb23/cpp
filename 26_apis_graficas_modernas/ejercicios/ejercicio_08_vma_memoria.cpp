/* =============================================================================
   MÓDULO 26 - EJERCICIO 08: VMA — selección de memory type y budget
   =============================================================================
   DIFICULTAD: Media-difícil
   APOYO: Lección 08 (Vulkan Memory Allocator: tipos de memoria, heaps,
   budget y persistent mapping).

   CONSIGNA
   --------
   1. `elegirMemoryType(types, requerido, preferido)`: devuelve el INDICE
      del primer type que tenga TODOS los bits `requerido`; entre
      empates gana el que matchee más bits del `preferido` (la
      heurística de VMA). Ninguno cumple -> -1 (error real de Vulkan).
   2. `BudgetHeap::dentro()`: uso <= 75% del budget recomendado por
      vmaGetBudget (el driver se reserva lo demás). Match exacto del
      borde: used*4 <= budget*3 ES "dentro".
   3. `motivoStaging(lado_px)`: para subir una textura RGBA8 score:
      bytes = lado*lado*4; SI los bytes <= 1 MiB -> "directo", si no
      -> "staging" (transfer buffer grande).
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace {

constexpr std::uint32_t M_DEVICE_LOCAL  = 0x1;
constexpr std::uint32_t M_HOST_VISIBLE  = 0x2;
constexpr std::uint32_t M_HOST_CACHED   = 0x8;
constexpr std::uint32_t M_LAZY          = 0x10;

struct MemoryType { std::uint32_t heap_index; std::uint32_t flags; };

int elegirMemoryType(const std::vector<MemoryType>& types,
                     std::uint32_t requerido, std::uint32_t preferido) {
    // TODO
    return -2;   // stub distinto del -1 real: los tests lo notan
}

struct BudgetHeap {
    std::uint64_t used_mb = 0;
    std::uint64_t budget_mb = 0;
    bool dentro() const {
        // TODO: 75% del budget (borde inclusive)
        return false;
    }
};

std::string motivoStaging(int lado_px) {
    // TODO: "directo" si <= 1 MiB (1048576), si no "staging"
    return "";
}

} // namespace

/* ------------------------------ TESTS (no tocar)-------------------------- */
/* Matriz típica de una iGPU: DEVICE_LOCAL exclusivo, BAR mappable y
   staging HOST_VISIBLE+HOST_CACHED. */
static const std::vector<MemoryType> kTypes = {
    {0, M_DEVICE_LOCAL},
    {0, M_DEVICE_LOCAL | M_HOST_VISIBLE},
    {0, M_HOST_VISIBLE | M_HOST_CACHED},
};

TEST_CASE(VertexQuedaEnDeviceLocal) {
    ASSERT_EQ(elegirMemoryType(kTypes, M_DEVICE_LOCAL, 0), 0);
}

TEST_CASE(UniformEligeLaBarMappable) {
    /* requerido HOST_VISIBLE, preferido DEVICE_LOCAL: type=1 (BAR) */
    ASSERT_EQ(elegirMemoryType(kTypes, M_HOST_VISIBLE, M_DEVICE_LOCAL), 1);
}

TEST_CASE(StagingEligeHostCached) {
    ASSERT_EQ(elegirMemoryType(kTypes, M_HOST_VISIBLE, M_HOST_CACHED), 2);
}

TEST_CASE(CombinacionImposible) {
    ASSERT_EQ(elegirMemoryType(kTypes, M_LAZY | M_HOST_VISIBLE, 0), -1);
}

TEST_CASE(PresupuestoAlBorde) {
    /* 75% exacto -> dentro; un mega más -> fuera */
    const BudgetHeap b1{3072u, 4096u};
    ASSERT_TRUE(b1.dentro());
    const BudgetHeap b2{3073u, 4096u};
    ASSERT_FALSE(b2.dentro());
    const BudgetHeap b0{0u, 4096u};
    ASSERT_TRUE(b0.dentro());
    const BudgetHeap b3{750u, 1000u};
    ASSERT_TRUE(b3.dentro());
    const BudgetHeap b4{751u, 1000u};
    ASSERT_FALSE(b4.dentro());
}

TEST_CASE(MotivoDeStaging) {
    ASSERT_EQ(motivoStaging(512), "directo");   /* 512*512*4 = 1MiB: borde */
    ASSERT_EQ(motivoStaging(256), "directo");
    ASSERT_EQ(motivoStaging(513), "staging");
    ASSERT_EQ(motivoStaging(2048), "staging");
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
