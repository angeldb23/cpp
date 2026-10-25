/* =============================================================================
   MÓDULO 02 - EJERCICIO 07: Función constexpr
   =============================================================================
   DIFICULTAD: Intermedio
   
   CONSIGNA:
   Implementa la función constexpr `int factorial(int n)` que calcule el factorial
   de n de forma recursiva. La función debe poder evaluarse en tiempo de
   compilación para valores constantes.
   ============================================================================= */

#include <iostream>
#include "../../include/test_framework.hpp"

// constexpr factorial implementation
constexpr int factorial(int n) {
    return (n <= 1) ? 1 : (n * factorial(n - 1));
}

TEST_CASE(TestFactorial) {
    static_assert(factorial(0) == 1);
    static_assert(factorial(5) == 120);
    ASSERT_EQ(factorial(4), 24);
}

int main() {
    std::cout << "Ejecutando tests de Ejercicio 07..." << std::endl;
    return RUN_ALL_TESTS();
}
