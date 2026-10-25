/* =============================================================================
   MÓDULO 23 - SOLUCIÓN 02: Spatial hash (grid uniforme)
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
    explicit SpatialHash(float cellSize) : cellSize_(cellSize) {
        if (cellSize_ <= f2d::kEps) cellSize_ = 1.0f;
    }

    void clear() {
        celdas_.clear();
        cajas_.clear();
    }

    void insertar(int id, const AABB& caja) {
        cajas_.push_back({id, caja});
        const int x0 = f2d::floorDiv(caja.minX, cellSize_);
        const int x1 = f2d::floorDiv(caja.maxX, cellSize_);
        const int y0 = f2d::floorDiv(caja.minY, cellSize_);
        const int y1 = f2d::floorDiv(caja.maxY, cellSize_);
        for (int y = y0; y <= y1; ++y) {
            for (int x = x0; x <= x1; ++x) {
                celdas_[{x, y}].push_back(id);
            }
        }
    }

    std::vector<Pair> pares() const {
        std::set<Pair> unicos;
        for (const auto& [clave, ids] : celdas_) {
            (void)clave;
            for (size_t i = 0; i < ids.size(); ++i) {
                for (size_t j = i + 1; j < ids.size(); ++j) {
                    const Pair p{std::min(ids[i], ids[j]), std::max(ids[i], ids[j])};
                    if (!seTocan(p.a, p.b)) continue;
                    unicos.insert(p);
                }
            }
        }
        return {unicos.begin(), unicos.end()};
    }

    size_t celdas() const { return celdas_.size(); }

private:
    bool seTocan(int a, int b) const {
        const AABB* ca = nullptr;
        const AABB* cb = nullptr;
        for (const auto& c : cajas_) {
            if (c.first == a) ca = &c.second;
            if (c.first == b) cb = &c.second;
        }
        return ca && cb && ca->overlaps(*cb);
    }

    float cellSize_ = 1.0f;
    std::map<std::pair<int, int>, std::vector<int>> celdas_;
    std::vector<std::pair<int, AABB>> cajas_;
};

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
    grid.insertar(0, AABB(0, 0, 25, 25));
    grid.insertar(1, AABB(5, 5, 15, 15));
    ASSERT_EQ(grid.pares().size(), 1u);
}

TEST_CASE(AabbQueSoloSeTocan) {
    SpatialHash grid(10.0f);
    grid.insertar(0, AABB(0, 0, 10, 10));
    grid.insertar(1, AABB(10, 0, 20, 10));
    ASSERT_EQ(grid.pares().size(), 1u);
}

TEST_CASE(CoordenadasNegativas) {
    SpatialHash grid(10.0f);
    grid.insertar(0, AABB(-15, -15, -5, -5));
    grid.insertar(1, AABB(-12, -12, -8, -8));
    ASSERT_EQ(grid.pares().size(), 1u);
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
    std::cout << "Solución 02 (spatial hash)...\n";
    return RUN_ALL_TESTS();
}
