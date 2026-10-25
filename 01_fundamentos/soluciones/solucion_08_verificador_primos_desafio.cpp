/* =============================================================================
   MÓDULO 01 - SOLUCIÓN 08: Verificador y Contador de Primos Optimizado
   ============================================================================= */

#include <iostream>
#include "../../include/test_framework.hpp"

bool esPrimo(unsigned long long n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;

    for (unsigned long long i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) {
            return false;
        }
    }
    return true;
}

unsigned int contarPrimosHasta(unsigned int limite) {
    unsigned int contador = 0;
    for (unsigned int i = 2; i <= limite; ++i) {
        if (esPrimo(i)) {
            contador++;
        }
    }
    return contador;
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
    std::cout << "[SOLUCIÓN] Ejecutando tests de Ejercicio 08...\n";
    return RUN_ALL_TESTS();
}
