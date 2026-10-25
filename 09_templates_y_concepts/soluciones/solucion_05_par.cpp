/* =============================================================================
   MÓDULO 09 - SOLUCIÓN 05: Template de clase con dos tipos: Par<A, B>
   =============================================================================
   POR QUÉ FUNCIONA:
     - Par<A, B> y Par<B, A> son tipos distintos; el template puede devolver otra instanciación de sí mismo.
     - '= default' en operator== (C++20) compara todos los miembros en orden.
     - hacerPar deduce A y B de los argumentos, sin escribir los tipos.
     - Al ser un agregado, Par se construye con llaves y sin constructor propio.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_05_par.cpp -o sol05 && ./sol05
   ============================================================================= */
#include <iostream>
#include <string>
#include <utility>
#include "../../include/test_framework.hpp"

template <typename A, typename B>
struct Par {
    A primero;
    B segundo;

    // Par<B, A> es OTRO tipo: dentro del template podemos nombrar otras
    // instanciaciones del mismo template.
    Par<B, A> intercambiar() const {
        return Par<B, A>{segundo, primero};
    }

    // C++20: el compilador genera == comparando miembro a miembro.
    bool operator==(const Par&) const = default;
};

// Una función auxiliar deduce A y B sola (antes de C++17 era la forma de
// evitar escribir los tipos a mano; hoy convive con CTAD).
template <typename A, typename B>
Par<A, B> hacerPar(A a, B b) {
    return Par<A, B>{a, b};
}

// ============= TESTS (los mismos del ejercicio) =============
TEST_CASE(TestIntercambiar) {
    Par<int, std::string> p{1, "uno"};
    auto q = p.intercambiar();                 // Par<std::string, int>
    ASSERT_EQ(q.primero, std::string("uno"));
    ASSERT_EQ(q.segundo, 1);
}

TEST_CASE(TestIntercambiarDosVeces) {
    Par<double, char> p{2.5, 'x'};
    auto q = p.intercambiar().intercambiar();
    ASSERT_NEAR(q.primero, 2.5, 1e-9);
    ASSERT_EQ(q.segundo, 'x');
}

TEST_CASE(TestIgualdad) {
    Par<int, int> a{1, 2};
    Par<int, int> b{1, 2};
    Par<int, int> c{2, 1};
    ASSERT_TRUE(a == b);
    ASSERT_FALSE(a == c);
}

TEST_CASE(TestHacerPar) {
    auto p = hacerPar(3, 4.5);                 // Par<int, double>
    ASSERT_EQ(p.primero, 3);
    ASSERT_NEAR(p.segundo, 4.5, 1e-9);
    auto s = hacerPar(std::string("k"), 7);
    ASSERT_EQ(s.primero, std::string("k"));
    ASSERT_EQ(s.segundo, 7);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 05 (Template de clase con dos tipos: Par<A, B>)...\n";
    return RUN_ALL_TESTS();
}
