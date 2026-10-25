/* =============================================================================
   MÓDULO 04 - SOLUCIÓN 10: Referencias vs punteros en APIs
   =============================================================================
   POR QUÉ FUNCIONA:
     - std::vector<int>& permite modificar el vector del llamador sin copiarlo.
     - intercambiarRef trabaja sobre los objetos, no sobre copias.
     - masLargo devuelve una referencia a uno de los argumentos: ambos viven en
       el llamador, así que la referencia sigue válida.
     - elementoEn usa at(), que valida el índice y lanza std::out_of_range.
     - duplicarPorPuntero comprueba nullptr antes de escribir.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_10_referencias.cpp -o sol10 && ./sol10
   ============================================================================= */
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

void escalarVector(std::vector<int>& v, int factor) {
    for (int& x : v) x *= factor;
}

void intercambiarRef(int& a, int& b) {
    int temporal = a;
    a = b;
    b = temporal;
}

const std::string& masLargo(const std::string& a, const std::string& b) {
    return (a.size() >= b.size()) ? a : b;
}

int& elementoEn(std::vector<int>& v, std::size_t i) {
    return v.at(i);
}

void duplicarPorPuntero(int* p) {
    if (p != nullptr) *p *= 2;
}

// ============= TESTS =============
TEST_CASE(TestEscalarVector) {
    std::vector<int> v = {1, 2, 3};
    escalarVector(v, 3);
    ASSERT_EQ(v[0], 3);
    ASSERT_EQ(v[1], 6);
    ASSERT_EQ(v[2], 9);
}

TEST_CASE(TestIntercambiarRef) {
    int a = 1;
    int b = 2;
    intercambiarRef(a, b);
    ASSERT_EQ(a, 2);
    ASSERT_EQ(b, 1);
}

TEST_CASE(TestMasLargo) {
    std::string corto = "corto";
    std::string largo = "mucho mas largo";
    const std::string& r = masLargo(corto, largo);
    ASSERT_TRUE(&r == &largo);
    ASSERT_EQ(r, largo);
}

TEST_CASE(TestMasLargoEmpate) {
    std::string a = "abc";
    std::string b = "xyz";
    ASSERT_TRUE(&masLargo(a, b) == &a);
}

TEST_CASE(TestElementoEn) {
    std::vector<int> v = {10, 20, 30};
    ASSERT_EQ(elementoEn(v, 1), 20);

    elementoEn(v, 1) = 42;
    ASSERT_EQ(v[1], 42);
}

TEST_CASE(TestElementoFueraDeRango) {
    std::vector<int> v = {1, 2, 3};
    bool lanzo = false;
    try {
        elementoEn(v, 10);
    } catch (const std::out_of_range&) {
        lanzo = true;
    }
    ASSERT_TRUE(lanzo);
}

TEST_CASE(TestDuplicarPorPuntero) {
    int x = 7;
    duplicarPorPuntero(&x);
    ASSERT_EQ(x, 14);
    duplicarPorPuntero(nullptr);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 10...\n";
    return RUN_ALL_TESTS();
}
