/* =============================================================================
   MÓDULO 17 - SOLUCIÓN 02: TextureAtlas — Grid de Frames y Sub-rects
   =============================================================================
   Resuelve ejercicio_02_atlas_grid.cpp. La fórmula es la fila-major del
   atlas: cada frame avanza (frameW+padding) en X y (frameH+padding) en Y.

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_02_atlas_grid.cpp -o sol && ./sol
   ============================================================================= */
#include <iostream>
#include "../../include/test_framework.hpp"

// ============================ STUBS (no tocar) ===============================
struct Rectangle { float x, y, width, height; };

// ============================ SOLUCIÓN =======================================
class TextureAtlas {
public:
    int frameW = 32, frameH = 32;
    int cols = 8, rows = 4;
    int padding = 2;
    int margin = 4;

    Rectangle GetRect(int col, int row) const {
        // TODO: solución. (1) validación temprana: rango de índices
        if (col < 0 || row < 0 || col >= cols || row >= rows) {
            return Rectangle{0.0f, 0.0f, 0.0f, 0.0f};   // "no dibujes nada"
        }
        // (2) paso del grid: cada frame separa padding + margin en el borde
        const float x = float(margin + col * (frameW + padding));
        const float y = float(margin + row * (frameH + padding));
        return Rectangle{x, y, float(frameW), float(frameH)};
    }
};

// ============================ TESTS (iguales) ================================
TEST_CASE(FrameOrigenConMargin) {
    TextureAtlas atlas;
    Rectangle r = atlas.GetRect(0, 0);
    ASSERT_NEAR(r.x, 4.0f, 0.001f);
    ASSERT_NEAR(r.y, 4.0f, 0.001f);
    ASSERT_NEAR(r.width, 32.0f, 0.001f);
    ASSERT_NEAR(r.height, 32.0f, 0.001f);
}

TEST_CASE(FrameColumna2ConPadding) {
    TextureAtlas atlas;
    Rectangle r = atlas.GetRect(2, 0);
    ASSERT_NEAR(r.x, 72.0f, 0.001f);
    ASSERT_NEAR(r.y, 4.0f, 0.001f);
}

TEST_CASE(FrameFila1RespectPaddingEnY) {
    TextureAtlas atlas;
    Rectangle r = atlas.GetRect(0, 1);
    ASSERT_NEAR(r.x, 4.0f, 0.001f);
    ASSERT_NEAR(r.y, 38.0f, 0.001f);
}

TEST_CASE(FueraDeRangoDevuelveRectVacio) {
    TextureAtlas atlas;
    Rectangle r = atlas.GetRect(8, 0);
    ASSERT_NEAR(r.width, 0.0f, 0.001f);
    ASSERT_NEAR(r.height, 0.0f, 0.001f);
    Rectangle r2 = atlas.GetRect(0, 4);
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
    std::cout << "Solución 02: TextureAtlas\n";
    return RUN_ALL_TESTS();
}
