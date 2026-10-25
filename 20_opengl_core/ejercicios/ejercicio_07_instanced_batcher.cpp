/* =============================================================================
   MÓDULO 20 - EJERCICIO 07: Batcher de sprites con instancing
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 14 (cada variar de bind de textura / draw call es la
          letal del CPU: glDrawArraysInstanced + ordenar por texId deja 1
          draw call POR TEXTURA en lugar de 1 por sprite).

   CONSIGNA (lógica pura; nada de GL):
     struct Sprite { int id; int texId; float x; float y; };
       id = identidad del sprite (verificar si el orden interno queda estable)
     struct Rango  { int texId; int desde; int cantidad; };
       un glDrawArraysInstanced desde `desde` (en el buffer de instancias)

     1. `ordenarPorTex(sprites)`: TODO el array ordenado por texId
        ASCENDENTE, con orden ESTABLE: a igual texId conservan el orden
        ORIGINAL (std::stable_sort; compartir textura es lo único que
        importa para el bind).
     2. `rangos(ordenado)`: SOBRE la lista YA ordenada, un rango por
        corrida de texId iguales: {texId, índice inicial, cantidad}.
     3. `drawCalls(ordenado)`: 1 llamada POR RANGO.

   TESTS (no tocar). COMPILACIÓN Y CORRIDA:
     g++ -std=c++20 -Wall -Wextra ejercicio_07_instanced_batcher.cpp -o ej && ./ej
   ============================================================================ */
#include "../../include/test_framework.hpp"
#include <vector>

struct Sprite { int id; int texId; float x; float y; };
struct Rango  { int texId; int desde; int cantidad; };

/* ------------------------------ TU CÓDIGO AQUÍ ---------------------------- */

// TODO 1: orden estable por texId
static std::vector<Sprite> ordenarPorTex(const std::vector<Sprite>& sprites) {
    (void)sprites;
    return {};
}

// TODO 2: rangos contiguos de la lista ya ordenada
static std::vector<Rango> rangos(const std::vector<Sprite>& ordenado) {
    (void)ordenado;
    return {};
}

// TODO 3: cuántas draw calls
static int drawCalls(const std::vector<Sprite>& ordenado) {
    (void)ordenado;
    return 0;
}

/* --------------------------- TESTS (no tocar) -------------------------- */

TEST_CASE(OrdenStable) {
    // texturas entremezcladas: 3 1 3 1 2 1
    std::vector<Sprite> s = {
        {10, 3, 0.f, 0.f}, {11, 1, 1.f, 0.f}, {12, 3, 2.f, 0.f},
        {13, 1, 3.f, 0.f}, {14, 2, 4.f, 0.f}, {15, 1, 5.f, 0.f},
    };
    std::vector<Sprite> o = ordenarPorTex(s);
    ASSERT_EQ(o.size(), (size_t)6);            // guard del stub vacío
    // tex 1: originales 11, 13, 15 EN ESE ORDEN (estable); no 11, 15, 13
    ASSERT_EQ(o[0].id, 11); ASSERT_EQ(o[0].texId, 1);
    ASSERT_EQ(o[1].id, 13); ASSERT_EQ(o[1].texId, 1);
    ASSERT_EQ(o[2].id, 15); ASSERT_EQ(o[2].texId, 1);
    ASSERT_EQ(o[3].id, 14); ASSERT_EQ(o[3].texId, 2);
    ASSERT_EQ(o[4].id, 10); ASSERT_EQ(o[4].texId, 3);
    ASSERT_EQ(o[5].id, 12); ASSERT_EQ(o[5].texId, 3);
    // posiciones VIAJAN con el sprite (el buffer de instancias las lleva)
    ASSERT_NEAR(o[4].x, 0.f, 1e-6f);           // el sprite 10 vivía en x=0
    ASSERT_NEAR(o[5].x, 2.f, 1e-6f);           // y el 12 en x=2
}

TEST_CASE(RangosYCalls) {
    std::vector<Sprite> s = {
        {10, 3, 0.f, 0.f}, {11, 1, 1.f, 0.f}, {12, 3, 2.f, 0.f},
        {13, 1, 3.f, 0.f}, {14, 2, 4.f, 0.f}, {15, 1, 5.f, 0.f},
    };
    std::vector<Sprite> o = ordenarPorTex(s);
    std::vector<Rango> r = rangos(o);
    ASSERT_EQ(r.size(), (size_t)3);            // guard: stub devuelve {}
    ASSERT_EQ(r[0].texId, 1); ASSERT_EQ(r[0].desde, 0); ASSERT_EQ(r[0].cantidad, 3);
    ASSERT_EQ(r[1].texId, 2); ASSERT_EQ(r[1].desde, 3); ASSERT_EQ(r[1].cantidad, 1);
    ASSERT_EQ(r[2].texId, 3); ASSERT_EQ(r[2].desde, 4); ASSERT_EQ(r[2].cantidad, 2);
    ASSERT_EQ(drawCalls(o), 3);                // 6 sprites → 3 draw calls
}

TEST_CASE(Degeneres) {
    ASSERT_EQ(drawCalls({}), 0);               // nada que dibujar
    ASSERT_TRUE(rangos({}).empty());
    std::vector<Sprite> solo = {{7, 5, 1.f, 1.f}};
    ASSERT_EQ(drawCalls(solo), 1);
    ASSERT_EQ(rangos(solo).size(), (size_t)1);
    // TODOS la misma textura: UNA draw call con todas las instancias
    std::vector<Sprite> misma = {
        {1, 9, 0, 0}, {2, 9, 0, 0}, {3, 9, 0, 0}, {4, 9, 0, 0},
    };
    ASSERT_EQ(drawCalls(misma), 1);
    std::vector<Rango> r3 = rangos(misma);
    ASSERT_EQ(r3.size(), (size_t)1);           // guard: con stub vacío no indexamos
    ASSERT_EQ(r3[0].cantidad, 4);
}

int main() { return RUN_ALL_TESTS(); }
