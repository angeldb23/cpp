/* =============================================================================
   MÓDULO 01 - EJERCICIO 07: Potencia Entera en Tiempo de Compilación (constexpr)
   =============================================================================
   DIFICULTAD: Intermedio

   CONSIGNA:
   Implementa la función:
     'constexpr long long potencia(long long base, unsigned int exponente)'
   - base^0 = 1
   - Calcula base elevado al exponente mediante un bucle iterativo o exponenciación rápida.
   - Al estar marcada como 'constexpr', debe ser válida tanto en tiempo de ejecución
     como en expresiones 'static_assert' en tiempo de compilación.
   ============================================================================= */

#include <iostream>
#include "../../include/test_framework.hpp"

// TODO: Implementa 'potencia' marcada como constexpr
constexpr long long potencia(long long base, unsigned int exponente) {
    // TU CÓDIGO AQUÍ:
    (void)base;
    (void)exponente;
    return 0LL;
}

// Verificación en tiempo de compilación (si la función estuviera resuelta, pasaría static_assert)
// static_assert(potencia(2, 3) == 8, "2^3 debe ser 8 en compilacion");

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
    std::cout << "Ejecutando tests de Ejercicio 07...\n";
    return RUN_ALL_TESTS();
}
