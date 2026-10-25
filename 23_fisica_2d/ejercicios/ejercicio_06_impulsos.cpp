/* =============================================================================
   MÓDULO 23 - EJERCICIO 06: Resolución por impulsos
   =============================================================================
   DIFICULTAD: Alto
   APOYO: Lección 06 (impulso normal, restitución, fricción de Coulomb).

   CONSIGNA:
   Implementa `resolverImpulso(a, b, manifold, material)`:
     1. Calcular la velocidad relativa en el punto de contacto
        (incluyendo ω × r).
     2. Si se están separando (dot(vRel, n) > 0), no hacer nada.
     3. Impulso normal j = -(1+e)*vRelN / denom, con
        denom = invMassA+invMassB+invIA*(rA×n)²+invIB*(rB×n)².
        A recibe -j*n, B recibe +j*n.
     4. Recalcular vRel y aplicar fricción tangencial, clampeada a ±μ*j.
   `e` se fuerza a 0 si |vRelN| < umbralRestitucion.
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
    Vec2 normal;  // de A hacia B
    Vec2 punto;
    float penetracion = 0.0f;
};

struct Material {
    float restitucion = 0.0f;
    float friccion = 0.0f;
    float umbralRestitucion = 1.0f;
};

void resolverImpulso(Cuerpo& a, Cuerpo& b, const Manifold& m, const Material& mat) {
    // TODO
    (void)a;
    (void)b;
    (void)m;
    (void)mat;
}

// ============================================================================
// TESTS (no tocar)
// ============================================================================
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
    std::cout << "Ejercicio 06 (impulsos)...\n";
    return RUN_ALL_TESTS();
}
