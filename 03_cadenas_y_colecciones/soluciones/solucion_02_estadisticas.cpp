/* =============================================================================
   MÓDULO 03 - SOLUCIÓN 02: Estadísticas de Vector
   =============================================================================
   POR QUÉ FUNCIONA:
     - max_element y min_element devuelven ITERADORES: hay que desreferenciar con *.
     - accumulate arranca con 0.0 (double). Con 0 (int) la suma se truncaría a entero.
     - rango reutiliza maximo y minimo: no repetimos lógica.
     - Chequear empty() antes de desreferenciar evita comportamiento indefinido.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_02_estadisticas.cpp -o sol02 && ./sol02
   ============================================================================= */
#include <iostream>
#include <vector>
#include <span>
#include <algorithm>
#include <numeric>
#include "../../include/test_framework.hpp"

// Todas reciben std::span<const double>: aceptan vector, array o array de C
// sin copiar nada. Con un span vacío devolvemos 0.0 (decisión de diseño).
double maximo(std::span<const double> datos) {
    if (datos.empty()) return 0.0;
    return *std::max_element(datos.begin(), datos.end());
}

double minimo(std::span<const double> datos) {
    if (datos.empty()) return 0.0;
    return *std::min_element(datos.begin(), datos.end());
}

double promedio(std::span<const double> datos) {
    if (datos.empty()) return 0.0;
    double suma = std::accumulate(datos.begin(), datos.end(), 0.0);
    return suma / static_cast<double>(datos.size());
}

double rango(std::span<const double> datos) {
    return maximo(datos) - minimo(datos);
}

// ============= TESTS (los mismos del ejercicio) =============
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
    std::cout << "Ejecutando pruebas para Solución 02 (Estadísticas de Vector)...\n";
    return RUN_ALL_TESTS();
}
