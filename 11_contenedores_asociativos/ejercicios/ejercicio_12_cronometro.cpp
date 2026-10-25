/* =============================================================================
   MÓDULO 11 - EJERCICIO 12: el cronómetro del frame (lección 16)
   =============================================================================
   DIFICULTAD: Basico
   CONSIGNA (tema 9 de TEMAS.md: std::chrono):
     Un pequeño cronómetro para medir el tiempo por frame, con las
     piezas mínimas de la L16 (steady_clock, duration_cast, count):

       1. class Cronometro:
            void arrancar(): guarda t0 = steady_clock::now().
            double stopMicros(): el delta (now() - t0) EN MICROSEGUNDOS
                 (duration_cast<microseconds>(delta).count() como
                 double). El steady_clock es el reloj correcto (L16).
       2. double medirTiempo(Cronometro& c, func): midé cuánto tarda
          ejecutar func: arrancar + func() + stopMicros.
       3. El test usa sleep_for como "trabajo" y exige que el delta
          medido caiga DENTRO del intervalo esperado (con margen) y
          que sea razonable en la unidad pedida (micros, L16).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_12_cronometro.cpp -o ej12 && ./ej12
   ============================================================================= */
#include <chrono>
#include <functional>
#include <iostream>
#include <thread>
#include "../../include/test_framework.hpp"

class Cronometro {
  public:
    // TODO 1: guardar el punto de partida (L16: steady_clock::now()):
    void arrancar() { }

    // TODO 2: el delta en MICROSEGUNDOS (como double) desde arrancar.
    double stopMicros() {
        return 0.0;    // placeholder
    }

  private:
    // TU punto t0 acá (tipo: std::chrono::steady_clock::time_point):
    // TU CODIGO AQUI
};

// TODO: la máquina de medir: arrancar, ejecutar func, devolver micros:
double medirTiempo([[maybe_unused]] Cronometro& c,
                   [[maybe_unused]] const std::function<void()>& func) {
    return 0.0;    // placeholder
}

// ============= TESTS =============
TEST_CASE(MideUnSleepDe150Millis) {
    using namespace std::chrono;
    Cronometro c;
    const double micros = medirTiempo(c, [] {
        std::this_thread::sleep_for(150ms);
    });
    std::cout << "  (mide: " << micros << " us)\n";
    ASSERT_TRUE(micros > 120000.0);        // >= ~150ms con margen
    ASSERT_TRUE(micros < 400000.0);        // no demasiado arriba
}

TEST_CASE(StopSinArrancarNoEsNegativo) {
    Cronometro c;
    const double micros = c.stopMicros();
    ASSERT_TRUE(micros >= 0.0);
}

TEST_CASE(SeVuelveAArrancar) {
    using namespace std::chrono;
    Cronometro c;
    c.arrancar();
    std::this_thread::sleep_for(50ms);
    const double primera = c.stopMicros();
    ASSERT_TRUE(primera >= 40000.0);       // >= ~50ms con margen
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 12 (cronometro del frame)...\n";
    return RUN_ALL_TESTS();
}
