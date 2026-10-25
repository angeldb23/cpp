/* =============================================================================
   MÓDULO 02 - EJERCICIO 05: Uso de Argumentos con Valor por Defecto
   =============================================================================
   DIFICULTAD: Básico
   
   CONSIGNA:
   Implementa la función 'int multiplicaConDefault(int a, int b = 2)' que devuelve
   a * b. Si el segundo argumento se omite, se usará el valor por defecto 2.
   Escribe tests que verifiquen ambas llamadas.
   ============================================================================= */

#include <iostream>
#include "../../include/test_framework.hpp"

// TODO: Implementar multiplicaConDefault
int multiplicaConDefault(int a, int b = 2) {
    // TU CÓDIGO AQUÍ:
    return 0;
}

TEST_CASE(TestMultiplicaDefault) {
    ASSERT_EQ(multiplicaConDefault(5), 10); // usa default
    ASSERT_EQ(multiplicaConDefault(5, 3), 15);
}

int main() {
    std::cout << "Ejecutando tests de Ejercicio 05..." << std::endl;
    return RUN_ALL_TESTS();
}
