/* =============================================================================
   MÓDULO 26 - LECCIÓN 05: SDL_GPU parte 2 — buffers, texturas, samplers y el
   frame completo (triángulo y quad texturizado)
   =============================================================================
   TEMA 3 (parte 2): "textures y render passes"; cierre del tema:
   triángulo en SDL_GPU y quad texturizado.

   EL CAMINO QUE TOMA UN DATO HASTA EL RENDER
   ------------------------------------------
   1. CREACIÓN (una vez): SDL_CreateGPUBuffer / SDL_CreateGPUTexture /
      SDL_CreateGPUSampler. El struct info describe el USO
      (SDL_GPU_TEXTUREUSAGE_SAMPLER, etc.). Estos tres nombres y la
      estructura del upload están verificados en SDL_gpu.h 3.4.18.
   2. UPLOAD (por carga de recurso): SDL_CreateGPUTransferBuffer ->
      SDL_MapGPUTransferBuffer -> memcpy ->
      SDL_UnmapGPUTransferBuffer -> SDL_UploadToGPUBuffer /
      SDL_UploadToGPUTexture. El transfer buffer es la "arena" de envío:
      nunca copiás directo a un GPUBuffer (que le pasa el GPU cuando
      quiere).
   3. RENDER (por frame): SDL_BeginRenderPass ->
      SDL_BindGPUGraphicsPipeline -> SDL_BindGPUVertexBuffers /
      SDL_BindGPUFragmentSamplers -> SDL_DrawGPUIndexedPrimitives =
      Draw -> SDL_EndRenderPass -> SDL_SubmitGPUCommandBuffer.

   EL QUAD TEXTURIZADO (cierre del tema; código real en demos/)
   ------------------------------------------------------------
   Quad con posición/UV por vértice: 4 vértices, 6 índices (dos
   triángulos, CCW). La textura la generamos en CPU (checkerboard 8x8)
   y la subimos por transfer buffer. Sampler LINEAR/CLAMP para foto o
   NEAREST/REPEAT para pixel art. El código completo está en
   demos/demo_02_quad_sdl_gpu.cpp (compilado y linkeado en la sesión,
   NUNCA ejecutado: abre ventana).

   UV de atlas: u,v normalizados [0..1]; el sub-rect del sprite es
   (x/tw..(x+w)/tw, y/th..(y+h)/th), la misma matemática del módulo 17
   con raylib, pero ahora la emite TU código a un GPUBuffer.
   ============================================================================= */
#include <cstdint>
#include <cstdio>
#include <vector>

namespace {

struct QuadUV { float u0, v0, u1, v1; };

/* UV del sub-rect [x,y,w,h] de una textura total tw x th. */
QuadUV uvSubRect(int x, int y, int w, int h, int tw, int th) {
    return { float(x) / float(tw),        float(y) / float(th),
             float(x + w) / float(tw),    float(y + h) / float(th) };
}

/* Índices del quad clásico (dos triángulos CCW). */
std::vector<std::uint32_t> indicesQuad() { return {0, 1, 2, 0, 2, 3}; }

/* Checkerboard procedural: alterna por paridad de celda 8x8. */
std::vector<std::uint8_t> checkerboard(int lado, int celdas) {
    std::vector<std::uint8_t> px(lado * lado * 4); // RGBA8
    const int tam_celda = lado / celdas;
    for (int y = 0; y < lado; ++y) {
        for (int x = 0; x < lado; ++x) {
            const bool par = ((x / tam_celda) + (y / tam_celda)) % 2 == 0;
            const std::uint8_t v = par ? 255 : 30;
            std::uint8_t* p = &px[(std::size_t(y) * lado + x) * 4];
            p[0] = p[1] = p[2] = v; p[3] = 255;
        }
    }
    return px;
}

/* Sampler al estilo SDL_GPUSamplerCreateInfo (valores reales del
   header: SDL_GPU_FILTER_NEAREST/LINEAR; ADDRESSMODE: REPEAT,
   MIRRORED_REPEAT, CLAMP_TO_EDGE). */
enum Filter { FIL_NEAREST, FIL_LINEAR };
enum Addr   { ADDR_REPEAT, ADDR_MIRRORED, ADDR_CLAMP };
struct SamplerSpec { int filtro; int wrap_u; int wrap_v; };

void mostrarSampler(const char* uso, const SamplerSpec& s) {
    static const char* F[] = {"NEAREST", "LINEAR"};
    static const char* A[] = {"REPEAT", "MIRRORED_REPEAT", "CLAMP_TO_EDGE"};
    std::printf("sampler %-14s filtro=%-8s wrap_u=%-15s wrap_v=%-15s\n",
                uso, F[s.filtro], A[s.wrap_u], A[s.wrap_v]);
}

} // namespace

int main() {
    const auto uv = uvSubRect(16, 16, 32, 32, 128, 128);
    std::printf("sprite 32x32 px en atlas 128x128: u=[%.3f..%.3f] v=[%.3f..%.3f]\n",
                uv.u0, uv.u1, uv.v0, uv.v1);

    const auto idx = indicesQuad();
    std::printf("quad indices: %u %u %u %u %u %u (6 índices = 2 tris)\n",
                idx[0], idx[1], idx[2], idx[3], idx[4], idx[5]);

    const auto tex = checkerboard(128, 8);
    std::printf("checkerboard 128x128 RGBA8 = %zu bytes (sube por transfer)\n",
                tex.size());

    mostrarSampler("sprites pixel", SamplerSpec{FIL_NEAREST, ADDR_REPEAT, ADDR_REPEAT});
    mostrarSampler("fotos",         SamplerSpec{FIL_LINEAR,  ADDR_CLAMP,  ADDR_CLAMP});
    /* Regla: pixel art -> NEAREST + REPEAT; fotos -> LINEAR + CLAMP
       (para que el borde del atlas no "sangre" las UV entre sprites). */
    return 0;
}
