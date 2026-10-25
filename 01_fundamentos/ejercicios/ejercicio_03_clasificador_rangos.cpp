/* =============================================================================
   MÓDULO 01 - EJERCICIO 03: Clasificador de Rangos de Daño con enum class
   =============================================================================
   DIFICULTAD: Básico

   CONSIGNA:
   Dado un enum class 'TipoGolpe' con valores: INEFECTIVO, NORMAL, CRITICO, LETAL.
   Implementa la función 'TipoGolpe clasificarDanio(int danio)':
   - danio < 10: INEFECTIVO
   - 10 <= danio < 50: NORMAL
   - 50 <= danio < 100: CRITICO
   - danio >= 100: LETAL
   ============================================================================= */

#include <iostream>
#include "../../include/test_framework.hpp"

enum class TipoGolpe {
    INEFECTIVO,
    NORMAL,
    CRITICO,
    LETAL
};

// TODO: Implementa 'clasificarDanio'
TipoGolpe clasificarDanio(int danio) {
    // TU CÓDIGO AQUÍ:
    (void)danio;
    return TipoGolpe::INEFECTIVO;
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
    std::cout << "Ejecutando tests de Ejercicio 03...\n";
    return RUN_ALL_TESTS();
}
