/* =============================================================================
   MÓDULO 13 - EJERCICIO 04: noexcept condicional
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA:
     Implementá una clase template `Contenedor<T>` que guarda un std::vector<T>
     y tiene un método `agregar(T&& valor)` cuyo noexcept depende de T:
     - noexcept si T es nothrow move constructible
     - NO noexcept en caso contrario
     
     Usá: noexcept(std::is_nothrow_move_constructible_v<T>)
     
     También implementá:
     - `size()` constexpr noexcept
     - `vacío()` constexpr noexcept
     
     Probá con:
     - int (nothrow move)
     - std::string (nothrow move en implementaciones modernas)
     - Un tipo con move que lanza (definilo vos)
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_04_noexcept_condicional.cpp -o ej04 && ./ej04
   ============================================================================= */
#include <iostream>
#include <vector>
#include <string>
#include <type_traits>
#include <utility>
#include "../../include/test_framework.hpp"

// TODO: struct TipoMoveQueLanza { ... };

// TODO: template<typename T>
// class Contenedor { ... };

// ============= TESTS =============
TEST_CASE(TestContenedorIntNoexcept) {
    // static_assert(noexcept(std::declval<Contenedor<int>&>().agregar(42)));
    ASSERT_TRUE(false);
}

TEST_CASE(TestContenedorStringNoexcept) {
    // static_assert(noexcept(std::declval<Contenedor<std::string>&>().agregar(std::string("hola"))));
    ASSERT_TRUE(false);
}

TEST_CASE(TestContenedorTipoLanzaNoNoexcept) {
    // static_assert(!noexcept(std::declval<Contenedor<TipoMoveQueLanza>&>().agregar(TipoMoveQueLanza{})));
    ASSERT_TRUE(false);
}

TEST_CASE(TestSizeYVacioSonNoexcept) {
    // Contenedor<int> c;
    // static_assert(noexcept(c.size()));
    // static_assert(noexcept(c.vacio()));
    ASSERT_TRUE(false);
}

TEST_CASE(TestAgregarFunciona) {
    // Contenedor<int> c;
    // c.agregar(1);
    // c.agregar(2);
    // ASSERT_EQ(c.size(), 2u);
    // ASSERT_FALSE(c.vacio());
    ASSERT_TRUE(false);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 04 (noexcept condicional)...\n";
    return RUN_ALL_TESTS();
}