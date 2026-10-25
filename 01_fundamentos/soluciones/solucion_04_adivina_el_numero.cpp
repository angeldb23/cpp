/* =============================================================================
   MÓDULO 01 - SOLUCIÓN 04: Lógica de Comparación de Número Secreto
   ============================================================================= */

#include <iostream>
#include "../../include/test_framework.hpp"

int evaluarIntento(int numeroSecreto, int intento) {
    if (intento < numeroSecreto) return -1;
    if (intento > numeroSecreto) return 1;
    return 0;
}

int contarIntentosParaAdivinar(int numeroSecreto, const int intentos[], int cantidad) {
    for (int i = 0; i < cantidad; ++i) {
        if (evaluarIntento(numeroSecreto, intentos[i]) == 0) {
            return i + 1; // 1-indexado para contar intentos
        }
    }
    return -1;
}

TEST_CASE(TestEvaluarIntento) {
    ASSERT_EQ(evaluarIntento(50, 20), -1);
    ASSERT_EQ(evaluarIntento(50, 80), 1);
    ASSERT_EQ(evaluarIntento(50, 50), 0);
}

TEST_CASE(TestContarIntentos) {
    int intentos1[] = {10, 25, 40, 50, 60};
    ASSERT_EQ(contarIntentosParaAdivinar(50, intentos1, 5), 4);

    int intentos2[] = {50, 10, 20};
    ASSERT_EQ(contarIntentosParaAdivinar(50, intentos2, 3), 1);

    int intentos3[] = {1, 2, 3};
    ASSERT_EQ(contarIntentosParaAdivinar(50, intentos3, 3), -1);
}

int main() {
    std::cout << "[SOLUCIÓN] Ejecutando tests de Ejercicio 04...\n";
    return RUN_ALL_TESTS();
}
