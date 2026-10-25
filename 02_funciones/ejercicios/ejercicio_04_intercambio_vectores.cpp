/* =============================================================================
   MÓDULO 02 - EJERCICIO 04: Intercambio de Vectores por Referencia
   =============================================================================
   DIFICULTAD: Básico
   
   CONSIGNA:
   Implementa la función 'void intercambiarVectores(std::vector<int>& a, std::vector<int>& b)'
   que intercambia el contenido de dos vectores usando std::swap.
   ============================================================================= */

#include <iostream>
#include <vector>
#include "../../include/test_framework.hpp"

// TODO: Implementar intercambiarVectores
void intercambiarVectores(std::vector<int>& a, std::vector<int>& b) {
    // TU CÓDIGO AQUÍ:
    (void)a; (void)b;
}

TEST_CASE(TestIntercambioVectores) {
    std::vector<int> v1 = {1,2,3};
    std::vector<int> v2 = {4,5};
    intercambiarVectores(v1, v2);
    ASSERT_EQ(v1.size(), 2);
    ASSERT_EQ(v2.size(), 3);
    ASSERT_EQ(v1[0], 4);
    ASSERT_EQ(v2[0], 1);
}

int main() {
    std::cout << "Ejecutando tests de Ejercicio 04..." << std::endl;
    return RUN_ALL_TESTS();
}
