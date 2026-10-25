/* =============================================================================
   MÓDULO 03 - SOLUCIÓN 05: Histograma de Frecuencias
   =============================================================================
   POR QUÉ FUNCIONA:
     - vector<int>(n, 0) crea n contadores inicializados en cero.
     - Restar min_val 'corre' el rango para que el primer valor caiga en el índice 0.
     - El chequeo de rango evita un acceso fuera de límites (comportamiento indefinido).
     - Los static_cast<size_t> hacen explícita la conversión int -> índice.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_05_histograma.cpp -o sol05 && ./sol05
   ============================================================================= */
#include <iostream>
#include <vector>
#include <span>
#include "../../include/test_framework.hpp"

// El valor v se guarda en el índice (v - min_val). Los valores fuera del
// rango [min_val, max_val] se ignoran en vez de escribir fuera del vector.
std::vector<int> frecuencias(std::span<const int> datos, int min_val, int max_val) {
    std::vector<int> freq(static_cast<std::size_t>(max_val - min_val + 1), 0);
    for (int d : datos) {
        if (d >= min_val && d <= max_val) {
            ++freq[static_cast<std::size_t>(d - min_val)];
        }
    }
    return freq;
}

// ============= TESTS (los mismos del ejercicio) =============
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
    std::cout << "Ejecutando pruebas para Solución 05 (Histograma de Frecuencias)...\n";
    return RUN_ALL_TESTS();
}
