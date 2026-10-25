/* =============================================================================
   MÓDULO 02 - SOLUCIÓN 04: Intercambio de Vectores por Referencia
   =============================================================================
   Implementación de la función que intercambia el contenido de dos vectores usando std::swap.
   Incluye pruebas unitarias.
   ============================================================================= */

#include <vector>
#include <algorithm>
#include "../../include/test_framework.hpp"

void intercambiarVectores(std::vector<int>& a, std::vector<int>& b) {
    std::swap(a, b);
}

TEST_CASE(TestIntercambioVectores) {
    std::vector<int> v1 = {1, 2, 3};
    std::vector<int> v2 = {4, 5};
    intercambiarVectores(v1, v2);
    ASSERT_EQ(v1.size(), 2);
    ASSERT_EQ(v2.size(), 3);
    ASSERT_EQ(v1[0], 4);
    ASSERT_EQ(v1[1], 5);
    ASSERT_EQ(v2[0], 1);
    ASSERT_EQ(v2[1], 2);
    ASSERT_EQ(v2[2], 3);
}

int main() {
    std::cout << "[SOLUCIÓN] Ejecutando tests de Intercambio Vectores..." << std::endl;
    return RUN_ALL_TESTS();
}
