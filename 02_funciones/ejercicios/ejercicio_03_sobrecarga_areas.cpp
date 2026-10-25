/* =============================================================================
   MÓDULO 02 - EJERCICIO 03: Sobrecarga de Funciones de Cálculo Geométrico
   =============================================================================
   DIFICULTAD: Básico

   CONSIGNA:
   Implementa tres sobrecargas de la función 'calcularArea':
   1. 'double calcularArea(double lado)' -> Área de un cuadrado (lado * lado).
   2. 'double calcularArea(double ancho, double alto)' -> Área de un rectángulo.
   3. 'double calcularArea(double radio, bool esCirculo)' -> Si esCirculo es true,
      calcula PI * radio * radio (usa PI = 3.141592653589793).
   ============================================================================= */

#include <iostream>
#include "../../include/test_framework.hpp"

// TODO 1: calcularArea(cuadrado)
double calcularArea(double lado) {
    // TU CÓDIGO AQUÍ:
    (void)lado;
    return 0.0;
}

// TODO 2: calcularArea(rectángulo)
double calcularArea(double ancho, double alto) {
    // TU CÓDIGO AQUÍ:
    (void)ancho;
    (void)alto;
    return 0.0;
}

// TODO 3: calcularArea(círculo)
double calcularArea(double radio, bool esCirculo) {
    // TU CÓDIGO AQUÍ:
    (void)radio;
    (void)esCirculo;
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
    std::cout << "Ejecutando tests de Ejercicio 03...\n";
    return RUN_ALL_TESTS();
}
