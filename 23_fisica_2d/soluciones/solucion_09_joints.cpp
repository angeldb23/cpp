/* =============================================================================
   MÓDULO 23 - SOLUCIÓN 09: Joints de distancia, motor y límites
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
    const Vec2 d = e.posB - e.posA;
    const float largo = f2d::length(d);
    const float error = largo - e.longitudReposo;

    if (largo <= f2d::kEps) return {{}, {}, error};

    const float denom = e.invMassA + e.invMassB;
    if (denom <= f2d::kEps) return {{}, {}, error};

    const Vec2 n = d / largo;
    const float j = error / denom;
    return {n * (j * e.invMassA), -n * (j * e.invMassB), error};
}

float motorAngular(float omegaActual, float omegaObjetivo, float maxTorque,
                   float invInertia, float dt) {
    if (invInertia <= f2d::kEps) return 0.0f;
    const float deseado = (omegaObjetivo - omegaActual) / invInertia;
    const float limite = maxTorque * dt;
    return f2d::clampf(deseado, -limite, limite);
}

float limitarPrismatico(float q, float minimo, float maximo) {
    return f2d::clampf(q, minimo, maximo);
}

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
    std::cout << "Solución 09 (Joints)...\n";
    return RUN_ALL_TESTS();
}
