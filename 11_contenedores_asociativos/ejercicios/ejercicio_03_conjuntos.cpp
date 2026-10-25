/* =============================================================================
   MÓDULO 11 - EJERCICIO 03: conjuntos y tabulados (lecciones 03 y 04)
   =============================================================================
   DIFICULTAD: Medio
   CONSIGNA (tema 2 de TEMAS.md: set/unordered_set, multiset y orden):
     Un league registra cartas y daños (dos mundos: set y multiset):
       1. sinRepeticiones(entrada): dado un std::vector<int> de la
          entrada, devuelve el std::set<int> de los IDs distintos.
          (el set deduplica solo: basta insert (L03).
       2. enAmbos(a, b): la CANTIDAD de IDs que están en ambos sets.
          (recorrido del set pequeño + contains en el otro (L03:
          intersección sin estructuras raras).)
       3. losMasFuertes(danios, k): el std::vector<int> con los k golpes
          MÁS FUERTES de una lista de golpes (SIN sort: multiset con
          std::greater para tener los "mayores primero", también de
          la L04). Si k > cantidad de golpes, devolver TODOS.
       4. contarYdescartar(claves): devuelve la DISTRIBUCION:
          un std::map<int, int> clave -> CUENTA (multiset::count o
          mapa auxiliar valen: la L04 muestra los dos caminos).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_03_conjuntos.cpp -o ej03 && ./ej03
   ============================================================================= */
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include "../../include/test_framework.hpp"

// TODO 1: set con los elementos distintos de la entrada (sin duplicados).
std::set<int> sinRepeticiones([[maybe_unused]] const std::vector<int>& entrada) {
    return {};    // placeholder
}

// TODO 2: cantidad de ids presentes en ambos sets.
int enAmbos([[maybe_unused]] const std::set<int>& a,
            [[maybe_unused]] const std::set<int>& b) {
    return 0;    // placeholder
}

// TODO 3: los k golpes mas fuertes (sin sort; multiset + greater).
std::vector<int> losMasFuertes([[maybe_unused]] const std::vector<int>& danios,
                               [[maybe_unused]] int k) {
    return {};    // placeholder
}

// TODO 4: distribucion clave -> cuantas veces aparece cada clave (multiset).
std::map<int, int> contarYdescartar(
    [[maybe_unused]] const std::vector<int>& claves) {
    return {};    // placeholder
}

// ============= TESTS =============
TEST_CASE(sinRepeticionesDejaUnaFila) {
    const std::set<int> unicos = sinRepeticiones({3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5});
    ASSERT_EQ(unicos.size(), 7u);
    ASSERT_TRUE(unicos.contains(9));
    ASSERT_EQ(*unicos.begin(), 1);    // el recorrido: ordenado
}

TEST_CASE(enAmboSoloCuentaComunes) {
    const std::set<int> a{1, 2, 3, 4};
    const std::set<int> b{3, 4, 5};
    ASSERT_EQ(enAmbos(a, b), 2);
    const std::set<int> vacio;
    ASSERT_EQ(enAmbos(a, vacio), 0);
}

TEST_CASE(losMasFuertesSinSort) {
    const std::vector<int> danios{12, 8, 6, 22, 15, 3, 22};
    const std::vector<int> top3 = losMasFuertes(danios, 3);
    ASSERT_TRUE(top3.size() == 3);
    ASSERT_EQ(top3[0], 22);          // pueden repetirse: es multiset
    ASSERT_EQ(top3[1], 22);
    ASSERT_EQ(top3[2], 15);
    const std::vector<int> todos = losMasFuertes(danios, 100);
    ASSERT_EQ(todos.size(), danios.size());    // k grande = todos
}

TEST_CASE(distribucionPorClave) {
    const std::map<int, int> dist = contarYdescartar({1, 2, 2, 3, 3, 3});
    ASSERT_EQ(dist.size(), 3u);
    ASSERT_EQ(dist.at(2), 2);
    ASSERT_EQ(dist.at(3), 3);
    ASSERT_EQ(dist.count(9), 0u);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 03 (conjuntos y tabulados)...\n";
    return RUN_ALL_TESTS();
}
