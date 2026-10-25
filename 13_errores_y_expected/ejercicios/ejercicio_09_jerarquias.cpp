/* =============================================================================
   MÓDULO 13 - EJERCICIO 09: Jerarquías de error propias
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA:
     En L01 usamos enum class para errores simples. Para errores ricos con
     contexto (ruta de archivo, línea, errno, mensaje de usuario), usamos
     una JERARQUÍA DE CLASES (polimorfismo).
     
     Implementá:
     1. class ErrorBase (abstracta) con:
        - virtual std::string que() const = 0;  // mensaje técnico
        - virtual std::string mensajeUsuario() const = 0;  // para UI
        - virtual ~ErrorBase() = default;
     2. class ErrorArchivo : public ErrorBase
        - miembros: std::string ruta_, int errno_;
        - que(): "Error archivo 'ruta': strerror(errno)"
        - mensajeUsuario(): "No se pudo leer el archivo 'ruta'"
     3. class ErrorFormato : public ErrorBase
        - miembros: std::string ruta_, size_t linea_, std::string detalle_;
        - que(): "Error formato en 'ruta':línea: detalle"
        - mensajeUsuario(): "El archivo 'ruta' tiene formato inválido (línea N)"
     4. class ErrorRed : public ErrorBase
        - miembros: std::string host_, int puerto_, std::string detalle_;
        - que()/mensajeUsuario() apropiados
     
     5. Función parseConfig(const std::string& ruta) -> mini::expected<Config, std::unique_ptr<ErrorBase>>
        que usa la jerarquía y devuelve errores polimórficos.
     
     Config es un struct simple { int puerto; std::string host; }.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../include ejercicio_09_jerarquias.cpp -o ej09 && ./ej09
   ============================================================================= */
#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <cerrno>
#include "../../include/mini_expected.hpp"
#include "../../include/test_framework.hpp"

// TODO: struct Config { int puerto; std::string host; };

// TODO: class ErrorBase { ... };

// TODO: class ErrorArchivo : public ErrorBase { ... };

// TODO: class ErrorFormato : public ErrorBase { ... };

// TODO: class ErrorRed : public ErrorBase { ... };

// TODO: mini::expected<Config, std::unique_ptr<ErrorBase>> parseConfig(const std::string& ruta);

// ============= TESTS =============
TEST_CASE(TestErrorArchivoQue) {
    // auto err = std::make_unique<ErrorArchivo>("/etc/passwd", ENOENT);
    // ASSERT_TRUE(err->que().find("passwd") != std::string::npos);
    // ASSERT_TRUE(err->que().find("No such file") != std::string::npos);
    ASSERT_TRUE(false);
}

TEST_CASE(TestErrorArchivoMensajeUsuario) {
    // auto err = std::make_unique<ErrorArchivo>("/etc/passwd", ENOENT);
    // ASSERT_TRUE(err->mensajeUsuario().find("leer") != std::string::npos);
    ASSERT_TRUE(false);
}

TEST_CASE(TestErrorFormatoQue) {
    // auto err = std::make_unique<ErrorFormato>("config.ini", 5, "falta =");
    // ASSERT_TRUE(err->que().find("config.ini") != std::string::npos);
    // ASSERT_TRUE(err->que().find("5") != std::string::npos);
    // ASSERT_TRUE(err->que().find("falta =") != std::string::npos);
    ASSERT_TRUE(false);
}

TEST_CASE(TestErrorFormatoMensajeUsuario) {
    // auto err = std::make_unique<ErrorFormato>("config.ini", 5, "falta =");
    // ASSERT_TRUE(err->mensajeUsuario().find("inválido") != std::string::npos);
    // ASSERT_TRUE(err->mensajeUsuario().find("5") != std::string::npos);
    ASSERT_TRUE(false);
}

TEST_CASE(TestErrorRedQue) {
    // auto err = std::make_unique<ErrorRed>("localhost", 8080, "timeout");
    // ASSERT_TRUE(err->que().find("localhost") != std::string::npos);
    // ASSERT_TRUE(err->que().find("8080") != std::string::npos);
    ASSERT_TRUE(false);
}

TEST_CASE(TestParseConfigExitoso) {
    // // Crear archivo temporal o mock - para test simplificado:
    // // Verificamos que la firma compila y devuelve expected
    // // En test real se usaría filesystem
    ASSERT_TRUE(false);
}

TEST_CASE(TestParseConfigErrorArchivo) {
    // auto r = parseConfig("/no/existe.ini");
    // ASSERT_FALSE(r.has_value());
    // ASSERT_TRUE(dynamic_cast<ErrorArchivo*>(r.error().get()) != nullptr);
    ASSERT_TRUE(false);
}

TEST_CASE(TestParseConfigErrorFormato) {
    // // Simular archivo con formato inválido
    ASSERT_TRUE(false);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 09 (Jerarquías de error)...\n";
    return RUN_ALL_TESTS();
}