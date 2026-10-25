/* =============================================================================
   MÓDULO 04 - EJERCICIO 01: Direcciones, nullptr e intercambio
   =============================================================================
   DIFICULTAD: Inicial
   PRACTICA: lecciones 01 y 02 (direcciones, &, *, nullptr, desreferenciación
             segura).

   CONSIGNA:
     1. duplicar(int* p): si p no es nullptr, multiplicá por 2 el valor
        apuntado. Si es nullptr, no hagas nada.
     2. intercambiar(int* a, int* b): si ambos son válidos, intercambiá los
        valores apuntados. Si alguno es nullptr, no toques nada.
     3. mayor(int* a, int* b): devolvé el PUNTERO al mayor de los dos valores.
        Si alguno es nullptr, devolvé el otro; si ambos lo son, nullptr.
     4. apuntanAlMismo(const int* a, const int* b): true si ambos son nullptr
        o si guardan la misma dirección.
     5. leerODefecto(const int* p, int defecto): devolvé *p si p no es nullptr;
        si es nullptr, devolvé defecto.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra ejercicio_01_intercambio_y_duplicado.cpp \
         -o ej01 && ./ej01
   ============================================================================= */
#include <iostream>
#include "../../include/test_framework.hpp"

// TODO: implementá las cinco funciones.
void duplicar(int* p) {
    (void)p;  // placeholder
}

void intercambiar(int* a, int* b) {
    (void)a; (void)b;  // placeholder
}

int* mayor(int* a, int* b) {
    (void)a; (void)b;
    return nullptr;  // placeholder
}

bool apuntanAlMismo(const int* a, const int* b) {
    (void)a; (void)b;
    return false;  // placeholder
}

int leerODefecto(const int* p, int defecto) {
    (void)p; (void)defecto;
    return 0;  // placeholder
}

// ============= TESTS =============
TEST_CASE(TestDuplicar) {
    int x = 10;
    duplicar(&x);
    ASSERT_EQ(x, 20);
    duplicar(nullptr);  // no debe romper
}

TEST_CASE(TestIntercambiar) {
    int a = 1;
    int b = 2;
    intercambiar(&a, &b);
    ASSERT_EQ(a, 2);
    ASSERT_EQ(b, 1);
    intercambiar(nullptr, &a);  // no debe romper
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
    std::cout << "Ejecutando pruebas para Ejercicio 01...\n";
    return RUN_ALL_TESTS();
}
