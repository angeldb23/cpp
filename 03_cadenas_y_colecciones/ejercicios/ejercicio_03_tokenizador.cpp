/* =============================================================================
   MÓDULO 03 - EJERCICIO 03: Tokenizador de Comandos
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA:
     Implementá tokenizar(string_view linea, char separador) que divide una
     cadena en tokens según el separador dado y retorna vector<string>.
     Ejemplo: tokenizar("mover,10,20", ',') → {"mover","10","20"}
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_03_tokenizador.cpp -o ej03 && ./ej03
   ============================================================================= */
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include "../../include/test_framework.hpp"

// TODO: implementar esta función
// TU CÓDIGO AQUÍ:

std::vector<std::string> tokenizar(std::string_view linea, char separador) {
    return {}; // placeholder
}

// ============= TESTS =============
TEST_CASE(TestTokenizarComa) {
    auto r = tokenizar("mover,10,20", ',');
    ASSERT_EQ(static_cast<int>(r.size()), 3);
    ASSERT_EQ(r[0], std::string("mover"));
    ASSERT_EQ(r[1], std::string("10"));
    ASSERT_EQ(r[2], std::string("20"));
}

TEST_CASE(TestTokenizarEspacio) {
    auto r = tokenizar("hola mundo fin", ' ');
    ASSERT_EQ(static_cast<int>(r.size()), 3);
    ASSERT_EQ(r[0], std::string("hola"));
    ASSERT_EQ(r[2], std::string("fin"));
}

TEST_CASE(TestTokenizarVacio) {
    auto r = tokenizar("", ',');
    ASSERT_EQ(static_cast<int>(r.size()), 0);
}

TEST_CASE(TestTokenizarUno) {
    auto r = tokenizar("solo", ',');
    ASSERT_EQ(static_cast<int>(r.size()), 1);
    ASSERT_EQ(r[0], std::string("solo"));
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 03 (Tokenizador)...\n";
    return RUN_ALL_TESTS();
}
