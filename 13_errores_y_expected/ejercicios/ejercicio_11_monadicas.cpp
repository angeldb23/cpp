/* =============================================================================
   MÓDULO 13 - EJERCICIO 11: Operaciones monádicas (and_then, transform, transform_error, or_else)
   =============================================================================
   DIFICULTAD: Intermedio-Avanzado
   CONSIGNA:
     Usando mini::expected, implementá una pipeline de procesamiento:
     
     Funciones base (ya dadas):
     - leerArchivo(ruta) -> expected<string, string> (contenido o error)
     - parseLineas(contenido) -> expected<vector<string>, string> (líneas no vacías)
     - validarLineas(lineas) -> expected<vector<string>, string> (cada línea: "clave=valor")
     - convertirAConfig(lineas) -> expected<Config, string> (struct con map)
     
     Tu tarea: encadenarlas con and_then (cada una devuelve expected).
     
     Luego agregá transformaciones:
     - transform: Config -> string (serializar para log)
     - transform_error: string -> ErrorUI (struct amigable para UI)
     - or_else: en caso de error, devolver Config por defecto
     
     Config:
     struct Config { std::map<std::string, std::string> datos; };
     
     ErrorUI:
     struct ErrorUI { std::string mensaje; bool recuperable; };
     
     Pipeline completa:
     auto resultado = leerArchivo("config.txt")
         .and_then(parseLineas)
         .and_then(validarLineas)
         .and_then(convertirAConfig)
         .transform(serializar)
         .transform_error(aErrorUI)
         .or_else(usarDefault);
     
     Probar con archivo válido, archivo inexistente, archivo con formato inválido.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../include ejercicio_11_monadicas.cpp -o ej11 && ./ej11
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include "../../include/mini_expected.hpp"
#include "../../include/test_framework.hpp"

// TODO: struct Config { std::map<std::string, std::string> datos; };
// TODO: struct ErrorUI { std::string mensaje; bool recuperable; };

// TODO: mini::expected<std::string, std::string> leerArchivo(const std::string& ruta);
// TODO: mini::expected<std::vector<std::string>, std::string> parseLineas(const std::string& contenido);
// TODO: mini::expected<std::vector<std::string>, std::string> validarLineas(const std::vector<std::string>& lineas);
// TODO: mini::expected<Config, std::string> convertirAConfig(const std::vector<std::string>& lineas);
// TODO: std::string serializar(const Config& cfg);
// TODO: ErrorUI aErrorUI(const std::string& error);
// TODO: mini::expected<Config, ErrorUI> usarDefault(const ErrorUI& error);

// ============= TESTS =============
TEST_CASE(TestPipelineExitosa) {
    // Crear archivo temporal válido
    // std::ofstream tmp("test_pipe.ini");
    // tmp << "puerto=8080\nhost=localhost\n";
    // tmp.close();
    // 
    // auto r = leerArchivo("test_pipe.ini")
    //     .and_then(parseLineas)
    //     .and_then(validarLineas)
    //     .and_then(convertirAConfig)
    //     .transform(serializar)
    //     .transform_error(aErrorUI)
    //     .or_else(usarDefault);
    // 
    // ASSERT_TRUE(r.has_value());
    // ASSERT_TRUE(r.value().find("puerto=8080") != std::string::npos);
    // std::remove("test_pipe.ini");
    ASSERT_TRUE(false);
}

TEST_CASE(TestPipelineArchivoNoExiste) {
    // auto r = leerArchivo("no_existe.ini")
    //     .and_then(parseLineas)
    //     .and_then(validarLineas)
    //     .and_then(convertirAConfig)
    //     .transform(serializar)
    //     .transform_error(aErrorUI)
    //     .or_else(usarDefault);
    // 
    // // or_else debe recuperar con default
    // ASSERT_TRUE(r.has_value());
    // ASSERT_EQ(r.value().datos.at("puerto"), "8080"); // default
    ASSERT_TRUE(false);
}

TEST_CASE(TestPipelineFormatoInvalido) {
    // std::ofstream tmp("bad_pipe.ini");
    // tmp << "puerto=abc\n"; // inválido
    // tmp.close();
    // 
    // auto r = leerArchivo("bad_pipe.ini")
    //     .and_then(parseLineas)
    //     .and_then(validarLineas)
    //     .and_then(convertirAConfig)
    //     .transform(serializar)
    //     .transform_error(aErrorUI)
    //     .or_else(usarDefault);
    // 
    // ASSERT_TRUE(r.has_value()); // or_else recupera
    // ASSERT_EQ(r.value().datos.at("puerto"), "8080");
    // std::remove("bad_pipe.ini");
    ASSERT_TRUE(false);
}

TEST_CASE(TestTransformCambiaTipo) {
    // mini::expected<int, std::string> e = 5;
    // auto e2 = e.transform([](int x) { return std::to_string(x); });
    // static_assert(std::is_same_v<decltype(e2), mini::expected<std::string, std::string>>);
    // ASSERT_EQ(e2.value(), "5");
    ASSERT_TRUE(false);
}

TEST_CASE(TestTransformErrorCambiaTipoError) {
    // mini::expected<int, std::string> e = std::string("fallo");
    // auto e2 = e.transform_error([](const std::string& s) { return ErrorUI{s, true}; });
    // static_assert(std::is_same_v<decltype(e2), mini::expected<int, ErrorUI>>);
    // ASSERT_FALSE(e2.has_value());
    // ASSERT_EQ(e2.error().mensaje, "fallo");
    // ASSERT_TRUE(e2.error().recuperable);
    ASSERT_TRUE(false);
}

TEST_CASE(TestAndThenEncadenaExpected) {
    // auto r = mini::expected<int, std::string>(10)
    //     .and_then([](int x) { return mini::expected<int, std::string>(x * 2); })
    //     .and_then([](int x) { return mini::expected<int, std::string>(x + 1); });
    // ASSERT_TRUE(r.has_value());
    // ASSERT_EQ(r.value(), 21);
    ASSERT_TRUE(false);
}

TEST_CASE(TestAndThenCortaEnError) {
    // bool segundo_corrio = false;
    // auto r = mini::expected<int, std::string>(std::string("error"))
    //     .and_then([&](int x) { segundo_corrio = true; return mini::expected<int, std::string>(x * 2); });
    // ASSERT_FALSE(r.has_value());
    // ASSERT_FALSE(segundo_corrio); // no se ejecutó
    ASSERT_TRUE(false);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 11 (Operaciones monádicas)...\n";
    return RUN_ALL_TESTS();
}