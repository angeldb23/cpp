/* =============================================================================
   MÓDULO 04 - SOLUCIÓN 05: Arreglos dinámicos con new[] y delete[]
   =============================================================================
   POR QUÉ FUNCIONA:
     - copiarArreglo es el único que reserva; documenta que el llamador libera.
     - n == 0 devuelve nullptr: no hay nada que reservar.
     - Todos los recorridos comprueban el puntero antes de leer.
     - promedio convierte a double antes de dividir para no truncar.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_05_arreglo_dinamico.cpp -o sol05 && ./sol05
   ============================================================================= */
#include <cstddef>
#include <iostream>
#include "../../include/test_framework.hpp"

int* copiarArreglo(const int* origen, std::size_t n) {
    if (n == 0 || origen == nullptr) return nullptr;
    int* copia = new int[n];
    for (std::size_t i = 0; i < n; ++i) copia[i] = origen[i];
    return copia;
}

void llenarSecuencia(int* destino, std::size_t n, int inicio, int paso) {
    if (destino == nullptr) return;
    for (std::size_t i = 0; i < n; ++i) {
        destino[i] = inicio + static_cast<int>(i) * paso;
    }
}

int sumaArreglo(const int* datos, std::size_t n) {
    if (datos == nullptr) return 0;
    int total = 0;
    for (std::size_t i = 0; i < n; ++i) total += datos[i];
    return total;
}

std::size_t contarMayores(const int* datos, std::size_t n, int umbral) {
    if (datos == nullptr) return 0;
    std::size_t total = 0;
    for (std::size_t i = 0; i < n; ++i) {
        if (datos[i] > umbral) ++total;
    }
    return total;
}

double promedio(const int* datos, std::size_t n) {
    if (datos == nullptr || n == 0) return 0.0;
    return static_cast<double>(sumaArreglo(datos, n)) / static_cast<double>(n);
}

// ============= TESTS =============
TEST_CASE(TestCopiarArreglo) {
    int origen[] = {4, 8, 15, 16, 23, 42};
    int* copia = copiarArreglo(origen, 6);
    ASSERT_TRUE(copia != nullptr);
    ASSERT_TRUE(copia != origen);
    for (int i = 0; i < 6; ++i) ASSERT_EQ(copia[i], origen[i]);
    delete[] copia;
}

TEST_CASE(TestCopiarArregloVacioONulo) {
    ASSERT_TRUE(copiarArreglo(nullptr, 0) == nullptr);
    int x = 1;
    ASSERT_TRUE(copiarArreglo(nullptr, 3) == nullptr);
    (void)x;
}

TEST_CASE(TestLlenarSecuencia) {
    const std::size_t n = 5;
    int* datos = new int[n];
    llenarSecuencia(datos, n, 3, 2);
    ASSERT_EQ(datos[0], 3);
    ASSERT_EQ(datos[1], 5);
    ASSERT_EQ(datos[2], 7);
    ASSERT_EQ(datos[3], 9);
    ASSERT_EQ(datos[4], 11);
    delete[] datos;
    llenarSecuencia(nullptr, 3, 0, 1);
}

TEST_CASE(TestSumaArreglo) {
    int datos[] = {1, 2, 3, 4};
    ASSERT_EQ(sumaArreglo(datos, 4), 10);
    ASSERT_EQ(sumaArreglo(datos, 0), 0);
    ASSERT_EQ(sumaArreglo(nullptr, 4), 0);
}

TEST_CASE(TestContarMayores) {
    int datos[] = {2, 5, 9, 1, 10};
    ASSERT_EQ(contarMayores(datos, 5, 4), 3u);
    ASSERT_EQ(contarMayores(datos, 5, 10), 0u);
}

TEST_CASE(TestPromedio) {
    int datos[] = {2, 4, 6};
    ASSERT_NEAR(promedio(datos, 3), 4.0, 0.0001);
    ASSERT_NEAR(promedio(datos, 0), 0.0, 0.0001);
    ASSERT_NEAR(promedio(nullptr, 3), 0.0, 0.0001);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 05...\n";
    return RUN_ALL_TESTS();
}
