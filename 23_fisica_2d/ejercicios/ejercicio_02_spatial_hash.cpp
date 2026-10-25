/* =============================================================================
   MÓDULO 23 - EJERCICIO 02: Spatial hash (grid uniforme)
   =============================================================================
   DIFICULTAD: Medio
   APOYO: Lección 02 (spatial hash, celdas y deduplicación).

   CONSIGNA:
   Implementa un broad-phase con grid uniforme:
     - `insertar(id, caja)` mete el id en TODAS las celdas que toca el AABB.
     - `pares()` devuelve los pares únicos (a < b) cuyos AABBs se solapan.
     - `celdas()` devuelve la cantidad de celdas ocupadas.
   Usá `f2d::floorDiv` para las coordenadas negativas y un `std::set<Pair>`
   para deduplicar: dos cajas pueden compartir varias celdas.
   ============================================================================= */
#include "../include/fisica2d.hpp"
#include "../../include/test_framework.hpp"

#include <map>
#include <set>
#include <utility>
#include <vector>

using f2d::AABB;
using f2d::Pair;

class SpatialHash {
public:
    explicit SpatialHash(float cellSize) : cellSize_(cellSize) {}

    void clear() {
        // TODO
    }

    void insertar(int id, const AABB& caja) {
        // TODO
        (void)id;
        (void)caja;
    }

    std::vector<Pair> pares() const {
        // TODO
        return {};
    }

    size_t celdas() const {
        // TODO
        return 0;
    }

private:
    float cellSize_ = 1.0f;
    std::map<std::pair<int, int>, std::vector<int>> celdas_;
    std::vector<std::pair<int, AABB>> cajas_;
};

// ============================================================================
// TESTS (no tocar)
// ============================================================================
TEST_CASE(DosCajasSuperpuestas) {
    SpatialHash grid(10.0f);
    grid.insertar(0, AABB(0, 0, 5, 5));
    grid.insertar(1, AABB(3, 3, 8, 8));
    const auto p = grid.pares();
    ASSERT_EQ(p.size(), 1u);
    ASSERT_EQ(p[0].a, 0);
    ASSERT_EQ(p[0].b, 1);
    ASSERT_EQ(grid.celdas(), 1u);
}

TEST_CASE(CajasSeparadasNoSeEmparejan) {
    SpatialHash grid(10.0f);
    grid.insertar(0, AABB(0, 0, 5, 5));
    grid.insertar(1, AABB(20, 20, 25, 25));
    ASSERT_EQ(grid.pares().size(), 0u);
    ASSERT_EQ(grid.celdas(), 2u);
}

TEST_CASE(DedupeEnVariasCeldas) {
    SpatialHash grid(10.0f);
    grid.insertar(0, AABB(0, 0, 25, 25));   // 3x3 celdas
    grid.insertar(1, AABB(5, 5, 15, 15));   // 1 celda, dentro de las 9
    ASSERT_EQ(grid.pares().size(), 1u);
}

TEST_CASE(AabbQueSoloSeTocan) {
    SpatialHash grid(10.0f);
    grid.insertar(0, AABB(0, 0, 10, 10));
    grid.insertar(1, AABB(10, 0, 20, 10));
    // Borde inclusivo: tocarse cuenta como candidato de broad-phase.
    ASSERT_EQ(grid.pares().size(), 1u);
}

TEST_CASE(CoordenadasNegativas) {
    SpatialHash grid(10.0f);
    grid.insertar(0, AABB(-15, -15, -5, -5));
    grid.insertar(1, AABB(-12, -12, -8, -8));
    ASSERT_EQ(grid.pares().size(), 1u);
    // floorDiv(-15,10) = -2 y floorDiv(-5,10) = -1: 2x2 celdas.
    ASSERT_EQ(grid.celdas(), 4u);
}

TEST_CASE(ClearVaciaElGrid) {
    SpatialHash grid(10.0f);
    grid.insertar(0, AABB(0, 0, 5, 5));
    grid.clear();
    ASSERT_EQ(grid.celdas(), 0u);
    ASSERT_EQ(grid.pares().size(), 0u);
}

int main() {
    std::cout << "Ejercicio 02 (spatial hash)...\n";
    return RUN_ALL_TESTS();
}
