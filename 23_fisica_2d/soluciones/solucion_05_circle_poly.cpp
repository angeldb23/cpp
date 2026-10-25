/* =============================================================================
   MÓDULO 23 - SOLUCIÓN 05: Círculo vs polígono convexo
   ============================================================================= */
#include "../include/fisica2d.hpp"
#include "../../include/test_framework.hpp"

#include <limits>
#include <vector>

using f2d::Vec2;

struct Poly {
    std::vector<Vec2> pts;
};

struct Circle {
    Vec2 center;
    float radius = 0.0f;
};

struct CirclePolyResult {
    bool hit = false;
    Vec2 normal;
    float depth = 0.0f;
    Vec2 point;
};

namespace {

Vec2 puntoEnSegmento(const Vec2& p, const Vec2& a, const Vec2& b) {
    const Vec2 ab = b - a;
    const float den = f2d::lengthSq(ab);
    if (den <= f2d::kEps) return a;
    const float t = f2d::clampf(f2d::dot(p - a, ab) / den, 0.0f, 1.0f);
    return a + ab * t;
}

bool dentroDeConvexo(const Poly& p, const Vec2& q) {
    bool pos = false;
    bool neg = false;
    for (size_t i = 0; i < p.pts.size(); ++i) {
        const Vec2& a = p.pts[i];
        const Vec2& b = p.pts[(i + 1) % p.pts.size()];
        const float c = f2d::cross(b - a, q - a);
        if (c > f2d::kEps) pos = true;
        if (c < -f2d::kEps) neg = true;
        if (pos && neg) return false;
    }
    return true;
}

} // namespace

CirclePolyResult circlePoly(const Circle& c, const Poly& p) {
    if (p.pts.size() < 3) return {};

    Vec2 mejor = p.pts[0];
    float mejorD2 = std::numeric_limits<float>::max();
    for (size_t i = 0; i < p.pts.size(); ++i) {
        const Vec2 q = puntoEnSegmento(c.center, p.pts[i],
                                       p.pts[(i + 1) % p.pts.size()]);
        const float d2 = f2d::lengthSq(c.center - q);
        if (d2 < mejorD2) {
            mejorD2 = d2;
            mejor = q;
        }
    }

    const float dist = std::sqrt(mejorD2);
    if (dentroDeConvexo(p, c.center)) {
        const Vec2 n = f2d::normalize(mejor - c.center);
        return {true, n, c.radius + dist, mejor};
    }
    if (dist >= c.radius) return {};
    const Vec2 n = f2d::normalize(c.center - mejor);
    return {true, n, c.radius - dist, mejor};
}

namespace {
Poly caja(float hx, float hy) {
    return {{{-hx, -hy}, {hx, -hy}, {hx, hy}, {-hx, hy}}};
}
} // namespace

TEST_CASE(CirculoFueraQueToca) {
    const auto r = circlePoly({{1.2f, 0.0f}, 0.5f}, caja(1, 1));
    ASSERT_TRUE(r.hit);
    ASSERT_NEAR(r.depth, 0.3f, 1e-4f);
    ASSERT_NEAR(r.normal.x, 1.0f, 1e-4f);
    ASSERT_NEAR(r.normal.y, 0.0f, 1e-4f);
    ASSERT_NEAR(r.point.x, 1.0f, 1e-4f);
}

TEST_CASE(CirculoAdentro) {
    const auto r = circlePoly({{0.0f, 0.0f}, 0.5f}, caja(1, 1));
    ASSERT_TRUE(r.hit);
    ASSERT_NEAR(r.depth, 1.5f, 1e-4f);
}

TEST_CASE(CirculoLejos) {
    const auto r = circlePoly({{3.0f, 0.0f}, 0.5f}, caja(1, 1));
    ASSERT_FALSE(r.hit);
}

TEST_CASE(JustoTocandoNoEsContacto) {
    const auto r = circlePoly({{1.5f, 0.0f}, 0.5f}, caja(1, 1));
    ASSERT_FALSE(r.hit);
}

TEST_CASE(Esquina) {
    const auto r = circlePoly({{1.3f, 1.3f}, 0.5f}, caja(1, 1));
    ASSERT_TRUE(r.hit);
    ASSERT_NEAR(r.point.x, 1.0f, 1e-4f);
    ASSERT_NEAR(r.point.y, 1.0f, 1e-4f);
    ASSERT_NEAR(r.depth, 0.5f - std::sqrt(0.18f), 1e-4f);
    ASSERT_NEAR(r.normal.x, 0.707107f, 1e-4f);
    ASSERT_NEAR(r.normal.y, 0.707107f, 1e-4f);
}

TEST_CASE(EsquinaLejana) {
    const auto r = circlePoly({{1.4f, 1.4f}, 0.5f}, caja(1, 1));
    ASSERT_FALSE(r.hit);
}

int main() {
    std::cout << "Solución 05 (círculo vs polígono)...\n";
    return RUN_ALL_TESTS();
}
