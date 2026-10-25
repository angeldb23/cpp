/* =============================================================================
   MÓDULO 25 - SOLUCIÓN 06: delay con feedback, wet/dry
   =============================================================================
   El anillo: leer primero, escribir DESPUÉS, incrementar al final. Con esa
   corrección el eco sale en exactamente delaySamples frames. El mix aplica
   Crossfade simple entre la señal seca y el eco. NOTA: el guard de
   `feedback >= 1.0f` es lo que evita el "eco infinito" (inestabilidad):
   miniaudio valida lo mismo a nivel ma_delay_node con el campo delay/decay.
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <cstddef>
#include <vector>

class DelayLine {
public:
    bool config(std::size_t delaySamples, float feedback, float mix) {
        if (delaySamples == 0) {
            return false;
        }
        if (feedback >= 1.0f || feedback < 0.0f) {
            return false;
        }
        anillo_.assign(delaySamples, 0.0f);
        delay_ = delaySamples;
        cursor_ = 0;
        feedback_ = feedback;
        mix_ = mix;
        return true;
    }

    float procesar(float entrada) {
        const float eco = anillo_[cursor_];              /* D frames atrás */
        anillo_[cursor_] = entrada + eco * feedback_;    /* recursión */
        cursor_ = (cursor_ + 1) % delay_;
        return (1.0f - mix_) * entrada + mix_ * eco;
    }

    std::size_t delay() const { return delay_; }

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
    ASSERT_FALSE(d.config(0, 0.5f, 1.0f));
    ASSERT_FALSE(d.config(480, 1.0f, 1.0f));
    ASSERT_FALSE(d.config(480, 1.2f, 1.0f));
    ASSERT_FALSE(d.config(480, -0.5f, 1.0f));
    ASSERT_TRUE(d.config(480, 0.999f, 1.0f));
    ASSERT_FALSE(d.config(0, 0.9f, 1.0f));
}

TEST_CASE(ImpulsoEnElDelay) {
    DelayLine d;
    ASSERT_TRUE(d.config(4, 0.5f, 1.0f));
    ASSERT_NEAR(d.procesar(1.0f), 0.0f, 0.000001f);
    ASSERT_NEAR(d.procesar(0.0f), 0.0f, 0.000001f);
    ASSERT_NEAR(d.procesar(0.0f), 0.0f, 0.000001f);
    ASSERT_NEAR(d.procesar(0.0f), 0.0f, 0.000001f);
    ASSERT_NEAR(d.procesar(0.0f), 1.0f, 0.000001f);
    for (int i = 0; i < 3; ++i) {
        d.procesar(0.0f);
    }
    ASSERT_NEAR(d.procesar(0.0f), 0.5f, 0.000001f);
    for (int i = 0; i < 3; ++i) {
        d.procesar(0.0f);
    }
    ASSERT_NEAR(d.procesar(0.0f), 0.25f, 0.000001f);
}

TEST_CASE(MezclaWetDry) {
    DelayLine d;
    ASSERT_TRUE(d.config(2, 0.0f, 0.5f));
    ASSERT_NEAR(d.procesar(1.0f), 0.5f, 0.000001f);
    ASSERT_NEAR(d.procesar(0.0f), 0.0f, 0.000001f);
    ASSERT_NEAR(d.procesar(0.0f), 0.5f, 0.000001f);
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
