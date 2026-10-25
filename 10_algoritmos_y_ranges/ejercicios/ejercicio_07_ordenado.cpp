/* =============================================================================
   MÓDULO 10 - EJERCICIO 07: búsqueda sobre ordenado (lección 09)
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA (tema 5 de TEMAS.md: ordenamiento y búsqueda): la tabla ordenada.
     `puntajes` SIEMPRE llega ordenado ASC (es TU invariante: los tests
     parten de vectores ordenados y pedidos insertados mantienen el orden).
       1. rango(v, valor): devuelve el PAR (lower_bound, upper_bound) en
          UNA llamada (equal_range) con las DISTANCIAS al begin para que
          el test no dependa del tipo del iterador: el tramo de los
          iguales (tras primeros y fin: [primer, fin)).
       2. existe(v, valor): bool con binary_search.
       3. insertarManteniendoOrden(v, nuevo): ubica con lower_bound e
          inserta; result: el vector sigue ASC y el `nuevo` quedó en su
          lugar (L09: la jugada de inserción ordenada).
       4. ranking(v, miPuntaje): la CANTIDAD de puntajes exactamente
          iguales a `miPuntaje` (es asumible que el vector puede traer
          repetidos: por eso lower + upper dan el grueso del tramo).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_07_ordenado.cpp -o ej07 && ./ej07
   ============================================================================= */
#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>
#include "../../include/test_framework.hpp"

// tipo del par que devuelve rango (en índices desde el begin):
using Tramo = std::pair<std::size_t, std::size_t>;

// TODO 1: rango: equal_range y convertir a índices std::size_t (distance).
// TU CODIGO AQUI:
Tramo rango([[maybe_unused]] const std::vector<int>& v,
            [[maybe_unused]] int valor) {
    return {0, 0};    // placeholder
}

// TODO 2: existe: binary_search.
// TU CODIGO AQUI:
bool existe([[maybe_unused]] const std::vector<int>& v,
            [[maybe_unused]] int valor) {
    return false;    // placeholder
}

// TODO 3: insertarManteniendoOrden: lower_bound + insert.
// TU CODIGO AQUI:
void insertarManteniendoOrden([[maybe_unused]] std::vector<int>& v,
                              [[maybe_unused]] int nuevo) {
}

// TODO 4: ranking: cuántos elementos son EXACTOS al puntaje dado.
// TU CODIGO AQUI:
std::size_t ranking([[maybe_unused]] const std::vector<int>& v,
                    [[maybe_unused]] int miPuntaje) {
    return 0;    // placeholder
}

// ============= TESTS =============
TEST_CASE(RangoDelimitaElTramoDeIguales) {
    const std::vector<int> v{10, 20, 30, 30, 30, 40};
    const auto [pris, finsi] = rango(v, 30);
    ASSERT_EQ(pris, 2u);
    ASSERT_EQ(finsi, 5u);       // los tres 30: [2, 5)
    const auto [a, b] = rango(v, 25);
    ASSERT_EQ(a, 2u);           // no hay iguales: v.lower = donde iría
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
    insertarManteniendoOrden(v, 45); // repetido: lower_bound va AL PRIMERO
    ASSERT_TRUE((v == std::vector<int>{5, 12, 45, 45, 50, 78, 90}));
}

TEST_CASE(RankingConRepetidos) {
    const std::vector<int> v{10, 20, 30, 30, 30, 40};
    ASSERT_EQ(ranking(v, 30), 3u);
    ASSERT_EQ(ranking(v, 10), 1u);
    ASSERT_EQ(ranking(v, 99), 0u);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 07 (ordenado)...\n";
    return RUN_ALL_TESTS();
}
