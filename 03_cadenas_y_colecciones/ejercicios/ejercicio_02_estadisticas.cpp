/* =============================================================================
   MÓDULO 03 - EJERCICIO 02: Estadísticas de Vector
   =============================================================================
   DIFICULTAD: Básico
   CONSIGNA:
     Implementá funciones para calcular estadísticas sobre un vector<double>:
     1. maximo(span<const double>) → double: mayor elemento
     2. minimo(span<const double>) → double: menor elemento
     3. promedio(span<const double>) → double: media aritmética
     4. rango(span<const double>) → double: max - min
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_02_estadisticas.cpp -o ej02 && ./ej02
   ============================================================================= */
#include <iostream>
#include <vector>
#include <span>
#include <limits>
#include "../../include/test_framework.hpp"

// TODO: implementar estas funciones
// TU CÓDIGO AQUÍ:

double maximo(std::span<const double> datos) {
    return 0.0; // placeholder
}

double minimo(std::span<const double> datos) {
    return 0.0; // placeholder
}

double promedio(std::span<const double> datos) {
    return 0.0; // placeholder
}

double rango(std::span<const double> datos) {
    return 0.0; // placeholder
}

// ============= TESTS =============
TEST_CASE(TestMaximo) {
    std::vector<double> v = {3.0, 1.0, 4.0, 1.0, 5.0, 9.0, 2.0};
    ASSERT_NEAR(maximo(v), 9.0, 1e-9);
    std::vector<double> neg = {-5.0, -1.0, -3.0};
    ASSERT_NEAR(maximo(neg), -1.0, 1e-9);
}

TEST_CASE(TestMinimo) {
    std::vector<double> v = {3.0, 1.0, 4.0, 1.0, 5.0};
    ASSERT_NEAR(minimo(v), 1.0, 1e-9);
    std::vector<double> pos = {10.0, 5.0, 7.0};
    ASSERT_NEAR(minimo(pos), 5.0, 1e-9);
}

TEST_CASE(TestPromedio) {
    std::vector<double> v = {2.0, 4.0, 6.0};
    ASSERT_NEAR(promedio(v), 4.0, 1e-9);
    std::vector<double> v2 = {1.0, 2.0, 3.0, 4.0, 5.0};
    ASSERT_NEAR(promedio(v2), 3.0, 1e-9);
}

TEST_CASE(TestRango) {
    std::vector<double> v = {2.0, 8.0, 5.0};
    ASSERT_NEAR(rango(v), 6.0, 1e-9);
    std::vector<double> v2 = {-3.0, 7.0};
    ASSERT_NEAR(rango(v2), 10.0, 1e-9);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 02 (Estadísticas)...\n";
    return RUN_ALL_TESTS();
}
