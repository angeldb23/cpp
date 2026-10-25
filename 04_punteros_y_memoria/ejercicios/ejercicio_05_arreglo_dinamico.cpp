/* =============================================================================
   MÓDULO 04 - EJERCICIO 05: Arreglos dinámicos con new[] y delete[]
   =============================================================================
   DIFICULTAD: Intermedio
   PRACTICA: lecciones 09 y 10 (new/delete, new[]/delete[], arreglos dinámicos).

   CONSIGNA:
     1. copiarArreglo(const int* origen, std::size_t n): reservá con new[] un
        arreglo de n enteros, copiá los valores y devolvé el puntero. Si n es
        0, devolvé nullptr. Si origen es nullptr y n > 0, devolvé nullptr.
        El llamador libera con delete[].
     2. llenarSecuencia(int* destino, std::size_t n, int inicio, int paso):
        escribí destino[i] = inicio + i * paso. Si destino es nullptr, no hagas
        nada.
     3. sumaArreglo(const int* datos, std::size_t n): suma; 0 si está vacío o
        si datos es nullptr.
     4. contarMayores(const int* datos, std::size_t n, int umbral): cuenta los
        estrictamente mayores que umbral.
     5. promedio(const int* datos, std::size_t n): devolvé double; 0.0 si n es
        0 o datos es nullptr.

   OJO: no uses std::vector en este ejercicio; la idea es practicar el par
   new[]/delete[] y el contrato de propiedad.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra ejercicio_05_arreglo_dinamico.cpp \
         -o ej05 && ./ej05
   ============================================================================= */
#include <cstddef>
#include <iostream>
#include "../../include/test_framework.hpp"

// TODO: implementá las cinco funciones.
int* copiarArreglo(const int* origen, std::size_t n) {
    (void)origen; (void)n;
    return nullptr;  // placeholder
}

void llenarSecuencia(int* destino, std::size_t n, int inicio, int paso) {
    (void)destino; (void)n; (void)inicio; (void)paso;  // placeholder
}

int sumaArreglo(const int* datos, std::size_t n) {
    (void)datos; (void)n;
    return 0;  // placeholder
}

std::size_t contarMayores(const int* datos, std::size_t n, int umbral) {
    (void)datos; (void)n; (void)umbral;
    return 0;  // placeholder
}

double promedio(const int* datos, std::size_t n) {
    (void)datos; (void)n;
    return 0.0;  // placeholder
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
    llenarSecuencia(nullptr, 3, 0, 1);  // no debe romper
}

TEST_CASE(TestSumaArreglo) {
    int datos[] = {1, 2, 3, 4};
    ASSERT_EQ(sumaArreglo(datos, 4), 10);
    ASSERT_EQ(sumaArreglo(datos, 0), 0);
    ASSERT_EQ(sumaArreglo(nullptr, 4), 0);
}

TEST_CASE(TestContarMayores) {
    int datos[] = {2, 5, 9, 1, 10};
    ASSERT_EQ(contarMayores(datos, 5, 4), 3);
    ASSERT_EQ(contarMayores(datos, 5, 10), 0);
}

TEST_CASE(TestPromedio) {
    int datos[] = {2, 4, 6};
    ASSERT_NEAR(promedio(datos, 3), 4.0, 0.0001);
    ASSERT_NEAR(promedio(datos, 0), 0.0, 0.0001);
    ASSERT_NEAR(promedio(nullptr, 3), 0.0, 0.0001);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 05...\n";
    return RUN_ALL_TESTS();
}
