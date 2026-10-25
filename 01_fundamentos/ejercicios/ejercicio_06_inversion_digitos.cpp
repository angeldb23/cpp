/* =============================================================================
   MÓDULO 01 - EJERCICIO 06: Inversión de Dígitos Enteros
   =============================================================================
   DIFICULTAD: Intermedio

   CONSIGNA:
   Implementa la función 'int invertirNumero(int n)'
   Dado un número entero, invierte sus dígitos numéricos.
   Ejemplos:
     1234 -> 4321
     -567 -> -765
     1200 -> 21
        0 -> 0
   ============================================================================= */

#include <iostream>
#include "../../include/test_framework.hpp"

// TODO: Implementa 'invertirNumero'
int invertirNumero(int n) {
    // TU CÓDIGO AQUÍ:
    (void)n;
    return 0;
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
    std::cout << "Ejecutando tests de Ejercicio 06...\n";
    return RUN_ALL_TESTS();
}
