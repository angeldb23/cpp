/* =============================================================================
   MÓDULO 04 - SOLUCIÓN 08: Lambdas, capturas y algoritmos
   =============================================================================
   POR QUÉ FUNCIONA:
     - Las lambdas sin captura se usan como comparadores/predicados simples.
     - [centro] captura una copia: el comparador no depende de que centro siga
       vivo ni de cambios posteriores.
     - [umbral] y [factor] capturan por valor para no leer variables externas
       desde dentro del algoritmo.
     - std::transform escribe en un vector nuevo del mismo tamaño; copy_if usa
       back_inserter porque el tamaño final no se conoce de antemano.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_08_lambdas.cpp -o sol08 && ./sol08
   ============================================================================= */
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <vector>
#include "../../include/test_framework.hpp"

void ordenarDescendente(std::vector<int>& v) {
    std::sort(v.begin(), v.end(), [](int a, int b) { return a > b; });
}

void ordenarPorDistanciaA(std::vector<int>& v, int centro) {
    std::sort(v.begin(), v.end(), [centro](int a, int b) {
        int da = (a > centro) ? a - centro : centro - a;
        int db = (b > centro) ? b - centro : centro - b;
        if (da != db) return da < db;
        return a < b;
    });
}

std::size_t contarMayoresQue(const std::vector<int>& v, int umbral) {
    auto total = std::count_if(v.begin(), v.end(),
                               [umbral](int x) { return x > umbral; });
    return static_cast<std::size_t>(total);
}

std::vector<int> multiplicarPor(const std::vector<int>& v, int factor) {
    std::vector<int> resultado(v.size());
    std::transform(v.begin(), v.end(), resultado.begin(),
                   [factor](int x) { return x * factor; });
    return resultado;
}

std::vector<int> filtrarPares(const std::vector<int>& v) {
    std::vector<int> resultado;
    resultado.reserve(v.size());
    std::copy_if(v.begin(), v.end(), std::back_inserter(resultado),
                 [](int x) { return x % 2 == 0; });
    return resultado;
}

// ============= TESTS =============
TEST_CASE(TestOrdenarDescendente) {
    std::vector<int> v = {3, 1, 4, 1, 5};
    ordenarDescendente(v);
    ASSERT_EQ(v.size(), 5u);
    ASSERT_EQ(v[0], 5);
    ASSERT_EQ(v[1], 4);
    ASSERT_EQ(v[2], 3);
    ASSERT_EQ(v[3], 1);
    ASSERT_EQ(v[4], 1);
}

TEST_CASE(TestOrdenarPorDistancia) {
    std::vector<int> v = {1, 8, 3, 10};
    ordenarPorDistanciaA(v, 5);
    ASSERT_EQ(v[0], 3);
    ASSERT_EQ(v[1], 8);
    ASSERT_EQ(v[2], 1);
    ASSERT_EQ(v[3], 10);
}

TEST_CASE(TestOrdenarPorDistanciaEmpate) {
    std::vector<int> v = {7, 3, 9, 1};
    ordenarPorDistanciaA(v, 5);
    ASSERT_EQ(v[0], 3);
    ASSERT_EQ(v[1], 7);
    ASSERT_EQ(v[2], 1);
    ASSERT_EQ(v[3], 9);
}

TEST_CASE(TestContarMayoresQue) {
    std::vector<int> v = {1, 5, 8, 2};
    ASSERT_EQ(contarMayoresQue(v, 4), 2u);
    ASSERT_EQ(contarMayoresQue(v, 100), 0u);
    ASSERT_EQ(contarMayoresQue({}, 0), 0u);
}

TEST_CASE(TestMultiplicarPor) {
    std::vector<int> v = {1, 2, 3};
    std::vector<int> r = multiplicarPor(v, -2);
    ASSERT_EQ(r.size(), 3u);
    ASSERT_EQ(r[0], -2);
    ASSERT_EQ(r[1], -4);
    ASSERT_EQ(r[2], -6);
    ASSERT_EQ(v[0], 1);
}

TEST_CASE(TestFiltrarPares) {
    std::vector<int> v = {1, 2, 4, 7, 10};
    std::vector<int> r = filtrarPares(v);
    ASSERT_EQ(r.size(), 3u);
    ASSERT_EQ(r[0], 2);
    ASSERT_EQ(r[1], 4);
    ASSERT_EQ(r[2], 10);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 08...\n";
    return RUN_ALL_TESTS();
}
