/* =============================================================================
   MÓDULO 04 - SOLUCIÓN 06: Histograma dinámico
   =============================================================================
   POR QUÉ FUNCIONA:
     - new int[n]() value-initializa: todos los contadores arrancan en 0.
     - Se filtran los valores fuera de [0, maxValor] antes de indexar.
     - valorMasFrecuente compara con > (no >=), así conserva el menor índice
       en los empates.
     - delete[] libera exactamente el bloque pedido con new[].
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_06_histograma.cpp -o sol06 && ./sol06
   ============================================================================= */
#include <cstddef>
#include <iostream>
#include "../../include/test_framework.hpp"

int* crearHistograma(const int* datos, std::size_t n, int maxValor) {
    if (maxValor < 0) return nullptr;
    if (datos == nullptr && n > 0) return nullptr;

    int* histograma = new int[maxValor + 1]();
    for (std::size_t i = 0; i < n; ++i) {
        int valor = datos[i];
        if (valor >= 0 && valor <= maxValor) ++histograma[valor];
    }
    return histograma;
}

int valorMasFrecuente(const int* histograma, int maxValor) {
    if (histograma == nullptr || maxValor < 0) return -1;
    int mejorIndice = -1;
    int mejorConteo = 0;
    for (int i = 0; i <= maxValor; ++i) {
        if (histograma[i] > mejorConteo) {
            mejorConteo = histograma[i];
            mejorIndice = i;
        }
    }
    return mejorIndice;
}

int totalContado(const int* histograma, int maxValor) {
    if (histograma == nullptr || maxValor < 0) return 0;
    int total = 0;
    for (int i = 0; i <= maxValor; ++i) total += histograma[i];
    return total;
}

// ============= TESTS =============
TEST_CASE(TestCrearHistograma) {
    int datos[] = {0, 1, 1, 3, 3, 3};
    int* h = crearHistograma(datos, 6, 3);
    ASSERT_TRUE(h != nullptr);
    ASSERT_EQ(h[0], 1);
    ASSERT_EQ(h[1], 2);
    ASSERT_EQ(h[2], 0);
    ASSERT_EQ(h[3], 3);
    delete[] h;
}

TEST_CASE(TestHistogramaIgnoraFueraDeRango) {
    int datos[] = {-1, 0, 5, 2, 2, 99};
    int* h = crearHistograma(datos, 6, 3);
    ASSERT_TRUE(h != nullptr);
    ASSERT_EQ(h[0], 1);
    ASSERT_EQ(h[2], 2);
    ASSERT_EQ(totalContado(h, 3), 3);
    delete[] h;
}

TEST_CASE(TestHistogramaInvalido) {
    int datos[] = {1, 2};
    ASSERT_TRUE(crearHistograma(datos, 2, -1) == nullptr);
    ASSERT_TRUE(crearHistograma(nullptr, 2, 3) == nullptr);
    int* vacio = crearHistograma(nullptr, 0, 3);
    ASSERT_TRUE(vacio != nullptr);
    delete[] vacio;
}

TEST_CASE(TestValorMasFrecuente) {
    int h[] = {1, 2, 0, 3};
    ASSERT_EQ(valorMasFrecuente(h, 3), 3);
}

TEST_CASE(TestValorMasFrecuenteEmpate) {
    int h[] = {0, 2, 2, 0};
    ASSERT_EQ(valorMasFrecuente(h, 3), 1);
}

TEST_CASE(TestValorMasFrecuenteVacio) {
    int h[] = {0, 0, 0};
    ASSERT_EQ(valorMasFrecuente(h, 2), -1);
    ASSERT_EQ(valorMasFrecuente(nullptr, 2), -1);
    ASSERT_EQ(valorMasFrecuente(h, -1), -1);
}

TEST_CASE(TestTotalContado) {
    int h[] = {1, 2, 3};
    ASSERT_EQ(totalContado(h, 2), 6);
    ASSERT_EQ(totalContado(nullptr, 2), 0);
    ASSERT_EQ(totalContado(h, -1), 0);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 06...\n";
    return RUN_ALL_TESTS();
}
