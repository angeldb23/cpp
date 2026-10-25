/* =============================================================================
   MÓDULO 13 - EJERCICIO 03: noexcept básico y operador noexcept
   =============================================================================
   DIFICULTAD: Básico-Intermedio
   CONSIGNA:
     1. Implementá tres funciones:
        - suma(int a, int b) -> int: marcada noexcept (nunca lanza)
        - division(int a, int b) -> int: NO marcada noexcept (puede lanzar)
        - identidad(int x) -> int: marcada noexcept
     2. Implementá una función template:
        template<typename F, typename... Args>
        bool esNoexcept(F&& f, Args&&... args)
        que devuelve true si la llamada f(args...) es noexcept (usá noexcept operator).
     3. Implementá una función que tome un puntero a función y use noexcept
        para decidir en tiempo de compilación qué rama tomar (if constexpr).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_03_noexcept_basico.cpp -o ej03 && ./ej03
   ============================================================================= */
#include <iostream>
#include <utility>
#include "../../include/test_framework.hpp"

// TODO: int suma(int a, int b) noexcept;
// TODO: int division(int a, int b);  // sin noexcept
// TODO: int identidad(int x) noexcept;

// TODO: template<typename F, typename... Args>
// bool esNoexcept(F&& f, Args&&... args);

// TODO: template<typename F, typename... Args>
// auto llamarSeguro(F&& f, Args&&... args) 
//     -> std::conditional_t<noexcept(f(args...)), int, std::string>;

// ============= TESTS =============
TEST_CASE(TestSumaEsNoexcept) {
    // ASSERT_TRUE(noexcept(suma(1, 2)));
    ASSERT_TRUE(false);
}

TEST_CASE(TestDivisionNoEsNoexcept) {
    // ASSERT_FALSE(noexcept(division(1, 2)));
    ASSERT_TRUE(false);
}

TEST_CASE(TestIdentidadEsNoexcept) {
    // ASSERT_TRUE(noexcept(identidad(42)));
    ASSERT_TRUE(false);
}

TEST_CASE(TestEsNoexceptConSuma) {
    // ASSERT_TRUE(esNoexcept(suma, 10, 20));
    ASSERT_TRUE(false);
}

TEST_CASE(TestEsNoexceptConDivision) {
    // ASSERT_FALSE(esNoexcept(division, 10, 2));
    ASSERT_TRUE(false);
}

TEST_CASE(TestLlamarSeguroConNoexcept) {
    // auto r = llamarSeguro(suma, 3, 4);
    // static_assert(std::is_same_v<decltype(r), int>);
    // ASSERT_EQ(r, 7);
    ASSERT_TRUE(false);
}

TEST_CASE(TestLlamarSeguroSinNoexcept) {
    // auto r = llamarSeguro(division, 10, 2);
    // static_assert(std::is_same_v<decltype(r), std::string>);
    // ASSERT_EQ(r, "puede lanzar");
    ASSERT_TRUE(false);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 03 (noexcept básico)...\n";
    return RUN_ALL_TESTS();
}