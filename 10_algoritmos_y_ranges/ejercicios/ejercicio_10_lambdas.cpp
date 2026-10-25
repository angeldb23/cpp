/* =============================================================================
   MÓDULO 10 - EJERCICIO 10: lambdas con estado (lecciones 12 y 13)
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA (tema 7 de TEMAS.md: lambdas a fondo): contadores que viajan.
     1. fabricaContador(): devuelve UNA lambda con init-capture
        `[n = 0]` y `mutable` (L12). : cada llamada de la
        Cada llamada de la MISMA lambda avanza su `n`; dos contadores
        creados con dos llamadas de fabricaContador arrancan de cero
        cada uno (el estado vive DENTRO de cada objeto lambda).
        Devolvé la lambda con auto: su tipo es único (NO hace falta
        std::function: el caller la mantiene concreta con auto).
     2. acumularHistoria(historia): con init-capture y std::move, guardar
        el vector DENTRO de la lambda (la jugada `[c = std::move(c)]`
        de la L12): la lambda queda ÚNICA dueña. La lambda devuelve
        cuántos elementos guardó (y gracias al move, el original queda
        vacío: el test lo espera).
        Firma sugerida: auto; aceptá el vector por REFERENCIA RVALUE
        (std::vector<std::string>&&) para que el move drene al original
        (por valor moverías una copia y el original seguiría lleno).
        `auto zoo = acumularHistoria(std::vector<std::string>{...});`
     3. ordenarPorHp<T>(secuencia): UNA función template con UN
        comparador GENÉRICO por el campo `hp` (la L13): la misma lambda
        sirve para Crew y para Torreta (los dos tienen .hp).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_10_lambdas.cpp -o ej10 && ./ej10
   ============================================================================= */
#include <algorithm>
#include <string>
#include <utility>
#include <vector>
#include "../../include/test_framework.hpp"

struct Crew {
    std::string nombre;
    int hp;
};

struct Torreta {
    std::string nombre;
    int hp;
};

// TODO 1: fabricaContador(): devuelve una lambda con SU n=0 (init-
//         capture) y mutable:
// TU CODIGO AQUI (firma con auto)
auto fabricaContador() {
    return []() { return 0; };    // placeholder (no es mutable todavia)
}

// TODO 2: acumularHistoria: la lambda queda UNICA dueña del vector.
// TU CODIGO AQUI (firma con auto y aceptando el vector por valor)
auto acumularHistoria([[maybe_unused]] std::vector<std::string>&& historia) {
    return []() { return 0; };    // placeholder
}

// TODO 3: ordenarPorHp: sort con comparador generico por el campo hp.
// TU CODIGO AQUI (template del contenedor):
template <typename Secuencia>
void ordenarPorHp([[maybe_unused]] Secuencia& secuencia) {
}

// ============= TESTS =============
TEST_CASE(CadaContadorEsIndepenciente) {
    auto a = fabricaContador();
    auto b = fabricaContador();
    ASSERT_EQ(a(), 1);          // el primero
    ASSERT_EQ(a(), 2);          // sigue
    ASSERT_EQ(b(), 1);          // el otro arranca de cero
    ASSERT_EQ(a(), 3);
}

TEST_CASE(ContadorNoColapsa) {
    auto c = fabricaContador();
    (void)c(); (void)c();
    ASSERT_EQ(c(), 3);          // la misma lambda sigue sumando
}

TEST_CASE(HistoriaGuardadaPorMove) {
    std::vector<std::string> historia{"tanda1", "tanda2", "tanda3"};
    auto guardia = acumularHistoria(std::move(historia));  // el traspaso
    ASSERT_EQ(guardia(), 3);    // la lambda tiene LOS 3
    ASSERT_TRUE(historia.empty());   // el original no queda con datos (move)
    auto copia = guardia;       // copiar la lambda copia su vector
    ASSERT_EQ(copia(), 3);
}

TEST_CASE(OrdenarGenericoPorHp) {
    std::vector<Crew> equipo{
        {"Quesito", 90}, {"Mordel", 60}, {"Turbo", 120},
    };
    std::vector<Torreta> defensas{
        {"Pistola", 70}, {"Cañon", 30},
    };
    ordenarPorHp(equipo);
    ordenarPorHp(defensas);
    ASSERT_EQ(equipo[0].hp, 60);   // asc
    ASSERT_EQ(equipo[2].hp, 120);
    ASSERT_EQ(defensas[0].hp, 30);
    ASSERT_EQ(defensas[1].hp, 70);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 10 (lambdas)...\n";
    return RUN_ALL_TESTS();
}
