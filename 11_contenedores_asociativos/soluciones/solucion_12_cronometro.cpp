/* =============================================================================
   MÓDULO 11 - SOLUCIÓN 12: el cronómetro del frame (ejercicio 12)
   =============================================================================
   L16 es la lección de respaldo: steady_clock::now(), restar dos
   time_points da una duration, duration_cast lleva a la unidad pedida
   (microsegundos) y count() es el valor bruto en ESA unidad.
   ============================================================================= */
#include <chrono>
#include <functional>
#include <iostream>
#include <thread>
#include "../../include/test_framework.hpp"

class Cronometro {
  public:
    void arrancar() { t0 = std::chrono::steady_clock::now(); }

    double stopMicros() {
        const auto dt = std::chrono::steady_clock::now() - t0;   // duration
        return static_cast<double>(
            std::chrono::duration_cast<std::chrono::microseconds>(dt).count());
    }

  private:
    std::chrono::steady_clock::time_point t0{};   // epoch al arrancar
};

double medirTiempo(Cronometro& c, const std::function<void()>& func) {
    c.arrancar();
    func();
    return c.stopMicros();
}

// ============= TESTS =============
TEST_CASE(MideUnSleepDe150Millis) {
    using namespace std::chrono;
    Cronometro c;
    const double micros = medirTiempo(c, [] {
        std::this_thread::sleep_for(150ms);
    });
    std::cout << "  (mide: " << micros << " us)\n";
    ASSERT_TRUE(micros > 120000.0);
    ASSERT_TRUE(micros < 400000.0);
}

TEST_CASE(StopSinArrancarNoEsNegativo) {
    Cronometro c;
    ASSERT_TRUE(c.stopMicros() >= 0.0);
}

TEST_CASE(SeVuelveAArrancar) {
    using namespace std::chrono;
    Cronometro c;
    c.arrancar();
    std::this_thread::sleep_for(50ms);
    ASSERT_TRUE(c.stopMicros() >= 40000.0);
}

int main() {
    std::cout << "Ejecutando pruebas para Solucion 12 (cronometro del frame)...\n";
    return RUN_ALL_TESTS();
}
