/* =============================================================================
   MÓDULO 10 - SOLUCIÓN 09: partition y minmax_element (lección 11)
   =============================================================================
   Resolución:
       - separar: partition con el pred esVivo; el índice del corte es
         el distance al iterador devuelto (partition NO conserva el orden
         adentro de cada lado: eso es stable_partition).
       - puntoDeCorte: sobre un rango YA particionado: partition_point
         lo encuentra en O(log n) con el mismo pred.
       - extremosDe: minmax_element en UNA pasada; el par de iteradores
         se desreferencia y sale como par de valores.
   ============================================================================= */
#include <algorithm>
#include <cstddef>
#include <iterator>
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

// SOLUCIÓN 1: partition + distance del corte:
std::size_t separar(std::vector<Enemigo>& oleada) {
    const auto corte = std::partition(oleada.begin(), oleada.end(), esVivo);
    return static_cast<std::size_t>(std::distance(oleada.begin(), corte));
}

// SOLUCIÓN 2: el mismo punto, en O(log n), sin re-recorrer:
std::size_t puntoDeCorte(const std::vector<Enemigo>& oleada) {
    const auto punto = std::partition_point(oleada.begin(), oleada.end(),
                                            esVivo);
    return static_cast<std::size_t>(std::distance(oleada.begin(), punto));
}

// SOLUCIÓN 3: minmax_element: min y max en una sola pasada:
std::pair<int, int> extremosDe(const std::vector<int>& hps) {
    const auto extremos = std::minmax_element(hps.begin(), hps.end());
    return {*extremos.first, *extremos.second};
}

// ============= TESTS =============
TEST_CASE(SepararAgrupaVivosSinOrdenarAdentro) {
    std::vector<Enemigo> oleada{
        {"Blob", 20}, {"Muerto", 0}, {"Vespa", 45}, {"Otro", 0},
    };
    const auto corte = separar(oleada);
    ASSERT_EQ(corte, 2u);
    ASSERT_TRUE(std::all_of(oleada.begin(),
                            oleada.begin() + static_cast<long>(corte),
                            esVivo));
    ASSERT_TRUE(std::all_of(oleada.begin() + static_cast<long>(corte),
                            oleada.end(),
                            [](const Enemigo& e) { return !esVivo(e); }));
    ASSERT_EQ(std::count_if(oleada.begin(), oleada.end(), esVivo), 2);
}

TEST_CASE(PuntoDeCorteEncuentraLaMismaFrontera) {
    std::vector<Enemigo> oleada{
        {"A", 10}, {"B", 20}, {"C", 0}, {"D", 5},
    };
    (void)separar(oleada);
    ASSERT_EQ(puntoDeCorte(oleada), 3u);
}

TEST_CASE(ExtremosEnUnaPasada) {
    const std::vector<int> hps{90, 15, 60, 5, 72};
    const auto [minimo, maximo] = extremosDe(hps);
    ASSERT_EQ(minimo, 5);
    ASSERT_EQ(maximo, 90);
}

int main() {
    std::cout << "Solución 09 (triage)...\n";
    return RUN_ALL_TESTS();
}
