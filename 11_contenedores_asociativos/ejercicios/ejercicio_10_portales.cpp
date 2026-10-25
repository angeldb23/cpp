/* =============================================================================
   MÓDULO 11 - EJERCICIO 10: entidades de tipos alternativos (lección 14)
   =============================================================================
   DIFICULTAD: Medio
   CONSIGNA (tema 7 de TEMAS.md: std::variant y std::visit):
     Un portal abre entidades de dos mundos; cada entidad trae o un
     HUMANO (std::string: su nombre) o un GOLEM (int: su fuerza):
       struct Entidad = std::variant<std::string, int>;  // humano | golem
       1. esteMundo(e): el string "humano" si es string, "golem" si es
          int (holds_alternative: L14).
       2. BONUS DE ESTILO: podés resolver el 1 con std::visit y una
          lambda POR ALTERNATIVA ("overloaded pattern", L14) o con
          holds_alternative: el tesis acepta AMBAS variantes.
       3. fuerza(v): el int SI es golem (get<int>, o get_if),
          o 0 si es humano (sin lanzar: get_if de la L14).
       4. pelea(v1, v2): si los dos son golems, el MAYOR de las dos
          fuerzas; si uno es golem y otro humano, la fuerza del
          golem; si ambos son humanos: 0.
   (los TODO son únicamente las funciones; el resto ya está listo).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_10_portales.cpp -o ej10 && ./ej10
   ============================================================================= */
#include <iostream>
#include <string>
#include <variant>
#include "../../include/test_framework.hpp"

using Entidad = std::variant<std::string, int>;   // string=humano, int=golem

// TODO 1: el nombre del mundo de la entidad:
//         "humano" si es string, "golem" si es int.
std::string esteMundo([[maybe_unused]] const Entidad& e) {
    return "";    // placeholder
}

// TODO 2: la fuerza del golem (o 0 si es humano) SIN lanzar (get_if):
int fuerza([[maybe_unused]] const Entidad& e) {
    return 0;    // placeholder
}

// TODO 3: pelea: si los dos son golems, el MAYOR de las dos fuerzas;
//         si solo uno es golem, la fuerza de ESE; 0 si ninguno.
int pelea([[maybe_unused]] const Entidad& a, [[maybe_unused]] const Entidad& b) {
    return 0;    // placeholder
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
    ASSERT_EQ(pelea(Entidad{10}, Entidad{6}), 10);      // golem vs golem
    ASSERT_EQ(pelea(Entidad{10}, Entidad{std::string("ana")}), 10);
    ASSERT_EQ(pelea(Entidad{std::string("ana")}, Entidad{3}), 3);
    ASSERT_EQ(pelea(Entidad{std::string("ana")}, Entidad{std::string("bl")}), 0);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 10 (entidades variant)...\n";
    return RUN_ALL_TESTS();
}
