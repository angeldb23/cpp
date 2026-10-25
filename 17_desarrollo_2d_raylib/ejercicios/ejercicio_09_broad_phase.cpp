/* =============================================================================
   MÓDULO 17 - EJERCICIO 09: Broad-Phase — Spatial Hash con Grid Uniforme
   =============================================================================
   DIFICULTAD: Avanzado
   APOYO: Lección 07 (Broad-phase con grid uniforme)

   CONSIGNA:
   Completa SpatialGrid con celda de 100x100:

   1) CellOf(coord): índice de celda = floor(coord / cellSize). Con negativos
      FUNCIONA con floor (floor(-1/100) = -1, la celda de la izquierda).
   2) Insert(index): registra el índice en TODAS las celdas que toca el AABB
      (borde superior-izq inclusivo, inferior-dcho con umbral), con clave
      empaquetada KeyCell(cx, cy).
   3) QueryIndices(rect): junta los índices de las celdas del rect SIN
      repetirlos (dedup con unordered_set).
   4) CollectPairs(grid, aabbs): por cada entidad consulta sus candidatos;
      pares sin duplicado con i<j; NARROW phase (RectsOverlap) recortando.

   La GARANTÍA de la lección 07 que el test verifica: para el dataset fijo,
   CollectPairs devuelve EXACTAMENTE el par (0, 2) (los pares de la grilla
   coinciden con lo que daría O(N²)).

   TESTS (no tocar): floor con negativos, cantidad de celdas por entidad,
   query con dedup y pares esperados.

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_09_broad_phase.cpp -o ej && ./ej
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

// Dataset FIJO:
//   e0: (120..170, 40..70)   celdas: x∈{1}, y∈{0}      => 1 celda
//   e1: (40..90, 120..150)   celdas: x∈{0}, y∈{1}      => 1 celda
//   e2: (160..230, 20..60)   celdas: x∈{1,2}, y∈{0}    => 2 celdas
//   e3: (250..310, 130..190) celdas: x∈{2,3}, y∈{1}    => 2 celdas
// Overlaps reales (narrow): SOLO e0 con e2.
inline Rectangle GetEntity(long i) {
    switch (i) {
        case 0: return {120, 40, 50, 30};
        case 1: return {40, 120, 40, 30};
        case 2: return {160, 20, 70, 40};
        default: return {250, 130, 60, 60};
    }
}

// ============================ TU CÓDIGO AQUÍ ================================
class SpatialGrid {
public:
    static float CellSize() { return CELL; }

    // TODO 1: índice de celda con floor (funciona para negativos)
    static long CellOf(float coord) {
        return 0;   // placeholder
    }

    // TODO 2: inserta el índice en cada celda que toca el AABB
    void Insert(const Rectangle& aabb, long index) {
        (void)aabb; (void)index;
        // Pistas:
        // x0 = CellOf(aabb.x), x1 = CellOf(aabb.x + w - EPS)  (borde excl.)
        // idem y. Después doble loop (for y0..y1) (for x0..x1)
        // agregando el índice en cells_[KeyCell(cx, cy)] e increments.
    }

    // TODO 3: índices de las celdas tocadas (sin repeticiones)
    void QueryIndices(const Rectangle& area, std::vector<long>& out) const {
        (void)area; (void)out;
        // Pista: recorre las celdas del área; por cada una, busca en cells_
        // y agrega al output solo si no estaba (unordered_set de dedup)
    }

    // Diagnóstico de test
    long RegistroCount() const { return totalInserts_; }

private:
    static uint64_t KeyCell(long cx, long cy) {
        // convertí cada long a uint32_t ANTES del shift (lección 07)
        return (static_cast<uint64_t>(static_cast<uint32_t>(cy)) << 32)
             |  static_cast<uint32_t>(cx);
    }
    std::unordered_map<uint64_t, std::vector<long>> cells_;
    long totalInserts_ = 0;
};

// TODO 4: broad + narrow con pares (i<j) únicos
inline void CollectPairs(const SpatialGrid& grid, const std::vector<Rectangle>& aabbs,
                         std::vector<std::pair<long, long>>& out) {
    (void)grid; (void)aabbs; (void)out;
    // Pistas (lección 07):
    // a) out.clear()
    // b) por cada i, grid.QueryIndices(aabbs[i], candidates)
    // c) por cada j de candidates (j != i), RectsOverlap => par propulsionado
    // d) dedupe pares como (min, max) con unordered_set de pairs
}

// ============================ TESTS (no tocar) ==============================
TEST_CASE(CellOfFloorConNegativos) {
    ASSERT_EQ(SpatialGrid::CellOf(50.0f), 0);
    ASSERT_EQ(SpatialGrid::CellOf(99.99f), 0);
    ASSERT_EQ(SpatialGrid::CellOf(100.0f), 1);
    ASSERT_EQ(SpatialGrid::CellOf(-1.0f), -1);      // floor(-0.01) = -1
    ASSERT_EQ(SpatialGrid::CellOf(-100.0f), -1);
    ASSERT_EQ(SpatialGrid::CellOf(-101.0f), -2);
}

TEST_CASE(InsertCuentaCeldasTocadas) {
    SpatialGrid grid;
    grid.Insert(GetEntity(0), 0);   // e0: 1 celda (1, 0)
    ASSERT_EQ(grid.RegistroCount(), 1);
    grid.Insert(GetEntity(1), 1);   // e1: 1 celda (0, 1)
    ASSERT_EQ(grid.RegistroCount(), 2);
    grid.Insert(GetEntity(2), 2);   // e2: 2 celdas ((1,0) y (2,0))
    ASSERT_EQ(grid.RegistroCount(), 4);
    grid.Insert(GetEntity(3), 3);   // e3: x celdas 2 y 3; y celda 1 => 2 celdas
    ASSERT_EQ(grid.RegistroCount(), 6);
}

TEST_CASE(QueryIndicesDevuelveCandidatosSinRepetir) {
    SpatialGrid grid;
    for (long i = 0; i < N_ENTITIES; ++i) grid.Insert(GetEntity(i), i);
    // Área de e0: celdas (1,0). Ahí están registrados e0 y e2 (idx 0 y 2)
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
    for (long i = 0; i < N_ENTITIES; ++i) aabbs.push_back(GetEntity(i));
    std::vector<std::pair<long, long>> pairs;
    CollectPairs(grid, aabbs, pairs);
    // El único par real solapado en el dataset fijo
    ASSERT_EQ(pairs.size(), 1u);
    ASSERT_EQ(pairs[0].first, 0);
    ASSERT_EQ(pairs[0].second, 2);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 09 (Broad-Phase)...\n";
    return RUN_ALL_TESTS();
}
