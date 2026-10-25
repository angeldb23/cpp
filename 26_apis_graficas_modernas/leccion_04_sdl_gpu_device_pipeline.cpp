/* =============================================================================
   MÓDULO 26 - LECCIÓN 04: SDL_GPU parte 1 — device, ventana, swapchain y
   pipeline
   =============================================================================
   TEMA 3 (parte 1): "SDL_GPU: abstracción portable (Vulkan/Metal/DX12),
   SDL_GPUDevice, pipeline, buffers" — 2 lecciones (esta es la 1°).

   SDL_GPU reproduce el MISMO modelo que Vulkan (device, command buffer,
   pipeline explícita, transferencia de recursos) pero sin la puesta en
   escena: no hay instance/physical device/surface que armar a mano y SDL
   elige el backend por vos. Para un motor indie es el punto dulce de
   abstracción: control real con una fracción del código de arranque.

   EL DÍA A DÍA (nombres verificados en SDL_gpu.h del SDL3 3.4.18 del
   sistema; ese código exacto está compilado + linkeado en
   demos/demo_01_triangulo_sdl_gpu.cpp de este módulo):

     // creación (una vez):
     SDL_GPUDevice* dev = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV,
                                              true,   // debug_mode
                                              "vulkan");
     SDL_ClaimWindowForGPUDevice(dev, ventana);
     // por frame:
     SDL_GPUCommandBuffer* cmds = SDL_AcquireGPUCommandBuffer(dev);
     SDL_GPUTexture* swapchain; Uint32 w, h;
     SDL_WaitAndAcquireGPUSwapchainTexture(cmds, ventana, &swapchain, &w, &h);
     SDL_GPURenderPass* pass =
         SDL_BeginRenderPass(cmds, &color_target, 1, nullptr);
     SDL_BindGPUGraphicsPipeline(pass, pipeline);
     SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);
     SDL_EndRenderPass(pass);
     SDL_SubmitGPUCommandBuffer(cmds);

   HECHOS VERIFICADOS EN EL HEADER SDL_gpu.h (SDL3 3.4.18):
   * formato de shader = bitfield Uint32 con codificación exacta:
       PRIVATE=1<<0, SPIRV=1<<1, DXBC=1<<2, DXIL=1<<3, MSL=1<<4,
       METALLIB=1<<5. Mapeo oficial (header + wiki SDL3):
       SPIR-V solo Vulkan; DXBC (SM5.1)/DXIL (SM6.0) solo D3D12; MSL y
       METALLIB solo Metal. ("Each format corresponds to a specific
       backend that accepts it").
   * SDL_GPUShaderStage solo tiene VERTEX y FRAGMENT: no hay stages de
     geometry ni tesselation; el compute es un pass aparte
     (SDL_BeginGPUComputePass).
   * SDL_GPUSwapchainComposition: SDR, SDR_LINEAR, HDR_EXTENDED_LINEAR,
     HDR10_ST2084, consultable con SDL_WindowSupportsGPUSwapchainComposition.
     En la UHD 600 del dueño la práctica es SDR.
   * SDL_GPUPrimitiveType incluye TRIANGLELIST, LINESTRIP, POINTLIST, ...
   * firma verificada: SDL_CreateGPUDevice(format_flags, debug_mode, name)
     y SDL_GPUSupportsShaderFormats(format_flags, name) para chequear
     ANTES de crear si el sistema tiene un backend que coma esos shaders.

   EN ESTA LECCIÓN: simulador de LA CREACIÓN (el costo fijo de L01):
   validar backend vs formato de shader y sample counts de MSAA son las
   MISMAS reglas que application le driver, y son el contenido del
   ejercicio 04. El frame paso a paso está en la LECCIÓN 05 (triángulo y
   quad texturizado).
   ============================================================================= */
#include "include/f26.hpp"

#include <cstdint>
#include <cstdio>
#include <string>

namespace {

enum class Backend { Vulkan, D3D12, Metal };

const char* nombre_backend(Backend b) {
    switch (b) {
        case Backend::Vulkan: return "vulkan";
        case Backend::D3D12:  return "d3d12";
        case Backend::Metal:  return "metal";
    }
    return "?";
}

/* Bitfield con la MISMA codificación del header SDL_gpu.h (3.4.18). */
enum : std::uint32_t {
    FMT_PRIVATE   = 1u << 0, FMT_SPIRV = 1u << 1, FMT_DXBC = 1u << 2,
    FMT_DXIL      = 1u << 3, FMT_MSL   = 1u << 4, FMT_METALLIB = 1u << 5,
};

/* Formatos que cada backend acepta (comentarios oficiales del header:
   Vulkan requiere Vulkan 1.0 + extensiones; D3D12 requiere SM6; Metal
   requiere macOS). */
std::uint32_t formatos_backend(Backend b) {
    switch (b) {
        case Backend::Vulkan: return FMT_SPIRV;
        case Backend::D3D12:  return FMT_DXBC | FMT_DXIL;
        case Backend::Metal:  return FMT_MSL | FMT_METALLIB;
    }
    return 0;
}

struct Resultado { bool ok = false; std::string error; };

/* La creación de una graphics pipeline concentra TODO el estado del
   render en un solo objeto (topología, MSAA, depth, shaders). Las dos
   validaciones reales que simulamos para los tests del 04:
   1) el formato de shader tiene que estar entre los que acepta el
      backend elegido en la CREACIÓN del device;
   2) los sample counts de MSAA habilitados son 1/2/4/8 (las potencias
      de dos que el driver expone y valida al crear la pipeline). */
Resultado crearPipeline(Backend b, const f26::PipelineSpec& spec) {
    const std::uint32_t fmts = formatos_backend(b);
    const bool fmt_ok =
        (spec.shaderFmt == "SPIR-V" && (fmts & FMT_SPIRV) != 0) ||
        (spec.shaderFmt == "DXIL"   && (fmts & FMT_DXIL)  != 0) ||
        (spec.shaderFmt == "MSL"    && (fmts & FMT_MSL)   != 0);
    if (!fmt_ok) {
        return {false,
                "formato no soportado por " + std::string(nombre_backend(b))};
    }
    const bool samples_ok = spec.samples == 1 || spec.samples == 2 ||
                            spec.samples == 4 || spec.samples == 8;
    if (!samples_ok) return {false, "samples MSAA no válido"};
    return {true, ""};
}

} // namespace

int main() {
    /* El caso del motor del dueño: SPIR-V compilado con glslangValidator
       (LECCIÓN 07) y backend vulkan (la UHD 600 tiene ANV, ver
       vulkaninfo en DECISIONES). Metal no existe en Linux: falla de
       fábrica SDL, no un bug. */
    const f26::PipelineSpec triangulos{f26::Topologia::Triangulos, 1, true, false, "SPIR-V"};
    const f26::PipelineSpec msaa8     {f26::Topologia::Triangulos, 8, true, false, "SPIR-V"};
    const f26::PipelineSpec msaa3     {f26::Topologia::Triangulos, 3, true, false, "SPIR-V"};

    for (Backend b : {Backend::Vulkan, Backend::D3D12, Backend::Metal}) {
        const auto r = crearPipeline(b, triangulos);
        std::printf("backend=%-7s SPIR-V -> %s\n", nombre_backend(b),
                    r.ok ? "OK" : r.error.c_str());
    }
    const auto r2 = crearPipeline(Backend::Vulkan, msaa8);
    const auto r3 = crearPipeline(Backend::Vulkan, msaa3);
    std::printf("MSAA x8: %s | MSAA x3: %s\n",
                r2.ok ? "OK" : r2.error.c_str(),
                r3.ok ? "OK" : r3.error.c_str());
    return 0;
}
