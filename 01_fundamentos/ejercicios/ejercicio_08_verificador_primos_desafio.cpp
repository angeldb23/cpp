/* =============================================================================
   MÓDULO 01 - EJERCICIO 08: Verificador y Contador de Primos Optimizado
   =============================================================================
   DIFICULTAD: Desafío

   CONSIGNA:
   1. Implementa 'bool esPrimo(unsigned long long n)':
      - Un número <= 1 no es primo.
      - 2 y 3 son primos.
      - Si n es divisible por 2 o 3, no es primo.
      - Itera verificando divisores hasta la raíz cuadrada de n: bucle de paso 6 (i = 5; i * i <= n; i += 6).
      - Retorna true si es primo, false si es compuesto.

   2. Implementa 'unsigned int contarPrimosHasta(unsigned int limite)':
      - Retorna cuántos números primos existen en el rango [1, limite].
   ============================================================================= */

#include <iostream>
#include "../../include/test_framework.hpp"

// TODO 1: Implementa esPrimo
bool esPrimo(unsigned long long n) {
    // TU CÓDIGO AQUÍ:
    (void)n;
    return false;
}

// TODO 2: Implementa contarPrimosHasta
unsigned int contarPrimosHasta(unsigned int limite) {
    // TU CÓDIGO AQUÍ:
    (void)limite;
    return 0;
}

TEST_CASE(TestCasosBasePrimos) {
    ASSERT_FALSE(esPrimo(0));
    ASSERT_FALSE(esPrimo(1));
    ASSERT_TRUE(esPrimo(2));
    ASSERT_TRUE(esPrimo(3));
    ASSERT_FALSE(esPrimo(4));
    ASSERT_TRUE(esPrimo(5));
}

TEST_CASE(TestPrimosGrandes) {
    ASSERT_TRUE(esPrimo(104729)); // El primo número 10,000
    ASSERT_FALSE(esPrimo(104730));
    ASSERT_TRUE(esPrimo(9999999967ULL));
}

TEST_CASE(TestContarPrimos) {
    ASSERT_EQ(contarPrimosHasta(10), 4U);   // 2, 3, 5, 7
    ASSERT_EQ(contarPrimosHasta(20), 8U);   // + 11, 13, 17, 19
    ASSERT_EQ(contarPrimosHasta(100), 25U);
}

int main() {
    std::cout << "Ejecutando tests de Ejercicio 08...\n";
    return RUN_ALL_TESTS();
}
