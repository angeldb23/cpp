/* =============================================================================
   MÓDULO 02 - SOLUCIÓN 05: Uso de Argumentos con Valor por Defecto
   =============================================================================
   Implementación de la función 'multiplicaConDefault' que devuelve a * b.
   Si b no se especifica, se usa el valor por defecto 2.
   ============================================================================= */

#include <iostream>
#include "../../include/test_framework.hpp"

// Declaración con argumento por defecto
int multiplicaConDefault(int a, int b = 2) {
    return a * b;
}

TEST_CASE(TestMultiplicaConDefault) {
    ASSERT_EQ(multiplicaConDefault(3, 4), 12);
    ASSERT_EQ(multiplicaConDefault(5), 10); // usa b=2 por defecto
}

int main() {
    std::cout << "[SOLUCIÓN] Ejecutando tests de Solución 05..." << std::endl;
    return RUN_ALL_TESTS();
}
