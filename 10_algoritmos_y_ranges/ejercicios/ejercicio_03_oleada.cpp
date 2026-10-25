/* =============================================================================
   MÓDULO 10 - EJERCICIO 03: all_any_none + for_each con estado (lección 04)
   =============================================================================
   DIFICULTAD: Basico-Intermedio
   CONSIGNA (tema 2 de TEMAS.md: no modificantes): una oleada de enemigos
   (struct con nombre, hp y envenenado). Con SOLO los algoritmos de la
   lección:
     1. todosCasiMuertos(oleada): true si TODOS tienen hp < 20 (all_of;
        recordá la convención con rango vacío de la lección: decidí vos
        qué devolver y explicá la respuesta (los tests piden false)).
     2. existeEnvenenado(oleada): true si ALGUNO está envenenado (any_of);
        cortá apenas aparezca: no cuentes de más (se puede verificar con
        un pred sin estado: el test no fuerza la optimización, for_each
        sí).
     3. totalYMayor(oleada, total, mayor): con UN solo `for_each` llenar
        total (suma de hp) y mayor (el hp más grande). Parámetros por
        referencia (la captura [&] de la lección). `mayor` inicializá con
        algo inválido y el for_each corrige (o usá std::numeric_limits).
     4. contarConFunctor(oleada): usa for_each con un FUNCTOR por valor
        que haga el conteo, y aprovecha que for_each DEVUELVE el functor
        (la sección 3 de la lección): ese valor devuelto es tu contador.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_03_oleada.cpp -o ej03 && ./ej03
   ============================================================================= */
#include <algorithm>
#include <limits>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

struct Enemigo {
    std::string nombre;
    int hp;
    bool envenenado;
};

// TODO 1: todosCasiMuertos: all_of con hp < 20.
// TU CODIGO AQUI:
bool todosCasiMuertos([[maybe_unused]] const std::vector<Enemigo>& oleada) {
    return false;    // placeholder
}

// TODO 2: existeEnvenenado: any_of envenenado.
// TU CODIGO AQUI:
bool existeEnvenenado([[maybe_unused]] const std::vector<Enemigo>& oleada) {
    return false;    // placeholder
}

// TODO 3: totalYMayor con UN solo for_each (por referencia modification):
// TU CODIGO AQUI:
void totalYMayor([[maybe_unused]] const std::vector<Enemigo>& oleada,
                 [[maybe_unused]] int& total,
                 [[maybe_unused]] int& mayor) {
    // TU CODIGO AQUI (for_each con lambda por referencia)
}

// TODO 4: contarConFunctor: cuántos envenenados, contados por un functor
//         pasado POR VALOR, usando el RETORNO de for_each.
// TU CODIGO AQUI:
int contarConFunctor([[maybe_unused]] const std::vector<Enemigo>& oleada) {
    return 0;    // placeholder
}

// ============= TESTS =============
TEST_CASE(TodosCasiMuertosExigeTodos) {
    ASSERT_EQ(todosCasiMuertos({
        {"A", 10, false}, {"B", 19, false},
    }), true);
    ASSERT_EQ(todosCasiMuertos({
        {"A", 25, false}, {"B", 19, false},
    }), false);
    ASSERT_EQ(todosCasiMuertos({}), false);     // vacio NO es "todos": pregunta
}

TEST_CASE(ExisteEnvenenado) {
    ASSERT_EQ(existeEnvenenado({
        {"A", 50, false}, {"B", 30, true},
    }), true);
    ASSERT_EQ(existeEnvenenado({
        {"A", 50, false},
    }), false);
    ASSERT_EQ(existeEnvenenado({}), false);
}

TEST_CASE(TotalYMayorUnaSolaPasada) {
    const std::vector<Enemigo> oleada{
        {"Blob", 30, false}, {"Vespa", 12, true}, {"Murk", 60, false},
    };
    int total = 0;
    int mayor = std::numeric_limits<int>::min();
    totalYMayor(oleada, total, mayor);
    ASSERT_EQ(total, 102);
    ASSERT_EQ(mayor, 60);
}

TEST_CASE(TotalYMayorVacio) {
    int total = 7;
    int mayor = std::numeric_limits<int>::min();
    totalYMayor({}, total, mayor);
    ASSERT_EQ(total, 0);                        // nada sumo
    ASSERT_EQ(mayor, std::numeric_limits<int>::min());
}

TEST_CASE(ContarConFunctorPorValor) {
    const std::vector<Enemigo> oleada{
        {"A", 10, true}, {"B", 20, false}, {"C", 30, true}, {"D", 40, true},
    };
    ASSERT_EQ(contarConFunctor(oleada), 3);
    ASSERT_EQ(contarConFunctor({}), 0);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 03 (oleada)...\n";
    return RUN_ALL_TESTS();
}
