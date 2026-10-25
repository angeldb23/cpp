/* =============================================================================
   MÓDULO 20 - SOLUCIÓN 08: FBO, MSAA y post-proceso (viñeta + blit)
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lecciones 15 y 16. Resuelve el ejercicio_08_fbo_msaa.cpp.

   El MSAA de GL 3.3 core es POTENCIA DE 2 (el specc no exige, pero el
   hw sí: glRenderbufferStorageMultisample con samples no-potencia falla
   o clampa según el driver). La viñeta es un post-proceso barato:
   un factor por píxel que multiplica el color del pase final.
   ============================================================================ */
#include "../../include/test_framework.hpp"
#include <cmath>

struct Region { int px, py, sw, sh; bool valida; };

// TODO 1
static int sampleValido(int maxHW, int pedido) {
    if (pedido < 2 || maxHW < 2) return 0;    // 1 sample = FBO simple (no MSAA)
    int tope = (pedido < maxHW) ? pedido : maxHW;
    int s = 1;
    while (s * 2 <= tope) s *= 2;             // mayor potencia de 2 <= tope
    return s;
}

// TODO 2
static float factorVineta(float px, float py) {
    const float cx = 960.f, cy = 540.f;
    const float radio = std::sqrt(cx * cx + cy * cy);   // centro → esquina
    float t = std::sqrt((px - cx) * (px - cx) + (py - cy) * (py - cy)) / radio;
    if (t <= 0.6f) return 1.0f;
    float f = (t - 0.6f) / 0.4f;              // 0 en t=0.6 ... 1 en t=1.0
    if (f > 1.0f) f = 1.0f;                   // fuera del radio: no baja más
    return 1.0f - 0.6f * f;                   // 1.0 ... 0.4
}

// TODO 3
static Region clampRegion(int px, int py, int sw, int sh) {
    const int W = 1920, H = 1080;
    int x0 = (px < 0) ? 0 : px;
    int y0 = (py < 0) ? 0 : py;
    int x1 = (px + sw > W) ? W : px + sw;
    int y1 = (py + sh > H) ? H : py + sh;
    int rsw = x1 - x0;
    int rsh = y1 - y0;
    if (rsw <= 0 || rsh <= 0) return Region{x0, y0, 0, 0, false};
    return Region{x0, y0, rsw, rsh, true};
}

/* --------------------------- TESTS (no tocar) -------------------------- */
static const float CX    = 960.f;
static const float CY    = 540.f;
static const float RADIO = std::sqrt(CX * CX + CY * CY);   // centro → esquina

TEST_CASE(SampleDePotenciaDos) {
    ASSERT_EQ(sampleValido(8, 6), 4);
    ASSERT_EQ(sampleValido(8, 8), 8);
    ASSERT_EQ(sampleValido(4, 100), 4);
    ASSERT_EQ(sampleValido(2, 3), 2);
}

TEST_CASE(SampleInvalido) {
    ASSERT_EQ(sampleValido(1, 8), 0);
    ASSERT_EQ(sampleValido(8, 1), 0);
    ASSERT_EQ(sampleValido(8, 0), 0);
}

TEST_CASE(VinetaCentroYEsquinas) {
    ASSERT_NEAR(factorVineta(960.f, 540.f), 1.0f, 1e-5f);
    ASSERT_NEAR(factorVineta(960.f + RADIO, 540.f), 0.4f, 1e-5f);
    ASSERT_NEAR(factorVineta(960.f + 2.f * RADIO, 540.f), 0.4f, 1e-5f);
}

TEST_CASE(VinetaIntermedia) {
    ASSERT_NEAR(factorVineta(960.f + 0.6f * RADIO, 540.f), 1.0f, 1e-4f);
    ASSERT_NEAR(factorVineta(960.f + 0.8f * RADIO, 540.f), 0.7f, 1e-3f);
}

TEST_CASE(ClampRegion) {
    Region r = clampRegion(-10, -20, 200, 150);
    ASSERT_EQ(r.px, 0);   ASSERT_EQ(r.py, 0);
    ASSERT_EQ(r.sw, 190); ASSERT_EQ(r.sh, 130);
    ASSERT_TRUE(r.valida);
    Region m = clampRegion(1000, 500, 2000, 2000);
    ASSERT_EQ(m.sw, 920);  ASSERT_EQ(m.sh, 580);
    ASSERT_TRUE(m.valida);
    Region f = clampRegion(2000, 1200, 100, 100);
    ASSERT_FALSE(f.valida);
    Region p = clampRegion(1900, 1070, 200, 200);
    ASSERT_EQ(p.px, 1900); ASSERT_EQ(p.sw, 20); ASSERT_EQ(p.sh, 10);
    ASSERT_TRUE(p.valida);
}

int main() { return RUN_ALL_TESTS(); }
