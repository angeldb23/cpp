/* =============================================================================
   MÓDULO 02 - EJERCICIO 06: Función Plantilla Simple
   =============================================================================
   DIFICULTAD: Intermedio
   
   CONSIGNA:
   Implementa la función plantilla `maximo<T>(const T& a, const T& b)` que devuelve
   el mayor de los dos valores usando el operador `>`. La función debe
   trabajar con cualquier tipo que soporte comparación.
   ============================================================================= */

#include <iostream>
#include "../../include/test_framework.hpp"

// TODO: Implementar plantilla maximo
template <typename T>
T maximo(const T& a, const T& b) {
    // TU CÓDIGO AQUÍ:
    return a; // placeholder
}

TEST_CASE(TestMaximo) {
    ASSERT_EQ(maximo(3, 5), 5);
    ASSERT_EQ(maximo(10.5, 2.3), 10.5);
    std::string s1 = "apple", s2 = "banana";
    ASSERT_EQ(maximo(s1, s2), s2);
}

int main() {
    std::cout << "Ejecutando tests de Ejercicio 06..." << std::endl;
    return RUN_ALL_TESTS();
}
