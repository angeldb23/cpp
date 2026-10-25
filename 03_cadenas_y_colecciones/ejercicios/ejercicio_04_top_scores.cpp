/* =============================================================================
   MÓDULO 03 - EJERCICIO 04: Tabla de Puntuaciones (array fijo)
   =============================================================================
   DIFICULTAD: Básico
   CONSIGNA:
     Usá std::array<int, 5> para representar las 5 mejores puntuaciones.
     Implementá:
     1. insertarPuntuacion(array<int,5>&, int nueva) — inserta manteniendo orden descendente
        (si la nueva supera alguna existente, desplaza las menores)
     2. topScore(const array<int,5>&) → int — retorna la mayor puntuación
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_04_top_scores.cpp -o ej04 && ./ej04
   ============================================================================= */
#include <iostream>
#include <array>
#include "../../include/test_framework.hpp"

// TODO: implementar estas funciones
// TU CÓDIGO AQUÍ:

void insertarPuntuacion(std::array<int, 5>& tabla, int nueva) {
    // placeholder: no hace nada
    (void)tabla; (void)nueva;
}

int topScore(const std::array<int, 5>& tabla) {
    return 0; // placeholder
}

// ============= TESTS =============
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
    std::cout << "Ejecutando pruebas para Ejercicio 04 (Top Scores)...\n";
    return RUN_ALL_TESTS();
}
