/* =============================================================================
   MÓDULO 04 - EJERCICIO 02: Puntero a puntero y salida por parámetro
   =============================================================================
   DIFICULTAD: Inicial/Intermedio
   PRACTICA: lección 04 (puntero a puntero, paso por puntero, * vs **).

   CONSIGNA:
     1. apuntarAlMayor(int** salida, int* a, int* b) -> bool:
        - Si salida es nullptr, devolvé false.
        - Si a es nullptr, *salida = b; si b es nullptr, *salida = a.
        - Si ambos son válidos, *salida apunta al mayor.
        - Devolvé true si pudo escribir la salida.
     2. intercambiarPunteros(int** a, int** b): intercambiá los punteros del
        llamador (no los valores apuntados). Si alguno es nullptr, no hagas nada.
     3. sumarApuntados(int* const* datos, int n): sumá los valores apuntados,
        ignorando los punteros nulos. Si datos es nullptr, devolvé 0.
     4. contarNoNulos(int* const* datos, int n): contá cuántos punteros no son
        nulos.
     5. El ejercicio incluye un arreglo de punteros para probar el recorrido.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra ejercicio_02_puntero_a_puntero.cpp \
         -o ej02 && ./ej02
   ============================================================================= */
#include <iostream>
#include "../../include/test_framework.hpp"

// TODO: implementá las cuatro funciones.
bool apuntarAlMayor(int** salida, int* a, int* b) {
    (void)salida; (void)a; (void)b;
    return false;  // placeholder
}

void intercambiarPunteros(int** a, int** b) {
    (void)a; (void)b;  // placeholder
}

int sumarApuntados(int* const* datos, int n) {
    (void)datos; (void)n;
    return 0;  // placeholder
}

int contarNoNulos(int* const* datos, int n) {
    (void)datos; (void)n;
    return 0;  // placeholder
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
    ASSERT_EQ(a, 1);  // los valores no se tocaron
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
    std::cout << "Ejecutando pruebas para Ejercicio 02...\n";
    return RUN_ALL_TESTS();
}
