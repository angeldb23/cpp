/* =============================================================================
   MÓDULO 08 - EJERCICIO 07: El vector crece: noexcept manda
   =============================================================================
   DIFICULTAD: Intermedio
   PRACTICA: lecciones 06 y 10 (noexcept como promesa consultada por la
   STL; el vector mueve al crecer SOLO si el movimiento es noexcept).

   CONSIGNA:
   La clase `Grano` ya tiene sus dos constructores completos. Hoy el
   operador de movimiento funciona PERO NO ESTÁ DECLARADO noexcept,
   y por eso el vector no lo usa al crecer (L10): copia cada grano.

      1. Agregale `noexcept` al constructor de movimiento (y a la
         asignación de movimiento).
      2. Los tests exigen: `noexcept(...)` == true y, después de
         meter 32 granos en un vector (que crece varias veces),
         `copias` NO haber crecido: todos los transportes fueron
         por movimiento.

   PISTA: es literalmente una palabra. Pero es LA palabra: sin ella
   todo tu esfuerzo de movimiento es ignorado por los contenedores.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_07_vector_crecimiento.cpp -o ej07 && ./ej07
   =============================================================================
 */
#include <cstddef>
#include <iostream>
#include <utility>
#include <vector>
#include "../../include/test_framework.hpp"

class Grano {
public:
    explicit Grano(int id) : id_(id) {}

    Grano(const Grano& otro) : id_(otro.id_) { ++copias_; }

    // TODO: agregar noexcept acá (constructor de movimiento):
    Grano(Grano&& otro) : id_(otro.id_) { ++movimientos_; }

    // TODO: y acá (asignación de movimiento):
    Grano& operator=(Grano&& otro) {
        id_ = otro.id_;
        ++movimientos_;
        return *this;
    }

    static int copias() { return copias_; }
    static int movimientos() { return movimientos_; }

private:
    int id_;
    inline static int copias_ = 0;
    inline static int movimientos_ = 0;
};

// ============= TESTS =============
TEST_CASE(MovimientosSonNoexcept) {
    ASSERT_EQ(noexcept(Grano(std::declval<Grano&&>())), true);
    ASSERT_EQ(noexcept(std::declval<Grano&>() = std::declval<Grano&&>()), true);
}

TEST_CASE(ElVectorMueveAlCrecer) {
    const int copiasAntes = Grano::copias();
    const int movesAntes = Grano::movimientos();
    std::vector<Grano> silo;                          // SIN reserve: que crezca solo
    for (int i = 0; i < 32; ++i) silo.push_back(Grano{i});
    ASSERT_EQ(silo.size(), std::size_t(32));
    ASSERT_EQ(Grano::copias(), copiasAntes);          // cero copias: todo movido
    ASSERT_TRUE(Grano::movimientos() > movesAntes);
}

TEST_CASE(PushBackConMoveExplicito) {
    const int copiasAntes = Grano::copias();
    std::vector<Grano> silo;
    Grano g(42);
    silo.push_back(std::move(g));
    ASSERT_EQ(Grano::copias(), copiasAntes);
    ASSERT_EQ(silo.size(), std::size_t(1));
}

int main() {
    std::cout << "Ejecutando tests de Ejercicio 07...\n";
    return RUN_ALL_TESTS();
}
