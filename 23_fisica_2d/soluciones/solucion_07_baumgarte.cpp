/* =============================================================================
   MÓDULO 23 - SOLUCIÓN 07: Baumgarte y warm starting
   ============================================================================= */
#include "../include/fisica2d.hpp"
#include "../../include/test_framework.hpp"

using f2d::Vec2;

struct Correccion {
    Vec2 a;
    Vec2 b;
};

Correccion baumgarte(Vec2 normal, float penetracion, float invMassA,
                     float invMassB, float percent = 0.2f, float slop = 0.01f) {
    const float suma = invMassA + invMassB;
    if (suma <= f2d::kEps || penetracion <= slop) return {};
    const float magnitud = (penetracion - slop) * percent / suma;
    const Vec2 n = f2d::normalize(normal);
    return {-n * (magnitud * invMassA), n * (magnitud * invMassB)};
}

float warmStart(float acumulado, float nuevo, float maximo) {
    return f2d::clampf(acumulado + nuevo, 0.0f, maximo);
}

TEST_CASE(CorreccionBasica) {
    const auto c = baumgarte({1, 0}, 0.1f, 1.0f, 1.0f);
    ASSERT_NEAR(c.a.x, -0.009f, 1e-5f);
    ASSERT_NEAR(c.a.y, 0.0f, 1e-5f);
    ASSERT_NEAR(c.b.x, 0.009f, 1e-5f);
}

TEST_CASE(PenetracionDentroDelSlop) {
    const auto c = baumgarte({1, 0}, 0.005f, 1.0f, 1.0f);
    ASSERT_NEAR(c.a.x, 0.0f, 1e-6f);
    ASSERT_NEAR(c.b.x, 0.0f, 1e-6f);
}

TEST_CASE(CuerpoEstaticoAbsorbeTodo) {
    const auto c = baumgarte({1, 0}, 0.1f, 0.0f, 1.0f);
    ASSERT_NEAR(c.a.x, 0.0f, 1e-6f);
    ASSERT_NEAR(c.b.x, 0.018f, 1e-5f);
}

TEST_CASE(WarmStartSumaYClampea) {
    ASSERT_NEAR(warmStart(2.0f, 5.0f, 6.0f), 6.0f, 1e-6f);
    ASSERT_NEAR(warmStart(2.0f, -5.0f, 6.0f), 0.0f, 1e-6f);
    ASSERT_NEAR(warmStart(1.0f, 2.0f, 10.0f), 3.0f, 1e-6f);
}

TEST_CASE(CorreccionUsaLaNormal) {
    const auto c = baumgarte({0, 1}, 0.1f, 1.0f, 1.0f);
    ASSERT_NEAR(c.a.y, -0.009f, 1e-5f);
    ASSERT_NEAR(c.b.y, 0.009f, 1e-5f);
}

int main() {
    std::cout << "Solución 07 (Baumgarte)...\n";
    return RUN_ALL_TESTS();
}
