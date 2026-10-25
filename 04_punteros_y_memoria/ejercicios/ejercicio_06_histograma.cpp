/* =============================================================================
   MÓDULO 04 - EJERCICIO 06: Histograma dinámico
   =============================================================================
   DIFICULTAD: Intermedio
   PRACTICA: lección 10 (new int[n](), conteo, recorrido y delete[]).

   CONSIGNA:
     1. crearHistograma(const int* datos, std::size_t n, int maxValor):
        - Devolvé un arreglo dinámico de (maxValor + 1) enteros, todos en 0.
        - Contá cuántas veces aparece cada valor entre 0 y maxValor inclusive.
        - Ignorá los valores negativos o mayores que maxValor.
        - Si maxValor < 0, devolvé nullptr.
        - Si datos es nullptr y n > 0, devolvé nullptr.
        El llamador libera con delete[].
     2. valorMasFrecuente(const int* histograma, int maxValor):
        - Devolvé el índice con mayor conteo.
        - Si hay empate, devolvé el menor índice.
        - Si el histograma es nullptr, maxValor < 0 o todos los conteos son 0,
          devolvé -1.
     3. totalContado(const int* histograma, int maxValor): sumá todos los
        conteos; 0 si el histograma es nullptr o maxValor < 0.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra ejercicio_06_histograma.cpp -o ej06 && ./ej06
   ============================================================================= */
#include <cstddef>
#include <iostream>
#include "../../include/test_framework.hpp"

// TODO: implementá las tres funciones.
int* crearHistograma(const int* datos, std::size_t n, int maxValor) {
    (void)datos; (void)n; (void)maxValor;
    return nullptr;  // placeholder
}

int valorMasFrecuente(const int* histograma, int maxValor) {
    (void)histograma; (void)maxValor;
    return -1;  // placeholder
}

int totalContado(const int* histograma, int maxValor) {
    (void)histograma; (void)maxValor;
    return 0;  // placeholder
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
    int* vacio = crearHistograma(nullptr, 0, 3);  // histograma vacío válido
    ASSERT_TRUE(vacio != nullptr);
    delete[] vacio;
}

TEST_CASE(TestValorMasFrecuente) {
    int h[] = {1, 2, 0, 3};
    ASSERT_EQ(valorMasFrecuente(h, 3), 3);
}

TEST_CASE(TestValorMasFrecuenteEmpate) {
    int h[] = {0, 2, 2, 0};
    ASSERT_EQ(valorMasFrecuente(h, 3), 1);  // empate: menor índice
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
    std::cout << "Ejecutando pruebas para Ejercicio 06...\n";
    return RUN_ALL_TESTS();
}
