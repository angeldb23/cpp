/* =============================================================================
   MÓDULO 02 - SOLUCIÓN 03: Sobrecarga de Funciones de Cálculo Geométrico
   ============================================================================= */

#include <iostream>
#include "../../include/test_framework.hpp"

double calcularArea(double lado) {
    return lado * lado;
}

double calcularArea(double ancho, double alto) {
    return ancho * alto;
}

double calcularArea(double radio, bool esCirculo) {
    if (esCirculo) {
        constexpr double PI = 3.141592653589793;
        return PI * radio * radio;
    }
    return 0.0;
}

TEST_CASE(TestSobrecargaCuadrado) {
    ASSERT_NEAR(calcularArea(4.0), 16.0, 0.001);
}

TEST_CASE(TestSobrecargaRectangulo) {
    ASSERT_NEAR(calcularArea(3.0, 5.0), 15.0, 0.001);
}

TEST_CASE(TestSobrecargaCirculo) {
    ASSERT_NEAR(calcularArea(2.0, true), 12.56637, 0.001);
}

int main() {
    std::cout << "[SOLUCIÓN] Ejecutando tests de Ejercicio 03...\n";
    return RUN_ALL_TESTS();
}
