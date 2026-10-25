/* =============================================================================
   MÓDULO 20 - EJERCICIO 04: Atlas packer (shelf packing de sprites)
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 11 (atlas: una textura, sub-rects por sprite; el packer
          que menciona la lección es el EJERCICIO).

   CONSIGNA:
   Un atlas es UNA textura con TODOS los sprites en sub-rectángulos. El
   "shelf packer" empaca por filas (estantes) de izquierda a derecha:

     struct Item { std::string nombre; int w; int h; }
     struct Slot { std::string nombre; int x, y, w, h; }   // px en el atlas

     1. `empacar(items, anchoAtlas)`:
        - recorre los items EN EL ORDEN DADO;
        - si el item entra en la fila actual (x_actual + w <= anchoAtlas):
          se coloca ahí; avanza x += w;
        - si NO entra (o se acabó de arrancar): NUEVA fila en y += alto_
          de la fila anterior, x = 0;
        - la ALTURA de cada fila es la del MÁS ALTO de sus items (los
          shorts "dejá huecos de aire" ingrementales: shelf puro).
        - el resultado conserva el ORDEN de items (slot[i] corresponde a
          items[i]).
     2. `altoTotal(slots)`: el alto del atlas necesario (el mayor
        y + h de前面 los slots; 0 si no hay slots).
     3. `uvDe(slot, anchoAtlas, altoAtlas)`: UV del sub-boomerang: las 4
        floats "u0 v0 u1 v1" (u = x/ancho, v = y/alto; exacto:_mapper sin
        half-texel: el inset lo decide el shader de la lección 11).
     4. `itemInaceptable` (item.w > anchoAtlas): NO se empaca: los slots
        no lo incluyen Y se devuelve cuántos falloato.

   TESTS (no tocar). COMPILACIÓN Y CORRIDA:
     g++ -std=c++20 -Wall -Wextra ejercicio_04_atlas_packer.cpp -o ej && ./ej
   ============================================================================ */
#include "../../include/test_framework.hpp"
#include <string>
#include <vector>

struct Item  { std::string nombre; int w; int h; };
struct Slot  { std::string nombre; int x, y, w, h; };

/* ------------------------------ TU CÓDIGO AQUÍ ---------------------------- */

// TODO 1: el shelf packer (fila con el alto del item más alto)
static std::vector<Slot> empacar(const std::vector<Item>& items, int anchoAtlas) {
    (void)items; (void)anchoAtlas;
    return {};
}

// TODO 2: alto total del atlas (max y+h; 0 si vacío)
static int altoTotal(const std::vector<Slot>& slots) {
    (void)slots;
    return 0;
}

// TODO 3: UV window u0 v0 u1 v1 (floats; sin half-texel)
static void uvDe(const Slot& s, int anchoAtlas, int altoAtlas,
                 float& u0, float& v0, float& u1, float& v1) {
    (void)s; (void)anchoAtlas; (void)altoAtlas;
    u0 = v0 = u1 = v1 = 0.0f;
}

// TODO 4: cuántos items NO entran (w > anchoAtlas)
static int inaceptables(const std::vector<Item>& items, int anchoAtlas) {
    (void)items; (void)anchoAtlas;
    return 0;
}

/* --------------------------- TESTS (no tocar) -------------------------- */
static const std::vector<Item> SPRITES = {
    {"mago",    16, 16},
    {"enemigo", 24, 24},
    {"moneda",   8,  8},
    {"fondo",   32,  6},
};

TEST_CASE(EmpaquetadoSimpleUnaFila) {
    // ancho 16+24+8+32 = 80: TODOS en una fila
    std::vector<Slot> s = empacar(SPRITES, 80);
    ASSERT_EQ(s.size(), (size_t)4);
    ASSERT_EQ(s[0].x, 0);  ASSERT_EQ(s[0].y, 0);
    ASSERT_EQ(s[1].x, 16);                      // tras el mago (16 de ancho)
    ASSERT_EQ(s[2].x, 40);                      // tras el enemigo (24)
    ASSERT_EQ(s[3].x, 48);                      // tras la moneda (8)
    // el ALTO de la fila es el del más alto (24)
    for (const Slot& x : s) ASSERT_EQ(x.y, 0);
}

TEST_CASE(DosFilas) {
    std::vector<Slot> s = empacar(SPRITES, 48);
    // mago(16) + enemigo(24) + moneda(8) = 48 EXACTO: fila 1 completa;
    // fondo(32) ya no entra: fila nueva en y=alto de fila 1 (24)
    ASSERT_EQ(s.size(), (size_t)4);
    ASSERT_EQ(s[0].y, 0);   ASSERT_EQ(s[1].y, 0);
    ASSERT_EQ(s[2].y, 0);   ASSERT_EQ(s[2].x, 40);
    ASSERT_EQ(s[3].y, 24);  ASSERT_EQ(s[3].x, 0);
    ASSERT_EQ(altoTotal(s), 30);        // fila2 y=24 + fondo h=6 -> 30
}

TEST_CASE(UVExactas) {
    std::vector<Slot> s = empacar({{"mago", 16, 16}}, 80);
    ASSERT_EQ(s.size(), (size_t)1);   // (sin resolver: guard del stub vacío)
    float u0, v0, u1, v1;
    uvDe(s[0], 80, 16, u0, v0, u1, v1);
    ASSERT_NEAR(u0, 0.0f,   1e-6f);
    ASSERT_NEAR(v0, 0.0f,   1e-6f);
    ASSERT_NEAR(u1, 0.2f,   1e-6f);   // 16/80
    ASSERT_NEAR(v1, 1.0f,   1e-6f);
}

TEST_CASE(Inaceptables) {
    ASSERT_EQ(inaceptables(SPRITES, 80), 0);
    ASSERT_EQ(inaceptables(SPRITES, 20), 2);   // enemigo(24) y fondo(32)
}

int main() { return RUN_ALL_TESTS(); }
