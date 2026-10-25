/* =============================================================================
   MÓDULO 25 - SOLUCIÓN 02: mezclador con saturación y medidor de picos
   =============================================================================
   Lo importante:
   - el PICO se mide PRE-clamp (es el medidor honesto de la señal real);
   - el BUFFER queda en [-1, +1] (el DAC no acepta más);
   - un float* a un std::vector se recorre plano: el callback del device no
     sabe de vektors de C++ (ésa es la razón de la firma cruda).
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <cstddef>
#include <vector>

class Mezclador {
public:
    explicit Mezclador(std::size_t n) : buf_(n, 0.0f) {}

    bool sumar(const float* entrada, std::size_t n, float ganancia) {
        if (n != buf_.size()) {
            return false;
        }
        for (std::size_t i = 0; i < n; ++i) {
            const float v = buf_[i] + entrada[i] * ganancia; /* PRE-clamp */
            if (v > pico_) {
                pico_ = v;
            } else if (-v > pico_) {
                pico_ = -v;
            }
            float salida = v;
            if (salida > 1.0f) { salida = 1.0f; clipeado_ = true; }
            else if (salida < -1.0f) { salida = -1.0f; clipeado_ = true; }
            buf_[i] = salida;
        }
        return true;
    }

    float pico() const { return pico_; }
    bool clipeado() const { return clipeado_; }
    const std::vector<float>& buffer() const { return buf_; }

    void limpiar() {
        for (float& f : buf_) { f = 0.0f; }
        pico_ = 0.0f;
        clipeado_ = false;
    }

private:
    std::vector<float> buf_;
    float pico_ = 0.0f;
    bool clipeado_ = false;
};

/* ------------------------------ TESTS (no tocar)-------------------------- */
TEST_CASE(SumaSinClamp) {
    Mezclador m(2);
    const float a[2] = { 0.4f, 0.4f };
    const float b[2] = { 0.3f, 0.3f };
    ASSERT_TRUE(m.sumar(a, 2, 1.0f));
    ASSERT_TRUE(m.sumar(b, 2, 1.0f));
    ASSERT_NEAR(m.buffer()[0], 0.7f, 0.000001f);
    ASSERT_NEAR(m.buffer()[1], 0.7f, 0.000001f);
    ASSERT_NEAR(m.pico(), 0.7f, 0.000001f);
    ASSERT_FALSE(m.clipeado());
}

TEST_CASE(Saturacion) {
    Mezclador m(2);
    const float a[2] = { 0.8f, 0.8f };
    const float b[2] = { 0.5f, 0.5f };
    ASSERT_TRUE(m.sumar(a, 2, 1.0f));
    ASSERT_TRUE(m.sumar(b, 2, 1.0f));
    ASSERT_NEAR(m.buffer()[0], 1.0f, 0.000001f);
    ASSERT_NEAR(m.pico(), 1.3f, 0.000001f);
    ASSERT_TRUE(m.clipeado());
}

TEST_CASE(GananciaYPicoNegativo) {
    Mezclador m(1);
    const float a[1] = { -1.4f };
    ASSERT_TRUE(m.sumar(a, 1, 0.5f));
    ASSERT_NEAR(m.buffer()[0], -0.7f, 0.000001f);
    ASSERT_NEAR(m.pico(), 0.7f, 0.000001f);
    const float b[1] = { -1.4f };
    m.limpiar();
    ASSERT_TRUE(m.sumar(b, 1, 1.0f));
    ASSERT_NEAR(m.buffer()[0], -1.0f, 0.000001f);
    ASSERT_NEAR(m.pico(), 1.4f, 0.000001f);
    ASSERT_TRUE(m.clipeado());
}

TEST_CASE(Limpiar) {
    Mezclador m(3);
    const float a[3] = { 0.9f, -0.9f, 0.5f };
    ASSERT_TRUE(m.sumar(a, 3, 1.0f));
    m.limpiar();
    ASSERT_NEAR(m.pico(), 0.0f, 0.000001f);
    ASSERT_FALSE(m.clipeado());
    ASSERT_NEAR(m.buffer()[1], 0.0f, 0.000001f);
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
