/* =============================================================================
   MÓDULO 13 - EJERCICIO 10: mini::expected básico
   =============================================================================
   DIFICULTAD: Básico-Intermedio
   CONSIGNA:
     Usando mini::expected (include/mini_expected.hpp), implementá:
     
     1. parseEntero(const std::string&) -> mini::expected<int, std::string>
        - string vacío -> error "string vacío"
        - carácter no dígito -> error "carácter inválido: X"
        - OK -> valor parseado (std::stoi)
     
     2. dividirSeguro(int a, int b) -> mini::expected<int, std::string>
        - b == 0 -> error "división por cero"
        - OK -> a / b
     
     3. En main/tests:
        - Construcción implícita OK: mini::expected<int, std::string> e = 42;
        - Construcción implícita ERROR: mini::expected<int, std::string> e = std::string("fallo");
        - Construcción explícita error: mini::expected<int, std::string> e(mini::unexpect, "fallo");
        - Consultas: has_value(), operator bool, value(), error()
        - value_or(default) para caso error
        - value() sobre error lanza std::runtime_error
     
     Recordá: mini::expected usa std::optional<T> y std::optional<E> por dentro
     (L11 explica la diferencia con la unión del std::expected real).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../include ejercicio_10_mini_expected_basico.cpp -o ej10 && ./ej10
   ============================================================================= */
#include <iostream>
#include <string>
#include <stdexcept>
#include "../../include/mini_expected.hpp"
#include "../../include/test_framework.hpp"

// TODO: mini::expected<int, std::string> parseEntero(const std::string& s);

// TODO: mini::expected<int, std::string> dividirSeguro(int a, int b);

// ============= TESTS =============
TEST_CASE(TestConstruccionImplicitaOk) {
    // mini::expected<int, std::string> e = 42;
    // ASSERT_TRUE(e.has_value());
    // ASSERT_EQ(e.value(), 42);
    // ASSERT_TRUE(static_cast<bool>(e));
    ASSERT_TRUE(false);
}

TEST_CASE(TestConstruccionImplicitaError) {
    // mini::expected<int, std::string> e = std::string("fallo");
    // ASSERT_FALSE(e.has_value());
    // ASSERT_EQ(e.error(), std::string("fallo"));
    // ASSERT_FALSE(static_cast<bool>(e));
    ASSERT_TRUE(false);
}

TEST_CASE(TestConstruccionExplicitaUnExpect) {
    // mini::expected<int, std::string> e(mini::unexpect, std::string("explícito"));
    // ASSERT_FALSE(e.has_value());
    // ASSERT_EQ(e.error(), std::string("explícito"));
    ASSERT_TRUE(false);
}

TEST_CASE(TestParseEnteroExitoso) {
    // auto r = parseEntero("123");
    // ASSERT_TRUE(r.has_value());
    // ASSERT_EQ(r.value(), 123);
    ASSERT_TRUE(false);
}

TEST_CASE(TestParseEnteroVacio) {
    // auto r = parseEntero("");
    // ASSERT_FALSE(r.has_value());
    // ASSERT_EQ(r.error(), std::string("string vacío"));
    ASSERT_TRUE(false);
}

TEST_CASE(TestParseEnteroCaracterInvalido) {
    // auto r = parseEntero("12a3");
    // ASSERT_FALSE(r.has_value());
    // ASSERT_TRUE(r.error().find("carácter inválido") != std::string::npos);
    ASSERT_TRUE(false);
}

TEST_CASE(TestDividirSeguroExitoso) {
    // auto r = dividirSeguro(10, 2);
    // ASSERT_TRUE(r.has_value());
    // ASSERT_EQ(r.value(), 5);
    ASSERT_TRUE(false);
}

TEST_CASE(TestDividirSeguroPorCero) {
    // auto r = dividirSeguro(10, 0);
    // ASSERT_FALSE(r.has_value());
    // ASSERT_EQ(r.error(), std::string("división por cero"));
    ASSERT_TRUE(false);
}

TEST_CASE(TestValueOr) {
    // auto r = parseEntero("abc");
    // ASSERT_EQ(r.value_or(-1), -1);
    // auto r2 = parseEntero("42");
    // ASSERT_EQ(r2.value_or(-1), 42);
    ASSERT_TRUE(false);
}

TEST_CASE(TestValueLanzaEnError) {
    // auto r = parseEntero("");
    // bool lanzo = false;
    // try {
    //     r.value();
    // } catch (const std::runtime_error&) {
    //     lanzo = true;
    // }
    // ASSERT_TRUE(lanzo);
    ASSERT_TRUE(false);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 10 (mini::expected básico)...\n";
    return RUN_ALL_TESTS();
}