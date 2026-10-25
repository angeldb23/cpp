/* =============================================================================
   MÓDULO 09 - EJERCICIO 05: Template de clase con dos tipos: Par<A, B>
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA:
     Completá la struct template Par<A, B>:
     1. intercambiar() const -> Par<B, A>: devuelve un Par con los valores invertidos
     2. operator==: dos pares son iguales si lo son sus dos componentes
     3. hacerPar(a, b): función que construye un Par deduciendo los tipos
     La struct es un agregado: se inicializa con llaves, Par<int, std::string> p{1, "uno"}.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_05_par.cpp -o ej05 && ./ej05
   ============================================================================= */
#include <iostream>
#include <string>
#include <utility>
#include "../../include/test_framework.hpp"

// TODO 1: intercambiar() devuelve Par<B, A> con primero y segundo intercambiados
// TODO 2: operator== compara primero y segundo (en C++20 alcanza con '= default')
// TU CÓDIGO AQUÍ:
template <typename A, typename B>
struct Par {
    A primero;
    B segundo;

    Par<B, A> intercambiar() const {
        return Par<B, A>{};   // placeholder
    }

    bool operator==([[maybe_unused]] const Par& otro) const {
        return false;         // placeholder
    }
};

// TODO 3: hacerPar construye un Par<A, B> a partir de dos valores
// TU CÓDIGO AQUÍ:
template <typename A, typename B>
Par<A, B> hacerPar([[maybe_unused]] A a, [[maybe_unused]] B b) {
    return Par<A, B>{};       // placeholder
}

// ============= TESTS =============
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
    std::cout << "Ejecutando pruebas para Ejercicio 05 (Template de clase con dos tipos: Par<A, B>)...\n";
    return RUN_ALL_TESTS();
}
