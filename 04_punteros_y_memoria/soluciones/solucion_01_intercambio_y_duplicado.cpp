/* =============================================================================
   MÓDULO 04 - SOLUCIÓN 01: Direcciones, nullptr e intercambio
   =============================================================================
   POR QUÉ FUNCIONA:
     - Se comprueba nullptr antes de cada desreferenciación.
     - intercambiar usa una variable temporal para no pisar el primer valor.
     - mayor devuelve la dirección del objeto, no una copia del valor.
     - apuntanAlMismo compara las direcciones; dos nullptr son iguales.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_01_intercambio_y_duplicado.cpp -o sol01 && ./sol01
   ============================================================================= */
#include <iostream>
#include "../../include/test_framework.hpp"

void duplicar(int* p) {
    if (p != nullptr) *p *= 2;
}

void intercambiar(int* a, int* b) {
    if (a == nullptr || b == nullptr) return;
    int temporal = *a;
    *a = *b;
    *b = temporal;
}

int* mayor(int* a, int* b) {
    if (a == nullptr) return b;
    if (b == nullptr) return a;
    return (*a >= *b) ? a : b;
}

bool apuntanAlMismo(const int* a, const int* b) {
    return a == b;
}

int leerODefecto(const int* p, int defecto) {
    return (p != nullptr) ? *p : defecto;
}

// ============= TESTS =============
TEST_CASE(TestDuplicar) {
    int x = 10;
    duplicar(&x);
    ASSERT_EQ(x, 20);
    duplicar(nullptr);
}

TEST_CASE(TestIntercambiar) {
    int a = 1;
    int b = 2;
    intercambiar(&a, &b);
    ASSERT_EQ(a, 2);
    ASSERT_EQ(b, 1);
    intercambiar(nullptr, &a);
    ASSERT_EQ(a, 2);
}

TEST_CASE(TestMayorDevuelvePuntero) {
    int a = 3;
    int b = 9;
    int* p = mayor(&a, &b);
    ASSERT_TRUE(p == &b);
    ASSERT_EQ(*p, 9);
}

TEST_CASE(TestMayorConNulos) {
    int a = 4;
    ASSERT_TRUE(mayor(nullptr, &a) == &a);
    ASSERT_TRUE(mayor(&a, nullptr) == &a);
    ASSERT_TRUE(mayor(nullptr, nullptr) == nullptr);
}

TEST_CASE(TestApuntanAlMismo) {
    int a = 1;
    int b = 2;
    ASSERT_TRUE(apuntanAlMismo(&a, &a));
    ASSERT_FALSE(apuntanAlMismo(&a, &b));
    ASSERT_TRUE(apuntanAlMismo(nullptr, nullptr));
}

TEST_CASE(TestLeerODefecto) {
    int a = 7;
    ASSERT_EQ(leerODefecto(&a, -1), 7);
    ASSERT_EQ(leerODefecto(nullptr, -1), -1);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 01...\n";
    return RUN_ALL_TESTS();
}
