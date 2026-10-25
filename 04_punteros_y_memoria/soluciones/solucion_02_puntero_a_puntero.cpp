/* =============================================================================
   MÓDULO 04 - SOLUCIÓN 02: Puntero a puntero y salida por parámetro
   =============================================================================
   POR QUÉ FUNCIONA:
     - apuntarAlMayor escribe en *salida, que es el puntero del llamador.
     - Se devuelve false solo cuando salida es nullptr: no hay dónde escribir.
     - intercambiarPunteros mueve direcciones; los valores apuntados no cambian.
     - sumarApuntados recorre un arreglo de punteros y salta los nulos.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_02_puntero_a_puntero.cpp -o sol02 && ./sol02
   ============================================================================= */
#include <iostream>
#include "../../include/test_framework.hpp"

bool apuntarAlMayor(int** salida, int* a, int* b) {
    if (salida == nullptr) return false;
    if (a == nullptr) {
        *salida = b;
        return true;
    }
    if (b == nullptr) {
        *salida = a;
        return true;
    }
    *salida = (*a >= *b) ? a : b;
    return true;
}

void intercambiarPunteros(int** a, int** b) {
    if (a == nullptr || b == nullptr) return;
    int* temporal = *a;
    *a = *b;
    *b = temporal;
}

int sumarApuntados(int* const* datos, int n) {
    if (datos == nullptr) return 0;
    int total = 0;
    for (int i = 0; i < n; ++i) {
        if (datos[i] != nullptr) total += *datos[i];
    }
    return total;
}

int contarNoNulos(int* const* datos, int n) {
    if (datos == nullptr) return 0;
    int total = 0;
    for (int i = 0; i < n; ++i) {
        if (datos[i] != nullptr) ++total;
    }
    return total;
}

// ============= TESTS =============
TEST_CASE(TestApuntarAlMayor) {
    int a = 4;
    int b = 9;
    int* elegido = nullptr;
    ASSERT_TRUE(apuntarAlMayor(&elegido, &a, &b));
    ASSERT_TRUE(elegido == &b);
    ASSERT_EQ(*elegido, 9);
}

TEST_CASE(TestApuntarAlMayorConNulos) {
    int a = 4;
    int* elegido = nullptr;

    ASSERT_FALSE(apuntarAlMayor(nullptr, &a, &a));

    ASSERT_TRUE(apuntarAlMayor(&elegido, nullptr, &a));
    ASSERT_TRUE(elegido == &a);

    elegido = nullptr;
    ASSERT_TRUE(apuntarAlMayor(&elegido, &a, nullptr));
    ASSERT_TRUE(elegido == &a);
}

TEST_CASE(TestIntercambiarPunteros) {
    int a = 1;
    int b = 2;
    int* pa = &a;
    int* pb = &b;

    intercambiarPunteros(&pa, &pb);
    ASSERT_TRUE(pa == &b);
    ASSERT_TRUE(pb == &a);
    ASSERT_EQ(*pa, 2);
    ASSERT_EQ(*pb, 1);
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, 2);
}

TEST_CASE(TestSumarApuntados) {
    int a = 5;
    int b = 7;
    int c = 11;
    int* datos[] = {&a, nullptr, &b, &c};

    ASSERT_EQ(sumarApuntados(datos, 4), 23);
    ASSERT_EQ(sumarApuntados(nullptr, 4), 0);
}

TEST_CASE(TestContarNoNulos) {
    int a = 5;
    int* datos[] = {&a, nullptr, &a, nullptr};
    ASSERT_EQ(contarNoNulos(datos, 4), 2);
    ASSERT_EQ(contarNoNulos(nullptr, 4), 0);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 02...\n";
    return RUN_ALL_TESTS();
}
