/* =============================================================================
   MÓDULO 20 - EJERCICIO 05: Cadena de mipmaps (cantidades y tamaños)
   =============================================================================
   DIFICULTAD: Fácil
   APOYO: Lección 10 (mipmaps: la GPU sube toda la CADENA, no solo la base;
          saber cuántos niveles y cuántos bytes pesa es lo que determina el
          ancho de la llamada glTexStorage2D y cuánta VRAM come la textura).

   CONSIGNA (todo lógica pura: NADA de GL aquí):
   La cadena de mips de una textura base (w, h) es:
     nivel 0 = (w, h); nivel k+1 = (max(w/2,1), max(h/2,1)); se termina
     cuando el nivel es (1, 1). Ojo: la DIVISIÓN es entera y una dimensión
     puede llegar a 1 antes que la otra (61x41 → 30x20 → 15x10 → 7x5 →
     3x2 → 1x1: el 0.5 se redondea por abajo).

     struct Tamanio { int w, h; };

     1. `cadenaMips(w, h)`: TODA la cadena INCLUDING el nivel 0 y el (1,1)
        final. Para (1,1) de base la cadena es UN SOLO elemento.
     2. `nivelesMips(w, h)`: cuántos niveles tiene (== cadenaMips.size()).
     3. `bytesMips(w, h, bytesPorPx)`: bytes TOTALES de la cadena completa
        (cada nivel aporta w*h*bytesPorPx). Verificalo a mano: es ~1/3 más
        que el nivel 0 solo (la famosa "+33% de VRAM" de la lección).

   TESTS (no tocar). COMPILACIÓN Y CORRIDA:
     g++ -std=c++20 -Wall -Wextra ejercicio_05_mip_chain.cpp -o ej && ./ej
   ============================================================================ */
#include "../../include/test_framework.hpp"
#include <vector>

struct Tamanio { int w, h; };

/* ------------------------------ TU CÓDIGO AQUÍ ---------------------------- */

// TODO 1: la cadena completa (nivel 0 incluido, hasta 1x1 incluido)
static std::vector<Tamanio> cadenaMips(int w, int h) {
    (void)w; (void)h;
    return {};
}

// TODO 2: cuántos niveles (== tamaño de la cadena)
static int nivelesMips(int w, int h) {
    (void)w; (void)h;
    return 0;
}

// TODO 3: bytes totales de la cadena completa
static size_t bytesMips(int w, int h, int bytesPorPx) {
    (void)w; (void)h; (void)bytesPorPx;
    return 0;
}

/* --------------------------- TESTS (no tocar) -------------------------- */

TEST_CASE(CadenaDe61x41) {
    // división entera: el nivel impar hace el piso y una dimensión llega
    // a 1 antes que la otra
    std::vector<Tamanio> c = cadenaMips(61, 41);
    ASSERT_EQ(c.size(), (size_t)6);                       // guard del stub vacío
    ASSERT_EQ(c[0].w, 61);  ASSERT_EQ(c[0].h, 41);
    ASSERT_EQ(c[1].w, 30);  ASSERT_EQ(c[1].h, 20);
    ASSERT_EQ(c[2].w, 15);  ASSERT_EQ(c[2].h, 10);
    ASSERT_EQ(c[3].w, 7);   ASSERT_EQ(c[3].h, 5);
    ASSERT_EQ(c[4].w, 3);   ASSERT_EQ(c[4].h, 2);
    ASSERT_EQ(c[5].w, 1);   ASSERT_EQ(c[5].h, 1);
}

TEST_CASE(NivelesYPesos) {
    ASSERT_EQ(nivelesMips(61, 41), 6);
    // nivel 0: 61*41 = 2501 px. Cadena completa: 2501+600+150+35+6+1 = 3293 px
    // RGBA (4 bytes): 13172 bytes = 3228 más que el nivel 0 solo (+33% famosa)
    ASSERT_EQ(bytesMips(61, 41, 4), (size_t)13172);
    // RGBA8 base 61x41 solo: 2501*4 = 10004. La cadena come 3228 más: el
    // "+33% de VRAM" de la lección (3293 = 2501 * 1.316).
}

TEST_CASE(CuadradaPerfecta) {
    std::vector<Tamanio> c = cadenaMips(8, 8);
    ASSERT_EQ(c.size(), (size_t)4);
    ASSERT_EQ(c[1].w, 4); ASSERT_EQ(c[3].w, 1); ASSERT_EQ(c[3].h, 1);
    ASSERT_EQ(nivelesMips(8, 8), 4);
    // 64+16+4+1 = 85 px
    ASSERT_EQ(bytesMips(8, 8, 1), (size_t)85);
}

TEST_CASE(Degeneradas) {
    // base 1x1: la cadena es UN solo nivel y no baja más
    std::vector<Tamanio> c = cadenaMips(1, 1);
    ASSERT_EQ(c.size(), (size_t)1);
    ASSERT_EQ(c[0].w, 1); ASSERT_EQ(c[0].h, 1);
    ASSERT_EQ(nivelesMips(1, 1), 1);
    ASSERT_EQ(bytesMips(1, 1, 4), (size_t)4);
    // 3x1: el lado bajo YA está en 1; el 3 hace 1 → termina
    std::vector<Tamanio> d = cadenaMips(3, 1);
    ASSERT_EQ(d.size(), (size_t)2);
    ASSERT_EQ(d[1].w, 1); ASSERT_EQ(d[1].h, 1);
}

int main() { return RUN_ALL_TESTS(); }
