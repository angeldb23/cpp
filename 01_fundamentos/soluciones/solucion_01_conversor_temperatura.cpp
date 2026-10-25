/* =============================================================================
   MÓDULO 01 - SOLUCIÓN 01: Conversor de Temperatura Exacto
   =============================================================================
 */

#include "../../include/test_framework.hpp"
#include <iostream>

// Implementación exacta con tipos de punto flotante de doble precisión
double celsiusAFahrenheit(double celsius) {
  return (celsius * 9.0 / 5.0) + 32.0;
}

double celsiusAKelvin(double celsius) { return celsius + 273.15; }

// =============================================================================
// TESTS AUTOMÁTICOS
// =============================================================================
TEST_CASE(TestConversionCeroGrados) {
  ASSERT_NEAR(celsiusAFahrenheit(0.0), 32.0, 0.001);
  ASSERT_NEAR(celsiusAKelvin(0.0), 273.15, 0.001);
}

TEST_CASE(TestConversionEbullicion) {
  ASSERT_NEAR(celsiusAFahrenheit(100.0), 212.0, 0.001);
  ASSERT_NEAR(celsiusAKelvin(100.0), 373.15, 0.001);
}

TEST_CASE(TestConversionBajoCero) {
  ASSERT_NEAR(celsiusAFahrenheit(-40.0), -40.0, 0.001);
  ASSERT_NEAR(celsiusAKelvin(-273.15), 0.0, 0.001);
}

int main() {
  std::cout << "[SOLUCIÓN] Ejecutando tests de Ejercicio 01...\n";
  return RUN_ALL_TESTS();
}
