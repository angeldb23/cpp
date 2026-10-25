/* =============================================================================
   MÓDULO 01 - SOLUCIÓN 05: Fibonacci Iterativo Seguro
   ============================================================================= */

#include <iostream>
#include "../../include/test_framework.hpp"

unsigned long long fibonacci(unsigned int n) {
    if (n == 0) return 0ULL;
    if (n == 1) return 1ULL;

    unsigned long long anterior = 0ULL;
    unsigned long long actual = 1ULL;

    for (unsigned int i = 2; i <= n; ++i) {
        unsigned long long siguiente = anterior + actual;
        anterior = actual;
        actual = siguiente;
    }

    return actual;
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
    std::cout << "[SOLUCIÓN] Ejecutando tests de Ejercicio 05...\n";
    return RUN_ALL_TESTS();
}
