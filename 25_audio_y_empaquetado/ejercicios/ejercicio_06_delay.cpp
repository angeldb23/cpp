/* =============================================================================
   MÓDULO 25 - EJERCICIO 06: delay con feedback, wet/dry
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 06 (delay line circular, feedback, wet/dry).

   CONSIGNA
   --------
   Completá `DelayLine`:
   1. `config(delaySamples, feedback)`: dimensiona el anillo. Devuelve false
      (sin tocar estado) si delaySamples == 0 o feedback >= 1.0f (el eco
      infinito rompe el efecto) o feedback < 0.
   2. `procesar(x)`: devuelve (dry * entrada + mix * eco) donde el eco es
      lo leído del anillo Y ANTES de escribir el nuevo valor:
         eco      = anillo[cursor]            (lo que entra hace D frames)
         anillo[cursor] = entrada + eco * feedback
         cursor   = (cursor + 1) % D          (después de leer/escribir)
      y la mezcla final es: (1.0f - mix) * entrada + mix * eco.
   El ímpulso clásico con feedback 0.5: ecos en D, 2D, 3D con amplitud
   1, 0.5, 0.25 (cada vuelta a mitad = -6 dB/paso).
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <cstddef>
#include <vector>

class DelayLine {
public:
    bool config(std::size_t delaySamples, float feedback, float mix) {
        // TODO
        (void)delaySamples; (void)feedback; (void)mix;
        return false;
    }

    float procesar(float entrada) {
        // TODO
        (void)entrada;
        return 0.0f;
    }

    std::size_t delay() const {
        // TODO
        return 0;
    }

private:
    std::vector<float> anillo_;
    std::size_t cursor_ = 0;
    std::size_t delay_ = 0;
    float feedback_ = 0.0f;
    float mix_ = 1.0f;
};

/* ------------------------------ TESTS (no tocar)-------------------------- */
TEST_CASE(ConfigInvalida) {
    DelayLine d;
    ASSERT_FALSE(d.config(0, 0.5f, 1.0f));   /* delay 0: sin efecto */
    ASSERT_FALSE(d.config(480, 1.0f, 1.0f)); /* feedback >= 1: rompe */
    ASSERT_FALSE(d.config(480, 1.2f, 1.0f));
    ASSERT_FALSE(d.config(480, -0.5f, 1.0f));
    ASSERT_TRUE(d.config(480, 0.999f, 1.0f));
    ASSERT_FALSE(d.config(0, 0.9f, 1.0f));   /* re-config con delay 0 */
}

TEST_CASE(ImpulsoEnElDelay) {
    DelayLine d;
    ASSERT_TRUE(d.config(4, 0.5f, 1.0f)); /* delay 4, feedback 0.5, full wet */
    /* impulso de 1 en t=0, silencio después: */
    ASSERT_NEAR(d.procesar(1.0f), 0.0f, 0.000001f);
    ASSERT_NEAR(d.procesar(0.0f), 0.0f, 0.000001f);
    ASSERT_NEAR(d.procesar(0.0f), 0.0f, 0.000001f);
    ASSERT_NEAR(d.procesar(0.0f), 0.0f, 0.000001f);
    /* t=4: PRIMER eco (amplitud 1.0) */
    ASSERT_NEAR(d.procesar(0.0f), 1.0f, 0.000001f);
    for (int i = 0; i < 3; ++i) {
        d.procesar(0.0f);
    }
    /* t=8: segundo eco a mitad */
    ASSERT_NEAR(d.procesar(0.0f), 0.5f, 0.000001f);
    for (int i = 0; i < 3; ++i) {
        d.procesar(0.0f);
    }
    /* t=12: tercero a un cuarto */
    ASSERT_NEAR(d.procesar(0.0f), 0.25f, 0.000001f);
}

TEST_CASE(MezclaWetDry) {
    DelayLine d;
    ASSERT_TRUE(d.config(2, 0.0f, 0.5f)); /* 50% wet, 50% dry, sin feedback */
    ASSERT_NEAR(d.procesar(1.0f), 0.5f, 0.000001f); /* dry = 0.5 x 1 */
    ASSERT_NEAR(d.procesar(0.0f), 0.0f, 0.000001f);
    ASSERT_NEAR(d.procesar(0.0f), 0.5f, 0.000001f); /* wet del eco #1 */
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
