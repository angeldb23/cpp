/* =============================================================================
   MÓDULO 09 - EJERCICIO 06: Especialización total y parcial
   =============================================================================
   DIFICULTAD: Avanzado
   CONSIGNA:
     Dado el template primario NombreTipo<T> (devuelve "desconocido"), agregá especializaciones para que:
     - NombreTipo<int>, <double>, <bool>, <char> y <std::string> devuelvan "int", "double", "bool", "char" y "string" (especialización TOTAL)
     - NombreTipo<T*> devuelva "puntero a " + NombreTipo<T>::nombre() (PARCIAL)
     - NombreTipo<std::vector<T>> devuelva "vector de " + NombreTipo<T>::nombre() (PARCIAL)
     Además completá la variable template esPuntero<T>: false por defecto, true para punteros.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_06_especializacion.cpp -o ej06 && ./ej06
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

// Plantilla primaria: se usa cuando ninguna especialización coincide.
template <typename T>
struct NombreTipo {
    static std::string nombre() { return "desconocido"; }
};

// TODO 1: especializaciones TOTALES (template <> struct NombreTipo<int> {...})
//         para int, double, bool, char y std::string
// TODO 2: especialización PARCIAL para punteros: template <typename T> struct NombreTipo<T*>
// TODO 3: especialización PARCIAL para std::vector<T, A>
//         (vector tiene 2 parámetros: el tipo y el allocator)
// TU CÓDIGO AQUÍ:


// TODO 4: variable template. Hoy vale false para todo; hacé que valga true para T*
template <typename T>
constexpr bool esPuntero = false;
// TU CÓDIGO AQUÍ (la especialización parcial de la variable):

// ============= TESTS =============
TEST_CASE(TestTiposBasicos) {
    ASSERT_EQ(NombreTipo<int>::nombre(), std::string("int"));
    ASSERT_EQ(NombreTipo<double>::nombre(), std::string("double"));
    ASSERT_EQ(NombreTipo<bool>::nombre(), std::string("bool"));
    ASSERT_EQ(NombreTipo<char>::nombre(), std::string("char"));
    ASSERT_EQ(NombreTipo<std::string>::nombre(), std::string("string"));
}

TEST_CASE(TestTipoDesconocido) {
    ASSERT_EQ(NombreTipo<float>::nombre(), std::string("desconocido"));
}

TEST_CASE(TestPunteros) {
    ASSERT_EQ(NombreTipo<int*>::nombre(), std::string("puntero a int"));
    ASSERT_EQ(NombreTipo<double*>::nombre(), std::string("puntero a double"));
    ASSERT_EQ(NombreTipo<int**>::nombre(), std::string("puntero a puntero a int"));
}

TEST_CASE(TestVectores) {
    ASSERT_EQ(NombreTipo<std::vector<int>>::nombre(), std::string("vector de int"));
    ASSERT_EQ(NombreTipo<std::vector<double*>>::nombre(), std::string("vector de puntero a double"));
}

TEST_CASE(TestEsPuntero) {
    ASSERT_TRUE(esPuntero<int*>);
    ASSERT_TRUE(esPuntero<char*>);
    ASSERT_FALSE(esPuntero<int>);
    ASSERT_FALSE(esPuntero<std::string>);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 06 (Especialización total y parcial)...\n";
    return RUN_ALL_TESTS();
}
