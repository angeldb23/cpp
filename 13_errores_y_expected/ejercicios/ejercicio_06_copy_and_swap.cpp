/* =============================================================================
   MÓDULO 13 - EJERCICIO 06: Copy-and-swap idiom
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA:
     Implementá una clase `Matriz` que guarda un array 2D dinámico (int**)
     con filas y columnas. Implementá copy-and-swap para el operador de
     asignación por copia.
     
     Requisitos:
     1. Constructor: Matriz(size_t filas, size_t cols) - inicializa en 0
     2. Copy constructor (profundo)
     3. Move constructor (robo de recursos, noexcept)
     4. Copy assignment: USA COPY-AND-SWAP (toma por valor, swap)
     5. Move assignment (robo, noexcept)
     6. Destructor
     7. swap(Matriz&) noexcept
     8. operator()(f, c) para acceso (con bounds check que lanza out_of_range)
     9. filas(), cols() constexpr
     
     El copy-and-swap logra garantía FUERTE en asignación: si la copia
     falla (bad_alloc), *this no se modifica.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_06_copy_and_swap.cpp -o ej06 && ./ej06
   ============================================================================= */
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <cstddef>
#include "../../include/test_framework.hpp"

// TODO: class Matriz { ... };

// ============= TESTS =============
TEST_CASE(TestConstructorYAcceso) {
    // Matriz m(2, 3);
    // ASSERT_EQ(m.filas(), 2u);
    // ASSERT_EQ(m.cols(), 3u);
    // m(0, 0) = 5;
    // m(1, 2) = 10;
    // ASSERT_EQ(m(0, 0), 5);
    // ASSERT_EQ(m(1, 2), 10);
    ASSERT_TRUE(false);
}

TEST_CASE(TestBoundsCheck) {
    // Matriz m(2, 2);
    // bool lanzo = false;
    // try { m(2, 0) = 1; } catch (const std::out_of_range&) { lanzo = true; }
    // ASSERT_TRUE(lanzo);
    // lanzo = false;
    // try { m(0, 2) = 1; } catch (const std::out_of_range&) { lanzo = true; }
    // ASSERT_TRUE(lanzo);
    ASSERT_TRUE(false);
}

TEST_CASE(TestCopyConstructor) {
    // Matriz a(2, 2);
    // a(0, 0) = 1; a(0, 1) = 2;
    // a(1, 0) = 3; a(1, 1) = 4;
    // Matriz b(a);
    // ASSERT_EQ(b(0, 0), 1);
    // ASSERT_EQ(b(1, 1), 4);
    // // Modificar b no afecta a
    // b(0, 0) = 99;
    // ASSERT_EQ(a(0, 0), 1);
    ASSERT_TRUE(false);
}

TEST_CASE(TestMoveConstructor) {
    // Matriz a(2, 2);
    // a(0, 0) = 7;
    // Matriz b(std::move(a));
    // ASSERT_EQ(b(0, 0), 7);
    // ASSERT_EQ(a.filas(), 0u); // movido: queda vacía
    ASSERT_TRUE(false);
}

TEST_CASE(TestCopyAssignmentCopyAndSwap) {
    // Matriz a(2, 2);
    // a(0, 0) = 1; a(1, 1) = 2;
    // Matriz b(3, 3); // tamaño distinto
    // b = a; // copy-and-swap
    // ASSERT_EQ(b.filas(), 2u);
    // ASSERT_EQ(b.cols(), 2u);
    // ASSERT_EQ(b(0, 0), 1);
    // ASSERT_EQ(b(1, 1), 2);
    // // a intacto
    // ASSERT_EQ(a(0, 0), 1);
    ASSERT_TRUE(false);
}

TEST_CASE(TestMoveAssignment) {
    // Matriz a(2, 2);
    // a(0, 0) = 42;
    // Matriz b(1, 1);
    // b = std::move(a);
    // ASSERT_EQ(b(0, 0), 42);
    // ASSERT_EQ(a.filas(), 0u);
    ASSERT_TRUE(false);
}

TEST_CASE(TestSelfAssignment) {
    // Matriz a(2, 2);
    // a(0, 0) = 5;
    // a = a; // no debe romper
    // ASSERT_EQ(a(0, 0), 5);
    ASSERT_TRUE(false);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 06 (Copy-and-swap)...\n";
    return RUN_ALL_TESTS();
}