/* =============================================================================
   MÓDULO 01 - SOLUCIÓN 03: Clasificador de Rangos de Daño con enum class
   ============================================================================= */

#include <iostream>
#include "../../include/test_framework.hpp"

enum class TipoGolpe {
    INEFECTIVO,
    NORMAL,
    CRITICO,
    LETAL
};

TipoGolpe clasificarDanio(int danio) {
    if (danio < 10) {
        return TipoGolpe::INEFECTIVO;
    } else if (danio < 50) {
        return TipoGolpe::NORMAL;
    } else if (danio < 100) {
        return TipoGolpe::CRITICO;
    } else {
        return TipoGolpe::LETAL;
    }
}

TEST_CASE(TestClasificacionGolpes) {
    ASSERT_TRUE(clasificarDanio(5) == TipoGolpe::INEFECTIVO);
    ASSERT_TRUE(clasificarDanio(10) == TipoGolpe::NORMAL);
    ASSERT_TRUE(clasificarDanio(49) == TipoGolpe::NORMAL);
    ASSERT_TRUE(clasificarDanio(50) == TipoGolpe::CRITICO);
    ASSERT_TRUE(clasificarDanio(99) == TipoGolpe::CRITICO);
    ASSERT_TRUE(clasificarDanio(100) == TipoGolpe::LETAL);
    ASSERT_TRUE(clasificarDanio(250) == TipoGolpe::LETAL);
}

int main() {
    std::cout << "[SOLUCIÓN] Ejecutando tests de Ejercicio 03...\n";
    return RUN_ALL_TESTS();
}
