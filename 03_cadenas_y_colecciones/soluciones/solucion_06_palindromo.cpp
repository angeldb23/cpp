/* =============================================================================
   MÓDULO 03 - SOLUCIÓN 06: Palíndromo
   =============================================================================
   POR QUÉ FUNCIONA:
     - No hace falta comparar la mitad completa: alcanza con la primera mitad.
     - rbegin() es un iterador inverso: recorre el string desde el final hacia el principio.
     - Con texto vacío o de 1 letra la mitad es 0 y equal devuelve true.
     - Un palíndromo de largo impar ignora la letra del medio (size()/2 redondea hacia abajo).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_06_palindromo.cpp -o sol06 && ./sol06
   ============================================================================= */
#include <iostream>
#include <string>
#include <string_view>
#include <algorithm>
#include <cctype>
#include <cstddef>
#include "../../include/test_framework.hpp"

// 1) Armamos una copia "limpia": sin espacios y en minúsculas.
// 2) Comparamos la primera mitad contra la segunda leída al revés (rbegin).
bool esPalindromo(std::string_view texto) {
    std::string limpio;
    for (char c : texto) {
        if (c != ' ') {
            limpio += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
    }
    return std::equal(limpio.begin(), limpio.begin() + static_cast<std::ptrdiff_t>(limpio.size() / 2),
                      limpio.rbegin());
}

// ============= TESTS (los mismos del ejercicio) =============
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
    std::cout << "Ejecutando pruebas para Solución 06 (Palíndromo)...\n";
    return RUN_ALL_TESTS();
}
