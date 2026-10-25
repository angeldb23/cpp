/* =============================================================================
   MÓDULO 26 - SOLUCIÓN 08: VMA — selección de memory type y budget
   =============================================================================
   La solución de ejercicio_08_vma_memoria.cpp. Este picker en miniatura
   es la regla central de vmaFindMemoryType: requerido en forma de
   máscara, preferido como puntaje (std::popcount) y -1 si nada cumple
   (vkAllocateMemory devolvería VK_ERROR_... y VMA lo reporta). En el
   motor del dueño (iGPU) el reto es el heap único DEVICE_LOCAL: casi
   todo es type 1 (BAR mappable) y el budget importa MÁS que en una
   placa dedicada, porque el heap es la RAM visible del sistema.
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <bit>          // std::popcount
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
    int mejor = -1;
    int puntaje_mejor = -1;
    for (std::size_t i = 0; i < types.size(); ++i) {
        /* requerido: TODAS las propiedades en ese type. */
        if ((types[i].flags & requerido) != requerido) continue;
        /* preferido: cuantos más bits matchean, mejor (VMA "tie-break"). */
        const int puntaje = std::popcount(types[i].flags & preferido);
        if (puntaje > puntaje_mejor) {
            puntaje_mejor = puntaje;
            mejor = static_cast<int>(i);
        }
    }
    return mejor;
}

struct BudgetHeap {
    std::uint64_t used_mb = 0;
    std::uint64_t budget_mb = 0;
    /* El 75% del budget: used <= budget*3/4. Sin floats para que el
       borde sea EXACTO: used*4 <= budget*3. */
    bool dentro() const { return used_mb * 4 <= budget_mb * 3; }
};

std::string motivoStaging(int lado_px) {
    const std::uint64_t bytes =
        std::uint64_t(lado_px) * lado_px * 4;   /* RGBA8 */
    return bytes <= 1048576ull ? "directo" : "staging";
}

} // namespace

/* ------------------------------ TESTS (no tocar)-------------------------- */
static const std::vector<MemoryType> kTypes = {
    {0, M_DEVICE_LOCAL},
    {0, M_DEVICE_LOCAL | M_HOST_VISIBLE},
    {0, M_HOST_VISIBLE | M_HOST_CACHED},
};

TEST_CASE(VertexQuedaEnDeviceLocal) {
    ASSERT_EQ(elegirMemoryType(kTypes, M_DEVICE_LOCAL, 0), 0);
}

TEST_CASE(UniformEligeLaBarMappable) {
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
    ASSERT_EQ(motivoStaging(512), "directo");
    ASSERT_EQ(motivoStaging(256), "directo");
    ASSERT_EQ(motivoStaging(513), "staging");
    ASSERT_EQ(motivoStaging(2048), "staging");
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
