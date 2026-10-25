/* =============================================================================
   MÓDULO 26 - SOLUCIÓN 04: SDL_GPU — formato de shaders por backend y
   validación de sample counts
   =============================================================================
   La solución de ejercicio_04_pipeline_sdl_gpu.cpp. El bitfield está
   congelado con los valores reales del SDL_gpu.h del sistema (3.4.18)
   y la wiki oficial SDL3; la lista de sample counts es el enum
   SDL_GPUSampleCount (1/2/4/8, sin 16). El motor del dueño usa
   Vulkan+SPIR-V: FMT_SPIRV=2 y samples 1 (el resto es para porte).
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
    switch (b) {
        case Backend::Vulkan: return FMT_SPIRV;
        case Backend::D3D12:  return FMT_DXBC | FMT_DXIL;
        case Backend::Metal:  return FMT_MSL | FMT_METALLIB;
    }
    return 0;
}

bool aceptaShader(Backend b, std::uint32_t fmt) {
    return (formatosAceptados(b) & fmt) != 0;
}

/* SDL_GPUSampleCount enum: 1, 2, 4 y 8 (no hay 16). */
bool samplesValidos(int samples) {
    return samples == 1 || samples == 2 || samples == 4 || samples == 8;
}

bool pipelineValida(Backend b, std::uint32_t fmt, int samples) {
    return aceptaShader(b, fmt) && samplesValidos(samples);
}

} // namespace

/* ------------------------------ TESTS (no tocar)-------------------------- */
TEST_CASE(CodificacionDelBitfield) {
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
    ASSERT_FALSE(samplesValidos(16));
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
