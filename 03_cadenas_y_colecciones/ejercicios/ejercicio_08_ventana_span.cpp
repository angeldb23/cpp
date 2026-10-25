/* =============================================================================
   MÓDULO 03 - EJERCICIO 08: Ventanas con std::span
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA:
     Usá std::span para procesar partes de un arreglo sin copiarlo:
     1. sumaSpan(span<const int>) -> int: suma todos los elementos
     2. mejorVentana(span<const int>, size_t k) -> int: mayor suma de k elementos consecutivos
     3. duplicarEnLugar(span<int>): duplica cada elemento y modifica el original
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_08_ventana_span.cpp -o ej08 && ./ej08
   ============================================================================= */
#include <iostream>
#include <vector>
#include <array>
#include <span>
#include <numeric>
#include <algorithm>
#include "../../include/test_framework.hpp"

// TODO 1: sumaSpan suma todos los elementos del span (0 si está vacío).
// TODO 2: mejorVentana devuelve la MAYOR suma de 'k' elementos consecutivos.
//         Si k es 0 o mayor que el tamaño de los datos, devolvé 0.
//         Pista: datos.subspan(i, k) te da cada ventana sin copiar nada.
// TODO 3: duplicarEnLugar multiplica por 2 cada elemento del span MODIFICABLE.
// TU CÓDIGO AQUÍ:

int sumaSpan(std::span<const int> datos) {
    (void)datos;
    return 0; // placeholder
}

int mejorVentana(std::span<const int> datos, std::size_t k) {
    (void)datos; (void)k;
    return 0; // placeholder
}

void duplicarEnLugar(std::span<int> datos) {
    (void)datos; // placeholder: no hace nada
}

// ============= TESTS =============
TEST_CASE(TestSumaSpanOrigenes) {
    std::vector<int> v = {1, 2, 3, 4};
    std::array<int, 3> a = {10, 20, 30};
    int c[] = {5, 5, 5, 5, 5};
    ASSERT_EQ(sumaSpan(v), 10);
    ASSERT_EQ(sumaSpan(a), 60);
    ASSERT_EQ(sumaSpan(c), 25);
}

TEST_CASE(TestSumaSpanVacio) {
    std::vector<int> v;
    ASSERT_EQ(sumaSpan(v), 0);
}

TEST_CASE(TestMejorVentana) {
    std::vector<int> v = {1, 5, 2, 8, 3};
    ASSERT_EQ(mejorVentana(v, 2), 11);   // 8 + 3
    ASSERT_EQ(mejorVentana(v, 3), 15);   // 5 + 2 + 8
    ASSERT_EQ(mejorVentana(v, 5), 19);   // todo el vector
}

TEST_CASE(TestMejorVentanaInvalida) {
    std::vector<int> v = {1, 2, 3};
    ASSERT_EQ(mejorVentana(v, 0), 0);
    ASSERT_EQ(mejorVentana(v, 4), 0);
}

TEST_CASE(TestDuplicarEnLugar) {
    std::vector<int> v = {1, 2, 3, 4};
    std::span<int> vista(v);
    duplicarEnLugar(vista.first(2));
    ASSERT_EQ(v[0], 2);
    ASSERT_EQ(v[1], 4);
    ASSERT_EQ(v[2], 3);   // fuera de la vista: no se toca
    ASSERT_EQ(v[3], 4);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 08 (Ventanas con std::span)...\n";
    return RUN_ALL_TESTS();
}
