/* =============================================================================
   MÓDULO 09 - EJERCICIO 01: Funciones genéricas: maximo, minimo y acotar
   =============================================================================
   DIFICULTAD: Básico
   CONSIGNA:
     Escribí tres templates de función que funcionen con int, double y std::string:
     1. maximo(a, b): devuelve el mayor
     2. minimo(a, b): devuelve el menor
     3. acotar(valor, lo, hi): limita 'valor' al rango [lo, hi]
     Reglas: pedile a T lo MENOS posible (alcanza con operator<).
     [[maybe_unused]] del esqueleto solo evita warnings; podés sacarlo.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_01_maximo.cpp -o ej01 && ./ej01
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

// TODO 1: devolvé el mayor de a y b usando únicamente operator<
// TU CÓDIGO AQUÍ:
template <typename T>
T maximo(T a, [[maybe_unused]] T b) {
    return a;   // placeholder: reemplazalo
}

// TODO 2: devolvé el menor de a y b
// TU CÓDIGO AQUÍ:
template <typename T>
T minimo(T a, [[maybe_unused]] T b) {
    return a;   // placeholder: reemplazalo
}

// TODO 3: si valor < lo devolvé lo; si valor > hi devolvé hi; si no, valor.
//         Pista: podés reutilizar maximo y minimo.
// TU CÓDIGO AQUÍ:
template <typename T>
T acotar(T valor, [[maybe_unused]] T lo, [[maybe_unused]] T hi) {
    return valor;   // placeholder: reemplazalo
}

// ============= TESTS =============
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
    std::cout << "Ejecutando pruebas para Ejercicio 01 (Funciones genéricas: maximo, minimo y acotar)...\n";
    return RUN_ALL_TESTS();
}
