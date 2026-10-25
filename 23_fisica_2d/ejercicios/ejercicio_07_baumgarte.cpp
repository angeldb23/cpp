/* =============================================================================
   MÓDULO 23 - EJERCICIO 07: Baumgarte y warm starting
   =============================================================================
   DIFICULTAD: Medio
   APOYO: Lección 07 (corrección de posición, slop, percent y acumulación).

   CONSIGNA:
     - `baumgarte`: devolver los deltas de posición de A y B.
       magnitud = max(penetración - slop, 0) * percent / (invMassA + invMassB)
       A se mueve -normal * magnitud * invMassA
       B se mueve +normal * magnitud * invMassB
     - `warmStart`: acumulado = clamp(anterior + nuevo, 0, max).
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
    // TODO
    (void)normal;
    (void)penetracion;
    (void)invMassA;
    (void)invMassB;
    (void)percent;
    (void)slop;
    return {};
}

float warmStart(float acumulado, float nuevo, float maximo) {
    // TODO
    (void)acumulado;
    (void)nuevo;
    (void)maximo;
    return 0.0f;
}

// ============================================================================
// TESTS (no tocar)
// ============================================================================
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
    std::cout << "Ejercicio 07 (Baumgarte)...\n";
    return RUN_ALL_TESTS();
}
