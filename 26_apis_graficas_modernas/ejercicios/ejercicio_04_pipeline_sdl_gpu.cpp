/* =============================================================================
   MÓDULO 26 - EJERCICIO 04: SDL_GPU — formato de shaders por backend y
   validación de sample counts
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 04 (SDL_CreateGPUDevice formato de shader + creación del
   pipeline).

   CONSIGNA
   --------
   1. `formatosAceptados(b)`: bitfield Uint32 con la MISMA codificación
      del header SDL_gpu.h (SDL3 3.4.18):
        PRIVATE=1<<0, SPIRV=1<<1, DXBC=1<<2, DXIL=1<<3, MSL=1<<4,
        METALLIB=1<<5.
      Vulkan acepta SPIR-V; D3D12 acepta DXBC y DXIL; Metal acepta MSL y
      METALLIB (la wiki oficial: "Each format corresponds to a specific
      backend that accepts it").
   2. `aceptaShader(b, fmt)`: true si el bit correspondiente está entre
      los aceptados por ese backend.
   3. `samplesValidos(samples)`: solo 1, 2, 4 u 8 (potencias de dos que
      el driver expone), falsa cualquier otra cosa.
   4. `pipelineValida(b, fmt, samples)`: conjunción de acepta y samples.
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <cstdint>

namespace {

enum class Backend { Vulkan, D3D12, Metal };

enum : std::uint32_t {
    FMT_PRIVATE   = 1u << 0, FMT_SPIRV = 1u << 1, FMT_DXBC = 1u << 2,
    FMT_DXIL      = 1u << 3, FMT_MSL   = 1u << 4, FMT_METALLIB = 1u << 5,
};

std::uint32_t formatosAceptados(Backend b) {
    // TODO
    return 0;   // stub: ningún formato aceptado
}

bool aceptaShader(Backend b, std::uint32_t fmt) {
    // TODO
    return false;
}

bool samplesValidos(int samples) {
    // TODO
    return false;
}

bool pipelineValida(Backend b, std::uint32_t fmt, int samples) {
    // TODO
    return false;
}

} // namespace

/* ------------------------------ TESTS (no tocar)-------------------------- */
TEST_CASE(CodificacionDelBitfield) {
    /* Los valores congelados de SDL_gpu.h 3.4.18: no son a gusto */
    ASSERT_EQ(FMT_SPIRV,      2u);
    ASSERT_EQ(FMT_DXBC,       4u);
    ASSERT_EQ(FMT_DXIL,       8u);
    ASSERT_EQ(FMT_MSL,       16u);
    ASSERT_EQ(FMT_METALLIB,  32u);
}

TEST_CASE(CadaBackendSusFormatos) {
    ASSERT_EQ(formatosAceptados(Backend::Vulkan), FMT_SPIRV);
    ASSERT_EQ(formatosAceptados(Backend::D3D12), FMT_DXBC | FMT_DXIL);
    ASSERT_EQ(formatosAceptados(Backend::Metal), FMT_MSL | FMT_METALLIB);
}

TEST_CASE(AceptacionDeShaders) {
    ASSERT_TRUE(aceptaShader(Backend::Vulkan, FMT_SPIRV));
    ASSERT_FALSE(aceptaShader(Backend::Vulkan, FMT_DXIL));
    ASSERT_TRUE(aceptaShader(Backend::D3D12, FMT_DXBC));
    ASSERT_FALSE(aceptaShader(Backend::D3D12, FMT_SPIRV));
    ASSERT_TRUE(aceptaShader(Backend::Metal, FMT_MSL));
    ASSERT_FALSE(aceptaShader(Backend::Metal, FMT_DXBC));
}

TEST_CASE(SampleCounts) {
    ASSERT_TRUE(samplesValidos(1));
    ASSERT_TRUE(samplesValidos(2));
    ASSERT_TRUE(samplesValidos(4));
    ASSERT_TRUE(samplesValidos(8));
    ASSERT_FALSE(samplesValidos(3));
    ASSERT_FALSE(samplesValidos(16));  /* 16 NO está en la lista de SDL_GPU */
    ASSERT_FALSE(samplesValidos(0));
}

TEST_CASE(PipelineCompleta) {
    ASSERT_TRUE(pipelineValida(Backend::Vulkan, FMT_SPIRV, 1));
    ASSERT_TRUE(pipelineValida(Backend::Vulkan, FMT_SPIRV, 4));
    ASSERT_FALSE(pipelineValida(Backend::Vulkan, FMT_SPIRV, 3));
    ASSERT_FALSE(pipelineValida(Backend::Metal, FMT_SPIRV, 1));
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
