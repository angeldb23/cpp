/* =============================================================================
   MÓDULO 01 - SOLUCIÓN 02: Conversor de Unidades de Memoria
   ============================================================================= */

#include <iostream>
#include "../../include/test_framework.hpp"

unsigned long long megabytesABytes(unsigned int mb) {
    return static_cast<unsigned long long>(mb) * 1024ULL * 1024ULL;
}

double bytesAMegabytes(unsigned long long bytes) {
    return static_cast<double>(bytes) / (1024.0 * 1024.0);
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
    std::cout << "[SOLUCIÓN] Ejecutando tests de Ejercicio 02...\n";
    return RUN_ALL_TESTS();
}
