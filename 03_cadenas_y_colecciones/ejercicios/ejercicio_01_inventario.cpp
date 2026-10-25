/* =============================================================================
   MÓDULO 03 - EJERCICIO 01: Procesador de Inventario
   =============================================================================
   DIFICULTAD: Básico
   CONSIGNA:
     Implementá las siguientes funciones para gestionar un inventario de texto:
     1. contarPalabras(string_view texto) → int: cuenta palabras separadas por espacios
     2. primeraMayuscula(std::string s) → std::string: primera letra en mayúscula
     3. unirInventario(vector<string>) → string: une ítems con ", " entre ellos
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_01_inventario.cpp -o ej01 && ./ej01
   ============================================================================= */
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <sstream>
#include "../../include/test_framework.hpp"

// TODO: implementar estas funciones
// TU CÓDIGO AQUÍ:

int contarPalabras(std::string_view texto) {
    return 0; // placeholder
}

std::string primeraMayuscula(std::string s) {
    return s; // placeholder
}

std::string unirInventario(const std::vector<std::string>& items) {
    return ""; // placeholder
}

// ============= TESTS =============
TEST_CASE(TestContarPalabras) {
    ASSERT_EQ(contarPalabras("hola mundo"),     2);
    ASSERT_EQ(contarPalabras("uno dos tres"),   3);
    ASSERT_EQ(contarPalabras(""),               0);
    ASSERT_EQ(contarPalabras("solo"),           1);
}

TEST_CASE(TestPrimeraMayuscula) {
    ASSERT_EQ(primeraMayuscula("espada"), std::string("Espada"));
    ASSERT_EQ(primeraMayuscula("ARCO"),   std::string("ARCO"));
    ASSERT_EQ(primeraMayuscula(""),       std::string(""));
}

TEST_CASE(TestUnirInventario) {
    std::vector<std::string> v = {"Espada", "Escudo", "Poción"};
    ASSERT_EQ(unirInventario(v), std::string("Espada, Escudo, Poción"));
    std::vector<std::string> uno = {"X"};
    ASSERT_EQ(unirInventario(uno), std::string("X"));
    std::vector<std::string> vacio = {};
    ASSERT_EQ(unirInventario(vacio), std::string(""));
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 01 (Inventario)...\n";
    return RUN_ALL_TESTS();
}
