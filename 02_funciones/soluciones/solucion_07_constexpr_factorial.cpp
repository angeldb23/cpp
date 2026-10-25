/* =============================================================================
   MÓDULO 02 - SOLUCIÓN 07: Función constexpr factorial
   =============================================================================
   Implementación de la función constexpr `factorial` que calcula el factorial
   de forma recursiva y permite evaluación en tiempo de compilación.
   ============================================================================= */

#include <iostream>
#include "../../include/test_framework.hpp"

// constexpr factorial implementation
constexpr int factorial(int n) {
    return (n <= 1) ? 1 : (n * factorial(n - 1));
}

// Tests for the constexpr factorial
TEST_CASE(TestFactorialSolution) {
    static_assert(factorial(0) == 1);
    static_assert(factorial(5) == 120);
    ASSERT_EQ(factorial(4), 24);
}

int main() {
    std::cout << "[SOLUCIÓN] Ejecutando tests de Solución 07 (constexpr factorial)...\n";
    return RUN_ALL_TESTS();
}
