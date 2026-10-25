/* =============================================================================
   MÓDULO 10 - EJERCICIO 09: partition y minmax_element (lección 11)
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA (tema 6 de TEMAS.md: partición): el triage de la tanda.
     Los enemigos llegan en una std::vector<Enemigo>. Con partition /
     partition_point / minmax_element:
       1. separar(oleada): partition los VIVOS (hp > 0) a la izquierda;
          y nada más: no ordena adentro de cada lado. Devuelve el INDICE
          del corte (std::distance al begin del iterador que partition
          devuelve).
       2. puntoDeCorte(oleada): SOBRE un vector YA particionado, usa
          partition_point para el MISMO indice sin recorrer la secuencia
          (la lower_bound de los particionados: la L11).
       3. extremosDe(hps): el par (min, max) de UNA pasada con
          minmax_element, devuelta como pair de valores (desreferencia el
          par de iteradores; el vector no puede estar vacío en el test).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_09_triage.cpp -o ej09 && ./ej09
   ============================================================================= */
#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>
#include "../../include/test_framework.hpp"

struct Enemigo {
    std::string nombre;
    int hp;
};

namespace {
bool esVivo(const Enemigo& e) { return e.hp > 0; }
}  // namespace

// TODO 1: separar: partition vivos a la izquierda; devolver el INDICE.
// TU CODIGO AQUI:
std::size_t separar([[maybe_unused]] std::vector<Enemigo>& oleada) {
    return 0;    // placeholder
}

// TODO 2: puntoDeCorte: partition_point SOBRE un rango ya particionado.
// TU CODIGO AQUI (asumí que la invariante vale; el test te la da):
std::size_t puntoDeCorte([[maybe_unused]] const std::vector<Enemigo>& oleada) {
    return 0;    // placeholder
}

// TODO 3: extremosDe: minmax_element -> par de VALORES (min, max).
// TU CODIGO AQUI:
std::pair<int, int> extremosDe([[maybe_unused]] const std::vector<int>& hps) {
    return {0, 0};    // placeholder
}

// ============= TESTS =============
TEST_CASE(SepararAgrupaVivosSinOrdenarAdentro) {
    std::vector<Enemigo> oleada{
        {"Blob", 20}, {"Muerto", 0}, {"Vespa", 45}, {"Otro", 0},
    };
    const auto corte = separar(oleada);
    ASSERT_EQ(corte, 2u);
    // lo de la IZQUIERDA de `corte` son toda vivos:
    ASSERT_TRUE(std::all_of(oleada.begin(),
                            oleada.begin() + static_cast<long>(corte),
                            esVivo));
    // lo del lado derecho son todos NO vivos:
    ASSERT_TRUE(std::all_of(oleada.begin() + static_cast<long>(corte),
                            oleada.end(),
                            [](const Enemigo& e) { return !esVivo(e); }));
    // y los vivos conservan SU orden relativo (partition NO garantiza:
    // es exclusivo de stable_partition: ACA el test chequea solo el
    // corte correcto: es partición EXACTA: el conjunto de la izquierda
    // es el mismo conjunto de los vivos que entraron):
    ASSERT_EQ(std::count_if(oleada.begin(), oleada.end(), esVivo), 2);
}

TEST_CASE(PuntoDeCorteEncuentraLaMismaFrontera) {
    std::vector<Enemigo> oleada{
        {"A", 10}, {"B", 20}, {"C", 0}, {"D", 5},
    };
    (void)separar(oleada);          // la particiono con la función anterior
    ASSERT_EQ(puntoDeCorte(oleada), separar(oleada));
}

TEST_CASE(ExtremosEnUnaPasada) {
    const std::vector<int> hps{90, 15, 60, 5, 72};
    const auto [minimo, maximo] = extremosDe(hps);
    ASSERT_EQ(minimo, 5);
    ASSERT_EQ(maximo, 90);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 09 (triage)...\n";
    return RUN_ALL_TESTS();
}
