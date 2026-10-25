/* =============================================================================
   MÓDULO 03 - SOLUCIÓN 08: Ventanas con std::span
   =============================================================================
   POR QUÉ FUNCIONA:
     - span<const int> acepta vector, std::array y array de C sin copiar.
     - first(k) y subspan(i, k) devuelven vistas: costo O(1), no reservan memoria.
     - span<int> sin const deja modificar los datos originales; con const no compila.
     - La condición i + k <= size() evita pasarse del final (cuidado con size_t y restas).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_08_ventana_span.cpp -o sol08 && ./sol08
   ============================================================================= */
#include <iostream>
#include <vector>
#include <array>
#include <span>
#include <numeric>
#include <algorithm>
#include "../../include/test_framework.hpp"

int sumaSpan(std::span<const int> datos) {
    return std::accumulate(datos.begin(), datos.end(), 0);
}

// Cada ventana es un subspan: no se copia ningún dato, solo se mueve la "vista".
int mejorVentana(std::span<const int> datos, std::size_t k) {
    if (k == 0 || k > datos.size()) return 0;
    int mejor = sumaSpan(datos.first(k));
    for (std::size_t i = 1; i + k <= datos.size(); ++i) {
        mejor = std::max(mejor, sumaSpan(datos.subspan(i, k)));
    }
    return mejor;
}

// span<int> (sin const) permite escribir: modificamos el vector original.
void duplicarEnLugar(std::span<int> datos) {
    for (int& x : datos) x *= 2;
}

// ============= TESTS (los mismos del ejercicio) =============
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
    std::cout << "Ejecutando pruebas para Solución 08 (Ventanas con std::span)...\n";
    return RUN_ALL_TESTS();
}
