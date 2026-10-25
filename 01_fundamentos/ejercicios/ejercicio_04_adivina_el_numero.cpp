/* =============================================================================
   MÓDULO 01 - EJERCICIO 04: Lógica de Comparación de Número Secreto
   =============================================================================
   DIFICULTAD: Intermedio

   CONSIGNA:
   Implementa la función 'int evaluarIntento(int numeroSecreto, int intento)'
   - Retorna -1 si el intento es MENOR que el número secreto.
   - Retorna 1 si el intento es MAYOR que el número secreto.
   - Retorna 0 si el intento es EXACTAMENTE IGUAL (acertó).

   Y la función 'int contarIntentosParaAdivinar(int numeroSecreto, const int intentos[], int cantidad)'
   - Recorre el arreglo de intentos con un bucle.
   - Si acierta en algún intento, retorna la cantidad de intentos realizados hasta ese momento.
   - Si no acierta en ninguno, retorna -1.
   ============================================================================= */

#include <iostream>
#include "../../include/test_framework.hpp"

// TODO 1: Implementa evaluarIntento
int evaluarIntento(int numeroSecreto, int intento) {
    // TU CÓDIGO AQUÍ:
    (void)numeroSecreto;
    (void)intento;
    return 999;
}

// TODO 2: Implementa contarIntentosParaAdivinar
int contarIntentosParaAdivinar(int numeroSecreto, const int intentos[], int cantidad) {
    // TU CÓDIGO AQUÍ:
    (void)numeroSecreto;
    (void)intentos;
    (void)cantidad;
    return -999;
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
    std::cout << "Ejecutando tests de Ejercicio 04...\n";
    return RUN_ALL_TESTS();
}
