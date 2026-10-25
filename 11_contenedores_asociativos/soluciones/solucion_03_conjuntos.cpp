/* =============================================================================
   MÓDULO 11 - SOLUCIÓN 03: conjuntos y tabulados (ejercicio 03)
   =============================================================================
   L03-L04 son las lecciones de respaldo: set deduplica solo; contains
   acelera la intersección; multiset con greater ordena "mayor primero"
   sin sort; y el multiset::count (o map auxiliar) da la distribución.
   ============================================================================= */
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <vector>
#include "../../include/test_framework.hpp"

std::set<int> sinRepeticiones(const std::vector<int>& entrada) {
    return {entrada.begin(), entrada.end()};   // el set dedup + ordena solo
}

int enAmbos(const std::set<int>& a, const std::set<int>& b) {
    int iguales = 0;
    for (const int id : a) {
        if (b.contains(id)) ++iguales;    // recorrer el MENOR y preguntar
    }
    return iguales;
}

std::vector<int> losMasFuertes(const std::vector<int>& danios, int k) {
    std::multiset<int, std::greater<int>> montane;
    for (const int d : danios) montane.insert(d);

    std::vector<int> golpe_final;
    int quiere = k;
    for (const int d : montane) {
        golpe_final.push_back(d);
        if (--quiere == 0) break;
    }
    return golpe_final;
}

std::map<int, int> contarYdescartar(const std::vector<int>& claves) {
    std::map<int, int> cuenta;
    // La forma map (L01: operator[] como contador) sin multiset:
    for (const int c : claves) ++cuenta[c];
    return cuenta;
}

// ============= TESTS =============
TEST_CASE(sinRepeticionesDejaUnaFila) {
    const std::set<int> unicos = sinRepeticiones({3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5});
    ASSERT_EQ(unicos.size(), 7u);
    ASSERT_TRUE(unicos.contains(9));
    ASSERT_EQ(*unicos.begin(), 1);
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
    ASSERT_EQ(top3[0], 22);
    ASSERT_EQ(top3[1], 22);
    ASSERT_EQ(top3[2], 15);
    const std::vector<int> todos = losMasFuertes(danios, 100);
    ASSERT_EQ(todos.size(), danios.size());
}

TEST_CASE(distribucionPorClave) {
    const std::map<int, int> dist = contarYdescartar({1, 2, 2, 3, 3, 3});
    ASSERT_EQ(dist.size(), 3u);
    ASSERT_EQ(dist.at(2), 2);
    ASSERT_EQ(dist.at(3), 3);
    ASSERT_EQ(dist.count(9), 0u);
}

int main() {
    std::cout << "Ejecutando pruebas para Solucion 03 (conjuntos y tabulados)...\n";
    return RUN_ALL_TESTS();
}
