/* =============================================================================
   MÓDULO 09 - SOLUCIÓN 01: Funciones genéricas: maximo, minimo y acotar
   =============================================================================
   POR QUÉ FUNCIONA:
     - Un template de función se instancia una vez por cada tipo distinto con que se usa.
     - Usar solo operator< hace que la función sirva para más tipos (std::string incluido).
     - minimo y maximo reciben (b < a) / (a < b): la comparación siempre es con <, nunca con >.
     - acotar reutiliza las otras dos: el template llama a otros templates sin problema.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_01_maximo.cpp -o sol01 && ./sol01
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

// Solo usamos operator< (igual que la biblioteca estándar): así sirve para
// int, double, std::string y cualquier tipo que sepa compararse con <.
template <typename T>
T maximo(T a, T b) {
    return (a < b) ? b : a;
}

template <typename T>
T minimo(T a, T b) {
    return (b < a) ? b : a;
}

// "el mayor entre valor y lo, sin pasarse de hi"
template <typename T>
T acotar(T valor, T lo, T hi) {
    return minimo(maximo(valor, lo), hi);
}

// ============= TESTS (los mismos del ejercicio) =============
TEST_CASE(TestMaximoEnteros) {
    ASSERT_EQ(maximo(3, 7), 7);
    ASSERT_EQ(maximo(7, 3), 7);
    ASSERT_EQ(maximo(-5, -9), -5);
}

TEST_CASE(TestMaximoDoublesYStrings) {
    ASSERT_NEAR(maximo(2.5, 1.5), 2.5, 1e-9);
    ASSERT_NEAR(maximo(1.5, 2.5), 2.5, 1e-9);
    ASSERT_EQ(maximo(std::string("manzana"), std::string("pera")), std::string("pera"));
    ASSERT_EQ(maximo(std::string("pera"), std::string("manzana")), std::string("pera"));
}

TEST_CASE(TestMaximoTipoExplicito) {
    // Forzamos T = double aunque los argumentos sean int y double mezclados
    ASSERT_NEAR(maximo<double>(3, 2.5), 3.0, 1e-9);
    ASSERT_NEAR(maximo<double>(2, 2.5), 2.5, 1e-9);
}

TEST_CASE(TestMinimo) {
    ASSERT_EQ(minimo(3, 7), 3);
    ASSERT_EQ(minimo(7, 3), 3);
    ASSERT_NEAR(minimo(2.5, 1.5), 1.5, 1e-9);
    ASSERT_EQ(minimo(std::string("zeta"), std::string("alfa")), std::string("alfa"));
}

TEST_CASE(TestAcotar) {
    ASSERT_EQ(acotar(15, 0, 10), 10);
    ASSERT_EQ(acotar(-5, 0, 10), 0);
    ASSERT_EQ(acotar(5, 0, 10), 5);
    ASSERT_NEAR(acotar(2.5, 0.0, 1.0), 1.0, 1e-9);
    ASSERT_EQ(acotar(std::string("z"), std::string("a"), std::string("f")), std::string("f"));
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 01 (Funciones genéricas: maximo, minimo y acotar)...\n";
    return RUN_ALL_TESTS();
}
