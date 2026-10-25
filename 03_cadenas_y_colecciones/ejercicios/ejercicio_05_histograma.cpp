/* =============================================================================
   MÓDULO 03 - EJERCICIO 05: Histograma de Frecuencias
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA:
     Implementá frecuencias(span<const int> datos, int min_val, int max_val)
     que retorna un vector<int> con la cantidad de veces que aparece cada valor
     en el rango [min_val, max_val]. El vector tiene tamaño (max_val - min_val + 1),
     donde índice 0 corresponde a min_val.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_05_histograma.cpp -o ej05 && ./ej05
   ============================================================================= */
#include <iostream>
#include <vector>
#include <span>
#include "../../include/test_framework.hpp"

// TODO: implementar esta función
// TU CÓDIGO AQUÍ:

std::vector<int> frecuencias(std::span<const int> datos, int min_val, int max_val) {
    return {}; // placeholder
}

// ============= TESTS =============
TEST_CASE(TestFrecuenciasBasico) {
    std::vector<int> datos = {1, 2, 2, 3, 3, 3, 4};
    auto freq = frecuencias(datos, 1, 4);
    ASSERT_EQ(static_cast<int>(freq.size()), 4);
    ASSERT_EQ(freq[0], 1); // 1 aparece 1 vez
    ASSERT_EQ(freq[1], 2); // 2 aparece 2 veces
    ASSERT_EQ(freq[2], 3); // 3 aparece 3 veces
    ASSERT_EQ(freq[3], 1); // 4 aparece 1 vez
}

TEST_CASE(TestFrecuenciasTodos) {
    std::vector<int> dados = {1, 1, 6, 3, 2, 5, 4, 6, 6};
    auto freq = frecuencias(dados, 1, 6);
    ASSERT_EQ(static_cast<int>(freq.size()), 6);
    ASSERT_EQ(freq[0], 2); // 1 → 2
    ASSERT_EQ(freq[5], 3); // 6 → 3
}

TEST_CASE(TestFrecuenciasRangoUnico) {
    std::vector<int> v = {5, 5, 5};
    auto freq = frecuencias(v, 5, 5);
    ASSERT_EQ(static_cast<int>(freq.size()), 1);
    ASSERT_EQ(freq[0], 3);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 05 (Histograma)...\n";
    return RUN_ALL_TESTS();
}
