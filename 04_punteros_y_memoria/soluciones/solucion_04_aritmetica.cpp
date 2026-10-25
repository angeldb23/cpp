/* =============================================================================
   MÓDULO 04 - SOLUCIÓN 04: Aritmética de punteros, end y diferencia
   =============================================================================
   POR QUÉ FUNCIONA:
     - La resta de punteros del mismo arreglo da el número de ELEMENTOS.
     - Los mínimos/máximos comparan valores y guardan el puntero ganador.
     - Un rango vacío devuelve end, que nunca se desreferencia.
     - dentroDelRango usa <= end porque end es una posición válida del rango.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_04_aritmetica.cpp -o sol04 && ./sol04
   ============================================================================= */
#include <cstddef>
#include <iostream>
#include "../../include/test_framework.hpp"

std::ptrdiff_t indiceDe(const int* base, const int* p) {
    return p - base;
}

const int* minimoRango(const int* begin, const int* end) {
    if (begin == end) return end;
    const int* mejor = begin;
    for (const int* it = begin + 1; it != end; ++it) {
        if (*it < *mejor) mejor = it;
    }
    return mejor;
}

const int* maximoRango(const int* begin, const int* end) {
    if (begin == end) return end;
    const int* mejor = begin;
    for (const int* it = begin + 1; it != end; ++it) {
        if (*it > *mejor) mejor = it;
    }
    return mejor;
}

bool dentroDelRango(const int* begin, const int* end, const int* p) {
    return p >= begin && p <= end;
}

std::ptrdiff_t distancia(const int* begin, const int* end) {
    return end - begin;
}

// ============= TESTS =============
TEST_CASE(TestIndiceDe) {
    int datos[] = {10, 20, 30, 40};
    ASSERT_EQ(indiceDe(datos, datos), 0);
    ASSERT_EQ(indiceDe(datos, datos + 3), 3);
}

TEST_CASE(TestMinimoRango) {
    int datos[] = {8, 3, 9, 1, 7};
    ASSERT_TRUE(minimoRango(datos, datos + 5) == datos + 3);
    ASSERT_EQ(*minimoRango(datos, datos + 5), 1);

    ASSERT_TRUE(minimoRango(datos, datos) == datos);
}

TEST_CASE(TestMaximoRango) {
    int datos[] = {8, 3, 9, 1, 7};
    ASSERT_TRUE(maximoRango(datos, datos + 5) == datos + 2);
    ASSERT_EQ(*maximoRango(datos, datos + 5), 9);

    ASSERT_TRUE(maximoRango(datos, datos) == datos);
}

TEST_CASE(TestDentroDelRango) {
    int datos[] = {1, 2, 3};
    ASSERT_TRUE(dentroDelRango(datos, datos + 3, datos));
    ASSERT_TRUE(dentroDelRango(datos, datos + 3, datos + 2));
    ASSERT_TRUE(dentroDelRango(datos, datos + 3, datos + 3));
    ASSERT_FALSE(dentroDelRango(datos + 1, datos + 3, datos));
    ASSERT_FALSE(dentroDelRango(datos, datos + 3, datos + 4));
}

TEST_CASE(TestDistancia) {
    int datos[] = {1, 2, 3, 4, 5};
    ASSERT_EQ(distancia(datos, datos + 5), 5);
    ASSERT_EQ(distancia(datos + 1, datos + 4), 3);
    ASSERT_EQ(distancia(datos, datos), 0);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 04...\n";
    return RUN_ALL_TESTS();
}
