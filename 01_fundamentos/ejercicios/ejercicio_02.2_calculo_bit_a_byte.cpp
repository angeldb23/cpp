/* =============================================================================
   MÓDULO 01 - EJERCICIO 02: Conversor de Unidades de Memoria (Bytes, KB, MB)
   =============================================================================
   DIFICULTAD: Básico

   CONSIGNA:
   Implementa funciones para convertir tamaños de memoria:
   1. 'unsigned long long megabytesABytes(unsigned int mb)'
      (1 MB = 1024 * 1024 bytes = 1,048,576 bytes)
   2. 'double bytesAMegabytes(unsigned long long bytes)'
      (Devuelve el valor exacto en MB en punto flotante)
   ============================================================================= */

#include <iostream>
#include "../../include/test_framework.hpp"

// TODO 1: Implementa megabytesABytes
unsigned long long megabytesABytes(unsigned int mb) {
    // TU CÓDIGO AQUÍ:
    (void)mb;
    return 0ULL;
}

// TODO 2: Implementa bytesAMegabytes
double bytesAMegabytes(unsigned long long bytes) {
    // TU CÓDIGO AQUÍ:
    (void)bytes;
    return 0.0;
}

TEST_CASE(TestMegabytesABytes) {
    ASSERT_EQ(megabytesABytes(1), 1048576ULL);
    ASSERT_EQ(megabytesABytes(4), 4194304ULL);
    ASSERT_EQ(megabytesABytes(0), 0ULL);
}

TEST_CASE(TestBytesAMegabytes) {
    ASSERT_NEAR(bytesAMegabytes(1048576ULL), 1.0, 0.0001);
    ASSERT_NEAR(bytesAMegabytes(524288ULL), 0.5, 0.0001);
}

int main() {
    std::cout << "Ejecutando tests de Ejercicio 02...\n";
    return RUN_ALL_TESTS();
}
