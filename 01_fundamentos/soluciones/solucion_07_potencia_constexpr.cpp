/* =============================================================================
   MÓDULO 01 - SOLUCIÓN 07: Potencia Entera en Tiempo de Compilación (constexpr)
   ============================================================================= */

#include <iostream>
#include "../../include/test_framework.hpp"

constexpr long long potencia(long long base, unsigned int exponente) {
    long long resultado = 1LL;
    while (exponente > 0) {
        if (exponente % 2 == 1) {
            resultado *= base;
        }
        base *= base;
        exponente /= 2;
    }
    return resultado;
}

// Verificación garantizada en tiempo de compilación con static_assert:
static_assert(potencia(2, 3) == 8LL, "2^3 debe ser 8 en compilacion");
static_assert(potencia(10, 4) == 10000LL, "10^4 debe ser 10000 en compilacion");

TEST_CASE(TestPotenciaCasosBase) {
    ASSERT_EQ(potencia(5, 0), 1LL);
    ASSERT_EQ(potencia(0, 5), 0LL);
}

TEST_CASE(TestPotenciaValores) {
    ASSERT_EQ(potencia(2, 3), 8LL);
    ASSERT_EQ(potencia(2, 10), 1024LL);
    ASSERT_EQ(potencia(3, 4), 81LL);
    ASSERT_EQ(potencia(10, 5), 100000LL);
}

int main() {
    std::cout << "[SOLUCIÓN] Ejecutando tests de Ejercicio 07...\n";
    return RUN_ALL_TESTS();
}
