/* =============================================================================
   MÓDULO 02 - EJERCICIO 01: Intercambio Universal por Referencia
   =============================================================================
   DIFICULTAD: Básico

   CONSIGNA:
   Implementa la función 'void swapValores(int& a, int& b)' que intercambia los
   valores de dos enteros utilizando referencias.
   ============================================================================= */

#include <iostream>
#include "../../include/test_framework.hpp"

// TODO: Implementa 'swapValores'
void swapValores(int& a, int& b) {
    int temp = a;
    a = b;
    b = temp;
}

TEST_CASE(TestSwapValores) {
    int x = 10, y = 20;
    swapValores(x, y);
    ASSERT_EQ(x, 20);
    ASSERT_EQ(y, 10);

    int a = -5, b = 42;
    swapValores(a, b);
    ASSERT_EQ(a, 42);
    ASSERT_EQ(b, -5);
}

int main() {
    std::cout << "Ejecutando tests de Ejercicio 01 (Swap Referencias)...\n";
    return RUN_ALL_TESTS();
}
