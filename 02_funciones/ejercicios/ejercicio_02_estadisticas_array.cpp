/* =============================================================================
   MÓDULO 02 - EJERCICIO 02: Parámetros de Salida por Referencia
   =============================================================================
   DIFICULTAD: Básico

   CONSIGNA:
   Implementa la función:
   'void calcularEstadisticas(const int arreglo[], int tamano, int& outMin, int& outMax, double& outPromedio)'
   - Encuentra el mínimo y máximo elemento del arreglo y los asigna a 'outMin' y 'outMax'.
   - Calcula el promedio exacto y lo asigna a 'outPromedio'.
   ============================================================================= */

#include <iostream>
#include "../../include/test_framework.hpp"

// TODO: Implementa 'calcularEstadisticas'
void calcularEstadisticas(const int arreglo[], int tamano, int& outMin, int& outMax, double& outPromedio) {
    // TU CÓDIGO AQUÍ:
    (void)arreglo;
    (void)tamano;
    outMin = 0;
    outMax = 0;
    outPromedio = 0.0;
}

TEST_CASE(TestEstadisticasArreglo) {
    int datos[] = {10, 20, 5, 40, 25};
    int minVal = 0, maxVal = 0;
    double promedio = 0.0;

    calcularEstadisticas(datos, 5, minVal, maxVal, promedio);

    ASSERT_EQ(minVal, 5);
    ASSERT_EQ(maxVal, 40);
    ASSERT_NEAR(promedio, 20.0, 0.001);
}

int main() {
    std::cout << "Ejecutando tests de Ejercicio 02...\n";
    return RUN_ALL_TESTS();
}
