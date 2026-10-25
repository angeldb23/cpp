/* =============================================================================
   MÓDULO 23 - SOLUCIÓN 04: SAT para polígonos convexos
   ============================================================================= */
#include "../include/fisica2d.hpp"
#include "../../include/test_framework.hpp"

#include <vector>

using f2d::Vec2;

struct Poly {
    std::vector<Vec2> pts;
};

struct SatResult {
    bool overlap = false;
    Vec2 normal;
    float depth = 0.0f;
};

namespace {

Vec2 centro(const Poly& p) {
    Vec2 c;
    for (const auto& v : p.pts) c += v;
    if (!p.pts.empty()) c = c / static_cast<float>(p.pts.size());
    return c;
}

struct Proy {
    float min = 0.0f;
    float max = 0.0f;
};

Proy proyectar(const Poly& p, const Vec2& eje) {
    Proy r{dot(p.pts[0], eje), dot(p.pts[0], eje)};
    for (const auto& v : p.pts) {
        const float d = dot(v, eje);
        r.min = std::min(r.min, d);
        r.max = std::max(r.max, d);
    }
    return r;
}

} // namespace

SatResult satPolygons(const Poly& a, const Poly& b) {
    if (a.pts.size() < 3 || b.pts.size() < 3) return {};

    float menorSolape = 1e30f;
    Vec2 mejorEje;

    auto probar = [&](const Poly& p) -> bool {
        for (size_t i = 0; i < p.pts.size(); ++i) {
            const Vec2& v0 = p.pts[i];
            const Vec2& v1 = p.pts[(i + 1) % p.pts.size()];
            const Vec2 eje = f2d::normalize(f2d::perp(v1 - v0));
            if (f2d::lengthSq(eje) < f2d::kEps) continue;

            const Proy pa = proyectar(a, eje);
            const Proy pb = proyectar(b, eje);
            const float solape = std::min(pa.max, pb.max) - std::max(pa.min, pb.min);
            if (solape <= 0.0f) return false;
            if (solape < menorSolape) {
                menorSolape = solape;
                mejorEje = eje;
            }
        }
        return true;
    };

    if (!probar(a) || !probar(b)) return {};

    if (dot(mejorEje, centro(b) - centro(a)) < 0.0f) mejorEje = -mejorEje;
    return {true, mejorEje, menorSolape};
}

namespace {
Poly caja(float hx, float hy, Vec2 c = {}) {
    return {{{c.x - hx, c.y - hy}, {c.x + hx, c.y - hy},
             {c.x + hx, c.y + hy}, {c.x - hx, c.y + hy}}};
}
} // namespace

TEST_CASE(CajasSolapadas) {
    const auto r = satPolygons(caja(1, 1), caja(1, 1, {1.5f, 0.0f}));
    ASSERT_TRUE(r.overlap);
    ASSERT_NEAR(r.depth, 0.5f, 1e-4f);
    ASSERT_NEAR(r.normal.x, 1.0f, 1e-4f);
    ASSERT_NEAR(r.normal.y, 0.0f, 1e-4f);
}

TEST_CASE(CajasSeparadas) {
    const auto r = satPolygons(caja(1, 1), caja(1, 1, {4.0f, 0.0f}));
    ASSERT_FALSE(r.overlap);
}

TEST_CASE(CajasQueSeTocanNoSolapan) {
    const auto r = satPolygons(caja(1, 1), caja(1, 1, {2.0f, 0.0f}));
    ASSERT_FALSE(r.overlap);
}

TEST_CASE(Contencion) {
    const auto r = satPolygons(caja(2, 2), caja(0.5f, 0.5f));
    ASSERT_TRUE(r.overlap);
    ASSERT_NEAR(r.depth, 1.0f, 1e-4f);
}

TEST_CASE(RomboContraCaja) {
    const Poly rombo = {{{1.2f, 0.2f}, {2.2f, 1.2f}, {1.2f, 2.2f}, {0.2f, 1.2f}}};
    const auto r = satPolygons(caja(1, 1), rombo);
    ASSERT_TRUE(r.overlap);
    ASSERT_NEAR(r.depth, 0.424264f, 1e-4f);
    ASSERT_NEAR(std::fabs(r.normal.x), 0.707107f, 1e-4f);
    ASSERT_NEAR(std::fabs(r.normal.y), 0.707107f, 1e-4f);
}

TEST_CASE(NormalApuntaDeAHaciaB) {
    const auto r1 = satPolygons(caja(1, 1), caja(1, 1, {1.5f, 0.0f}));
    const auto r2 = satPolygons(caja(1, 1), caja(1, 1, {-1.5f, 0.0f}));
    ASSERT_TRUE(r1.overlap && r2.overlap);
    ASSERT_TRUE(r1.normal.x > 0.0f);
    ASSERT_TRUE(r2.normal.x < 0.0f);
}

int main() {
    std::cout << "Solución 04 (SAT)...\n";
    return RUN_ALL_TESTS();
}
