/* =============================================================================
   MÓDULO 10 - EJERCICIO 08: acumular y medir (lección 10)
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA (tema 6 de TEMAS.md: numéricos): las estadísticas de temporada.
       1. dañoNombres(nadao, armaduras): el daño NETO total: para cada i,
          daras[i] - armaduras[i] (inner_product con dos ops: combinar y
          acumular; y CORTÁ el rango al tamaño más chico si difieren: la
          lección advierte que inner_product NO valida).
       2. calculado(bonos): el producto de todos los bonos (accumulate
          con std::multiplies y 1 como INIT CORRECTO).
       3. promedio(velocidades): double (el accumulate con 0.0 SIEMPRE:
          los ints trunca).
       4. picksAcumulados(puntos): los acumulados PARCIALES como vector
          (partial_sum; resultado conCantidad == entrada.size()).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_08_numerico.cpp -o ej08 && ./ej08
   ============================================================================= */
#include <algorithm>
#include <cstddef>
#include <numeric>
#include <vector>
#include "../../include/test_framework.hpp"

// TODO 1: danioNeto: inner_product sobre las dos secuencias (combin: resta;
//         acumul: suma). Corta al tamaño más chico.
// TU CODIGO AQUI:
int danioNeto([[maybe_unused]] const std::vector<int>& danos,
              [[maybe_unused]] const std::vector<int>& armaduras) {
    return 0;    // placeholder
}

// TODO 2: multiplicarTodos: accumulate con multiplies (init 1, no 0!).
// TU CODIGO AQUI:
int multiplicarTodos([[maybe_unused]] const std::vector<int>& bonos) {
    return 0;    // placeholder
}

// TODO 3: promedio: accumulate double / size.
// TU CODIGO AQUI (asumí que la lista no está vacía):
double promedio([[maybe_unused]] const std::vector<int>& velocidades) {
    return 0.0;    // placeholder
}

// TODO 4: acumulados: partial_sum a un vector resultado.
// TU CODIGO AQUI:
std::vector<int> acumulados([[maybe_unused]] const std::vector<int>& puntos) {
    return {};    // placeholder
}

// ============= TESTS =============
TEST_CASE(DanioNetoPorElemento) {
    ASSERT_EQ(danioNeto({15, 20, 8, 32}, {2, 2, 0, 1}), 70);   // 13+18+8+31
    ASSERT_EQ(danioNeto({10}, {5}), 5);
    ASSERT_EQ(danioNeto({10, 10}, {5}), 5);    // el de sobra no suma
    ASSERT_EQ(danioNeto({}, {}), 0);
}

TEST_CASE(MultiplicarBonos) {
    ASSERT_EQ(multiplicarTodos({2, 3, 2}), 12);
    ASSERT_EQ(multiplicarTodos({5}), 5);
    ASSERT_EQ(multiplicarTodos({}), 1);        // producto vacio: identidad
}

TEST_CASE(PromedioEnDouble) {
    ASSERT_NEAR(promedio({2, 3}), 2.5, 1e-9);  // 5.0/2: el truncado seria 2
    ASSERT_NEAR(promedio({10, 20, 33}), 21.0, 1e-9);
}

TEST_CASE(AcumuladosParciales) {
    ASSERT_TRUE((acumulados({15, 20, 8}) == std::vector<int>{15, 35, 43}));
    ASSERT_TRUE((acumulados({10}) == std::vector<int>{10}));
    ASSERT_TRUE(acumulados({}).empty());
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 08 (numericos)...\n";
    return RUN_ALL_TESTS();
}
