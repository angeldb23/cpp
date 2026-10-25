/* =============================================================================
   MÓDULO 25 - EJERCICIO 02: mezclador con saturación y medidor de picos
   =============================================================================
   DIFICULTAD: Fácil-Media
   APOYO: Lección 02 (mezcla en el callback, suma ponderada, clipping).

   CONSIGNA
   --------
   Completá `Mezclador` (el "mixer sumado" del callback):
   1. `sumar(entrada, n, ganancia)`: suma entrya[i] * ganancia al buffer.
      ANTES de escribir cada muestra, calcula el valor PRÉCLAMP y guarda el
      máximo absoluto en `pico_` (el medidor "real"). DESPUÉS aplica el
      clamp a [-1,+1] (saturación) y marca `clipeado_ = true` si algún
      valor requería clamp.
      Devuelve true si la entrada llenó el buffer completo (n == tamaño),
      false si pidieron un n más chico (no toca el resto) o mayor.
   2. `pico()`: máximo absoluto PRE-clamp acumulado desde init/limpiar.
   3. `clipeado()`: true si hubo alguna saturación.
   4. `limpiar()`: buffer a 0, pico a 0, clipeado a false.
   Regla de la lección: "pico" es el medidor que reporta el audio REAL
   (pre-clamp), y el buffer es la señal con saturación (post-clamp).
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <cmath>
#include <cstddef>
#include <vector>

class Mezclador {
public:
    explicit Mezclador(std::size_t n) : buf_(n, 0.0f) {}

    bool sumar(const float* entrada, std::size_t n, float ganancia) {
        // TODO
        (void)entrada;
        (void)n;
        (void)ganancia;
        return false;
    }

    float pico() const {
        // TODO
        return 0.0f;
    }

    bool clipeado() const {
        // TODO
        return false;
    }

    const std::vector<float>& buffer() const { return buf_; }

    void limpiar() {
        // TODO
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
    /* el buffer queda saturado en +1, pero el PICO reporta 1.3 (real). */
    ASSERT_NEAR(m.buffer()[0], 1.0f, 0.000001f);
    ASSERT_NEAR(m.pico(), 1.3f, 0.000001f);
    ASSERT_TRUE(m.clipeado());
}

TEST_CASE(GananciaYPicoNegativo) {
    Mezclador m(1);
    const float a[1] = { -1.4f };
    ASSERT_TRUE(m.sumar(a, 1, 0.5f)); /* entra -0.7 (no satura) */
    ASSERT_NEAR(m.buffer()[0], -0.7f, 0.000001f);
    ASSERT_NEAR(m.pico(), 0.7f, 0.000001f); /* absoluto */
    const float b[1] = { -1.4f };
    m.limpiar();
    ASSERT_TRUE(m.sumar(b, 1, 1.0f)); /* -1.4 -> satura en -1.0 */
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
