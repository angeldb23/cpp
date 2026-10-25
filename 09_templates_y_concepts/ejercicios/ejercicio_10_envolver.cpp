/* =============================================================================
   MÓDULO 09 - EJERCICIO 10: El envoltorio que copia o mueve (lecciones 06 y 07)
   =============================================================================
   DIFICULTAD: Avanzado
   CONSIGNA (temas 7 y 8 de TEMAS.md: referencias universales, collapsing,
   forward):
     La clase Marco<T> guarda una COPIA del dato, pero quiere absorberlo por
     MOVIMIENTO cuando el argumento era un rvalue (ya no lo usa nadie) y
     copiarlo cuando era un lvalue (el dueño lo sigue usando). Eso es
     exactamente lo que la lección 07 llamó "el wrapper que guarda".

     1. En Marco, el constructor recibe U&& (referencia universal): cambiá
        el std::move FORZOSO por std::forward<U> para decidir según cómo
        llegó cada argumento.
     2. En envolver, reenviá `valor` a Marco conservando la categoría
        original (syn forward<T>, no hay nada decidido).
     La struct Rastreada cuenta copias y movimientos: los tests la usan
     para verificar que no se coba nada de más.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_10_envolver.cpp -o ej10 && ./ej10
   ============================================================================= */
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>
#include "../../include/test_framework.hpp"

// Tipo con contadores para VER qué pasó en cada llamada:
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

// El envoltorio: guarda el dato SIN la referencia (por eso
// remove_reference_t); aún no sabe mover de verdad (TODO 1).
template <typename T>
class Marco {
public:
    std::remove_reference_t<T> contenido;

    template <typename U>
    explicit Marco(U&& valor)
        : contenido(std::move(valor)) {}          // TODO 1: ¿cuándo move y cuándo copy?
};

// TODO 2: reenviá `valor` a Marco con su categoría original.
// TU CÓDIGO AQUÍ:
template <typename T>
Marco<std::remove_reference_t<T>> envolver(T&& valor) {
    return Marco<std::remove_reference_t<T>>(std::move(valor));   // placeholder
}

// ============= TESTS =============
TEST_CASE(TestEnvolverLvalueCopiaUnaVez) {
    reiniciar();
    Rastreada original("lago");
    auto m = envolver(original);            // lvalue: el dueño lo sigue usando
    ASSERT_EQ(Rastreada::copias, 1);
    ASSERT_EQ(Rastreada::movimientos, 0);
    ASSERT_EQ(m.contenido.texto, std::string("lago"));
    ASSERT_EQ(original.texto, std::string("lago"));     // no robó el contenido
}

TEST_CASE(TestEnvolverRvalueMueveUnaVez) {
    reiniciar();
    auto m = envolver(Rastreada("pico"));   // rvalue temporal: derecho al move
    ASSERT_EQ(Rastreada::copias, 0);
    ASSERT_EQ(Rastreada::movimientos, 1);
    ASSERT_EQ(m.contenido.texto, std::string("pico"));
}

TEST_CASE(TestEnvolverConMoveExplicitoTambienMueve) {
    reiniciar();
    Rastreada fuente("salto");
    auto m = envolver(std::move(fuente));   // rvalue explícito: el dueño lo soltó
    ASSERT_EQ(Rastreada::copias, 0);
    ASSERT_EQ(Rastreada::movimientos, 1);
    ASSERT_EQ(m.contenido.texto, std::string("salto"));
}

TEST_CASE(TestElTipoDelMarcoNoTieneReferencia) {
    // El interior del Marco ya no es una referencia, es el tipo pelado:
    static_assert(std::is_same_v<std::remove_reference_t<int&>, int>);
    static_assert(std::is_same_v<std::remove_reference_t<int&&>, int>);
    static_assert(!std::is_reference_v<decltype(envolver(Rastreada("x")).contenido)>);
    ASSERT_TRUE(true);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 10 (envolver con forward)...\n";
    return RUN_ALL_TESTS();
}
