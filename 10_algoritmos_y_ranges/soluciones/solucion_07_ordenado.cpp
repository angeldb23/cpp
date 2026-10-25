/* =============================================================================
   MÓDULO 10 - SOLUCIÓN 07: búsqueda sobre ordenado (lección 09)
   =============================================================================
   El esqueleto es TODO lower_bound/upper_bound:
       - rango: equal_range devuelve el PAR de iteradores; convertir cada
         uno en índice con distance sobre begin.
       - existe: binary_search (bool).
       - insertarManteniendoOrden: lower_bound da el lugar EXACTO donde
         va `nuevo`: insert ahí de deja todo ASC (con repetidos: lower
         inserta ANTES de los iguales).
       - ranking: `upper - lower` del tramo: la cuenta exacta de iguales
         (en O(log n), mucho mejor que un count lineal).
   ============================================================================= */
#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>
#include "../../include/test_framework.hpp"

using Tramo = std::pair<std::size_t, std::size_t>;

// SOLUCIÓN 1: equal_range y las distancias (el par de índices):
Tramo rango(const std::vector<int>& v, int valor) {
    const auto par = std::equal_range(v.begin(), v.end(), valor);
    return {static_cast<std::size_t>(std::distance(v.begin(), par.first)),
            static_cast<std::size_t>(std::distance(v.begin(), par.second))};
}

// SOLUCIÓN 2: binary_search:
bool existe(const std::vector<int>& v, int valor) {
    return std::binary_search(v.begin(), v.end(), valor);
}

// SOLUCIÓN 3: lower_bound + insert (el vector sigue ASC):
void insertarManteniendoOrden(std::vector<int>& v, int nuevo) {
    const auto pos = std::lower_bound(v.begin(), v.end(), nuevo);
    v.insert(pos, nuevo);
}

// SOLUCIÓN 4: el tramo completo de iguales, en O(log n):
std::size_t ranking(const std::vector<int>& v, int miPuntaje) {
    const auto [pri, fin] = rango(v, miPuntaje);
    return fin - pri;
}

// ============= TESTS =============
TEST_CASE(RangoDelimitaElTramoDeIguales) {
    const std::vector<int> v{10, 20, 30, 30, 30, 40};
    const auto [pris, finsi] = rango(v, 30);
    ASSERT_EQ(pris, 2u);
    ASSERT_EQ(finsi, 5u);
    const auto [a, b] = rango(v, 25);
    ASSERT_EQ(a, 2u);
    ASSERT_EQ(b, 2u);
}

TEST_CASE(ExisteRespondeAlBooleano) {
    const std::vector<int> v{10, 20, 30};
    ASSERT_EQ(existe(v, 30), true);
    ASSERT_EQ(existe(v, 25), false);
}

TEST_CASE(InsertarManteniendoOrden) {
    std::vector<int> v{12, 45, 78};
    insertarManteniendoOrden(v, 50);
    ASSERT_TRUE((v == std::vector<int>{12, 45, 50, 78}));
    insertarManteniendoOrden(v, 5);
    ASSERT_TRUE((v == std::vector<int>{5, 12, 45, 50, 78}));
    insertarManteniendoOrden(v, 90);
    ASSERT_TRUE((v == std::vector<int>{5, 12, 45, 50, 78, 90}));
    insertarManteniendoOrden(v, 45);
    ASSERT_TRUE((v == std::vector<int>{5, 12, 45, 45, 50, 78, 90}));
}

TEST_CASE(RankingConRepetidos) {
    const std::vector<int> v{10, 20, 30, 30, 30, 40};
    ASSERT_EQ(ranking(v, 30), 3u);
    ASSERT_EQ(ranking(v, 10), 1u);
    ASSERT_EQ(ranking(v, 99), 0u);
}

int main() {
    std::cout << "Solución 07 (ordenado)...\n";
    return RUN_ALL_TESTS();
}
