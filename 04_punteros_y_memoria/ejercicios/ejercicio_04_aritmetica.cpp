/* =============================================================================
   MÓDULO 04 - EJERCICIO 04: Aritmética de punteros, end y diferencia
   =============================================================================
   DIFICULTAD: Intermedio
   PRACTICA: lección 06 (p + i, end, ptrdiff_t, comparación de punteros).

   CONSIGNA:
     1. indiceDe(const int* base, const int* p): devolvé p - base como
        std::ptrdiff_t. Se supone que p pertenece al mismo arreglo que base.
     2. minimoRango(const int* begin, const int* end): puntero al menor; si el
        rango está vacío, devolvé end.
     3. maximoRango(const int* begin, const int* end): puntero al mayor; si el
        rango está vacío, devolvé end.
     4. dentroDelRango(const int* begin, const int* end, const int* p): true
        si p está entre begin y end, ambos inclusive. El end es válido como
        posición (aunque no se desreferencie).
     5. distancia(const int* begin, const int* end): cantidad de elementos,
        usando la resta de punteros (no un contador manual).

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra ejercicio_04_aritmetica.cpp -o ej04 && ./ej04
   ============================================================================= */
#include <cstddef>
#include <iostream>
#include "../../include/test_framework.hpp"

// TODO: implementá las cinco funciones.
std::ptrdiff_t indiceDe(const int* base, const int* p) {
    (void)base; (void)p;
    return -1;  // placeholder
}

const int* minimoRango(const int* begin, const int* end) {
    (void)begin; (void)end;
    return nullptr;  // placeholder
}

const int* maximoRango(const int* begin, const int* end) {
    (void)begin; (void)end;
    return nullptr;  // placeholder
}

bool dentroDelRango(const int* begin, const int* end, const int* p) {
    (void)begin; (void)end; (void)p;
    return false;  // placeholder
}

std::ptrdiff_t distancia(const int* begin, const int* end) {
    (void)begin; (void)end;
    return 0;  // placeholder
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
    ASSERT_TRUE(dentroDelRango(datos, datos + 3, datos + 3));  // end
    ASSERT_FALSE(dentroDelRango(datos + 1, datos + 3, datos));  // antes de begin
    ASSERT_FALSE(dentroDelRango(datos, datos + 3, datos + 4));
}

TEST_CASE(TestDistancia) {
    int datos[] = {1, 2, 3, 4, 5};
    ASSERT_EQ(distancia(datos, datos + 5), 5);
    ASSERT_EQ(distancia(datos + 1, datos + 4), 3);
    ASSERT_EQ(distancia(datos, datos), 0);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 04...\n";
    return RUN_ALL_TESTS();
}
