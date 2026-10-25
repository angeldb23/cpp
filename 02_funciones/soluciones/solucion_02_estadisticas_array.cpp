/* =============================================================================
   MÓDULO 02 - SOLUCIÓN 02: Parámetros de Salida por Referencia
   =============================================================================
   Implementación de la función 'calcularEstadisticas' con pruebas.
   ============================================================================= */

#include <iostream>
#include "../../include/test_framework.hpp"

void calcularEstadisticas(const int arreglo[], int tamano, int& outMin, int& outMax, double& outPromedio) {
    if (tamano <= 0) return;
    outMin = arreglo[0];
    outMax = arreglo[0];
    long long suma = 0;
    for (int i = 0; i < tamano; ++i) {
        if (arreglo[i] < outMin) outMin = arreglo[i];
        if (arreglo[i] > outMax) outMax = arreglo[i];
        suma += arreglo[i];
    }
    outPromedio = static_cast<double>(suma) / tamano;
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
    std::cout << "[SOLUCIÓN] Ejecutando tests de Solución 02..." << std::endl;
    return RUN_ALL_TESTS();
}
