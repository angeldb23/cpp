/* =============================================================================
   MÓDULO 03 - EJERCICIO 06: Palíndromo
   =============================================================================
   DIFICULTAD: Básico
   CONSIGNA:
     Implementá esPalindromo(string_view texto) que retorna true si el texto
     es un palíndromo (se lee igual al derecho y al revés), ignorando mayúsculas
     y espacios.
     Ejemplo: "A man a plan a canal Panama" → true (ignorando espacios)
              "radar" → true
              "hola"  → false
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_06_palindromo.cpp -o ej06 && ./ej06
   ============================================================================= */
#include <iostream>
#include <string>
#include <string_view>
#include "../../include/test_framework.hpp"

// TODO: implementar esta función
// TU CÓDIGO AQUÍ:

bool esPalindromo(std::string_view texto) {
    return false; // placeholder
}

// ============= TESTS =============
TEST_CASE(TestPalindromosSimples) {
    ASSERT_TRUE(esPalindromo("radar"));
    ASSERT_TRUE(esPalindromo("oso"));
    ASSERT_TRUE(esPalindromo("a"));
    ASSERT_TRUE(esPalindromo(""));
}

TEST_CASE(TestNoPalindromos) {
    ASSERT_FALSE(esPalindromo("hola"));
    ASSERT_FALSE(esPalindromo("mundo"));
    ASSERT_FALSE(esPalindromo("abc"));
}

TEST_CASE(TestPalindromoCaseInsensitive) {
    ASSERT_TRUE(esPalindromo("Radar"));
    ASSERT_TRUE(esPalindromo("LEVEL"));
}

TEST_CASE(TestPalindromoSinEspacios) {
    ASSERT_TRUE(esPalindromo("a man a plan a canal panama"));
    ASSERT_TRUE(esPalindromo("Anita lava la tina"));
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 06 (Palíndromo)...\n";
    return RUN_ALL_TESTS();
}
