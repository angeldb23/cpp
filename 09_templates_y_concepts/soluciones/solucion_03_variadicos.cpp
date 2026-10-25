/* =============================================================================
   MÓDULO 09 - SOLUCIÓN 03: Templates variádicos y fold expressions
   =============================================================================
   POR QUÉ FUNCIONA:
     - Args... es un parameter pack: representa cero o más tipos. args... son los valores.
     - La fold (args + ... + 0) expande a a1 + a2 + ... + 0; el 0 final cubre el pack vacío.
     - sizeof...(Args) cuenta los elementos del pack, sin recorrerlos.
     - En unir, la fold sobre el operador coma ejecuta 'imprimir y marcar primero=false' una vez por argumento.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_03_variadicos.cpp -o sol03 && ./sol03
   ============================================================================= */
#include <iostream>
#include <string>
#include <sstream>
#include <cstddef>
#include "../../include/test_framework.hpp"

// (args + ... + 0) es una "fold derecha-izquierda con valor inicial":
// con 0 argumentos devuelve 0 en vez de dar error de compilación.
template <typename... Args>
auto suma(Args... args) {
    return (args + ... + 0);
}

// Una fold unaria sobre && con el pack vacío vale true por definición.
template <typename... Args>
bool todos(Args... args) {
    return (... && args);
}

template <typename... Args>
constexpr std::size_t cantidad(Args...) {
    return sizeof...(Args);
}

// Fold sobre el operador coma: se ejecuta una expresión por argumento, en orden.
template <typename... Args>
std::string unir(const std::string& sep, const Args&... args) {
    std::ostringstream os;
    bool primero = true;
    (void)primero;   // con pack vacío la fold no se expande: evita el warning
    ((os << (primero ? std::string() : sep) << args, primero = false), ...);
    return os.str();
}

// ============= TESTS (los mismos del ejercicio) =============
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
    std::cout << "Ejecutando pruebas para Solución 03 (Templates variádicos y fold expressions)...\n";
    return RUN_ALL_TESTS();
}
