/* =============================================================================
   MÓDULO 26 - EJERCICIO 05: SDL_GPU — quad texturizado (UV de atlas, índices
   y checkerboard para el transfer buffer)
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 05 (buffers, texturas, samplers y el frame completo:
   triángulo y quad).

   CONSIGNA
   --------
   Escribí la lógica del quad que después el demo_02 manda a SDL_GPU:

   1. `uvSubRect(x, y, w, h, tw, th)`: UV normalizado del sprite dentro
      del atlas: (x/tw .. (x+w)/tw, y/th .. (y+h)/th).
   2. `indicesQuad()`: los 6 índices del quad con dos triángulos CCW:
      0,1,2, 0,2,3 (el orden EXACTO importa: front face).
   3. `checkerboard(lado, celdas)`: textura procedural RGBA8 (lado*lado*4
      bytes), alterna blanco(255)/oscuro(30) por celda con la paridad de
      (fila + columna) de celda. Alfa SIEMPRE 255.
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <cstdint>
#include <vector>

namespace {

struct QuadUV { float u0, v0, u1, v1; };

QuadUV uvSubRect(int x, int y, int w, int h, int tw, int th) {
    // TODO
    return {0.0f, 0.0f, 0.0f, 0.0f};   // stub inválido
}

std::vector<std::uint32_t> indicesQuad() {
    // TODO
    return {};   // stub vacío (el test protege con size)
}

std::vector<std::uint8_t> checkerboard(int lado, int celdas) {
    // TODO
    return {};   // stub vacío (el test protege con size)
}

} // namespace

/* ------------------------------ TESTS (no tocar)-------------------------- */
TEST_CASE(UvDelSpriteEnElAtlas) {
    /* sprite 32x32 px posicionado en (16,16) de un atlas 128x128 */
    const auto uv = uvSubRect(16, 16, 32, 32, 128, 128);
    ASSERT_NEAR(uv.u0, 0.125f, 0.0001f);
    ASSERT_NEAR(uv.v0, 0.125f, 0.0001f);
    ASSERT_NEAR(uv.u1, 0.375f, 0.0001f);
    ASSERT_NEAR(uv.v1, 0.375f, 0.0001f);
    /* atlás completo: UV [0..1] exacto */
    const auto full = uvSubRect(0, 0, 128, 128, 128, 128);
    ASSERT_NEAR(full.u1, 1.0f, 0.0001f);
    ASSERT_NEAR(full.v1, 1.0f, 0.0001f);
}

TEST_CASE(IndicesDelQuad) {
    const auto idx = indicesQuad();
    ASSERT_EQ(idx.size(), 6u);              /* guard: NO indexar vacío */
    ASSERT_EQ(idx[0], 0u);  ASSERT_EQ(idx[1], 1u);  ASSERT_EQ(idx[2], 2u);
    ASSERT_EQ(idx[3], 0u);  ASSERT_EQ(idx[4], 2u);  ASSERT_EQ(idx[5], 3u);
}

TEST_CASE(CheckerboardParaElTransfer) {
    const auto px = checkerboard(128, 8);
    ASSERT_EQ(px.size(), 128u * 128u * 4u);  /* guard: RGBA8 completo   */
    /* esquina (0,0): celda (0,0) par -> blanco */
    ASSERT_EQ(px[0], 255);   /* R */
    ASSERT_EQ(px[3], 255);   /* A */
    /* centro del cuadrado (0,0) sigue en la misma celda */
    ASSERT_EQ(px[(16 * 128 + 16) * 4], 255);
    /* celda (1,0): x=16..31 es la celda 1 -> oscuro */
    ASSERT_EQ(px[(8 * 128 + 24) * 4], 30);
    /* esquina inferior derecha: celda (7,7) -> par -> blanco */
    ASSERT_EQ(px[(127 * 128 + 127) * 4], 255);
    /* el alfa es 255 en todo el checker */
    for (std::size_t i = 3; i < px.size(); i += 4) {
        if (px[i] != 255) { ASSERT_TRUE(false); }
    }
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
