/* =============================================================================
   MÓDULO 20 - EJERCICIO 08: FBO, MSAA y post-proceso (viñeta + blit)
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lecciones 15 y 16 (render-to-texture con FBO, sizing del render
          buffer multisample, y el pase post de pantalla completa).

   CONSIGNA (lógica pura; nada de GL):
   1. `sampleValido(maxHW, pedido)`: MSAA en GL 3.3 core: el render buffer
      multisample pide una cantidad de samples que el hardware clampa a
      GL_MAX_SAMPLES. Reglas del ejercicio:
        - solo valen POTENCIAS DE 2 y > 1 (un sample sería el FBO común);
        - el sample elegido es el MAYOR potencial de 2 que entra
          (sample <= pedido Y sample <= maxHW);
        - si no hay ninguno válido: 0 (el llamador usa FBO simple).
      Ej: maxHW=8, pedido=6 → 4; maxHW=4, pedido=100 → 4; maxHW=1 → 0.
   2. `factorVineta(px, py)`: viñeta de pantalla de 1920x1080 ANCLADA al
      centro (960, 540), radio hasta la esquina (distancia centro →
      esquina): factor de brillo en [0.4, 1]:
        t = distancia / radio; si t <= 0.6 → 1.0 (centro intacto);
        si t  > 0.6 → interpola LINEAL hasta 0.4 en t = 1 y MÁS ALLÁ queda
        0.4 (las esquinas lejos del radio no bajan de ahí).
   3. `clampRegion(...)`: la región del FBO → pantalla (el blit de L16
      NUNCA sale de los límites): recortá la región pedida (px, py, sw, sh)
      contra el dst de 1920x1080 y devolvé la región RESULTANTE con
      valida = sw > 0 && sh > 0 (una región TOTALMENTE fuera es inválida).
      Nota de la lección: los commandos de copia que salen de rango son
      undefined behavior EN LA API — como acá es C++, el clamp es siempre.

     struct Region { int px, py, sw, sh; bool valida; };

   TESTS (no tocar). COMPILACIÓN Y CORRIDA:
     g++ -std=c++20 -Wall -Wextra ejercicio_08_fbo_msaa.cpp -o ej && ./ej
   ============================================================================ */
#include "../../include/test_framework.hpp"
#include <cmath>

struct Region { int px, py, sw, sh; bool valida; };

/* ------------------------------ TU CÓDIGO AQUÍ ---------------------------- */

// TODO 1: sample MSAA elegido (potencia de 2, > 1, <= min(pedido, maxHW); 0 si no hay)
static int sampleValido(int maxHW, int pedido) {
    (void)maxHW; (void)pedido;
    return 0;
}

// TODO 2: factor de brillo 1.0 (centro) ... 0.4 (borde y más allá)
static float factorVineta(float px, float py) {
    (void)px; (void)py;
    return 0.0f;
}

// TODO 3: región clipada al dst 1920x1080
static Region clampRegion(int px, int py, int sw, int sh) {
    (void)px; (void)py; (void)sw; (void)sh;
    return Region{0, 0, 0, 0, false};
}

/* --------------------------- TESTS (no tocar) -------------------------- */
static const int ANCHO = 1920;
static const int ALTO  = 1080;
static const float CX   = 960.f;
static const float CY   = 540.f;
static const float RADIO = std::sqrt(CX * CX + CY * CY);   // centro → esquina

TEST_CASE(SampleDePotenciaDos) {
    ASSERT_EQ(sampleValido(8, 6), 4);          // el mayor power-of-two <= 6
    ASSERT_EQ(sampleValido(8, 8), 8);          // pedido == tope: exacto
    ASSERT_EQ(sampleValido(4, 100), 4);        // el hw clampa
    ASSERT_EQ(sampleValido(2, 3), 2);          // 2 samples siempre pasa
}

TEST_CASE(SampleInvalido) {
    ASSERT_EQ(sampleValido(1, 8), 0);          // max 1: el MSAA no existe
    ASSERT_EQ(sampleValido(8, 1), 0);          // 1 sample ≡ FBO simple (no MSAA)
    ASSERT_EQ(sampleValido(8, 0), 0);          // pedido 0: tampoco
}

TEST_CASE(VinetaCentroYEsquinas) {
    ASSERT_NEAR(factorVineta(960.f, 540.f), 1.0f, 1e-5f);   // centro intacto
    // en el RADIO la cuenta da t = 1: factor mínimo 0.4
    ASSERT_NEAR(factorVineta(960.f + RADIO, 540.f), 0.4f, 1e-5f);
    // más allá del radio NO baja más
    ASSERT_NEAR(factorVineta(960.f + 2.f * RADIO, 540.f), 0.4f, 1e-5f);
}

TEST_CASE(VinetaIntermedia) {
    // t = 0.6 (60% del radio): el arranque de la caída, aún 1.0
    ASSERT_NEAR(factorVineta(960.f + 0.6f * RADIO, 540.f), 1.0f, 1e-4f);
    // t = 0.8: la mitad del fade: 1 - 0.6 * (0.2/0.4) = 0.7
    ASSERT_NEAR(factorVineta(960.f + 0.8f * RADIO, 540.f), 0.7f, 1e-3f);
}

TEST_CASE(ClampRegion) {
    Region r = clampRegion(-10, -20, 200, 150);
    ASSERT_EQ(r.px, 0);   ASSERT_EQ(r.py, 0);  // el origen se recorta
    ASSERT_EQ(r.sw, 190); ASSERT_EQ(r.sh, 130);
    ASSERT_TRUE(r.valida);
    Region m = clampRegion(1000, 500, 2000, 2000);
    ASSERT_EQ(m.sw, 920);  ASSERT_EQ(m.sh, 580);   // no sale de 1920x1080
    ASSERT_TRUE(m.valida);
    Region f = clampRegion(2000, 1200, 100, 100);  // TOTALMENTE fuera
    ASSERT_FALSE(f.valida);
    // asoma solo una esquina: el recorte deja 20x10 VISIBLES (y válidos)
    Region p = clampRegion(1900, 1070, 200, 200);
    ASSERT_EQ(p.px, 1900); ASSERT_EQ(p.sw, 20); ASSERT_EQ(p.sh, 10);
    ASSERT_TRUE(p.valida);
}

int main() { return RUN_ALL_TESTS(); }
