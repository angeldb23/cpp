/* =============================================================================
   MÓDULO 13 - SOLUCIÓN 03: noexcept básico y operador noexcept
   =============================================================================
   POR QUÉ FUNCIONA:
     - noexcept en la firma es un contrato: "esta función no lanza".
       El compilador optimiza (no genera tablas de unwinding) y std::vector
       usa move en lugar de copy en reallocation si el move es noexcept.
     - noexcept(expr) es un operador de TIEMPO DE COMPILACIÓN que devuelve
       true si la expresión no puede lanzar (según lo que el compilador sabe).
     - if constexpr + noexcept permite ramificación en compilación:
       el código muerto se descarta y no hay costo de runtime.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_03_noexcept_basico.cpp -o sol03 && ./sol03
   ============================================================================= */
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>
#include "../../include/test_framework.hpp"

int suma(int a, int b) noexcept {
    return a + b;
}

int division(int a, int b) {  // sin noexcept: puede lanzar (división por cero UB, pero conceptualmente)
    if (b == 0) throw std::domain_error("division por cero");
    return a / b;
}

int identidad(int x) noexcept {
    return x;
}

template<typename F, typename... Args>
bool esNoexcept(F&& f, Args&&... args) {
    return noexcept(std::forward<F>(f)(std::forward<Args>(args)...));
}

template<typename F, typename... Args>
auto llamarSeguro(F&& f, Args&&... args) 
    -> std::conditional_t<noexcept(std::forward<F>(f)(std::forward<Args>(args)...)), int, std::string> {
    if constexpr (noexcept(std::forward<F>(f)(std::forward<Args>(args)...))) {
        return std::forward<F>(f)(std::forward<Args>(args)...);
    } else {
        return std::string("puede lanzar");
    }
}

// ============= TESTS =============
TEST_CASE(TestSumaEsNoexcept) {
    ASSERT_TRUE(noexcept(suma(1, 2)));
}

TEST_CASE(TestDivisionNoEsNoexcept) {
    ASSERT_FALSE(noexcept(division(1, 2)));
}

TEST_CASE(TestIdentidadEsNoexcept) {
    ASSERT_TRUE(noexcept(identidad(42)));
}

TEST_CASE(TestEsNoexceptConSuma) {
    ASSERT_TRUE(esNoexcept(suma, 10, 20));
}

TEST_CASE(TestEsNoexceptConDivision) {
    ASSERT_FALSE(esNoexcept(division, 10, 2));
}

TEST_CASE(TestLlamarSeguroConNoexcept) {
    auto r = llamarSeguro(suma, 3, 4);
    static_assert(std::is_same_v<decltype(r), int>);
    ASSERT_EQ(r, 7);
}

TEST_CASE(TestLlamarSeguroSinNoexcept) {
    auto r = llamarSeguro(division, 10, 2);
    static_assert(std::is_same_v<decltype(r), std::string>);
    ASSERT_EQ(r, std::string("puede lanzar"));
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 03 (noexcept básico)...\n";
    return RUN_ALL_TESTS();
}