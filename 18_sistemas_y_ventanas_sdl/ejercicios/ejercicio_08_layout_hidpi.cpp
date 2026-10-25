/* =============================================================================
   MÓDULO 18 - EJERCICIO 08: Layout Hi-DPI: puntos -> pixels con escala
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 11 (GetWindowDisplayScale/GetWindowSizeInPixels/layout)
          y los rects del ej 03

   CONSIGNA:
   Con SDL_WINDOW_HIGH_PIXEL_DENSITY el back buffer puede tener MÁS pixels
   que el tamaño lógico de la ventana (lección 11). Completá la librería
   de layout (la escala se INYECTA como dato: los tests no usan SDL):

     struct Layout { float escala; }

     1. int aPixels(float puntos): la UI se piensa en PUNTOS (floats, sin
        truncar) y el back buffer está en pixels:
        pixel = round(puntos * escala) con redondeo half-up.
     2. int ventanaLogica(int pixels): la inversa: puntos enteros que
        CABEN en un buffer: trunc(pixels / escala) hacia abajo.
     3. int escalaPixelPerfect(int logicaPx, int bufferPx): el MÁXIMO k >= 1
        entero tal que logicaPx * k <= bufferPx (pixel-art: escala
        INTEGER para no romper tiles; 0 si ni x1 entra en el buffer).
     4. bool caben(int hudX, int hudW, int bufferW): el HUD (en pixels
        físicos) entra completo dentro del back buffer (borde right
        inclusivo: hudX + hudW == bufferW entra; negaciones fuera).

   REGLAS TESTEABLES (deterministas):
   - escalas enteras y fraccionales (1.5, 2.0, 1.75, 0.75)
   - el HUD NO se estira con la escala: SE REDIMENSIONA con el mismo
     redondeo en todos lados (10 puntos con escala 1.75 = 18 pixels,
     no 17.5 truncados).
   - pixel-perfect con ancho que NO divide al buffer (tiles de 16 px y
     buffer 58: k = 3, porque k = 4 daría 64 > 58).

   TESTS (no tocar). COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra ejercicio_08_layout_hidpi.cpp -o ej && ./ej
   ============================================================================= */
#include <cmath>
#include "../../include/test_framework.hpp"

// ============================ MODELO (no tocar) ==============================
struct Layout {
    float escala = 1.0f;   // SDL_GetWindowDisplayScale() inyectada
};

// ============================ TU CÓDIGO AQUÍ ================================
// TODO 1..2: puntos -> pixels (round half-up, como el layout de la L11)
static int aPixels(const Layout& ly, float puntos) {
    (void)ly; (void)puntos;
    return 0;
}

// TODO 3: pixels -> puntos truncados (la ventana lógica completa)
static int ventanaLogica(const Layout& ly, int pixels) {
    (void)ly; (void)pixels;
    return 0;
}

// TODO 4: pixel-perfect: máximo k entero con logica_px*k <= buffer
static int escalaPixelPerfect(int logicaPx, int bufferPx) {
    (void)logicaPx; (void)bufferPx;
    return 0;
}

// TODO 5: hud cupo completo (borde right inclusivo)
static bool caben(int hudX, int hudW, int bufferW) {
    (void)hudX; (void)hudW; (void)bufferW;
    return false;
}

// ============================ TESTS (no tocar) ==============================
TEST_CASE(EscalasEnterasYFraccionales) {
    Layout ly1{1.0f};
    ASSERT_EQ(aPixels(ly1, 64.0f), 64);
    Layout ly15{1.5f};
    ASSERT_EQ(aPixels(ly15, 120.0f), 180);
    ASSERT_EQ(aPixels(ly15, 13.5f), 20);     // 20.25 -> 20
    Layout ly2{2.0f};
    ASSERT_EQ(aPixels(ly2, 64.0f), 128);
    Layout ly075{0.75f};
    ASSERT_EQ(aPixels(ly075, 16.0f), 12);
}

TEST_CASE(VecinosHalfUpConsistentes) {
    Layout ly{1.75f};
    ASSERT_EQ(aPixels(ly, 10.0f), 18);       // 17.5 -> 18  (no 17!)
    ASSERT_EQ(aPixels(ly, 40.0f), 70);       // exacto
    ASSERT_EQ(aPixels(ly, 0.3f), 1);         // 0.525 -> 1
}

TEST_CASE(VentanaLogicaInversa) {
    Layout ly{2.0f};
    ASSERT_EQ(ventanaLogica(ly, 1920), 960);
    ASSERT_EQ(ventanaLogica(ly, 1921), 960);  // truncado abajo: no sale
    ASSERT_EQ(ventanaLogica(ly, 0), 0);
    Layout ly15{1.5f};
    ASSERT_EQ(ventanaLogica(ly15, 90), 60);
    ASSERT_EQ(ventanaLogica(ly15, 91), 60);
}

TEST_CASE(PixelPerfectEscalaInteger) {
    ASSERT_EQ(escalaPixelPerfect(16, 58), 3);  // 4*16=64 sale de 58
    ASSERT_EQ(escalaPixelPerfect(16, 64), 4);  // justo entra
    ASSERT_EQ(escalaPixelPerfect(16, 15), 0);  // ni 1x: 0 (HUD de advertencia)
    ASSERT_EQ(escalaPixelPerfect(0, 999), 0);  // logica 0: nada que escalar
}

TEST_CASE(HudCupoConBorde) {
    ASSERT_TRUE(caben(0, 240, 1920));
    ASSERT_TRUE(caben(1680, 240, 1920));   // right justo: 1680+240 == 1920
    ASSERT_FALSE(caben(1681, 240, 1920));  // se sale 1 pixel
    ASSERT_FALSE(caben(-1, 240, 1920));    // hudX negativo: fuera
}

int main() { return RUN_ALL_TESTS(); }
