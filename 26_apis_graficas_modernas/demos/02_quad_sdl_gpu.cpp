/* =============================================================================
   MÓDULO 26 - DEMO 2: Quad texturizado con SDL_GPU (cierre TEMA 8, inciso b)
   =============================================================================
   DEMO DE LECTURA: se COMPILO + LINKEA contra SDL3 del sistema, igual que
   el demo 1. Extiende el triángulo con los tres ingredientes que faltan
   para un sprite real:
     1. buffer de vértices SUBIDO por upload pass (lección 05).
     2. textura 2x2 generada por CPU + sampler LINEAR (lección 05).
     3. bindings de fragmento: 1 sampler + 1 texture (lección 05/06).

   Flujo que suma respecto al demo 1:
     - SDL_GPUBuffer + SDL_GPUTransferBuffer: primero se pide una zona
       de CPU (transfer), se mapea, se copian los datos, y un CopyPass
       los mueve al buffer/device definitivo (VMA de Vulkan, pero SDL
       lo gestiona solo).
     - SDL_CreateGPUTexture con tipo SDL_GPU_TEXTURETYPE_2D y uso
       SAMPLER; luego SDL_BindGPUFragmentSamplers con la textura.

   Compilación (desde la carpeta del módulo):
     g++ -std=c++20 -c demos/02_quad_sdl_gpu.cpp -I. $(pkg-config --cflags sdl3)
   ============================================================================= */
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace {

struct Vec2 { float x, y; };
struct Vec4 { float x, y, z, w; };

/* pos (x,y) + uv: el quad necesita 4 vértices únicos y 6 índices (dos
   triángulos). Triángulos menores de 16 px: 66% más de fragment shaders
   desperdiciados --- por eso en el motor real hay quads-in-atlas
   (lección 05, costos). */
constexpr Vec2 POS[4] = {
    {-0.5f, -0.5f}, {+0.5f, -0.5f}, {+0.5f, +0.5f}, {-0.5f, +0.5f},
};
constexpr Vec2 UV[4] = {
    {0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f},
};
constexpr std::uint16_t IDX[6] = {0, 1, 2, 2, 3, 0};

struct Vertex { Vec2 pos; Vec2 uv; };

}  // namespace

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) { return 1; }
    SDL_Window* ventana = SDL_CreateWindow("M26 demo 2: quad SDL_GPU",
                                           640, 480, 0);
    SDL_GPUDevice* dev = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV,
                                             true, "vulkan");
    if (ventana == nullptr || dev == nullptr ||
        !SDL_ClaimWindowForGPUDevice(dev, ventana)) {
        std::fprintf(stderr, "%s\n", SDL_GetError());
        return 1;
    }

    /* ---------- (1) buffer de vértices via transfer buffer ---------- */
    const std::vector<Vertex> verts{
        {POS[0], UV[0]}, {POS[1], UV[1]}, {POS[2], UV[2]}, {POS[3], UV[3]},
    };
    SDL_GPUTransferBufferCreateInfo tbci{};
    tbci.usage  = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    tbci.size   = static_cast<Uint32>(sizeof(Vertex) * verts.size());
    SDL_GPUTransferBuffer* transfer = SDL_CreateGPUTransferBuffer(dev, &tbci);

    void* mapa = SDL_MapGPUTransferBuffer(dev, transfer, false);
    SDL_memcpy(mapa, verts.data(), sizeof(Vertex) * verts.size());
    SDL_UnmapGPUTransferBuffer(dev, transfer);

    SDL_GPUBufferCreateInfo bci{};
    bci.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    bci.size  = static_cast<Uint32>(sizeof(Vertex) * verts.size());
    SDL_GPUBuffer* vbo = SDL_CreateGPUBuffer(dev, &bci);

    SDL_GPUCommandBuffer* carga = SDL_AcquireGPUCommandBuffer(dev);
    SDL_GPUCopyPass* cp = SDL_BeginGPUCopyPass(carga);
    SDL_GPUTransferBufferLocation src{transfer, 0};
    SDL_GPUBufferRegion dst{vbo, 0, tbci.size};
    SDL_UploadToGPUBuffer(cp, &src, &dst, false);
    SDL_EndGPUCopyPass(cp);
    SDL_SubmitGPUCommandBuffer(carga);
    SDL_ReleaseGPUTransferBuffer(dev, transfer);  /* ya copiado: va al GC */

    /* ---------- (2) textura 2x2 + sampler ---------- */
    SDL_GPUTextureCreateInfo tci{};
    tci.type                 = SDL_GPU_TEXTURETYPE_2D;
    tci.width                = 2;
    tci.height               = 2;
    tci.layer_count_or_depth = 1;
    tci.num_levels           = 1;
    tci.sample_count         = SDL_GPU_SAMPLECOUNT_1;  /* solo 1/2/4/8 */
    tci.format               = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    tci.usage                = SDL_GPU_TEXTUREUSAGE_SAMPLER;
    SDL_GPUTexture* tex = SDL_CreateGPUTexture(dev, &tci);

    SDL_GPUSamplerCreateInfo sci{};
    sci.min_filter    = SDL_GPU_FILTER_LINEAR;
    sci.mag_filter    = SDL_GPU_FILTER_LINEAR;
    sci.mipmap_mode   = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    sci.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    sci.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    sci.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    SDL_GPUSampler* sampler = SDL_CreateGPUSampler(dev, &sci);

    /* (el upload real de la textura iría en otro CopyPass con
        SDL_UploadToGPUTexture; el demo se centra en el binding) */

    /* ---------- (3) pipeline: shaders de lección 05 ---------- */
    SDL_GPUGraphicsPipeline* pipeline = nullptr;
    {
        SDL_IOStream* io_v = SDL_IOFromFile("shaders/quad.vert.spirv", "rb");
        SDL_IOStream* io_f = SDL_IOFromFile("shaders/quad.frag.spirv", "rb");
        std::size_t tv = 0, tf = 0;
        Uint8* cv = reinterpret_cast<Uint8*>(SDL_LoadFile_IO(io_v, &tv, false));
        Uint8* cf = reinterpret_cast<Uint8*>(SDL_LoadFile_IO(io_f, &tf, false));
        if (cv == nullptr || cf == nullptr) {
            std::fprintf(stderr, "faltan shaders (glslangValidator antes)\n");
            return 1;
        }
        SDL_GPUShaderCreateInfo vinfo{};
        vinfo.code = cv; vinfo.code_size = tv; vinfo.entrypoint = "main";
        vinfo.format = SDL_GPU_SHADERFORMAT_SPIRV;
        vinfo.stage  = SDL_GPU_SHADERSTAGE_VERTEX;
        SDL_GPUShaderCreateInfo finfo = vinfo;
        finfo.stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
        /* el frag declara: 1 uniform block + 1 sampler + 1 texture */
        finfo.num_samplers = 1;
        finfo.num_uniform_buffers = 1;

        SDL_GPUShader* vs = SDL_CreateGPUShader(dev, &vinfo);
        SDL_GPUShader* fs = SDL_CreateGPUShader(dev, &finfo);
        SDL_free(cv); SDL_free(cf);
        if (vs == nullptr || fs == nullptr) { return 1; }

        SDL_GPUVertexBufferDescription vb{};
        vb.slot = 0; vb.pitch = sizeof(Vertex);
        vb.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
        SDL_GPUVertexAttribute attr[2]{};
        attr[0].buffer_slot = 0; attr[0].location = 0; attr[0].offset = 0;
        attr[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
        attr[1].buffer_slot = 0; attr[1].location = 1;
        attr[1].offset = sizeof(Vec2);
        attr[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;

        SDL_GPUColorTargetDescription ctd{};
        ctd.format = SDL_GetGPUSwapchainTextureFormat(dev, ventana);
        SDL_GPUGraphicsPipelineCreateInfo pi{};
        pi.vertex_shader = vs; pi.fragment_shader = fs;
        pi.vertex_input_state.num_vertex_buffers = 1;
        pi.vertex_input_state.vertex_buffer_descriptions = &vb;
        pi.vertex_input_state.num_vertex_attributes = 2;
        pi.vertex_input_state.vertex_attributes = attr;
        pi.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
        SDL_GPUGraphicsPipelineTargetInfo tinfo{};
        tinfo.color_target_descriptions = &ctd;
        tinfo.num_color_targets = 1;
        pi.target_info = tinfo;
        pipeline = SDL_CreateGPUGraphicsPipeline(dev, &pi);

        SDL_ReleaseGPUShader(dev, vs);
        SDL_ReleaseGPUShader(dev, fs);
        if (pipeline == nullptr) { return 1; }
    }

    /* ---------- (4) index buffer ---------- */
    SDL_GPUTransferBufferCreateInfo ib{};
    ib.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    ib.size  = sizeof(IDX);
    SDL_GPUTransferBuffer* itr = SDL_CreateGPUTransferBuffer(dev, &ib);
    void* im = SDL_MapGPUTransferBuffer(dev, itr, false);
    SDL_memcpy(im, IDX, sizeof(IDX));
    SDL_UnmapGPUTransferBuffer(dev, itr);
    SDL_GPUBufferCreateInfo ibci{};
    ibci.usage = SDL_GPU_BUFFERUSAGE_INDEX;
    ibci.size  = sizeof(IDX);
    SDL_GPUBuffer* ibo = SDL_CreateGPUBuffer(dev, &ibci);
    SDL_GPUCommandBuffer* c2 = SDL_AcquireGPUCommandBuffer(dev);
    SDL_GPUCopyPass* cp2 = SDL_BeginGPUCopyPass(c2);
    SDL_GPUTransferBufferLocation s2{itr, 0};
    SDL_GPUBufferRegion d2{ibo, 0, ib.size};
    SDL_UploadToGPUBuffer(cp2, &s2, &d2, false);
    SDL_EndGPUCopyPass(cp2);
    SDL_SubmitGPUCommandBuffer(c2);
    SDL_ReleaseGPUTransferBuffer(dev, itr);

    /* ---------- (5) loop ---------- */
    bool corriendo = true;
    while (corriendo) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) corriendo = false;
        }
        SDL_GPUCommandBuffer* cmds = SDL_AcquireGPUCommandBuffer(dev);
        SDL_GPUTexture* swapchain = nullptr;
        Uint32 w = 0, h = 0;
        if (SDL_WaitAndAcquireGPUSwapchainTexture(cmds, ventana,
                                                  &swapchain, &w, &h)) {
            SDL_GPUColorTargetInfo color{};
            color.texture = swapchain;
            color.load_op = SDL_GPU_LOADOP_CLEAR;
            color.store_op = SDL_GPU_STOREOP_STORE;
            color.clear_color = {0.09f, 0.10f, 0.13f, 1.0f};
            SDL_GPURenderPass* pass =
                SDL_BeginGPURenderPass(cmds, &color, 1, nullptr);
            SDL_BindGPUGraphicsPipeline(pass, pipeline);
            SDL_GPUBufferBinding vb_r{vbo, 0};
            SDL_BindGPUVertexBuffers(pass, 0, &vb_r, 1);
            SDL_GPUBufferBinding ib_r{ibo, 0};
            SDL_BindGPUIndexBuffer(pass, &ib_r, SDL_GPU_INDEXELEMENTSIZE_16BIT);
            SDL_GPUTextureSamplerBinding tsb{tex, sampler};
            SDL_BindGPUFragmentSamplers(pass, 0, &tsb, 1);
            SDL_DrawGPUIndexedPrimitives(pass, 6, 1, 0, 0, 0);
            SDL_EndGPURenderPass(pass);
        }
        SDL_SubmitGPUCommandBuffer(cmds);
    }

    SDL_ReleaseGPUGraphicsPipeline(dev, pipeline);
    SDL_ReleaseGPUSampler(dev, sampler);
    SDL_ReleaseGPUTexture(dev, tex);
    SDL_ReleaseGPUBuffer(dev, ibo);
    SDL_ReleaseGPUBuffer(dev, vbo);
    SDL_ReleaseWindowFromGPUDevice(dev, ventana);
    SDL_DestroyGPUDevice(dev);
    SDL_DestroyWindow(ventana);
    SDL_Quit();
    return 0;
}
