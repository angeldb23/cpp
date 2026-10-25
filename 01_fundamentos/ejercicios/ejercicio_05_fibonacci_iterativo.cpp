/* =============================================================================
   MÓDULO 01 - EJERCICIO 05: Fibonacci Iterativo Seguro
   =============================================================================
   DIFICULTAD: Intermedio

   CONSIGNA:
   Implementa la función 'unsigned long long fibonacci(unsigned int n)'
   - F(0) = 0
   - F(1) = 1
   - F(n) = F(n-1) + F(n-2)
   
   ¡IMPORTANTE!:
   Debe ser resuelto de forma ITERATIVA (con un bucle for o while), no recursivo,
   para evitar el desbordamiento de la pila (Stack Overflow) y tener complejidad O(N).
   ============================================================================= */

#include <iostream>
#include "../../include/test_framework.hpp"

// TODO: Implementa 'fibonacci' iterativo
unsigned long long fibonacci(unsigned int n) {
    // TU CÓDIGO AQUÍ:
    (void)n;
    return 999999ULL;
}

TEST_CASE(TestCasosBase) {
    ASSERT_EQ(fibonacci(0), 0ULL);
    ASSERT_EQ(fibonacci(1), 1ULL);
}

TEST_CASE(TestValoresPrimeros) {
    ASSERT_EQ(fibonacci(2), 1ULL);
    ASSERT_EQ(fibonacci(3), 2ULL);
    ASSERT_EQ(fibonacci(4), 3ULL);
    ASSERT_EQ(fibonacci(5), 5ULL);
    ASSERT_EQ(fibonacci(6), 8ULL);
    ASSERT_EQ(fibonacci(7), 13ULL);
}

TEST_CASE(TestValorAvanzado) {
    ASSERT_EQ(fibonacci(10), 55ULL);
    ASSERT_EQ(fibonacci(20), 6765ULL);
}

int main() {
    std::cout << "Ejecutando tests de Ejercicio 05...\n";
    return RUN_ALL_TESTS();
}
