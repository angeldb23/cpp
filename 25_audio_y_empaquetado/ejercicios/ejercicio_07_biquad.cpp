/* =============================================================================
   MÓDULO 25 - EJERCICIO 07: biquad low-pass (Direct Form I)
   =============================================================================
   DIFICULTAD: Media-Alta
   APOYO: Lección 07 (biquad Direct Form I, RBJ Audio EQ Cookbook).

   CONSIGNA
   --------
   Completá `Biquad` (el filtro de la lección con coeficientes YA calculados,
   para verificar el motor de procesamiento en sí):

   Los coeficientes son los de un LPF fs=48000, f0=1000 Hz, Q=0.7071
   (Butterworth) del RBJ Audio EQ Cookbook, normalizados por a0:
     b0 = 0.003916126660547
     b1 = 0.007832253321095
     b2 = 0.003916126660547
     a1 = -1.815341082704568
     a2 = 0.831005589346758
   (El a0 ya está dividido: NO hace falta dividir al procesar.)

   1. `setCoef(b0, b1, b2, a1, a2)`: les asigna y limpia los estados (x/y
      anterior y tras-anterior a 0).
   2. `procesar(x)`: implementa la fórmula Direct Form I del ejercicio:
        y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
      y DESPUÉS rota los estados (x2<-x1, x1<-x; y2<-y1, y1<-y).
   3. La respuesta al impulso (test) arranca con esos taps exactos y
      decae (los últimos taps llegan a ~0: un filtro estable).
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <cmath>

class Biquad {
public:
    void setCoef(double b0v, double b1v, double b2v, double a1v, double a2v) {
        // TODO
        (void)b0v; (void)b1v; (void)b2v; (void)a1v; (void)a2v;
    }

    double procesar(double x) {
        // TODO
        (void)x;
        return 0.0;
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
    /* impulso: x = 1 en t=0, 0 después. Primeros taps congelados: */
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
