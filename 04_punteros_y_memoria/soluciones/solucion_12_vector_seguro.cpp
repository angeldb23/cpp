/* =============================================================================
   MÓDULO 04 - SOLUCIÓN 12: Reemplazar el arreglo crudo por std::vector
   =============================================================================
   POR QUÉ FUNCIONA:
     - El constructor vector(first, last) copia el rango sin new/delete manual.
     - recortar valida primero y usa at() para cada acceso.
     - sinDuplicados recibe por valor: ordena y modifica su propia copia.
     - sort + unique + erase deja solo la primera aparición de cada valor.
     - contarEnRango usa count_if con una lambda que captura los límites.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_12_vector_seguro.cpp -o sol12 && ./sol12
   ============================================================================= */
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <vector>
#include "../../include/test_framework.hpp"

std::vector<int> copiarRango(const int* datos, std::size_t n) {
    if (datos == nullptr || n == 0) return {};
    return std::vector<int>(datos, datos + n);
}

int sumaSegura(const std::vector<int>& v) {
    int total = 0;
    for (int x : v) total += x;
    return total;
}

std::vector<int> recortar(const std::vector<int>& v, std::size_t desde,
                          std::size_t hasta) {
    if (desde > hasta || hasta > v.size()) {
        throw std::out_of_range("rango de recorte invalido");
    }
    std::vector<int> resultado;
    resultado.reserve(hasta - desde);
    for (std::size_t i = desde; i < hasta; ++i) {
        resultado.push_back(v.at(i));
    }
    return resultado;
}

std::vector<int> sinDuplicados(std::vector<int> v) {
    std::sort(v.begin(), v.end());
    v.erase(std::unique(v.begin(), v.end()), v.end());
    return v;
}

std::size_t contarEnRango(const std::vector<int>& v, int minimo, int maximo) {
    auto total = std::count_if(v.begin(), v.end(), [minimo, maximo](int x) {
        return x >= minimo && x <= maximo;
    });
    return static_cast<std::size_t>(total);
}

// ============= TESTS =============
TEST_CASE(TestCopiarRango) {
    int datos[] = {4, 8, 15};
    std::vector<int> v = copiarRango(datos, 3);
    ASSERT_EQ(v.size(), 3u);
    ASSERT_EQ(v[0], 4);
    ASSERT_EQ(v[1], 8);
    ASSERT_EQ(v[2], 15);

    ASSERT_TRUE(copiarRango(nullptr, 3).empty());
    ASSERT_TRUE(copiarRango(datos, 0).empty());
}

TEST_CASE(TestSumaSegura) {
    ASSERT_EQ(sumaSegura({1, 2, 3, 4}), 10);
    ASSERT_EQ(sumaSegura({}), 0);
}

TEST_CASE(TestRecortar) {
    std::vector<int> v = {0, 1, 2, 3, 4};
    std::vector<int> r = recortar(v, 1, 4);
    ASSERT_EQ(r.size(), 3u);
    ASSERT_EQ(r[0], 1);
    ASSERT_EQ(r[1], 2);
    ASSERT_EQ(r[2], 3);

    ASSERT_TRUE(recortar(v, 2, 2).empty());
    ASSERT_EQ(v.size(), 5u);
}

TEST_CASE(TestRecortarFueraDeRango) {
    std::vector<int> v = {0, 1, 2};
    bool lanzo = false;
    try {
        recortar(v, 1, 9);
    } catch (const std::out_of_range&) {
        lanzo = true;
    }
    ASSERT_TRUE(lanzo);

    lanzo = false;
    try {
        recortar(v, 2, 1);
    } catch (const std::out_of_range&) {
        lanzo = true;
    }
    ASSERT_TRUE(lanzo);
}

TEST_CASE(TestSinDuplicados) {
    std::vector<int> original = {3, 1, 3, 2, 1};
    std::vector<int> r = sinDuplicados(original);
    ASSERT_EQ(r.size(), 3u);
    ASSERT_EQ(r[0], 1);
    ASSERT_EQ(r[1], 2);
    ASSERT_EQ(r[2], 3);
    ASSERT_EQ(original.size(), 5u);
}

TEST_CASE(TestContarEnRango) {
    std::vector<int> v = {-2, 0, 3, 5, 9};
    ASSERT_EQ(contarEnRango(v, 0, 5), 3u);
    ASSERT_EQ(contarEnRango(v, 10, 20), 0u);
    ASSERT_EQ(contarEnRango({}, 0, 10), 0u);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 12...\n";
    return RUN_ALL_TESTS();
}
