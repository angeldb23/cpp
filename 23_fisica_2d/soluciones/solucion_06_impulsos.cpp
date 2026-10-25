/* =============================================================================
   MÓDULO 23 - SOLUCIÓN 06: Resolución por impulsos
   ============================================================================= */
#include "../include/fisica2d.hpp"
#include "../../include/test_framework.hpp"

using f2d::Vec2;

struct Cuerpo {
    Vec2 pos;
    Vec2 vel;
    float angulo = 0.0f;
    float angVel = 0.0f;
    float invMass = 1.0f;
    float invInertia = 0.0f;
};

struct Manifold {
    Vec2 normal;
    Vec2 punto;
    float penetracion = 0.0f;
};

struct Material {
    float restitucion = 0.0f;
    float friccion = 0.0f;
    float umbralRestitucion = 1.0f;
};

namespace {

Vec2 velocidadEnPunto(const Cuerpo& c, const Vec2& r) {
    return c.vel + f2d::perp(r) * c.angVel;
}

void aplicar(Cuerpo& c, const Vec2& impulso, const Vec2& r) {
    c.vel += impulso * c.invMass;
    c.angVel += c.invInertia * f2d::cross(r, impulso);
}

float denom(const Cuerpo& a, const Cuerpo& b, const Vec2& ra, const Vec2& rb,
            const Vec2& eje) {
    const float ca = f2d::cross(ra, eje);
    const float cb = f2d::cross(rb, eje);
    return a.invMass + b.invMass + a.invInertia * ca * ca + b.invInertia * cb * cb;
}

} // namespace

void resolverImpulso(Cuerpo& a, Cuerpo& b, const Manifold& m, const Material& mat) {
    const Vec2 n = f2d::normalize(m.normal);
    if (f2d::lengthSq(n) < f2d::kEps) return;

    const Vec2 ra = m.punto - a.pos;
    const Vec2 rb = m.punto - b.pos;
    Vec2 vRel = velocidadEnPunto(b, rb) - velocidadEnPunto(a, ra);
    const float vRelN = f2d::dot(vRel, n);
    if (vRelN > 0.0f) return;

    const float e = (std::fabs(vRelN) < mat.umbralRestitucion) ? 0.0f : mat.restitucion;
    const float den = denom(a, b, ra, rb, n);
    if (den <= f2d::kEps) return;

    const float j = -(1.0f + e) * vRelN / den;
    const Vec2 impulsoN = n * j;
    aplicar(a, -impulsoN, ra);
    aplicar(b, impulsoN, rb);

    vRel = velocidadEnPunto(b, rb) - velocidadEnPunto(a, ra);
    Vec2 t = vRel - n * f2d::dot(vRel, n);
    if (f2d::lengthSq(t) <= f2d::kEps) return;
    t = f2d::normalize(t);

    const float denT = denom(a, b, ra, rb, t);
    if (denT <= f2d::kEps) return;
    float jt = -f2d::dot(vRel, t) / denT;
    const float limite = mat.friccion * j;
    jt = f2d::clampf(jt, -limite, limite);
    const Vec2 impulsoT = t * jt;
    aplicar(a, -impulsoT, ra);
    aplicar(b, impulsoT, rb);
}

TEST_CASE(FrontalInelastico) {
    Cuerpo a{{0, 0}, {1, 0}};
    Cuerpo b{{1, 0}, {0, 0}};
    resolverImpulso(a, b, {{1, 0}, {0.5f, 0}, 0.1f}, {0.0f, 0.0f, 1.0f});
    ASSERT_NEAR(a.vel.x, 0.5f, 1e-4f);
    ASSERT_NEAR(b.vel.x, 0.5f, 1e-4f);
}

TEST_CASE(FrontalElastico) {
    Cuerpo a{{0, 0}, {1, 0}};
    Cuerpo b{{1, 0}, {0, 0}};
    resolverImpulso(a, b, {{1, 0}, {0.5f, 0}, 0.1f}, {1.0f, 0.0f, 0.0f});
    ASSERT_NEAR(a.vel.x, 0.0f, 1e-4f);
    ASSERT_NEAR(b.vel.x, 1.0f, 1e-4f);
}

TEST_CASE(FriccionClampeada) {
    Cuerpo a{{0, 1}, {1, -1}};
    Cuerpo b{{0, 0}, {0, 0}};
    resolverImpulso(a, b, {{0, -1}, {0, 0}, 0.1f}, {0.0f, 0.5f, 1.0f});
    ASSERT_NEAR(a.vel.x, 0.75f, 1e-4f);
    ASSERT_NEAR(a.vel.y, -0.5f, 1e-4f);
    ASSERT_NEAR(b.vel.x, 0.25f, 1e-4f);
    ASSERT_NEAR(b.vel.y, -0.5f, 1e-4f);
}

TEST_CASE(UmbralDeRestitucion) {
    Cuerpo a{{0, 0}, {0.2f, 0}};
    Cuerpo b{{1, 0}, {0, 0}};
    resolverImpulso(a, b, {{1, 0}, {0.5f, 0}, 0.1f}, {0.8f, 0.0f, 1.0f});
    ASSERT_NEAR(a.vel.x, 0.1f, 1e-4f);
    ASSERT_NEAR(b.vel.x, 0.1f, 1e-4f);
}

TEST_CASE(ContactoDescentradoGeneraGiro) {
    Cuerpo a{{0, 0}, {1, 0}, 0, 0, 1, 1};
    Cuerpo b{{1, 0}, {0, 0}, 0, 0, 1, 1};
    resolverImpulso(a, b, {{1, 0}, {0.5f, 0.5f}, 0.1f}, {0.0f, 0.0f, 1.0f});
    ASSERT_NEAR(a.vel.x, 0.6f, 1e-4f);
    ASSERT_NEAR(b.vel.x, 0.4f, 1e-4f);
    ASSERT_NEAR(a.angVel, 0.2f, 1e-4f);
    ASSERT_NEAR(b.angVel, -0.2f, 1e-4f);
}

TEST_CASE(CuerposQueSeSeparanNoCambian) {
    Cuerpo a{{0, 0}, {-1, 0}};
    Cuerpo b{{1, 0}, {0, 0}};
    resolverImpulso(a, b, {{1, 0}, {0.5f, 0}, 0.1f}, {0.5f, 0.5f, 1.0f});
    ASSERT_NEAR(a.vel.x, -1.0f, 1e-4f);
    ASSERT_NEAR(b.vel.x, 0.0f, 1e-4f);
}

int main() {
    std::cout << "Solución 06 (impulsos)...\n";
    return RUN_ALL_TESTS();
}
