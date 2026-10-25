/* =============================================================================
   MÓDULO 26 - SOLUCIÓN 05: SDL_GPU — quad texturizado (UV de atlas, índices
   y checkerboard para el transfer buffer)
   =============================================================================
   La solución de ejercicio_05_quad_texturizado.cpp. Estos tres helpers
   son EXACTAMENTE los que demos/demo_02_quad_sdl_gpu.cpp usa con la
   API real: la geometría del quad (4 vértices + 6 índices) va en un
   GPUBuffer por transfer buffer, la textura se genera en CPU con
   checkerboard() y el sampler (LINEAR+CLAMP o NEAREST+REPEAT) es
   configuración aparte. La matemática de UV de atlas es la misma que
   módulo 17 (raylib): (x/tw..(x+w)/tw, y/th..(y+h)/th).
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <cstdint>
#include <vector>

namespace {

struct QuadUV { float u0, v0, u1, v1; };

QuadUV uvSubRect(int x, int y, int w, int h, int tw, int th) {
    return { float(x) / float(tw),     float(y) / float(th),
             float(x + w) / float(tw), float(y + h) / float(th) };
}

/* Dos triángulos CCW (front face por defecto en SDL_GPU y GL). */
std::vector<std::uint32_t> indicesQuad() { return {0, 1, 2, 0, 2, 3}; }

/* RGBA8: alterna blanco/oscuro por paridad de la celda (fila+columna). */
std::vector<std::uint8_t> checkerboard(int lado, int celdas) {
    std::vector<std::uint8_t> px(std::size_t(lado) * lado * 4);
    const int tam = lado / celdas;
    for (int y = 0; y < lado; ++y) {
        for (int x = 0; x < lado; ++x) {
            const bool par = ((x / tam) + (y / tam)) % 2 == 0;
            std::uint8_t* p = &px[(std::size_t(y) * lado + x) * 4];
            p[0] = p[1] = p[2] = par ? 255 : 30;
            p[3] = 255;
        }
    }
    return px;
}

} // namespace

/* ------------------------------ TESTS (no tocar)-------------------------- */
TEST_CASE(UvDelSpriteEnElAtlas) {
    const auto uv = uvSubRect(16, 16, 32, 32, 128, 128);
    ASSERT_NEAR(uv.u0, 0.125f, 0.0001f);
    ASSERT_NEAR(uv.v0, 0.125f, 0.0001f);
    ASSERT_NEAR(uv.u1, 0.375f, 0.0001f);
    ASSERT_NEAR(uv.v1, 0.375f, 0.0001f);
    const auto full = uvSubRect(0, 0, 128, 128, 128, 128);
    ASSERT_NEAR(full.u1, 1.0f, 0.0001f);
    ASSERT_NEAR(full.v1, 1.0f, 0.0001f);
}

TEST_CASE(IndicesDelQuad) {
    const auto idx = indicesQuad();
    ASSERT_EQ(idx.size(), 6u);
    ASSERT_EQ(idx[0], 0u);  ASSERT_EQ(idx[1], 1u);  ASSERT_EQ(idx[2], 2u);
    ASSERT_EQ(idx[3], 0u);  ASSERT_EQ(idx[4], 2u);  ASSERT_EQ(idx[5], 3u);
}

TEST_CASE(CheckerboardParaElTransfer) {
    const auto px = checkerboard(128, 8);
    ASSERT_EQ(px.size(), 128u * 128u * 4u);
    ASSERT_EQ(px[0], 255);
    ASSERT_EQ(px[3], 255);
    ASSERT_EQ(px[(16 * 128 + 16) * 4], 255);
    ASSERT_EQ(px[(8 * 128 + 24) * 4], 30);
    ASSERT_EQ(px[(127 * 128 + 127) * 4], 255);
    for (std::size_t i = 3; i < px.size(); i += 4) {
        if (px[i] != 255) { ASSERT_TRUE(false); }
    }
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
