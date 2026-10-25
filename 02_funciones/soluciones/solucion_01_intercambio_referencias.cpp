/* =============================================================================
   MÓDULO 02 - SOLUCIÓN 01: Intercambio Universal por Referencia
   =============================================================================
   Implementación de la función 'swapValores' y pruebas.
   ============================================================================= */

#include <iostream>
#include "../../include/test_framework.hpp"

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
    std::cout << "[SOLUCIÓN] Ejecutando tests de Solución 01 Intercambio..." << std::endl;
    return RUN_ALL_TESTS();
}
