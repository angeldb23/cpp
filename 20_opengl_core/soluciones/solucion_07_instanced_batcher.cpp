/* =============================================================================
   MÓDULO 20 - SOLUCIÓN 07: Batcher de sprites con instancing
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 14. Resuelve el ejercicio_07_instanced_batcher.cpp.

   estable = std::stable_sort: los sprites con LA MISMA textura quedan en
   el orden de submission (importa para el z-order artístico del juego).
   Una corrida de texId iguales = un Rango = UN glDrawArraysInstanced con
   `cantidad` instancias: 6 sprites → 3 draw calls en el test.
   ============================================================================ */
#include "../../include/test_framework.hpp"
#include <algorithm>
#include <vector>

struct Sprite { int id; int texId; float x; float y; };
struct Rango  { int texId; int desde; int cantidad; };

// TODO 1
static std::vector<Sprite> ordenarPorTex(const std::vector<Sprite>& sprites) {
    std::vector<Sprite> o = sprites;
    std::stable_sort(o.begin(), o.end(),
                     [](const Sprite& a, const Sprite& b) { return a.texId < b.texId; });
    return o;
}

// TODO 2
static std::vector<Rango> rangos(const std::vector<Sprite>& ordenado) {
    std::vector<Rango> r;
    size_t i = 0;
    while (i < ordenado.size()) {
        size_t j = i;
        while (j < ordenado.size() && ordenado[j].texId == ordenado[i].texId) ++j;
        r.push_back({ordenado[i].texId, (int)i, (int)(j - i)});
        i = j;
    }
    return r;
}

// TODO 3
static int drawCalls(const std::vector<Sprite>& ordenado) {
    return (int)rangos(ordenado).size();
}

/* --------------------------- TESTS (no tocar) -------------------------- */

TEST_CASE(OrdenStable) {
    std::vector<Sprite> s = {
        {10, 3, 0.f, 0.f}, {11, 1, 1.f, 0.f}, {12, 3, 2.f, 0.f},
        {13, 1, 3.f, 0.f}, {14, 2, 4.f, 0.f}, {15, 1, 5.f, 0.f},
    };
    std::vector<Sprite> o = ordenarPorTex(s);
    ASSERT_EQ(o.size(), (size_t)6);
    ASSERT_EQ(o[0].id, 11); ASSERT_EQ(o[0].texId, 1);
    ASSERT_EQ(o[1].id, 13); ASSERT_EQ(o[1].texId, 1);
    ASSERT_EQ(o[2].id, 15); ASSERT_EQ(o[2].texId, 1);
    ASSERT_EQ(o[3].id, 14); ASSERT_EQ(o[3].texId, 2);
    ASSERT_EQ(o[4].id, 10); ASSERT_EQ(o[4].texId, 3);
    ASSERT_EQ(o[5].id, 12); ASSERT_EQ(o[5].texId, 3);
    ASSERT_NEAR(o[4].x, 0.f, 1e-6f);
    ASSERT_NEAR(o[5].x, 2.f, 1e-6f);
}

TEST_CASE(RangosYCalls) {
    std::vector<Sprite> s = {
        {10, 3, 0.f, 0.f}, {11, 1, 1.f, 0.f}, {12, 3, 2.f, 0.f},
        {13, 1, 3.f, 0.f}, {14, 2, 4.f, 0.f}, {15, 1, 5.f, 0.f},
    };
    std::vector<Sprite> o = ordenarPorTex(s);
    std::vector<Rango> r = rangos(o);
    ASSERT_EQ(r.size(), (size_t)3);
    ASSERT_EQ(r[0].texId, 1); ASSERT_EQ(r[0].desde, 0); ASSERT_EQ(r[0].cantidad, 3);
    ASSERT_EQ(r[1].texId, 2); ASSERT_EQ(r[1].desde, 3); ASSERT_EQ(r[1].cantidad, 1);
    ASSERT_EQ(r[2].texId, 3); ASSERT_EQ(r[2].desde, 4); ASSERT_EQ(r[2].cantidad, 2);
    ASSERT_EQ(drawCalls(o), 3);
}

TEST_CASE(Degeneres) {
    ASSERT_EQ(drawCalls({}), 0);
    ASSERT_TRUE(rangos({}).empty());
    std::vector<Sprite> solo = {{7, 5, 1.f, 1.f}};
    ASSERT_EQ(drawCalls(solo), 1);
    ASSERT_EQ(rangos(solo).size(), (size_t)1);
    std::vector<Sprite> misma = {
        {1, 9, 0, 0}, {2, 9, 0, 0}, {3, 9, 0, 0}, {4, 9, 0, 0},
    };
    ASSERT_EQ(drawCalls(misma), 1);
    std::vector<Rango> r3 = rangos(misma);
    ASSERT_EQ(r3.size(), (size_t)1);
    ASSERT_EQ(r3[0].cantidad, 4);
}

int main() { return RUN_ALL_TESTS(); }
