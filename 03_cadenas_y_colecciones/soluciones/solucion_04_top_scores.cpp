/* =============================================================================
   MÓDULO 03 - SOLUCIÓN 04: Tabla de Puntuaciones (array fijo)
   =============================================================================
   POR QUÉ FUNCIONA:
     - std::array tiene tamaño fijo: insertar es 'pisar el último y reordenar', no crecer.
     - El bucle usa size_t: por eso la condición es i > 0 (con size_t no existe -1).
     - std::swap intercambia dos elementos sin escribir la variable temporal a mano.
     - topScore usa max_element, así funciona aunque la tabla no estuviera ordenada.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_04_top_scores.cpp -o sol04 && ./sol04
   ============================================================================= */
#include <iostream>
#include <array>
#include <algorithm>
#include <utility>
#include "../../include/test_framework.hpp"

// La tabla siempre está ordenada de mayor a menor. Si la nueva puntuación no
// supera a la peor (la última), se descarta. Si la supera, ocupa el último
// lugar y "burbujea" hacia adelante mientras sea mayor que su vecina.
void insertarPuntuacion(std::array<int, 5>& tabla, int nueva) {
    if (nueva <= tabla.back()) return;
    tabla.back() = nueva;
    for (std::size_t i = tabla.size() - 1; i > 0 && tabla[i] > tabla[i - 1]; --i) {
        std::swap(tabla[i], tabla[i - 1]);
    }
}

int topScore(const std::array<int, 5>& tabla) {
    return *std::max_element(tabla.begin(), tabla.end());
}

// ============= TESTS (los mismos del ejercicio) =============
TEST_CASE(TestTopScore) {
    std::array<int, 5> tabla = {0, 0, 0, 0, 0};
    insertarPuntuacion(tabla, 100);
    insertarPuntuacion(tabla, 50);
    insertarPuntuacion(tabla, 200);
    ASSERT_EQ(topScore(tabla), 200);
}

TEST_CASE(TestOrdenDescendente) {
    std::array<int, 5> tabla = {0, 0, 0, 0, 0};
    insertarPuntuacion(tabla, 30);
    insertarPuntuacion(tabla, 10);
    insertarPuntuacion(tabla, 50);
    insertarPuntuacion(tabla, 20);
    insertarPuntuacion(tabla, 40);
    ASSERT_EQ(tabla[0], 50);
    ASSERT_EQ(tabla[1], 40);
    ASSERT_EQ(tabla[2], 30);
}

TEST_CASE(TestSoloTop5) {
    std::array<int, 5> tabla = {0, 0, 0, 0, 0};
    for (int i = 1; i <= 8; ++i)
        insertarPuntuacion(tabla, i * 10);
    // solo los 5 mayores: 80 70 60 50 40
    ASSERT_EQ(tabla[0], 80);
    ASSERT_EQ(tabla[4], 40);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 04 (Tabla de Puntuaciones)...\n";
    return RUN_ALL_TESTS();
}
