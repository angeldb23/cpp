/* =============================================================================
   MÓDULO 04 - EJERCICIO 03: Recorrer arreglos con punteros
   =============================================================================
   DIFICULTAD: Inicial
   PRACTICA: lecciones 05 y 06 (decaimiento, arrays, begin/end, aritmética).

   CONSIGNA:
     Trabajá siempre con el rango semiabierto [begin, end): begin incluido,
     end excluido. Nunca desreferencies end.
     1. sumarRango(const int* begin, const int* end): suma los elementos.
        Si begin == end, devolvé 0.
     2. buscarValor(const int* begin, const int* end, int valor): devolvé el
        puntero al primer elemento igual a valor, o end si no está.
     3. invertirRango(int* begin, int* end): invertí los elementos en el lugar.
        Si el rango está vacío, no hagas nada.
     4. contarPares(const int* begin, const int* end): contá los elementos
        pares.
     5. ultimoValor(const int* begin, const int* end, int valor): devolvé el
        puntero al ÚLTIMO elemento igual a valor, o end si no está.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra ejercicio_03_recorrer_arreglos.cpp \
         -o ej03 && ./ej03
   ============================================================================= */
#include <cstddef>
#include <iostream>
#include "../../include/test_framework.hpp"

// TODO: implementá las cinco funciones.
int sumarRango(const int* begin, const int* end) {
    (void)begin; (void)end;
    return 0;  // placeholder
}

const int* buscarValor(const int* begin, const int* end, int valor) {
    (void)begin; (void)end; (void)valor;
    return nullptr;  // placeholder
}

void invertirRango(int* begin, int* end) {
    (void)begin; (void)end;  // placeholder
}

std::size_t contarPares(const int* begin, const int* end) {
    (void)begin; (void)end;
    return 0;  // placeholder
}

const int* ultimoValor(const int* begin, const int* end, int valor) {
    (void)begin; (void)end; (void)valor;
    return nullptr;  // placeholder
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
    invertirRango(datos, datos);  // no debe tocar nada
    ASSERT_EQ(datos[0], 1);
    ASSERT_EQ(datos[2], 3);
}

TEST_CASE(TestContarPares) {
    int datos[] = {1, 2, 4, 7, 8};
    ASSERT_EQ(contarPares(datos, datos + 5), 3);
    ASSERT_EQ(contarPares(datos, datos), 0);
}

TEST_CASE(TestUltimoValor) {
    int datos[] = {10, 20, 30, 20};
    ASSERT_TRUE(ultimoValor(datos, datos + 4, 20) == datos + 3);
    ASSERT_TRUE(ultimoValor(datos, datos + 4, 99) == datos + 4);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 03...\n";
    return RUN_ALL_TESTS();
}
