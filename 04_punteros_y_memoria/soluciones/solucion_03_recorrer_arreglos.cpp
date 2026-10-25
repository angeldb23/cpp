/* =============================================================================
   MÓDULO 04 - SOLUCIÓN 03: Recorrer arreglos con punteros
   =============================================================================
   POR QUÉ FUNCIONA:
     - Todos los bucles usan [begin, end): comparan con != end y nunca
       desreferencian end.
     - buscarValor/ultimoValor devuelven end como "no encontrado".
     - invertirRango mueve end hacia atrás antes de leer e intercambia con
       una variable temporal.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_03_recorrer_arreglos.cpp -o sol03 && ./sol03
   ============================================================================= */
#include <cstddef>
#include <iostream>
#include "../../include/test_framework.hpp"

int sumarRango(const int* begin, const int* end) {
    int total = 0;
    for (const int* it = begin; it != end; ++it) total += *it;
    return total;
}

const int* buscarValor(const int* begin, const int* end, int valor) {
    for (const int* it = begin; it != end; ++it) {
        if (*it == valor) return it;
    }
    return end;
}

void invertirRango(int* begin, int* end) {
    if (begin == end) return;
    --end;  // ahora apunta al último elemento real
    while (begin < end) {
        int temporal = *begin;
        *begin = *end;
        *end = temporal;
        ++begin;
        --end;
    }
}

std::size_t contarPares(const int* begin, const int* end) {
    std::size_t total = 0;
    for (const int* it = begin; it != end; ++it) {
        if (*it % 2 == 0) ++total;
    }
    return total;
}

const int* ultimoValor(const int* begin, const int* end, int valor) {
    const int* encontrado = end;
    for (const int* it = begin; it != end; ++it) {
        if (*it == valor) encontrado = it;
    }
    return encontrado;
}

// ============= TESTS =============
TEST_CASE(TestSumarRango) {
    int datos[] = {1, 2, 3, 4, 5};
    ASSERT_EQ(sumarRango(datos, datos + 5), 15);
    ASSERT_EQ(sumarRango(datos + 1, datos + 3), 5);
    ASSERT_EQ(sumarRango(datos, datos), 0);
}

TEST_CASE(TestBuscarValor) {
    int datos[] = {10, 20, 30, 20};
    ASSERT_TRUE(buscarValor(datos, datos + 4, 20) == datos + 1);
    ASSERT_TRUE(buscarValor(datos, datos + 4, 99) == datos + 4);
}

TEST_CASE(TestInvertirRango) {
    int datos[] = {1, 2, 3, 4, 5};
    invertirRango(datos, datos + 5);
    ASSERT_EQ(datos[0], 5);
    ASSERT_EQ(datos[1], 4);
    ASSERT_EQ(datos[2], 3);
    ASSERT_EQ(datos[3], 2);
    ASSERT_EQ(datos[4], 1);
}

TEST_CASE(TestInvertirRangoVacio) {
    int datos[] = {1, 2, 3};
    invertirRango(datos, datos);
    ASSERT_EQ(datos[0], 1);
    ASSERT_EQ(datos[2], 3);
}

TEST_CASE(TestContarPares) {
    int datos[] = {1, 2, 4, 7, 8};
    ASSERT_EQ(contarPares(datos, datos + 5), 3u);
    ASSERT_EQ(contarPares(datos, datos), 0u);
}

TEST_CASE(TestUltimoValor) {
    int datos[] = {10, 20, 30, 20};
    ASSERT_TRUE(ultimoValor(datos, datos + 4, 20) == datos + 3);
    ASSERT_TRUE(ultimoValor(datos, datos + 4, 99) == datos + 4);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 03...\n";
    return RUN_ALL_TESTS();
}
