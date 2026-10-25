/* =============================================================================
   MÓDULO 13 - SOLUCIÓN 02: Tres caminos: códigos, excepciones, expected
   =============================================================================
   POR QUÉ FUNCIONA:
     - Canal A (códigos): el error es un valor de retorno; el valor útil sale
       por referencia. Obliga a chequear, pero ensucia la firma y el sitio
       de llamada (variable extra + if).
     - Canal B (excepciones): el caso feliz limpio; el error viaja por la
       pila (canal lateral). No se ve en la firma; el chequeo es opcional
       (puede no haber try/catch). El unwinding tiene costo.
     - Canal C (expected): el error está EN el tipo de retorno (T o E).
       La firma documenta el fallo; el chequeo es obligatorio para leer T
       (has_value/value); no hay unwinding; composición monádica (L12).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../include solucion_02_tres_caminos.cpp -o sol02 && ./sol02
   ============================================================================= */
#include <iostream>
#include <string>
#include <stdexcept>
#include "../../include/mini_expected.hpp"
#include "../../include/test_framework.hpp"

enum class DivError {
    Ok,
    DivisionPorCero,
    DivisionNoExacta
};

constexpr int toInt(DivError e) noexcept { return static_cast<int>(e); }

DivError dividirConCodigo(int a, int b, int& out) {
    if (b == 0) return DivError::DivisionPorCero;
    if (a % b != 0) return DivError::DivisionNoExacta;
    out = a / b;
    return DivError::Ok;
}

int dividirConExcepcion(int a, int b) {
    if (b == 0) throw std::domain_error("division por cero");
    if (a % b != 0) throw std::domain_error("division no exacta");
    return a / b;
}

mini::expected<int, std::string> dividirConExpected(int a, int b) {
    if (b == 0) return std::string("division por cero");
    if (a % b != 0) return std::string("division no exacta");
    return a / b;
}

// ============= TESTS =============
TEST_CASE(TestCodigoCasoFeliz) {
    int out = 0;
    DivError err = dividirConCodigo(10, 2, out);
    ASSERT_EQ(toInt(err), toInt(DivError::Ok));
    ASSERT_EQ(out, 5);
}

TEST_CASE(TestCodigoDivisionPorCero) {
    int out = 0;
    DivError err = dividirConCodigo(10, 0, out);
    ASSERT_EQ(toInt(err), toInt(DivError::DivisionPorCero));
}

TEST_CASE(TestExcepcionCasoFeliz) {
    int r = dividirConExcepcion(10, 2);
    ASSERT_EQ(r, 5);
}

TEST_CASE(TestExcepcionDivisionPorCero) {
    bool lanzo = false;
    try {
        dividirConExcepcion(10, 0);
    } catch (const std::domain_error&) {
        lanzo = true;
    }
    ASSERT_TRUE(lanzo);
}

TEST_CASE(TestExcepcionDivisionNoExacta) {
    bool lanzo = false;
    try {
        dividirConExcepcion(10, 3);
    } catch (const std::domain_error&) {
        lanzo = true;
    }
    ASSERT_TRUE(lanzo);
}

TEST_CASE(TestExpectedCasoFeliz) {
    auto r = dividirConExpected(10, 2);
    ASSERT_TRUE(r.has_value());
    ASSERT_EQ(r.value(), 5);
}

TEST_CASE(TestExpectedDivisionPorCero) {
    auto r = dividirConExpected(10, 0);
    ASSERT_FALSE(r.has_value());
    ASSERT_EQ(r.error(), std::string("division por cero"));
}

TEST_CASE(TestExpectedDivisionNoExacta) {
    auto r = dividirConExpected(10, 3);
    ASSERT_FALSE(r.has_value());
    ASSERT_EQ(r.error(), std::string("division no exacta"));
}

TEST_CASE(TestExpectedValueOr) {
    auto r = dividirConExpected(10, 0);
    ASSERT_EQ(r.value_or(-1), -1);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 02 (Tres caminos)...\n";
    return RUN_ALL_TESTS();
}