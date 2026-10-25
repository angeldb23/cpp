/* =============================================================================
   MÓDULO 09 - SOLUCIÓN 06: Especialización total y parcial
   =============================================================================
   POR QUÉ FUNCIONA:
     - Especialización total: template <> y el tipo exacto. Parcial: sigue habiendo parámetros (T*).
     - El compilador elige la especialización MÁS específica que coincida; si ninguna, la primaria.
     - NombreTipo<int**> funciona por recursión: T* con T = int* y otra vez T* con T = int.
     - Las funciones template no se pueden especializar parcialmente; las clases y variables sí.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_06_especializacion.cpp -o sol06 && ./sol06
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

template <typename T>
struct NombreTipo {
    static std::string nombre() { return "desconocido"; }
};

// --- Especializaciones TOTALES: un tipo exacto ---
template <> struct NombreTipo<int>         { static std::string nombre() { return "int"; } };
template <> struct NombreTipo<double>      { static std::string nombre() { return "double"; } };
template <> struct NombreTipo<bool>        { static std::string nombre() { return "bool"; } };
template <> struct NombreTipo<char>        { static std::string nombre() { return "char"; } };
template <> struct NombreTipo<std::string> { static std::string nombre() { return "string"; } };

// --- Especializaciones PARCIALES: una familia de tipos ---
// Se aplica a int*, double**, etc. y se llama a sí misma para el tipo apuntado.
template <typename T>
struct NombreTipo<T*> {
    static std::string nombre() { return "puntero a " + NombreTipo<T>::nombre(); }
};

// std::vector<T> es en realidad std::vector<T, Allocator>: hay que nombrar los dos.
template <typename T, typename A>
struct NombreTipo<std::vector<T, A>> {
    static std::string nombre() { return "vector de " + NombreTipo<T>::nombre(); }
};

// Variable template con especialización parcial
template <typename T>
constexpr bool esPuntero = false;

template <typename T>
constexpr bool esPuntero<T*> = true;

// ============= TESTS (los mismos del ejercicio) =============
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
    std::cout << "Ejecutando pruebas para Solución 06 (Especialización total y parcial)...\n";
    return RUN_ALL_TESTS();
}
