/* =============================================================================
   MÓDULO 26 - DEMO 1: Triángulo con SDL_GPU (cierre TEMA 8, inciso a)
   =============================================================================
   DEMO DE LECTURA: se COMPILO + LINKEA (g++ y via CMake target
   `26_demo_01_triangulo`) contra el SDL3 3.4.18 del sistema (con
   /usr/include/SDL3/SDL_gpu.h real) pero NUNCA se ejecuta en la
   verificación del módulo: los tests del curso son headless y los demos
   gráficos no abren ventanas en `verificar.sh`.

   Corresponde a la lección 04/05: device, swapchain, pipeline, command
   buffer, render pass y draw. Es el MISMO flujo de la lección, con las
   structs reales de SDL_gpu.h (nombres verificados CONTRA EL HEADER DEL
   SISTEMA: SDL_GPUColorTargetInfo, SDL_BeginGPURenderPass,
   SDL_ReleaseWindowFromGPUDevice, SDL_LoadFile_IO).

   Compilación (desde la carpeta del módulo):
     g++ -std=c++20 -c demos/01_triangulo_sdl_gpu.cpp \
         $(pkg-config --cflags sdl3)
   ============================================================================= */
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <cstdio>
#include <vector>

namespace {

/* Shaders pre-compilados: los generamos ANTES con glslangValidator
   (lección 07):
     glslangValidator -V -S vert -o shaders/tri.vert.spirv shaders/tri.vert
     glslangValidator -V -S frag -o shaders/tri.frag.spirv shaders/tri.frag
   En esta máquina el driver es Vulkan (ANV) => formato SPIR-V (bit 1<<1,
   lección 04). Metal querría MSL, D3D12 DXIL: por eso SDL pide UN
   formato de shader al crear el device. */
SDL_GPUShader* cargar_shader(SDL_GPUDevice* dev, const char* ruta,
                             SDL_GPUShaderStage stage) {
    SDL_IOStream* io = SDL_IOFromFile(ruta, "rb");
    if (io == nullptr) {
        std::fprintf(stderr, "falta %s (correr glslangValidator antes)\n", ruta);
        return nullptr;
    }
    std::size_t tam = 0;
    /* SDL3: SDL_LoadFile_IO devuelve void* crudo; SPIR-V es un blob de
       codigo, sin estructura interpretable por CPU (leccion 07). */
    Uint8* bytes = static_cast<Uint8*>(SDL_LoadFile_IO(io, &tam, /*closeio=*/true));
    if (bytes == nullptr) return nullptr;

    SDL_GPUShaderCreateInfo info{};
    info.code       = bytes;
    info.code_size  = tam;
    info.entrypoint = "main";
    info.format     = SDL_GPU_SHADERFORMAT_SPIRV;
    info.stage      = stage;
    /* el triangulo no usa samplers ni uniform blocks */
    info.num_samplers          = 0;
    info.num_storage_buffers   = 0;
    info.num_storage_textures  = 0;
    info.num_uniform_buffers   = 0;
    SDL_GPUShader* sh = SDL_CreateGPUShader(dev, &info);
    SDL_free(bytes);
    return sh;
}

}  // namespace

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* ventana = SDL_CreateWindow("M26 demo 1: triangulo SDL_GPU",
                                           640, 480, SDL_WINDOW_RESIZABLE);
    if (ventana == nullptr) {
        std::fprintf(stderr, "CreateWindow: %s\n", SDL_GetError());
        return 1;
    }

    /* 1. device: SPIR-V porque aca el backend va a ser Vulkan (ANV).
       debug_mode=true activa capa de validacion/depuracion. */
    SDL_GPUDevice* dev = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV,
                                             true, "vulkan");
    if (dev == nullptr) {
        std::fprintf(stderr, "CreateGPUDevice: %s\n", SDL_GetError());
        return 1;
    }
    if (!SDL_ClaimWindowForGPUDevice(dev, ventana)) {
        std::fprintf(stderr, "ClaimWindow: %s\n", SDL_GetError());
        return 1;
    }

    /* 2. shaders + pipeline (estados fijos: la leccion 04 explica que el
       pipeline agrupa TODO lo mutable de GL en objetos inmutables) */
    SDL_GPUShader* vs = cargar_shader(dev, "shaders/tri.vert.spirv",
                                      SDL_GPU_SHADERSTAGE_VERTEX);
    SDL_GPUShader* fs = cargar_shader(dev, "shaders/tri.frag.spirv",
                                      SDL_GPU_SHADERSTAGE_FRAGMENT);
    if (vs == nullptr || fs == nullptr) {
        if (vs != nullptr) SDL_ReleaseGPUShader(dev, vs);
        if (fs != nullptr) SDL_ReleaseGPUShader(dev, fs);
        SDL_DestroyGPUDevice(dev);
        SDL_DestroyWindow(ventana);
        SDL_Quit();
        return 1;
    }

    SDL_GPUColorTargetDescription ctd{};
    ctd.format = SDL_GetGPUSwapchainTextureFormat(dev, ventana);
    ctd.blend_state.enable_blend = false; /* opaco para el demo */

    SDL_GPUGraphicsPipelineTargetInfo targets{};
    targets.color_target_descriptions = &ctd;
    targets.num_color_targets         = 1;

    SDL_GPUGraphicsPipelineCreateInfo pi{};
    pi.vertex_shader   = vs;
    pi.fragment_shader = fs;
    /* sin vertex buffers: el tri.vert clasico usa gl_VertexIndex para
       generar la posicion y el color DENTRO del shader (el demo 2
       agrega el VBO real de un quad) */
    pi.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
    pi.target_info    = targets;

    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(dev, &pi);
    if (pipeline == nullptr) {
        SDL_ReleaseGPUShader(dev, vs);
        SDL_ReleaseGPUShader(dev, fs);
        std::fprintf(stderr, "pipeline: %s\n", SDL_GetError());
        return 1;
    }

    /* 3. loop: acquire -> swapchain -> begin pass -> bind -> draw -> end
       -> submit. CERO reinicio de estado por frame (a diferencia de GL:
       el pipeline ya tiene todo, se "enciende" y dibuja) */
    bool corriendo = true;
    while (corriendo) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) corriendo = false;
        }

        SDL_GPUCommandBuffer* cmds = SDL_AcquireGPUCommandBuffer(dev);
        if (cmds == nullptr) break;

        SDL_GPUTexture* swapchain = nullptr;
        Uint32 w = 0, h = 0;
        if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmds, ventana,
                                                   &swapchain, &w, &h)) {
            SDL_SubmitGPUCommandBuffer(cmds);
            continue;
        }

        SDL_GPUColorTargetInfo color{};
        color.texture     = swapchain;
        color.load_op     = SDL_GPU_LOADOP_CLEAR;
        color.store_op    = SDL_GPU_STOREOP_STORE;
        color.clear_color = {0.09f, 0.10f, 0.13f, 1.0f};

        SDL_GPURenderPass* pass =
            SDL_BeginGPURenderPass(cmds, &color, 1, nullptr);
        SDL_BindGPUGraphicsPipeline(pass, pipeline);
        SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);
        SDL_EndGPURenderPass(pass);

        SDL_SubmitGPUCommandBuffer(cmds);
    }

    SDL_ReleaseGPUGraphicsPipeline(dev, pipeline);
    SDL_ReleaseGPUShader(dev, fs);
    SDL_ReleaseGPUShader(dev, vs);
    SDL_ReleaseWindowFromGPUDevice(dev, ventana);
    SDL_DestroyGPUDevice(dev);
    SDL_DestroyWindow(ventana);
    SDL_Quit();
    return 0;
}
