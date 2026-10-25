/* =============================================================================
   MÓDULO 04 - EJERCICIO 08: Lambdas, capturas y algoritmos
   =============================================================================
   DIFICULTAD: Intermedio
   PRACTICA: lección 12 (lambdas, captura por valor/referencia, mutable,
             lambdas como comparador y predicado).

   CONSIGNA (usá lambdas en todas; no escribas bucles manuales donde haya un
   algoritmo de <algorithm> que sirva):
     1. ordenarDescendente(std::vector<int>& v): ordená de mayor a menor con
        std::sort y un comparador lambda.
     2. ordenarPorDistanciaA(std::vector<int>& v, int centro): ordená por
        distancia |x - centro| ascendente. Si hay empate, menor valor primero.
        La lambda debe capturar `centro` por valor.
     3. contarMayoresQue(const std::vector<int>& v, int umbral): usá
        std::count_if con una lambda que capture umbral.
     4. multiplicarPor(const std::vector<int>& v, int factor): devolvé un
        vector nuevo con cada elemento multiplicado, usando std::transform.
     5. filtrarPares(const std::vector<int>& v): devolvé un vector nuevo solo
        con los pares, en el mismo orden, usando std::copy_if (o
        std::remove_copy_if) con una lambda.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra ejercicio_08_lambdas.cpp -o ej08 && ./ej08
   ============================================================================= */
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <vector>
#include "../../include/test_framework.hpp"

// TODO: implementá las cinco funciones.
void ordenarDescendente(std::vector<int>& v) {
    (void)v;  // placeholder
}

void ordenarPorDistanciaA(std::vector<int>& v, int centro) {
    (void)v; (void)centro;  // placeholder
}

std::size_t contarMayoresQue(const std::vector<int>& v, int umbral) {
    (void)v; (void)umbral;
    return 0;  // placeholder
}

std::vector<int> multiplicarPor(const std::vector<int>& v, int factor) {
    (void)v; (void)factor;
    return {};  // placeholder
}

std::vector<int> filtrarPares(const std::vector<int>& v) {
    (void)v;
    return {};  // placeholder
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
    ASSERT_EQ(v[0], 3);   // distancia 2
    ASSERT_EQ(v[1], 8);   // distancia 3
    ASSERT_EQ(v[2], 1);   // distancia 4
    ASSERT_EQ(v[3], 10);  // distancia 5
}

TEST_CASE(TestOrdenarPorDistanciaEmpate) {
    std::vector<int> v = {7, 3, 9, 1};
    ordenarPorDistanciaA(v, 5);
    ASSERT_EQ(v[0], 3);  // |3-5| = 2
    ASSERT_EQ(v[1], 7);  // |7-5| = 2, desempate por menor valor
    ASSERT_EQ(v[2], 1);  // |1-5| = 4
    ASSERT_EQ(v[3], 9);  // |9-5| = 4
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
    ASSERT_EQ(v[0], 1);  // el original no cambia
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
    std::cout << "Ejecutando pruebas para Ejercicio 08...\n";
    return RUN_ALL_TESTS();
}
