/* =============================================================================
   MÓDULO 09 - EJERCICIO 03: Templates variádicos y fold expressions
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA:
     Usá parameter packs y fold expressions (C++17) para escribir funciones con cantidad variable de argumentos:
     1. suma(args...): suma todo (0 si no hay argumentos); debe funcionar con int y double
     2. todos(args...): true si TODOS los argumentos son true (true si no hay ninguno)
     3. cantidad(args...): cuántos argumentos recibió (pista: sizeof...)
     4. unir(sep, args...): texto con los argumentos separados por 'sep'
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_03_variadicos.cpp -o ej03 && ./ej03
   ============================================================================= */
#include <iostream>
#include <string>
#include <sstream>
#include <cstddef>
#include "../../include/test_framework.hpp"

// TODO 1: suma. Pista: (args + ... + 0)
// TU CÓDIGO AQUÍ:
template <typename... Args>
auto suma([[maybe_unused]] Args... args) {
    return 0;   // placeholder
}

// TODO 2: todos. Pista: (... && args)
// TU CÓDIGO AQUÍ:
template <typename... Args>
bool todos([[maybe_unused]] Args... args) {
    return false;   // placeholder
}

// TODO 3: cantidad. Pista: sizeof...(Args)
// TU CÓDIGO AQUÍ:
template <typename... Args>
constexpr std::size_t cantidad([[maybe_unused]] Args... args) {
    return 0;   // placeholder
}

// TODO 4: unir. Pista: una fold sobre el operador coma y una bandera 'primero'
// TU CÓDIGO AQUÍ:
template <typename... Args>
std::string unir([[maybe_unused]] const std::string& sep, [[maybe_unused]] const Args&... args) {
    return "";   // placeholder
}

// ============= TESTS =============
TEST_CASE(TestSuma) {
    ASSERT_EQ(suma(1, 2, 3), 6);
    ASSERT_EQ(suma(10), 10);
    ASSERT_EQ(suma(), 0);
    ASSERT_NEAR(suma(1.5, 2.5), 4.0, 1e-9);
    ASSERT_NEAR(suma(1, 2.5), 3.5, 1e-9);
}

TEST_CASE(TestTodos) {
    ASSERT_TRUE(todos(true, true, true));
    ASSERT_FALSE(todos(true, false, true));
    ASSERT_TRUE(todos());
}

TEST_CASE(TestCantidad) {
    ASSERT_EQ(cantidad(), 0u);
    ASSERT_EQ(cantidad(1, 'a', 2.5), 3u);
    ASSERT_EQ(cantidad(1, 2, 3, 4, 5), 5u);
}

TEST_CASE(TestUnir) {
    ASSERT_EQ(unir(", "), std::string(""));
    ASSERT_EQ(unir(", ", 1, 2, 3), std::string("1, 2, 3"));
    ASSERT_EQ(unir("/", "a", "b"), std::string("a/b"));
    ASSERT_EQ(unir(" ", "x", 2, 3.5), std::string("x 2 3.5"));
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 03 (Templates variádicos y fold expressions)...\n";
    return RUN_ALL_TESTS();
}
