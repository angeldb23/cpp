/* =============================================================================
   MÓDULO 03 - SOLUCIÓN 01: Procesador de Inventario
   =============================================================================
   POR QUÉ FUNCIONA:
     - contarPalabras recorre una sola vez (O(n)) y detecta el INICIO de cada palabra.
     - primeraMayuscula recibe el string por valor: ya es una copia, la modificamos y la devolvemos.
     - unirInventario usa size_t como índice (el tipo de size()), sin warnings de signo.
     - Necesitás <cctype> para std::toupper.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_01_inventario.cpp -o sol01 && ./sol01
   ============================================================================= */
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <cctype>
#include "../../include/test_framework.hpp"

// Cuenta "palabras" como bloques de caracteres distintos de espacio.
// La bandera enPalabra evita contar dos veces la misma palabra.
int contarPalabras(std::string_view texto) {
    int cuenta = 0;
    bool enPalabra = false;
    for (char c : texto) {
        if (c == ' ') {
            enPalabra = false;
        } else if (!enPalabra) {
            enPalabra = true;
            ++cuenta;
        }
    }
    return cuenta;
}

// OJO: toupper espera un unsigned char convertido a int; pasarle un char
// negativo (por ejemplo una letra con tilde en UTF-8) es comportamiento indefinido.
std::string primeraMayuscula(std::string s) {
    if (!s.empty()) {
        s[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(s[0])));
    }
    return s;
}

// Ponemos el separador ANTES de cada ítem salvo el primero, así nunca
// queda una coma colgando al final.
std::string unirInventario(const std::vector<std::string>& items) {
    std::string resultado;
    for (std::size_t i = 0; i < items.size(); ++i) {
        if (i > 0) resultado += ", ";
        resultado += items[i];
    }
    return resultado;
}

// ============= TESTS (los mismos del ejercicio) =============
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
    std::cout << "Ejecutando pruebas para Solución 01 (Procesador de Inventario)...\n";
    return RUN_ALL_TESTS();
}
