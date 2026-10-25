/* =============================================================================
   MÓDULO 20 - SOLUCIÓN 05: Cadena de mipmaps
   =============================================================================
   DIFICULTAD: Fácil
   APOYO: Lección 10. Resuelve el ejercicio_05_mip_chain.cpp.

   La división entera y el max(...,1) son TODO el algoritmo: el GPU hace lo
   mismo cuando la textura se sube con glTexStorage2D(levels).
   ============================================================================ */
#include "../../include/test_framework.hpp"
#include <vector>

struct Tamanio { int w, h; };

// TODO 1
static std::vector<Tamanio> cadenaMips(int w, int h) {
    std::vector<Tamanio> c;
    if (w <= 0 || h <= 0) return c;   // texturas 0 no existen en GL
    int cw = w, ch = h;
    while (true) {
        c.push_back({cw, ch});
        if (cw == 1 && ch == 1) break;
        cw = (cw > 1) ? (cw / 2) : 1;   // max(w/2, 1) con división entera
        ch = (ch > 1) ? (ch / 2) : 1;
    }
    return c;
}

// TODO 2
static int nivelesMips(int w, int h) {
    return (int)cadenaMips(w, h).size();
}

// TODO 3
static size_t bytesMips(int w, int h, int bytesPorPx) {
    size_t total = 0;
    for (const Tamanio& t : cadenaMips(w, h))
        total += (size_t)t.w * (size_t)t.h * (size_t)bytesPorPx;
    return total;
}

/* --------------------------- TESTS (no tocar) -------------------------- */

TEST_CASE(CadenaDe61x41) {
    std::vector<Tamanio> c = cadenaMips(61, 41);
    ASSERT_EQ(c.size(), (size_t)6);
    ASSERT_EQ(c[0].w, 61);  ASSERT_EQ(c[0].h, 41);
    ASSERT_EQ(c[1].w, 30);  ASSERT_EQ(c[1].h, 20);
    ASSERT_EQ(c[2].w, 15);  ASSERT_EQ(c[2].h, 10);
    ASSERT_EQ(c[3].w, 7);   ASSERT_EQ(c[3].h, 5);
    ASSERT_EQ(c[4].w, 3);   ASSERT_EQ(c[4].h, 2);
    ASSERT_EQ(c[5].w, 1);   ASSERT_EQ(c[5].h, 1);
}

TEST_CASE(NivelesYPesos) {
    ASSERT_EQ(nivelesMips(61, 41), 6);
    ASSERT_EQ(bytesMips(61, 41, 4), (size_t)13172);
    // RGBA8 base 61x41 solo: 2501*4 = 10004. La cadena come 3228 más: el
    // "+33% de VRAM" de la lección (3293 = 2501 * 1.316).
}

TEST_CASE(CuadradaPerfecta) {
    std::vector<Tamanio> c = cadenaMips(8, 8);
    ASSERT_EQ(c.size(), (size_t)4);
    ASSERT_EQ(c[1].w, 4); ASSERT_EQ(c[3].w, 1); ASSERT_EQ(c[3].h, 1);
    ASSERT_EQ(nivelesMips(8, 8), 4);
    ASSERT_EQ(bytesMips(8, 8, 1), (size_t)85);
}

TEST_CASE(Degeneradas) {
    std::vector<Tamanio> c = cadenaMips(1, 1);
    ASSERT_EQ(c.size(), (size_t)1);
    ASSERT_EQ(c[0].w, 1); ASSERT_EQ(c[0].h, 1);
    ASSERT_EQ(nivelesMips(1, 1), 1);
    ASSERT_EQ(bytesMips(1, 1, 4), (size_t)4);
    std::vector<Tamanio> d = cadenaMips(3, 1);
    ASSERT_EQ(d.size(), (size_t)2);
    ASSERT_EQ(d[1].w, 1); ASSERT_EQ(d[1].h, 1);
}

int main() { return RUN_ALL_TESTS(); }
