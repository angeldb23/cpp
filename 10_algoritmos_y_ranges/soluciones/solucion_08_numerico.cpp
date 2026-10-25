/* =============================================================================
   MÓDULO 10 - SOLUCIÓN 08: acumular y medir (lección 10)
   =============================================================================
   Los cuatro pliegues:
       - danioNeto: inner_product de dos secuencias: `combinar = resta`,
         `acumular = suma`; el rango corto al tamaño min (recordá la
         advertencia de la lección: el algoritmo no chequea).
       - multiplicarTodos: accumulate con multiplies y init 1 (0 sería
         el error clásico: 0 * nada = 0).
       - promedio: el accumulate con 0.0 (double) / size: el init 0 es
         el init 0 truncaría cada paso: el bug que la lección mostró.
       - acumulados: partial_sum sobre un vector resultado del tamaño
         exacto.
   ============================================================================= */
#include <algorithm>
#include <cstddef>
#include <functional>
#include <numeric>
#include <vector>
#include "../../include/test_framework.hpp"

// SOLUCIÓN 1: inner_product con el resto como combinación y suma como
// accumulator: primero corta al mínimo (los tests piden ese contrato):
int danioNeto(const std::vector<int>& danos,
              const std::vector<int>& armaduras) {
    const std::size_t n = std::min(danos.size(), armaduras.size());
    return std::inner_product(
        danos.begin(), danos.begin() + static_cast<long>(n),
        armaduras.begin(), 0,
        std::plus<int>{},                              // acumular: +
        [](int dano, int armadura) { return dano - armadura; });  // combinar
}

// SOLUCIÓN 2: init 1 (el IDENTIDAD del producto):
int multiplicarTodos(const std::vector<int>& bonos) {
    return std::accumulate(bonos.begin(), bonos.end(), 1,
                           std::multiplies<int>{});
}

// SOLUCIÓN 3: el init en DOUBLE (0.0) decide el tipo del acumulador:
double promedio(const std::vector<int>& velocidades) {
    const double suma = std::accumulate(velocidades.begin(),
                                        velocidades.end(), 0.0);
    return suma / static_cast<double>(velocidades.size());
}

// SOLUCIÓN 4: partial_sum con espacio exacto:
std::vector<int> acumulados(const std::vector<int>& puntos) {
    std::vector<int> resultado;
    resultado.resize(puntos.size());
    std::partial_sum(puntos.begin(), puntos.end(), resultado.begin());
    return resultado;
}

// ============= TESTS =============
TEST_CASE(DanioNetoPorElemento) {
    ASSERT_EQ(danioNeto({15, 20, 8, 32}, {2, 2, 0, 1}), 70);
    ASSERT_EQ(danioNeto({10}, {5}), 5);
    ASSERT_EQ(danioNeto({10, 10}, {5}), 5);
    ASSERT_EQ(danioNeto({}, {}), 0);
}

TEST_CASE(MultiplicarBonos) {
    ASSERT_EQ(multiplicarTodos({2, 3, 2}), 12);
    ASSERT_EQ(multiplicarTodos({5}), 5);
    ASSERT_EQ(multiplicarTodos({}), 1);
}

TEST_CASE(PromedioEnDouble) {
    ASSERT_NEAR(promedio({2, 3}), 2.5, 1e-9);
    ASSERT_NEAR(promedio({10, 20, 33}), 21.0, 1e-9);
}

TEST_CASE(AcumuladosParciales) {
    ASSERT_TRUE((acumulados({15, 20, 8}) == std::vector<int>{15, 35, 43}));
    ASSERT_TRUE((acumulados({10}) == std::vector<int>{10}));
    ASSERT_TRUE(acumulados({}).empty());
}

int main() {
    std::cout << "Solución 08 (numericos)...\n";
    return RUN_ALL_TESTS();
}
