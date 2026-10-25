/* =============================================================================
   MÓDULO 13 - EJERCICIO 01: Códigos de retorno con enum class
   =============================================================================
   DIFICULTAD: Básico
   CONSIGNA:
     Implementá un sistema de códigos de retorno para una operación de parsing
     de configuración.
     1. Definí un enum class ConfigError { Ok, ArchivoNoEncontrado, FormatoInvalido, ClaveFaltante }
     2. Implementá parseConfig(const std::string& archivo, const std::string& clave, int& resultado)
        que devuelve ConfigError y escribe el valor parseado en 'resultado' (salida por referencia).
     3. La función debe simular:
        - Si archivo == "config.ini" y clave == "puerto": resultado = 8080, Ok
        - Si archivo != "config.ini": ArchivoNoEncontrado
        - Si clave == "": ClaveFaltante
        - Si clave != "puerto": FormatoInvalido
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_01_codigos_retorno.cpp -o ej01 && ./ej01
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

// TODO: Definí el enum class ConfigError aquí
// enum class ConfigError { ... };

// TODO: Implementá parseConfig
// ConfigError parseConfig(const std::string& archivo, const std::string& clave, int& resultado) { ... }

// ============= TESTS =============
TEST_CASE(TestArchivoValido) {
    int resultado = -1;
    // ConfigError err = parseConfig("config.ini", "puerto", resultado);
    // ASSERT_EQ(err, ConfigError::Ok);
    // ASSERT_EQ(resultado, 8080);
    (void)resultado; // Evitar warning hasta implementar
    ASSERT_TRUE(false); // Placeholder: borralo al implementar
}

TEST_CASE(TestArchivoNoEncontrado) {
    int resultado = -1;
    // ConfigError err = parseConfig("otro.ini", "puerto", resultado);
    // ASSERT_EQ(err, ConfigError::ArchivoNoEncontrado);
    ASSERT_TRUE(false);
}

TEST_CASE(TestClaveVacia) {
    int resultado = -1;
    // ConfigError err = parseConfig("config.ini", "", resultado);
    // ASSERT_EQ(err, ConfigError::ClaveFaltante);
    ASSERT_TRUE(false);
}

TEST_CASE(TestClaveInvalida) {
    int resultado = -1;
    // ConfigError err = parseConfig("config.ini", "otra", resultado);
    // ASSERT_EQ(err, ConfigError::FormatoInvalido);
    ASSERT_TRUE(false);
}

TEST_CASE(TestResultadoNoModificadoEnError) {
    int resultado = 42;
    // parseConfig("otro.ini", "puerto", resultado);
    // ASSERT_EQ(resultado, 42); // No debe tocarse si hay error
    ASSERT_TRUE(false);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 01 (Códigos de retorno)...\n";
    return RUN_ALL_TESTS();
}