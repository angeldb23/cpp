/* =============================================================================
   MÓDULO 13 - SOLUCIÓN 10: mini::expected básico
   =============================================================================
   POR QUÉ FUNCIONA:
     - mini::expected<T, E> guarda AMBOS (T y E) en optionals separados.
       Solo uno tiene valor (has_value() dice cuál).
     - Construcción implícita: `expected<int, E> e = 5;` -> OK
       `expected<int, E> e = E::Error;` -> ERROR
       Esto permite `return 5;` y `return E::Error;` en funciones.
     - Construcción explícita con `mini::unexpect` (tag type) para desambiguar
       cuando T y E son convertibles entre sí.
     - value() lanza std::runtime_error si no hay valor (la MINI lanza esto;
       el std::expected real lanza std::bad_expected_access<E>).
     - value_or(default) NUNCA lanza: devuelve el valor o el default.
     - operator bool y has_value() son equivalentes.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../include solucion_10_mini_expected_basico.cpp -o sol10 && ./sol10
   ============================================================================= */
#include <iostream>
#include <string>
#include <stdexcept>
#include "../../include/mini_expected.hpp"
#include "../../include/test_framework.hpp"

mini::expected<int, std::string> parseEntero(const std::string& s) {
    if (s.empty()) return std::string("string vacío");
    for (char c : s) {
        if (c < '0' || c > '9') {
            return std::string("carácter inválido: ") + c;
        }
    }
    return std::stoi(s);
}

mini::expected<int, std::string> dividirSeguro(int a, int b) {
    if (b == 0) return std::string("división por cero");
    return a / b;
}

// ============= TESTS =============
TEST_CASE(TestConstruccionImplicitaOk) {
    mini::expected<int, std::string> e = 42;
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e.value(), 42);
    ASSERT_TRUE(static_cast<bool>(e));
}

TEST_CASE(TestConstruccionImplicitaError) {
    mini::expected<int, std::string> e = std::string("fallo");
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error(), std::string("fallo"));
    ASSERT_FALSE(static_cast<bool>(e));
}

TEST_CASE(TestConstruccionExplicitaUnExpect) {
    mini::expected<int, std::string> e(mini::unexpect, std::string("explícito"));
    ASSERT_FALSE(e.has_value());
    ASSERT_EQ(e.error(), std::string("explícito"));
}

TEST_CASE(TestParseEnteroExitoso) {
    auto r = parseEntero("123");
    ASSERT_TRUE(r.has_value());
    ASSERT_EQ(r.value(), 123);
}

TEST_CASE(TestParseEnteroVacio) {
    auto r = parseEntero("");
    ASSERT_FALSE(r.has_value());
    ASSERT_EQ(r.error(), std::string("string vacío"));
}

TEST_CASE(TestParseEnteroCaracterInvalido) {
    auto r = parseEntero("12a3");
    ASSERT_FALSE(r.has_value());
    ASSERT_TRUE(r.error().find("carácter inválido") != std::string::npos);
}

TEST_CASE(TestDividirSeguroExitoso) {
    auto r = dividirSeguro(10, 2);
    ASSERT_TRUE(r.has_value());
    ASSERT_EQ(r.value(), 5);
}

TEST_CASE(TestDividirSeguroPorCero) {
    auto r = dividirSeguro(10, 0);
    ASSERT_FALSE(r.has_value());
    ASSERT_EQ(r.error(), std::string("división por cero"));
}

TEST_CASE(TestValueOr) {
    auto r = parseEntero("abc");
    ASSERT_EQ(r.value_or(-1), -1);
    auto r2 = parseEntero("42");
    ASSERT_EQ(r2.value_or(-1), 42);
}

TEST_CASE(TestValueLanzaEnError) {
    auto r = parseEntero("");
    bool lanzo = false;
    try {
        r.value();
    } catch (const std::runtime_error&) {
        lanzo = true;
    }
    ASSERT_TRUE(lanzo);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 10 (mini::expected básico)...\n";
    return RUN_ALL_TESTS();
}