/* =============================================================================
   MÓDULO 17 - EJERCICIO 11: Tilemap — Colisión por Tile con Movimiento y Snap
   =============================================================================
   DIFICULTAD: Avanzado
   APOYO: Lección 10 (Colisión por tile, move-and-slide, culling)

   CONSIGNA:
   Con el mapa estático del stub (un piso de GID 2 en la fila 4 y un muro de
   GID 3 en la columna 6, tiles de 32):

   1) Tilemap::IsSolidCell(tx, ty): true si el GID del tile está en
      solidGids_. Fuera de los límites: false (aire abierto).
   2) Tilemap::OverlapsSolid(rect): recorre las celdas del rect y devuelve
      true si ALGUNA es sólida.
   3) MoveAndSlideY(actor, dy, map): si el AABB con el desplazamiento dy
      NO choca: aplicar dy completo. Si choca: SNAP al borde exacto del
      tile bloqueador bajando (dy>0) o subiendo (dy<0) y velY = 0.
   4) MoveAndSlideX: la misma idea en el eje X.

   La prueba del snap es EXACTA: con g=800 y dt fijo, el actor termina
   clavado en y = fila*32 - altura (sin temblor ni pasarse).

   TESTS (no tocar): consulta sólida, caída con snap exacto, y "slide"
   contra el muro dejando pasar el movimiento en Y.

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_11_tilemap_slide.cpp -o ej && ./ej
   ============================================================================= */
#include <vector>
#include <unordered_set>
#include <cmath>
#include <iostream>
#include "../../include/test_framework.hpp"

// ============================ MAPA FIJO (no tocar) ===========================
constexpr int TILE = 32;
constexpr int MAP_W = 12, MAP_H = 6;   // 384x192 de mundo

// Fila del piso: ty = 4 con GID 2 en TODAS las columnas
// Muro: tx = 6 con GID 3 en las filas 0..3
inline int MapGid(int tx, int ty) {
    if (ty == 4 && tx >= 0 && tx < MAP_W) return 2;    // piso
    if (tx == 6 && ty >= 0 && ty <= 3) return 3;       // muro
    return 0;                                          // vacío
}

// ============================ TU CÓDIGO AQUÍ ================================
class Tilemap {
public:
    static int W() { return MAP_W; }
    static int H() { return MAP_H; }

    // TODO 1: ¿es sólida la celda?
    bool IsSolidCell(int tx, int ty) const {
        // Pista: MapGid, fuera de límites => false
        return false;   // placeholder
    }

    // TODO 2: ¿el rect toca algún tile sólido?
    bool OverlapsSolid(float x, float y, float w, float h) const {
        // Pista: convertir bordes a celdas con floor(x/TILE), doble recorrida
        return false;   // placeholder
    }
};

struct Actor {
    float x = 0, y = 0;
    float w = 24, h = 24;
    float velY = 0, velX = 0;
};

// TODO 3: movimiento en Y con snap exacto
template <typename MapT>
void MoveAndSlideY(Actor& a, float dy, const MapT& map) {
    // Pistas:
    // 1) probar el AABB "futuro" (x, y+dy, w, h): si no sólido => y += dy
    // 2) si sólido con dy>0: ty = floor((y+dy+h)/TILE) es la fila
    //    bloqueadora => y = ty*TILE - a.h (borde superior del piso)
    // 3) ídem con dy<0: y = (ty+1)*TILE
    // 4) velY = 0 tras el snap
}

// TODO 4: movimiento en X con snap (misma lógica)
template <typename MapT>
void MoveAndSlideX(Actor& a, float dx, const MapT& map) {
    (void)a; (void)dx; (void)map;
}

// ============================ TESTS (no tocar) ==============================
TEST_CASE(ConsultaSolidariaExacta) {
    Tilemap map;
    // Piso en fila 4
    ASSERT_TRUE(map.IsSolidCell(0, 4));
    ASSERT_TRUE(map.IsSolidCell(11, 4));
    // Muro en columna 6
    ASSERT_TRUE(map.IsSolidCell(6, 0));
    ASSERT_TRUE(map.IsSolidCell(6, 3));
    // Air (muro solo en filas 0..3 de la col 6; el piso es SOLO fila 4)
    ASSERT_FALSE(map.IsSolidCell(0, 0));
    ASSERT_TRUE(map.IsSolidCell(7, 4));    // el piso corre por TODAS las columnas
    ASSERT_FALSE(map.IsSolidCell(7, 0));
    // fuera del mapa: aire
    ASSERT_FALSE(map.IsSolidCell(-1, 0));
    ASSERT_FALSE(map.IsSolidCell(0, 99));
}

TEST_CASE(OverlapsSolidDetectaElPiso) {
    Tilemap map;
    const float y = 4 * TILE - 24;      // 104: el actor asentado toca el piso por abajo
    // AABB del actor en y=104: 104..128 toca la fila 4 que es piso
    ASSERT_TRUE(map.OverlapsSolid(96, y, 24, 24));
    // en el aire: celeste
    ASSERT_FALSE(map.OverlapsSolid(96, 0, 24, 24));
}

TEST_CASE(CaidaConSnapExacto) {
    Tilemap map;
    Actor p;
    p.x = 96; p.y = 0;                  // celda (3, ~): aire encima del piso
    const float dt = 1.0f / 60.0f;
    for (int f = 0; f < 120; ++f) {     // 2s a 60 FPS: seguro llega + snap
        p.velY += 800.0f * dt;
        MoveAndSlideY(p, p.velY * dt, map);
    }
    const float pisoY = 4 * TILE - p.h;  // 104
    ASSERT_NEAR(p.y, pisoY, 0.001f);     // snap EXACTO (sin temblor)
    ASSERT_NEAR(p.velY, 0.0f, 0.001f);   // velocidad anulada
}

TEST_CASE(SlideDiagonalContraMuro) {
    Tilemap map;
    Actor s;
    s.x = 8 * TILE;                     // 256: arranca a la derecha del muro
    s.y = 32;                           // fila 1 (el muro cubre filas 0..3)
    for (int f = 0; f < 30; ++f) {
        MoveAndSlideX(s, -8.0f, map);   // empuja a la izquierda
        MoveAndSlideY(s, 0.0f, map);
    }
    // El muro (col 6: x 192..224) lo fija: x EXACTAMENTE 224 = (7)*32,
    // que es (tx_bloqueador + 1) * TILE con tx_bloqueador = 6
    ASSERT_NEAR(s.x, 224.0f, 0.001f);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 11 (Tilemap Slide)...\n";
    return RUN_ALL_TESTS();
}
