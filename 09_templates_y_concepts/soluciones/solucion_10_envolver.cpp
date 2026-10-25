/* =============================================================================
   MÓDULO 09 - SOLUCIÓN 10: El envoltorio que copia o mueve
   =============================================================================
   POR QUÉ FUNCIONA:
     - `std::forward<U>(valor)` conserva la categoría del argumento de la
       llamada a Marco: si ligó a un lvalue (U dedujo referencia), el
       contenido se COPIA; si U dedujo el tipo pelado, se MUEVE.
     - `std::forward<T>(valor)` en envolvers es lo mismo con el T de la
       función: quien decide el comportamiento es la llamada, no el código.
     - remove_reference_t<T> hace copia del interior sin referencias: el
       Marco es dueño del dato, no un alias prestado.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_10_envolver.cpp -o sol10 && ./sol10
   ============================================================================= */
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>
#include "../../include/test_framework.hpp"

class Rastreada {
public:
    static inline int copias = 0;
    static inline int movimientos = 0;

    std::string texto;
    explicit Rastreada(std::string t) : texto(std::move(t)) {}
    Rastreada(const Rastreada& o) : texto(o.texto) { ++copias; }
    Rastreada(Rastreada&& o) noexcept : texto(std::move(o.texto)) { ++movimientos; }
};

static void reiniciar() {
    Rastreada::copias = 0;
    Rastreada::movimientos = 0;
}

template <typename T>
class Marco {
public:
    std::remove_reference_t<T> contenido;

    template <typename U>
    explicit Marco(U&& valor)
        : contenido(std::forward<U>(valor)) {}      // lvalue -> copia, rvalue -> move
};

template <typename T>
Marco<std::remove_reference_t<T>> envolver(T&& valor) {
    return Marco<std::remove_reference_t<T>>(std::forward<T>(valor));   // conservar categoría
}

// ============= TESTS (los mismos del ejercicio) =============
TEST_CASE(TestEnvolverLvalueCopiaUnaVez) {
    reiniciar();
    Rastreada original("lago");
    auto m = envolver(original);
    ASSERT_EQ(Rastreada::copias, 1);
    ASSERT_EQ(Rastreada::movimientos, 0);
    ASSERT_EQ(m.contenido.texto, std::string("lago"));
    ASSERT_EQ(original.texto, std::string("lago"));
}

TEST_CASE(TestEnvolverRvalueMueveUnaVez) {
    reiniciar();
    auto m = envolver(Rastreada("pico"));
    ASSERT_EQ(Rastreada::copias, 0);
    ASSERT_EQ(Rastreada::movimientos, 1);
    ASSERT_EQ(m.contenido.texto, std::string("pico"));
}

TEST_CASE(TestEnvolverConMoveExplicitoTambienMueve) {
    reiniciar();
    Rastreada fuente("salto");
    auto m = envolver(std::move(fuente));
    ASSERT_EQ(Rastreada::copias, 0);
    ASSERT_EQ(Rastreada::movimientos, 1);
    ASSERT_EQ(m.contenido.texto, std::string("salto"));
}

TEST_CASE(TestElTipoDelMarcoNoTieneReferencia) {
    static_assert(std::is_same_v<std::remove_reference_t<int&>, int>);
    static_assert(std::is_same_v<std::remove_reference_t<int&&>, int>);
    static_assert(!std::is_reference_v<decltype(envolver(Rastreada("x")).contenido)>);
    ASSERT_TRUE(true);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 10 (envolver con forward)...\n";
    return RUN_ALL_TESTS();
}
