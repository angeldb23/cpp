/* =============================================================================
   MÓDULO 17 - SOLUCIÓN 09: Broad-Phase — Spatial Hash con Grid Uniforme
   =============================================================================
   Resuelve ejercicio_09_broad_phase.cpp. Los puntos delicados (lección 07):
   1) CellOf usa floor, no trunc: trunc(-0.01) = 0 (CUBRE el negativo), floor
      = -1 (celda de la izquierda: correcto).
   2) El borde max usa el umbral -EPS para que el final exacto no "suba" de
      celda sin necesidad (170.0 -> floor(169.9999) = 1, no 2).
   3) La clave empaqueta (cx, cy) en 64 bits tras convertir a uint32_t: las
      celdas negativas no chocan con las positivas.
   4) Los pares se deduplican normalizando (min, max) por índice.

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_09_broad_phase.cpp -o sol && ./sol
   ============================================================================= */
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include "../../include/test_framework.hpp"

// ============================ STUBS (no tocar) ===============================
struct Rectangle { float x, y, width, height; };

inline bool RectsOverlap(const Rectangle& a, const Rectangle& b) {
    if (a.x + a.width < b.x || b.x + b.width < a.x) return false;
    if (a.y + a.height < b.y || b.y + b.height < a.y) return false;
    return true;
}
constexpr float CELL = 100.0f;
constexpr long N_ENTITIES = 4;

inline Rectangle GetEntity(long i) {
    switch (i) {
        case 0: return {120, 40, 50, 30};
        case 1: return {40, 120, 40, 30};
        case 2: return {160, 20, 70, 40};
        default: return {250, 130, 60, 60};
    }
}

// ============================ SOLUCIÓN ========================================
class SpatialGrid {
public:
    static float CellSize() { return CELL; }

    static long CellOf(float coord) {
        // TODO 1: solución. floor (NO trunc): así (-1, 0) es la celda
        // izquierda de la vez y no se fusiona con (0, 0) del trunc
        return static_cast<long>(std::floor(coord / CELL));
    }

    void Insert(const Rectangle& aabb, long index) {
        // TODO 2: solución
        constexpr float EPS = 0.0001f;
        const long x0 = CellOf(aabb.x);
        const long x1 = CellOf(aabb.x + aabb.width  - EPS);
        const long y0 = CellOf(aabb.y);
        const long y1 = CellOf(aabb.y + aabb.height - EPS);
        for (long cy = y0; cy <= y1; ++cy) {
            for (long cx = x0; cx <= x1; ++cx) {
                cells_[KeyCell(cx, cy)].push_back(index);
                ++totalInserts_;
            }
        }
    }

    void QueryIndices(const Rectangle& area, std::vector<long>& out) const {
        // TODO 3: solución: dedup con set, salida SIN repetidos
        constexpr float EPS = 0.0001f;
        const long x0 = CellOf(area.x);
        const long x1 = CellOf(area.x + area.width  - EPS);
        const long y0 = CellOf(area.y);
        const long y1 = CellOf(area.y + area.height - EPS);

        std::unordered_set<long> seen;
        out.clear();
        for (long cy = y0; cy <= y1; ++cy) {
            for (long cx = x0; cx <= x1; ++cx) {
                auto it = cells_.find(KeyCell(cx, cy));
                if (it == cells_.end()) continue;
                for (long idx : it->second) {
                    // insert() devuelve {posición, insertado}: segundo = true
                    // si ES NOVO (dedup limpio)
                    if (seen.insert(idx).second) {
                        out.push_back(idx);
                    }
                }
            }
        }
    }

    long RegistroCount() const { return totalInserts_; }

private:
    static uint64_t KeyCell(long cx, long cy) {
        return (static_cast<uint64_t>(static_cast<uint32_t>(cy)) << 32)
             |  static_cast<uint32_t>(cx);
    }
    std::unordered_map<uint64_t, std::vector<long>> cells_;
    long totalInserts_ = 0;
};

// TODO 4: solución: broad (candidates del grid) + narrow (overlap exacto)
inline void CollectPairs(const SpatialGrid& grid, const std::vector<Rectangle>& aabbs,
                         std::vector<std::pair<long, long>>& out) {
    out.clear();
    std::unordered_set<uint64_t> seenPairs;   // clave de par normalizado (i<j)
    std::vector<long> candidates;
    for (std::size_t i = 0; i < aabbs.size(); ++i) {
        grid.QueryIndices(aabbs[i], candidates);
        for (long j : candidates) {
            if (j == long(i)) continue;                      // self no es par
            if (!RectsOverlap(aabbs[i], aabbs[j])) continue; // narrow exacto
            const long lo = std::min(long(i), j);
            const long hi = std::max(long(i), j);             // normalización
            // Empaca el par normalizado en 64 bits: (lo<<32)|hi
            const uint64_t key =
                (static_cast<uint64_t>(static_cast<uint32_t>(hi)) << 32)
                | static_cast<uint32_t>(lo);
            if (seenPairs.insert(key).second) {
                out.emplace_back(lo, hi);
            }
        }
    }
}

// ============================ TESTS (iguales) ================================
TEST_CASE(CellOfFloorConNegativos) {
    ASSERT_EQ(SpatialGrid::CellOf(50.0f), 0);
    ASSERT_EQ(SpatialGrid::CellOf(99.99f), 0);
    ASSERT_EQ(SpatialGrid::CellOf(100.0f), 1);
    ASSERT_EQ(SpatialGrid::CellOf(-1.0f), -1);
    ASSERT_EQ(SpatialGrid::CellOf(-100.0f), -1);
    ASSERT_EQ(SpatialGrid::CellOf(-101.0f), -2);
}

TEST_CASE(InsertCuentaCeldasTocadas) {
    SpatialGrid grid;
    grid.Insert(GetEntity(0), 0);
    ASSERT_EQ(grid.RegistroCount(), 1);
    grid.Insert(GetEntity(1), 1);
    ASSERT_EQ(grid.RegistroCount(), 2);
    grid.Insert(GetEntity(2), 2);
    ASSERT_EQ(grid.RegistroCount(), 4);
    grid.Insert(GetEntity(3), 3);
    ASSERT_EQ(grid.RegistroCount(), 6);
}

TEST_CASE(QueryIndicesDevuelveCandidatosSinRepetir) {
    SpatialGrid grid;
    for (long i = 0; i < N_ENTITIES; ++i) grid.Insert(GetEntity(i), i);
    std::vector<long> out;
    grid.QueryIndices({120, 40, 50, 30}, out);
    ASSERT_EQ(out.size(), 2u);
    auto contains = [&](long idx) {
        return std::find(out.begin(), out.end(), idx) != out.end();
    };
    ASSERT_TRUE(contains(0));
    ASSERT_TRUE(contains(2));
}

TEST_CASE(CollectPairsIgualABruceForce) {
    SpatialGrid grid;
    std::vector<Rectangle> aabbs;
    for (long i = 0; i < N_ENTITIES; ++i) {
        aabbs.push_back(GetEntity(i));
        grid.Insert(GetEntity(i), i);   // PRIMERO poblar la grilla
    }
    std::vector<std::pair<long, long>> pairs;
    CollectPairs(grid, aabbs, pairs);
    ASSERT_EQ(pairs.size(), 1u);
    ASSERT_EQ(pairs[0].first, 0);
    ASSERT_EQ(pairs[0].second, 2);
}

int main() {
    std::cout << "Solución 09: Broad-Phase\n";
    return RUN_ALL_TESTS();
}
