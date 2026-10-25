/* =============================================================================
   MÓDULO 02 - SOLUCIÓN 06: Función Plantilla Simple - maximo
   =============================================================================
   Implementación de la función plantilla `maximo<T>` que devuelve el mayor
   de dos valores que soportan el operador `>`.
   ============================================================================= */

#include <utility>
#include "../../include/test_framework.hpp"

// Retorna el mayor de a y b usando el operador >
template <typename T>
T maximo(const T& a, const T& b) {
    return (a > b) ? a : b;
}

TEST_CASE(TestMaximo) {
    // Tipos íntegros
    ASSERT_EQ(maximo(3, 5), 5);
    // Tipos de punto flotante
    ASSERT_EQ(maximo(10.5, 2.3), 10.5);
    // Cadenas std::string
    std::string s1 = "apple", s2 = "banana";
    ASSERT_EQ(maximo(s1, s2), s2);
}

int main() {
    std::cout << "[SOLUCIÓN] Ejecutando tests de Solución 06 (template maximo)...\n";
    return RUN_ALL_TESTS();
}
