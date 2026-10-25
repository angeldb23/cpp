/* =============================================================================
   MÓDULO 17 - SOLUCIÓN 11: Tilemap — Colisión por Tile con Movimiento y Snap
   =============================================================================
   Resuelve ejercicio_11_tilemap_slide.cpp. La llave del move-and-slide es el
   separate-axis test: probar X e Y de forma independiente; el SNAP recoloca
   al borde exacto del tile bloqueado (ty*TILE - h para bajar, (tx+1)*TILE
   para ir a la izquierda). El detalle inclusivo del borde es igual al resto
   del módulo (convertir bordes con floor, la celda del borde derecho/down
   cuenta si toca).

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_11_tilemap_slide.cpp -o sol && ./sol
   ============================================================================= */
#include <vector>
#include <unordered_set>
#include <cmath>
#include <iostream>
#include "../../include/test_framework.hpp"

// ============================ MAPA FIJO (no tocar) ===========================
constexpr int TILE = 32;
constexpr int MAP_W = 12, MAP_H = 6;

inline int MapGid(int tx, int ty) {
    if (ty == 4 && tx >= 0 && tx < MAP_W) return 2;   // piso
    if (tx == 6 && ty >= 0 && ty <= 3) return 3;      // muro
    return 0;
}

// ============================ SOLUCIÓN =======================================
class Tilemap {
public:
    static int W() { return MAP_W; }
    static int H() { return MAP_H; }

    bool IsSolidCell(int tx, int ty) const {
        // TODO 1: solución — consulta O(1) + fuera de mapa = aire
        if (tx < 0 || ty < 0 || tx >= MAP_W || ty >= MAP_H) return false;
        return MapGid(tx, ty) == 2 || MapGid(tx, ty) == 3;
    }

    bool OverlapsSolid(float x, float y, float w, float h) const {
        // TODO 2: solución — recorre SOLO las celdas del rect (2x2 ~ 9 celdas);
        // bordes con floor (el toca-exactamente por abajo cuenta: piso)
        const int x0 = int(std::floor(x / TILE));
        const int x1 = int(std::floor((x + w) / TILE));
        const int y0 = int(std::floor(y / TILE));
        const int y1 = int(std::floor((y + h) / TILE));
        for (int ty = y0; ty <= y1; ++ty) {
            for (int tx = x0; tx <= x1; ++tx) {
                if (IsSolidCell(tx, ty)) return true;
            }
        }
        return false;
    }
};

struct Actor {
    float x = 0, y = 0;
    float w = 24, h = 24;
    float velY = 0, velX = 0;
};

// TODO 3: solución — eje Y por separado (separate-axis + snap)
template <typename MapT>
void MoveAndSlideY(Actor& a, float dy, const MapT& map) {
    const float futureY = a.y + dy;
    if (!map.OverlapsSolid(a.x, futureY, a.w, a.h)) {
        a.y = futureY;   // sin colisión: avanza todo
        return;
    }
    // hay bloqueo: SNAP al borde exacto del tile bloqueado
const long ty = (dy > 0.0f) ? long(std::floor((a.y + dy + a.h) / TILE))
                                : long(std::floor((a.y + dy) / TILE));
    if (dy > 0.0f) {
        a.y = float(ty * TILE) - a.h;    // borde superior del piso
    } else {
        a.y = float((ty + 1) * TILE);    // borde inferior del techo
    }
    a.velY = 0.0f;
}

// TODO 4: solución — eje X, misma idea
template <typename MapT>
void MoveAndSlideX(Actor& a, float dx, const MapT& map) {
    const float futureX = a.x + dx;
    if (!map.OverlapsSolid(futureX, a.y, a.w, a.h)) {
        a.x = futureX;
        return;
    }
    const long tx = (dx > 0.0f) ? long(std::floor((a.x + dx + a.w) / TILE))
                                : long(std::floor((a.x + dx) / TILE));
    if (dx > 0.0f) {
        a.x = float(tx * TILE) - a.w;
    } else {
        a.x = float((tx + 1) * TILE);
    }
    a.velX = 0.0f;
}

// ============================ TESTS (iguales) ================================
TEST_CASE(ConsultaSolidariaExacta) {
    Tilemap map;
    ASSERT_TRUE(map.IsSolidCell(0, 4));
    ASSERT_TRUE(map.IsSolidCell(11, 4));
    ASSERT_TRUE(map.IsSolidCell(6, 0));
    ASSERT_TRUE(map.IsSolidCell(6, 3));
    ASSERT_FALSE(map.IsSolidCell(0, 0));
    ASSERT_TRUE(map.IsSolidCell(7, 4));    // el piso corre por TODAS las columnas
    ASSERT_FALSE(map.IsSolidCell(7, 0));
    // fuera del mapa: aire
    ASSERT_FALSE(map.IsSolidCell(-1, 0));
    ASSERT_FALSE(map.IsSolidCell(0, 99));
}

TEST_CASE(OverlapsSolidDetectaElPiso) {
    Tilemap map;
    const float y = 4 * TILE - 24;   // actor asentado en el piso (104)
    ASSERT_TRUE(map.OverlapsSolid(96, y, 24, 24));
    ASSERT_FALSE(map.OverlapsSolid(96, 0, 24, 24));
}

TEST_CASE(CaidaConSnapExacto) {
    Tilemap map;
    Actor p;
    p.x = 96; p.y = 0;
    const float dt = 1.0f / 60.0f;
    for (int f = 0; f < 120; ++f) {
        p.velY += 800.0f * dt;
        MoveAndSlideY(p, p.velY * dt, map);
    }
    const float pisoY = 4 * TILE - p.h;
    ASSERT_NEAR(p.y, pisoY, 0.001f);
    ASSERT_NEAR(p.velY, 0.0f, 0.001f);
}

TEST_CASE(SlideDiagonalContraMuro) {
    Tilemap map;
    Actor s;
    s.x = 8 * TILE;
    s.y = 32;
    for (int f = 0; f < 30; ++f) {
        MoveAndSlideX(s, -8.0f, map);
        MoveAndSlideY(s, 0.0f, map);
    }
    const float muroEdge = 7 * TILE;     // borde derecho de la col del muro: (6+1)*32 = 224
    ASSERT_NEAR(s.x, muroEdge, 0.001f);
}

int main() {
    std::cout << "Solución 11: Tilemap Slide\n";
    return RUN_ALL_TESTS();
}
