/* =============================================================================
   MÓDULO 23 - EJERCICIO 09: Joints de distancia, motor y límites
   =============================================================================
   DIFICULTAD: Medio
   APOYO: Lección 09 (constraint de distancia, motor angular, límite prismático).

   CONSIGNA:
     - `resolverDistancia(e)`:
         error = |posB - posA| - longitudReposo
         n = normalize(posB - posA)
         j = error / (invMassA + invMassB)
         deltaA = +n * j * invMassA, deltaB = -n * j * invMassB
       Si los cuerpos coinciden o la suma de invMass es ~0, no mover nada
       (pero devolver el error calculado, sin NaN).
     - `motorAngular(ωActual, ωObjetivo, maxTorque, invInertia, dt)`:
         deseado = (ωObjetivo - ωActual) / invInertia
         devolver clamp(deseado, -maxTorque*dt, +maxTorque*dt)
       Con invInertia ~0 devolver 0.
     - `limitarPrismatico(q, minimo, maximo)` -> clamp.
   ============================================================================= */
#include "../include/fisica2d.hpp"
#include "../../include/test_framework.hpp"

using f2d::Vec2;

struct EntradaDistancia {
    Vec2 posA;
    Vec2 posB;
    Vec2 velA;
    Vec2 velB;
    float invMassA = 1.0f;
    float invMassB = 1.0f;
    float longitudReposo = 1.0f;
};

struct SalidaDistancia {
    Vec2 deltaA;
    Vec2 deltaB;
    float error = 0.0f;
};

SalidaDistancia resolverDistancia(const EntradaDistancia& e) {
    // TODO
    (void)e;
    return {};
}

float motorAngular(float omegaActual, float omegaObjetivo, float maxTorque,
                   float invInertia, float dt) {
    // TODO
    (void)omegaActual;
    (void)omegaObjetivo;
    (void)maxTorque;
    (void)invInertia;
    (void)dt;
    return 0.0f;
}

float limitarPrismatico(float q, float minimo, float maximo) {
    // TODO
    (void)q;
    (void)minimo;
    (void)maximo;
    return 0.0f;
}

// ============================================================================
// TESTS (no tocar)
// ============================================================================
TEST_CASE(CuerdaEstiradaSeAcerca) {
    const auto s = resolverDistancia({{0.0f, 0.0f}, {2.0f, 0.0f}, {}, {}, 1.0f, 1.0f, 1.0f});
    ASSERT_NEAR(s.error, 1.0f, 1e-6f);
    ASSERT_NEAR(s.deltaA.x, 0.5f, 1e-6f);
    ASSERT_NEAR(s.deltaB.x, -0.5f, 1e-6f);
}

TEST_CASE(CuerdaComprimidaSeSepara) {
    const auto s = resolverDistancia({{0.0f, 0.0f}, {0.5f, 0.0f}, {}, {}, 1.0f, 1.0f, 1.0f});
    ASSERT_NEAR(s.error, -0.5f, 1e-6f);
    ASSERT_NEAR(s.deltaA.x, -0.25f, 1e-6f);
    ASSERT_NEAR(s.deltaB.x, 0.25f, 1e-6f);
}

TEST_CASE(EstaticoNoSeMueve) {
    const auto s = resolverDistancia({{0.0f, 0.0f}, {2.0f, 0.0f}, {}, {}, 0.0f, 1.0f, 1.0f});
    ASSERT_NEAR(s.deltaA.x, 0.0f, 1e-6f);
    ASSERT_NEAR(s.deltaB.x, -1.0f, 1e-6f);
}

TEST_CASE(MismaPosicionNoGeneraNaNs) {
    const auto s = resolverDistancia({{1.0f, 1.0f}, {1.0f, 1.0f}, {}, {}, 1.0f, 1.0f, 0.5f});
    ASSERT_NEAR(s.error, -0.5f, 1e-6f);
    ASSERT_NEAR(s.deltaA.x, 0.0f, 1e-6f);
    ASSERT_NEAR(s.deltaA.y, 0.0f, 1e-6f);
    ASSERT_NEAR(s.deltaB.x, 0.0f, 1e-6f);
}

TEST_CASE(MotorRespetaParMaximo) {
    ASSERT_NEAR(motorAngular(0.0f, 10.0f, 100.0f, 0.5f, 0.1f), 10.0f, 1e-5f);
    ASSERT_NEAR(motorAngular(0.0f, 10.0f, 1000.0f, 0.5f, 0.1f), 20.0f, 1e-5f);
    ASSERT_NEAR(motorAngular(0.0f, -10.0f, 100.0f, 0.5f, 0.1f), -10.0f, 1e-5f);
}

TEST_CASE(MotorSinInerciaNoHaceNada) {
    ASSERT_NEAR(motorAngular(0.0f, 10.0f, 100.0f, 0.0f, 0.1f), 0.0f, 1e-6f);
}

TEST_CASE(LimitePrismatico) {
    ASSERT_NEAR(limitarPrismatico(5.0f, -2.0f, 3.0f), 3.0f, 1e-6f);
    ASSERT_NEAR(limitarPrismatico(-5.0f, -2.0f, 3.0f), -2.0f, 1e-6f);
    ASSERT_NEAR(limitarPrismatico(1.0f, -2.0f, 3.0f), 1.0f, 1e-6f);
}

int main() {
    std::cout << "Ejercicio 09 (Joints)...\n";
    return RUN_ALL_TESTS();
}
