/* =============================================================================
   MÓDULO 13 - EJERCICIO 02: Tres caminos: códigos, excepciones, expected
   =============================================================================
   DIFICULTAD: Básico-Intermedio
   CONSIGNA:
     Implementá la MISMA lógica de división entera segura en TRES estilos:
     1. dividirConCodigo(int a, int b, int& out) -> DivError (enum class)
     2. dividirConExcepcion(int a, int b) -> int (lanza std::domain_error)
     3. dividirConExpected(int a, int b) -> mini::expected<int, std::string>
        (usá mini::expected del header include/mini_expected.hpp)
     
     Reglas para los tres:
     - Si b == 0: error "division por cero"
     - Si a % b != 0: error "division no exacta" (solo para expected y excepcion)
     - Caso feliz: devuelve a / b
     
     En la función main() de test, probá los tres con (10, 2), (10, 0), (10, 3).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../include ejercicio_02_tres_caminos.cpp -o ej02 && ./ej02
   ============================================================================= */
#include <iostream>
#include <string>
#include <stdexcept>
#include "../../include/mini_expected.hpp"
#include "../../include/test_framework.hpp"

// TODO: enum class DivError { Ok, DivisionPorCero, DivisionNoExacta };

// TODO: DivError dividirConCodigo(int a, int b, int& out);

// TODO: int dividirConExcepcion(int a, int b);

// TODO: mini::expected<int, std::string> dividirConExpected(int a, int b);

// ============= TESTS =============
TEST_CASE(TestCodigoCasoFeliz) {
    int out = 0;
    // DivError err = dividirConCodigo(10, 2, out);
    // ASSERT_EQ(err, DivError::Ok);
    // ASSERT_EQ(out, 5);
    (void)out;
    ASSERT_TRUE(false);
}

TEST_CASE(TestCodigoDivisionPorCero) {
    int out = 0;
    // DivError err = dividirConCodigo(10, 0, out);
    // ASSERT_EQ(err, DivError::DivisionPorCero);
    ASSERT_TRUE(false);
}

TEST_CASE(TestExcepcionCasoFeliz) {
    // int r = dividirConExcepcion(10, 2);
    // ASSERT_EQ(r, 5);
    ASSERT_TRUE(false);
}

TEST_CASE(TestExcepcionDivisionPorCero) {
    bool lanzo = false;
    try {
        // dividirConExcepcion(10, 0);
    } catch (const std::domain_error&) {
        lanzo = true;
    }
    ASSERT_TRUE(lanzo);
}

TEST_CASE(TestExcepcionDivisionNoExacta) {
    bool lanzo = false;
    try {
        // dividirConExcepcion(10, 3);
    } catch (const std::domain_error&) {
        lanzo = true;
    }
    ASSERT_TRUE(lanzo);
}

TEST_CASE(TestExpectedCasoFeliz) {
    // auto r = dividirConExpected(10, 2);
    // ASSERT_TRUE(r.has_value());
    // ASSERT_EQ(r.value(), 5);
    ASSERT_TRUE(false);
}

TEST_CASE(TestExpectedDivisionPorCero) {
    // auto r = dividirConExpected(10, 0);
    // ASSERT_FALSE(r.has_value());
    // ASSERT_EQ(r.error(), std::string("division por cero"));
    ASSERT_TRUE(false);
}

TEST_CASE(TestExpectedDivisionNoExacta) {
    // auto r = dividirConExpected(10, 3);
    // ASSERT_FALSE(r.has_value());
    // ASSERT_EQ(r.error(), std::string("division no exacta"));
    ASSERT_TRUE(false);
}

TEST_CASE(TestExpectedValueOr) {
    // auto r = dividirConExpected(10, 0);
    // ASSERT_EQ(r.value_or(-1), -1);
    ASSERT_TRUE(false);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 02 (Tres caminos)...\n";
    return RUN_ALL_TESTS();
}