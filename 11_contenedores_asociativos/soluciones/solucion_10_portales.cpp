/* =============================================================================
   MÓDULO 11 - SOLUCIÓN 10: entidades de tipos alternativos (ejercicio 10)
   =============================================================================
   L14 es la lección de respaldo: holds_alternative/get_if/get, la
   solución usa visit + "overloaded pattern" para el mundo de cada
   alternativa, y get_if para las consultas sin excepciones.
   ============================================================================= */
#include <algorithm>
#include <iostream>
#include <string>
#include <variant>
#include "../../include/test_framework.hpp"

using Entidad = std::variant<std::string, int>;    // humano | golem

// El "overloaded pattern" de la L14:
template <class... Lambdas>
struct EnCadaTipo : Lambdas... {
    using Lambdas::operator()...;
};
template <class... Lambdas>
EnCadaTipo(Lambdas...) -> EnCadaTipo<Lambdas...>;

std::string esteMundo(const Entidad& e) {
    return std::visit(
        EnCadaTipo{
            [](const std::string&) { return std::string{"humano"}; },
            [](const int&) { return std::string{"golem"}; },
        },
        e);
}

int fuerza(const Entidad& e) {
    const int* f = std::get_if<int>(&e);    // get_if: sin lanzar (L14)
    return f == nullptr ? 0 : *f;
}

int pelea(const Entidad& a, const Entidad& b) {
    return std::max(fuerza(a), fuerza(b));  // humanos aportan fuerza 0
}

// ============= TESTS =============
TEST_CASE(EsteMundoPorAlternativa) {
    const Entidad humano{"ana"};
    const Entidad golem{42};
    ASSERT_EQ(esteMundo(humano), "humano");
    ASSERT_EQ(esteMundo(golem), "golem");
}

TEST_CASE(FuerzaSoloEnGolems) {
    ASSERT_EQ(fuerza(Entidad{7}), 7);
    ASSERT_EQ(fuerza(Entidad{std::string("ana")}), 0);
}

TEST_CASE(PeleaEntreMundos) {
    ASSERT_EQ(pelea(Entidad{10}, Entidad{6}), 10);
    ASSERT_EQ(pelea(Entidad{10}, Entidad{std::string("ana")}), 10);
    ASSERT_EQ(pelea(Entidad{std::string("ana")}, Entidad{3}), 3);
    ASSERT_EQ(pelea(Entidad{std::string("ana")}, Entidad{std::string("bl")}), 0);
}

int main() {
    std::cout << "Ejecutando pruebas para Solucion 10 (entidades variant)...\n";
    return RUN_ALL_TESTS();
}
