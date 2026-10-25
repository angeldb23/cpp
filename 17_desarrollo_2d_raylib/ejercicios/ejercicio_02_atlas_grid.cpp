/* =============================================================================
   MÓDULO 17 - EJERCICIO 02: TextureAtlas — Grid de Frames y Sub-rects
   =============================================================================
   DIFICULTAD: Básico
   APOYO: Lección 02 (Sprites: atlas, sub-rects)

   CONSIGNA:
   Completa TextureAtlas::GetRect(col, row): devuelve el sub-rectángulo del
   frame (col, row) dentro del atlas, respetando MARGIN y PADDING:

   x(col) = margin + col * (frameW + padding)
   y(row) = margin + row * (frameH + padding)
   size   = frameW x frameH

   FUERA DE RANGO: si col >= cols o row >= rows, devolver un rect vacío
   {0,0,0,0} (el llamador lo ve como "no dibujes nada").
   Los índices negativos también devuelven el rect vacío.

   TESTS (no tocar): mapping columna/fila con margen y padding, y límites.

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_02_atlas_grid.cpp -o ej && ./ej
   ============================================================================= */
#include <iostream>
#include "../../include/test_framework.hpp"

// ============================ STUBS (no tocar) ===============================
struct Rectangle { float x, y, width, height; };

// ============================ TU CÓDIGO AQUÍ ================================
class TextureAtlas {
public:
    int frameW = 32, frameH = 32;
    int cols = 8, rows = 4;
    int padding = 2;   // espacio ENTRE frames
    int margin = 4;    // borde alrededor del grid completo

    // TODO: sub-rect del frame (col, row)
    Rectangle GetRect(int col, int row) const {
        // Pista:
        // 1) validar rango (col en [0, cols), row en [0, rows))
        // 2) construir el rect con la fórmula del encabezado
        return {};   // placeholder
    }
};

// ============================ TESTS (no tocar) ==============================
TEST_CASE(FrameOrigenConMargin) {
    TextureAtlas atlas;   // 32x32, margin 4, padding 2
    Rectangle r = atlas.GetRect(0, 0);
    ASSERT_NEAR(r.x, 4.0f, 0.001f);        // margin
    ASSERT_NEAR(r.y, 4.0f, 0.001f);
    ASSERT_NEAR(r.width, 32.0f, 0.001f);
    ASSERT_NEAR(r.height, 32.0f, 0.001f);
}

TEST_CASE(FrameColumna2ConPadding) {
    TextureAtlas atlas;
    Rectangle r = atlas.GetRect(2, 0);
    // 4 + 2*(32+2) = 4 + 68 = 72
    ASSERT_NEAR(r.x, 72.0f, 0.001f);
    ASSERT_NEAR(r.y, 4.0f, 0.001f);
}

TEST_CASE(FrameFila1RespectPaddingEnY) {
    TextureAtlas atlas;
    Rectangle r = atlas.GetRect(0, 1);
    // y = 4 + 1*(32+2) = 38
    ASSERT_NEAR(r.x, 4.0f, 0.001f);
    ASSERT_NEAR(r.y, 38.0f, 0.001f);
}

TEST_CASE(FueraDeRangoDevuelveRectVacio) {
    TextureAtlas atlas;
    Rectangle r = atlas.GetRect(8, 0);   // col == cols: fuera de ambas
    ASSERT_NEAR(r.width, 0.0f, 0.001f);
    ASSERT_NEAR(r.height, 0.0f, 0.001f);

    Rectangle r2 = atlas.GetRect(0, 4);  // row == rows
    ASSERT_NEAR(r2.width, 0.0f, 0.001f);
}

TEST_CASE(IndiceNegativoDevuelveRectVacio) {
    TextureAtlas atlas;
    Rectangle r = atlas.GetRect(-1, 0);
    ASSERT_NEAR(r.width, 0.0f, 0.001f);
    Rectangle r2 = atlas.GetRect(0, -2);
    ASSERT_NEAR(r2.height, 0.0f, 0.001f);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 02 (Atlas Grid)...\n";
    return RUN_ALL_TESTS();
}
