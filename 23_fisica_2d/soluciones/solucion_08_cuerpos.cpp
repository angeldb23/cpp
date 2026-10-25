/* =============================================================================
   MÓDULO 23 - SOLUCIÓN 08: Cuerpos, masa, inercia y damping
   ============================================================================= */
#include "../include/fisica2d.hpp"
#include "../../include/test_framework.hpp"

using f2d::Vec2;

enum class TipoCuerpo { Static, Kinematic, Dynamic };

struct DefinicionCuerpo {
    TipoCuerpo tipo = TipoCuerpo::Dynamic;
    float masa = 1.0f;
    float inercia = 1.0f;
    Vec2 velocidad;
    float velocidadAngular = 0.0f;
    float dampingLineal = 0.0f;
    float dampingAngular = 0.0f;
};

struct Cuerpo {
    TipoCuerpo tipo = TipoCuerpo::Dynamic;
    Vec2 pos;
    Vec2 vel;
    float angulo = 0.0f;
    float angVel = 0.0f;
    float invMass = 0.0f;
    float invInertia = 0.0f;
    float dampingLineal = 0.0f;
    float dampingAngular = 0.0f;
};

float inerciaCaja(float masa, float ancho, float alto) {
    return masa * (ancho * ancho + alto * alto) / 12.0f;
}

float inerciaCirculo(float masa, float radio) {
    return 0.5f * masa * radio * radio;
}

Cuerpo crearCuerpo(const DefinicionCuerpo& def) {
    Cuerpo c;
    c.tipo = def.tipo;
    c.vel = def.velocidad;
    c.angVel = def.velocidadAngular;
    c.dampingLineal = def.dampingLineal;
    c.dampingAngular = def.dampingAngular;

    if (def.tipo == TipoCuerpo::Dynamic) {
        if (def.masa > 0.0f) c.invMass = 1.0f / def.masa;
        if (def.inercia > 0.0f) c.invInertia = 1.0f / def.inercia;
    }
    return c;
}

void integrar(Cuerpo& c, Vec2 fuerza, float torque, float dt) {
    if (c.tipo == TipoCuerpo::Static) return;

    if (c.tipo == TipoCuerpo::Dynamic) {
        c.vel += fuerza * (c.invMass * dt);
        c.angVel += torque * c.invInertia * dt;
    }

    c.vel *= 1.0f / (1.0f + c.dampingLineal * dt);
    c.angVel *= 1.0f / (1.0f + c.dampingAngular * dt);

    c.pos += c.vel * dt;
    c.angulo += c.angVel * dt;
}

TEST_CASE(InerciaDeFormas) {
    ASSERT_NEAR(inerciaCaja(2.0f, 4.0f, 2.0f), 20.0f / 6.0f, 1e-5f);
    ASSERT_NEAR(inerciaCirculo(2.0f, 3.0f), 9.0f, 1e-5f);
}

TEST_CASE(InversasSegunTipo) {
    const Cuerpo dinamico = crearCuerpo({TipoCuerpo::Dynamic, 2.0f, 4.0f, {}, 0.0f, 0.0f, 0.0f});
    ASSERT_NEAR(dinamico.invMass, 0.5f, 1e-6f);
    ASSERT_NEAR(dinamico.invInertia, 0.25f, 1e-6f);

    const Cuerpo estatico = crearCuerpo({TipoCuerpo::Static, 2.0f, 4.0f, {}, 0.0f, 0.0f, 0.0f});
    ASSERT_NEAR(estatico.invMass, 0.0f, 1e-6f);
    ASSERT_NEAR(estatico.invInertia, 0.0f, 1e-6f);

    const Cuerpo cinematico = crearCuerpo({TipoCuerpo::Kinematic, 2.0f, 4.0f, {}, 0.0f, 0.0f, 0.0f});
    ASSERT_NEAR(cinematico.invMass, 0.0f, 1e-6f);
    ASSERT_NEAR(cinematico.invInertia, 0.0f, 1e-6f);
}

TEST_CASE(MasaCeroNoDividePorCero) {
    const Cuerpo c = crearCuerpo({TipoCuerpo::Dynamic, 0.0f, 0.0f, {}, 0.0f, 0.0f, 0.0f});
    ASSERT_NEAR(c.invMass, 0.0f, 1e-6f);
    ASSERT_NEAR(c.invInertia, 0.0f, 1e-6f);
}

TEST_CASE(EulerSemiImplicitoUsaVelocidadNueva) {
    Cuerpo c = crearCuerpo({TipoCuerpo::Dynamic, 2.0f, 4.0f, {}, 0.0f, 0.0f, 0.0f});
    integrar(c, {2.0f, 0.0f}, 0.0f, 0.5f);
    ASSERT_NEAR(c.vel.x, 0.5f, 1e-6f);
    ASSERT_NEAR(c.pos.x, 0.25f, 1e-6f);
}

TEST_CASE(CinematicoIgnoraFuerzaPeroSeMueve) {
    Cuerpo c = crearCuerpo({TipoCuerpo::Kinematic, 1.0f, 1.0f, {2.0f, 0.0f}, 0.0f, 0.0f, 0.0f});
    integrar(c, {1000.0f, 0.0f}, 500.0f, 0.5f);
    ASSERT_NEAR(c.vel.x, 2.0f, 1e-6f);
    ASSERT_NEAR(c.pos.x, 1.0f, 1e-6f);
    ASSERT_NEAR(c.angVel, 0.0f, 1e-6f);
}

TEST_CASE(EstaticoNoSeMueve) {
    Cuerpo c = crearCuerpo({TipoCuerpo::Static, 1.0f, 1.0f, {5.0f, 0.0f}, 3.0f, 0.0f, 0.0f});
    integrar(c, {1000.0f, 0.0f}, 500.0f, 1.0f);
    ASSERT_NEAR(c.pos.x, 0.0f, 1e-6f);
    ASSERT_NEAR(c.angulo, 0.0f, 1e-6f);
    ASSERT_NEAR(c.vel.x, 5.0f, 1e-6f);
}

TEST_CASE(DampingReduceVelocidadAntesDeIntegrar) {
    Cuerpo c = crearCuerpo({TipoCuerpo::Dynamic, 1.0f, 1.0f, {10.0f, 0.0f}, 0.0f,
                            1.0f, 0.0f});
    integrar(c, {0.0f, 0.0f}, 0.0f, 1.0f);
    ASSERT_NEAR(c.vel.x, 5.0f, 1e-6f);
    ASSERT_NEAR(c.pos.x, 5.0f, 1e-6f);
}

TEST_CASE(TorqueCambiaVelocidadAngular) {
    Cuerpo c = crearCuerpo({TipoCuerpo::Dynamic, 1.0f, 4.0f, {}, 0.0f, 0.0f, 0.0f});
    integrar(c, {0.0f, 0.0f}, 10.0f, 0.1f);
    ASSERT_NEAR(c.angVel, 0.25f, 1e-6f);
    ASSERT_NEAR(c.angulo, 0.025f, 1e-6f);
}

int main() {
    std::cout << "Solución 08 (Cuerpos)...\n";
    return RUN_ALL_TESTS();
}
