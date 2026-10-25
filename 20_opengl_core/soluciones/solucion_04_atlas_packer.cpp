/* =============================================================================
   MÓDULO 20 - SOLUCIÓN 04: Atlas packer (shelf packing de sprites)
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 11. Resuelve el ejercicio_04_atlas_packer.cpp.

   Shelf packing: filas de izquierda a derecha; un item que no entra
   "abre" un estante nuevo en y = alto de la fila anterior; el alto de la
   fila es el del item más alto. Los huecos se quedan (la textura del
   packer "grande" del 22 recorta si importa).
   ============================================================================ */
#include "../../include/test_framework.hpp"
#include <string>
#include <vector>

struct Item  { std::string nombre; int w; int h; };
struct Slot  { std::string nombre; int x, y, w, h; };

// TODO 1
static std::vector<Slot> empacar(const std::vector<Item>& items, int anchoAtlas) {
    std::vector<Slot> out;
    out.reserve(items.size());
    int x = 0;
    int y = 0;
    int altoFila = 0;
    for (const Item& it : items) {
        if (it.w > anchoAtlas) continue;    // inaceptable: NO se empaca
        if (x + it.w > anchoAtlas) {        // no entra: estante nuevo
            x = 0;
            y += altoFila;
            altoFila = 0;
        }
        out.push_back({it.nombre, x, y, it.w, it.h});
        x += it.w;
        if (it.h > altoFila) altoFila = it.h;
    }
    return out;
}

// TODO 2
static int altoTotal(const std::vector<Slot>& slots) {
    int max = 0;
    for (const Slot& s : slots) {
        int fondo = s.y + s.h;
        if (fondo > max) max = fondo;
    }
    return max;
}

// TODO 3
static void uvDe(const Slot& s, int anchoAtlas, int altoAtlas,
                 float& u0, float& v0, float& u1, float& v1) {
    u0 = (float)s.x / (float)anchoAtlas;
    v0 = (float)s.y / (float)altoAtlas;
    u1 = (float)(s.x + s.w) / (float)anchoAtlas;
    v1 = (float)(s.y + s.h) / (float)altoAtlas;
}

// TODO 4
static int inaceptables(const std::vector<Item>& items, int anchoAtlas) {
    int n = 0;
    for (const Item& it : items) if (it.w > anchoAtlas) ++n;
    return n;
}

/* --------------------------- TESTS (no tocar) -------------------------- */
static const std::vector<Item> SPRITES = {
    {"mago",    16, 16},
    {"enemigo", 24, 24},
    {"moneda",   8,  8},
    {"fondo",   32,  6},
};

TEST_CASE(EmpaquetadoSimpleUnaFila) {
    std::vector<Slot> s = empacar(SPRITES, 80);
    ASSERT_EQ(s.size(), (size_t)4);
    ASSERT_EQ(s[0].x, 0);  ASSERT_EQ(s[0].y, 0);
    ASSERT_EQ(s[1].x, 16);
    ASSERT_EQ(s[2].x, 40);
    ASSERT_EQ(s[3].x, 48);
    for (const Slot& x : s) ASSERT_EQ(x.y, 0);
}

TEST_CASE(DosFilas) {
    std::vector<Slot> s = empacar(SPRITES, 48);
    ASSERT_EQ(s.size(), (size_t)4);
    ASSERT_EQ(s[0].y, 0);   ASSERT_EQ(s[1].y, 0);
    ASSERT_EQ(s[2].y, 0);   ASSERT_EQ(s[2].x, 40);
    ASSERT_EQ(s[3].y, 24);  ASSERT_EQ(s[3].x, 0);
    ASSERT_EQ(altoTotal(s), 30);
}

TEST_CASE(UVExactas) {
    std::vector<Slot> s = empacar({{"mago", 16, 16}}, 80);
    float u0, v0, u1, v1;
    uvDe(s[0], 80, 16, u0, v0, u1, v1);
    ASSERT_NEAR(u0, 0.0f, 1e-6f);
    ASSERT_NEAR(v0, 0.0f, 1e-6f);
    ASSERT_NEAR(u1, 0.2f, 1e-6f);
    ASSERT_NEAR(v1, 1.0f, 1e-6f);
}

TEST_CASE(Inaceptables) {
    ASSERT_EQ(inaceptables(SPRITES, 80), 0);
    ASSERT_EQ(inaceptables(SPRITES, 20), 2);
}

int main() { return RUN_ALL_TESTS(); }
