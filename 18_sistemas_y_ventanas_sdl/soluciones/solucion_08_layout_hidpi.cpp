/* =============================================================================
   MÓDULO 18 - SOLUCIÓN 08: Layout Hi-DPI: puntos -> pixels con escala
   =============================================================================
   MISMO ENUNCIADO que ejercicios/ejercicio_08_layout_hidpi.cpp.
   SOLUCIÓN COMENTADA: qué hace cada pieza y por qué.

   PUNTOS CLAVE:
   - aPixels: round half-up vía int(v + 0.5f) PARA VALORES no-negativos
     (UI en pantalla: siempre están a la derecha/abajo del 0); misma
     fórmula que rasterX del ej 03 y que la L11.
   - ventanaLogica: floor int: pixels / escala "hacia adentro" (si un
     punto no cabe completo, no entra).
   - escalaPixelPerfect: k máximo con logicaPx*k <= bufferPx. Sin loops
     "por fuerza bruta": k = buffer / logica (división entera). El caso
     borde: logicaPx == 0 -> 0 (nada que escalar).
   - caben: los dos bordes deben estar DENTRO (x >= 0 checked del test,
     y right inclusivo: x + w <= bufferW); el ej 08 agrega el caso
     right-exacto == buffer.
   ============================================================================= */
#include <cmath>
#include "../../include/test_framework.hpp"

// ============================ MODELO (no tocar) ==============================
struct Layout {
    float escala = 1.0f;   // SDL_GetWindowDisplayScale() inyectada
};

// ============================ SOLUCIÓN ======================================
// puntos -> pixels (round half-up): el back buffer puede dar más o menos
// que el size lógico, y el "pixel" siempre es un entero positivo
static int aPixels(const Layout& ly, float puntos) {
    const float v = puntos * ly.escala;
    if (v < 0.0f) return 0;            // fuera de la ventana: nada que pintar
    return static_cast<int>(v + 0.5f); // trunc(v+0.5) = round half-up (positivos)
}

// pixels -> puntos truncados: los puntos que COMPLETAMENTE caben
static int ventanaLogica(const Layout& ly, int pixels) {
    if (ly.escala <= 0.0f) return 0;   // defensivo: escala inválida anula
    return static_cast<int>(float(pixels) / ly.escala);   // trunc hacia 0
}

// pixel-perfect: máximo k entero con logicaPx * k <= bufferPx
static int escalaPixelPerfect(int logicaPx, int bufferPx) {
    if (logicaPx <= 0 || bufferPx < logicaPx) return 0;  // 0 o no entra 1x
    return bufferPx / logicaPx;                       // división entera: k máx
}

// el HUD (en pixels físicos) entra completo en el buffer
static bool caben(int hudX, int hudW, int bufferW) {
    return hudX >= 0 && (hudX + hudW) <= bufferW && bufferW >= 0;
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
    ASSERT_EQ(escalaPixelPerfect(16, 15), 0);  // ni 1x: 0
    ASSERT_EQ(escalaPixelPerfect(0, 999), 0);  // logica 0: nada que escalar
}

TEST_CASE(HudCupoConBorde) {
    ASSERT_TRUE(caben(0, 240, 1920));
    ASSERT_TRUE(caben(1680, 240, 1920));   // right justo: 1680+240 == 1920
    ASSERT_FALSE(caben(1681, 240, 1920));  // se sale 1 pixel
    ASSERT_FALSE(caben(-1, 240, 1920));    // hudX negativo: fuera
}

int main() { return RUN_ALL_TESTS(); }
