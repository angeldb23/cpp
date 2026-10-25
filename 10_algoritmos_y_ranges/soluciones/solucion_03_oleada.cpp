/* =============================================================================
   MÓDULO 10 - SOLUCIÓN 03: all_any_none + for_each con estado (lección 04)
   =============================================================================
   Puntos de la resolución:
       - todosCasiMuertos con all_of; para el rango VACÍO devolvemos
         false EXPLÍCITAMENTE (la convención matemática de all_of
         devolvería true: acá el negocio lo dicta el enunciado: una oleada
         vacía  sin enemigos no es "todos casi muertos": la convención no alcanza).
       - hay/exists: any_of (corta apenas aparece).
       - totalYMayor: UN for_each con captura [&total, &mayor]: dos
         variables de salida completadas en la misma pasada.
       - contarConFunctor: un functor POR VALOR con estado interno; el
         retorno de for_each trae ese estado acumulado (la sección 3
         de la lección).
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

// SOLUCIÓN 1: all_of + la convención del vacío decidida A MANO.
bool todosCasiMuertos(const std::vector<Enemigo>& oleada) {
    if (oleada.empty()) return false;      // vacio: no es "todos"
    return std::all_of(oleada.begin(), oleada.end(),
                       [](const Enemigo& e) { return e.hp < 20; });
}

// SOLUCIÓN 2: any_of corta apenas aparece.
bool existeEnvenenado(const std::vector<Enemigo>& oleada) {
    return std::any_of(oleada.begin(), oleada.end(),
                       [](const Enemigo& e) { return e.envenenado; });
}

// SOLUCIÓN 3: un for_each que llena DOS salidas por referencia:
void totalYMayor(const std::vector<Enemigo>& oleada, int& total, int& mayor) {
    total = 0;   // la función calcula DESDE CERO (el test del vacío lo pide)
    std::for_each(oleada.begin(), oleada.end(),
                  [&](const Enemigo& e) {
                      total += e.hp;
                      if (e.hp > mayor) mayor = e.hp;
                  });
}

// SOLUCIÓN 4: functor por VALOR; for_each lo devuelve con su estado.
int contarConFunctor(const std::vector<Enemigo>& oleada) {
    struct Contador {
        int n = 0;
        void operator()(const Enemigo& e) {
            if (e.envenenado) ++n;
        }
    };
    return std::for_each(oleada.begin(), oleada.end(), Contador{}).n;
}

// ============= TESTS =============
TEST_CASE(TodosCasiMuertosExigeTodos) {
    ASSERT_EQ(todosCasiMuertos({
        {"A", 10, false}, {"B", 19, false},
    }), true);
    ASSERT_EQ(todosCasiMuertos({
        {"A", 25, false}, {"B", 19, false},
    }), false);
    ASSERT_EQ(todosCasiMuertos({}), false);
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
    ASSERT_EQ(total, 0);
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
    std::cout << "Solución 03 (oleada)...\n";
    return RUN_ALL_TESTS();
}
