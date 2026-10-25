/* =============================================================================
   MÓDULO 25 - SOLUCIÓN 07: biquad low-pass (Direct Form I)
   =============================================================================
   Direct Form I literal: los cuatro estados (x1, x2, y1, y2) con la fórmula
   del RBJ cookbook. El orden de la rotación importa: calcular y[n] PRIMERO
   (con los estados viejos) y rotar DESPUÉS. Es exactamente la estructura
   interna de ma_biquad (con coeficientes pasados por
   ma_biquad_config_init(format, channels, b0..a2), firma verificada).
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <cmath>

class Biquad {
public:
    void setCoef(double b0v, double b1v, double b2v, double a1v, double a2v) {
        b0_ = b0v;
        b1_ = b1v;
        b2_ = b2v;
        a1_ = a1v;
        a2_ = a2v;
        x1_ = x2_ = y1_ = y2_ = 0.0;
    }

    double procesar(double x) {
        const double y = b0_ * x + b1_ * x1_ + b2_ * x2_
                       - a1_ * y1_ - a2_ * y2_;
        x2_ = x1_; x1_ = x;
        y2_ = y1_; y1_ = y;
        return y;
    }

private:
    double b0_ = 1.0, b1_ = 0.0, b2_ = 0.0, a1_ = 0.0, a2_ = 0.0;
    double x1_ = 0.0, x2_ = 0.0, y1_ = 0.0, y2_ = 0.0;
};

/* ------------------------------ TESTS (no tocar)-------------------------- */
TEST_CASE(RespuestaImpulso) {
    Biquad bq;
    bq.setCoef(0.003916126660547, 0.007832253321095, 0.003916126660547,
               -1.815341082704568, 0.831005589346758);
    ASSERT_NEAR(bq.procesar(1.0), 0.003916126661, 0.000000001);
    ASSERT_NEAR(bq.procesar(0.0), 0.014941358933, 0.000000001);
    ASSERT_NEAR(bq.procesar(0.0), 0.027785466220, 0.000000001);
    ASSERT_NEAR(bq.procesar(0.0), 0.038023745545, 0.000000001);
    ASSERT_NEAR(bq.procesar(0.0), 0.045936189675, 0.000000001);
    ASSERT_NEAR(bq.procesar(0.0), 0.051791907224, 0.000000001);
}

TEST_CASE(EstabilidadYDecaimiento) {
    Biquad bq;
    bq.setCoef(0.003916126660547, 0.007832253321095, 0.003916126660547,
               -1.815341082704568, 0.831005589346758);
    (void)bq.procesar(1.0); /* el impulso */
    double pico = 0.0;
    for (int i = 0; i < 2000; ++i) {
        const double v = bq.procesar(0.0);
        if (v > pico) { pico = v; }
    }
    /* los últimos 2000 taps del impulso del LPF decaen por debajo del 7% */
    ASSERT_TRUE(pico < 0.07 && pico > 0.0);
}

TEST_CASE(GananciaDC) {
    /* A DC (x constante) un LPF tiene ganancia ~1.0 (pasa la continua). */
    Biquad bq;
    bq.setCoef(0.003916126660547, 0.007832253321095, 0.003916126660547,
               -1.815341082704568, 0.831005589346758);
    double y = 0.0;
    for (int i = 0; i < 1000; ++i) {
        y = bq.procesar(1.0);
    }
    ASSERT_NEAR(y, 1.0, 0.01);
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
