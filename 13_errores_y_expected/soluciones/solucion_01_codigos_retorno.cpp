/* =============================================================================
   MÓDULO 13 - SOLUCIÓN 01: Códigos de retorno con enum class
   =============================================================================
   POR QUÉ FUNCIONA:
     - enum class evita conversiones implícitas a int (type safety).
     - El código de retorno es el canal "A" de L02: visible, comprobable,
       pero requiere escribir el check en cada llamada.
     - El valor útil viaja por referencia de salida: es el patrón clásico C
       que std::expected (L10/L11) viene a reemplazar.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_01_codigos_retorno.cpp -o sol01 && ./sol01
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

enum class ConfigError {
    Ok,
    ArchivoNoEncontrado,
    FormatoInvalido,
    ClaveFaltante
};

ConfigError parseConfig(const std::string& archivo, const std::string& clave, int& resultado) {
    if (archivo != "config.ini") {
        return ConfigError::ArchivoNoEncontrado;
    }
    if (clave.empty()) {
        return ConfigError::ClaveFaltante;
    }
    if (clave != "puerto") {
        return ConfigError::FormatoInvalido;
    }
    resultado = 8080;
    return ConfigError::Ok;
}

// Helper para comparar enum class en tests
constexpr int toInt(ConfigError e) noexcept { return static_cast<int>(e); }

// ============= TESTS =============
TEST_CASE(TestArchivoValido) {
    int resultado = -1;
    ConfigError err = parseConfig("config.ini", "puerto", resultado);
    ASSERT_EQ(toInt(err), toInt(ConfigError::Ok));
    ASSERT_EQ(resultado, 8080);
}

TEST_CASE(TestArchivoNoEncontrado) {
    int resultado = -1;
    ConfigError err = parseConfig("otro.ini", "puerto", resultado);
    ASSERT_EQ(toInt(err), toInt(ConfigError::ArchivoNoEncontrado));
}

TEST_CASE(TestClaveVacia) {
    int resultado = -1;
    ConfigError err = parseConfig("config.ini", "", resultado);
    ASSERT_EQ(toInt(err), toInt(ConfigError::ClaveFaltante));
}

TEST_CASE(TestClaveInvalida) {
    int resultado = -1;
    ConfigError err = parseConfig("config.ini", "otra", resultado);
    ASSERT_EQ(toInt(err), toInt(ConfigError::FormatoInvalido));
}

TEST_CASE(TestResultadoNoModificadoEnError) {
    int resultado = 42;
    parseConfig("otro.ini", "puerto", resultado);
    ASSERT_EQ(resultado, 42); // No debe tocarse si hay error
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 01 (Códigos de retorno)...\n";
    return RUN_ALL_TESTS();
}