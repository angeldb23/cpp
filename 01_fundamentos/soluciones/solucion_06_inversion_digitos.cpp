/* =============================================================================
   MÓDULO 01 - SOLUCIÓN 06: Inversión de Dígitos Enteros
   ============================================================================= */

#include <iostream>
#include "../../include/test_framework.hpp"

int invertirNumero(int n) {
    bool esNegativo = (n < 0);
    long long valor = esNegativo ? -static_cast<long long>(n) : n;
    long long invertido = 0;

    while (valor > 0) {
        invertido = (invertido * 10) + (valor % 10);
        valor /= 10;
    }

    return esNegativo ? -static_cast<int>(invertido) : static_cast<int>(invertido);
}

TEST_CASE(TestPositivos) {
    ASSERT_EQ(invertirNumero(1234), 4321);
    ASSERT_EQ(invertirNumero(9), 9);
    ASSERT_EQ(invertirNumero(1200), 21);
}

TEST_CASE(TestNegativos) {
    ASSERT_EQ(invertirNumero(-567), -765);
    ASSERT_EQ(invertirNumero(-10), -1);
}

TEST_CASE(TestCero) {
    ASSERT_EQ(invertirNumero(0), 0);
}

int main() {
    std::cout << "[SOLUCIÓN] Ejecutando tests de Ejercicio 06...\n";
    return RUN_ALL_TESTS();
}
