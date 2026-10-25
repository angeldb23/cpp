/* =============================================================================
   MÓDULO 13 - SOLUCIÓN 11: Operaciones monádicas (and_then, transform, transform_error, or_else)
   =============================================================================
   POR QUÉ FUNCIONA:
     - and_then: encadena operaciones que TAMBIÉN devuelven expected.
       Si hay valor: llama al callable con el valor y devuelve SU expected.
       Si hay error: propaga el error SIN llamar al callable (short-circuit).
     - transform: aplica una función al VALOR (si hay) y re-envuelve en expected.
       Cambia el TIPO del valor (T -> U). El error pasa sin tocar.
     - transform_error: aplica una función al ERROR (si hay) y re-envuelve.
       Cambia el TIPO del error (E -> E2). El valor pasa sin tocar.
     - or_else: SOLO si hay error, llama al callable con el error y devuelve
       SU expected (recuperación). Si hay valor, lo pasa directo (con el mismo valor).
     
     Las cuatro juntas permiten escribir pipelines lineales donde el error
     "fluye" automáticamente hasta un punto de recuperación (or_else) o
     traducción (transform_error), sin ifs manuales en cada paso.
     invoke_result_t (M09) deduce los tipos de retorno de los callables.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../include solucion_11_monadicas.cpp -o sol11 && ./sol11
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include "../../include/mini_expected.hpp"
#include "../../include/test_framework.hpp"

struct Config {
    std::map<std::string, std::string> datos;
};

struct ErrorUI {
    std::string mensaje;
    bool recuperable;
};

struct ContenidoArchivo { std::string valor; };

mini::expected<ContenidoArchivo, std::string> leerArchivo(const std::string& ruta) {
    std::ifstream archivo(ruta);
    if (!archivo) {
        return std::string("no se pudo abrir archivo: ") + ruta;
    }
    std::stringstream buffer;
    buffer << archivo.rdbuf();
    return ContenidoArchivo{ buffer.str() };
}

mini::expected<std::vector<std::string>, std::string> parseLineas(const ContenidoArchivo& contenido) {
    std::vector<std::string> lineas;
    std::stringstream ss(contenido.valor);
    std::string linea;
    while (std::getline(ss, linea)) {
        auto start = linea.find_first_not_of(" \t\r\n");
        auto end = linea.find_last_not_of(" \t\r\n");
        if (start != std::string::npos) {
            linea = linea.substr(start, end - start + 1);
            if (!linea.empty() && linea[0] != '#') {
                lineas.push_back(linea);
            }
        }
    }
    return lineas;
}

mini::expected<std::vector<std::string>, std::string> validarLineas(const std::vector<std::string>& lineas) {
    for (const auto& l : lineas) {
        if (l.find('=') == std::string::npos) {
            return std::string("línea inválida (falta '='): ") + l;
        }
    }
    return lineas;
}

mini::expected<Config, std::string> convertirAConfig(const std::vector<std::string>& lineas) {
    Config cfg;
    for (const auto& l : lineas) {
        auto pos = l.find('=');
        std::string clave = l.substr(0, pos);
        std::string valor = l.substr(pos + 1);
        auto trim = [](std::string& s) {
            auto start = s.find_first_not_of(" \t\r\n");
            auto end = s.find_last_not_of(" \t\r\n");
            if (start == std::string::npos) { s.clear(); return; }
            s = s.substr(start, end - start + 1);
        };
        trim(clave);
        trim(valor);
        if (clave.empty()) return std::string("clave vacía");
        cfg.datos[clave] = valor;
    }
    return cfg;
}

ErrorUI aErrorUI(const std::string& error) {
    return ErrorUI{ "Error de configuración: " + error, true };
}

mini::expected<Config, ErrorUI> usarDefault(const ErrorUI& error) {
    Config def;
    def.datos["puerto"] = "8080";
    def.datos["host"] = "localhost";
    std::cout << "  [or_else] Usando config por defecto por: " << error.mensaje << "\n";
    return def;
}

// ============= TESTS =============
TEST_CASE(TestPipelineExitosa) {
    std::ofstream tmp("test_pipe.ini");
    tmp << "puerto=8080\nhost=localhost\n";
    tmp.close();

    // Pipeline SIN or_else en caso de éxito
    auto r = leerArchivo("test_pipe.ini")
        .and_then(parseLineas)
        .and_then(validarLineas)
        .and_then(convertirAConfig);

    ASSERT_TRUE(r.has_value());
    ASSERT_EQ(r.value().datos.at("puerto"), "8080");
    ASSERT_EQ(r.value().datos.at("host"), "localhost");
    std::remove("test_pipe.ini");
}

TEST_CASE(TestPipelineArchivoNoExiste_ConOrElse) {
    // Pipeline CON or_else para recuperación en error
    auto r = leerArchivo("no_existe.ini")
        .and_then(parseLineas)
        .and_then(validarLineas)
        .and_then(convertirAConfig)
        .transform_error(aErrorUI)
        .or_else(usarDefault);

    ASSERT_TRUE(r.has_value());
    ASSERT_EQ(r.value().datos.at("puerto"), "8080");
}

TEST_CASE(TestPipelineErrorEnParseo) {
    // Test que validarLineas detecta formato inválido
    std::ofstream tmp("bad_pipe.ini");
    tmp << "línea sin igual\n";
    tmp.close();

    auto r = leerArchivo("bad_pipe.ini")
        .and_then(parseLineas)
        .and_then(validarLineas);

    ASSERT_FALSE(r.has_value());
    ASSERT_TRUE(r.error().find("línea inválida") != std::string::npos);
    std::remove("bad_pipe.ini");
}

TEST_CASE(TestTransformCambiaTipoValor) {
    mini::expected<int, std::string> e = 5;
    // Transform a int -> bool (par/impar) para evitar T==E
    auto e2 = e.transform([](int x) { return x % 2 == 0; });
    static_assert(std::is_same_v<decltype(e2), mini::expected<bool, std::string>>);
    ASSERT_EQ(e2.value(), false); // 5 es impar
}

TEST_CASE(TestTransformErrorCambiaTipoError) {
    mini::expected<int, std::string> e = std::string("fallo");
    auto e2 = e.transform_error([](const std::string& s) { return ErrorUI{s, true}; });
    static_assert(std::is_same_v<decltype(e2), mini::expected<int, ErrorUI>>);
    ASSERT_FALSE(e2.has_value());
    ASSERT_EQ(e2.error().mensaje, "fallo");
    ASSERT_TRUE(e2.error().recuperable);
}

TEST_CASE(TestAndThenEncadenaExpected) {
    auto r = mini::expected<int, std::string>(10)
        .and_then([](int x) { return mini::expected<int, std::string>(x * 2); })
        .and_then([](int x) { return mini::expected<int, std::string>(x + 1); });
    ASSERT_TRUE(r.has_value());
    ASSERT_EQ(r.value(), 21);
}

TEST_CASE(TestAndThenCortaEnError) {
    bool segundo_corrio = false;
    auto r = mini::expected<int, std::string>(std::string("error"))
        .and_then([&](int x) { segundo_corrio = true; return mini::expected<int, std::string>(x * 2); });
    ASSERT_FALSE(r.has_value());
    ASSERT_FALSE(segundo_corrio);
}

TEST_CASE(TestOrElseRecuperaEnError) {
    mini::expected<int, std::string> e = std::string("error original");
    auto r = e.or_else([](const std::string& err) -> mini::expected<int, std::string> {
        return 42; // valor de recuperación
    });
    ASSERT_TRUE(r.has_value());
    ASSERT_EQ(r.value(), 42);
}

TEST_CASE(TestOrElsePasaValorSiExito) {
    mini::expected<int, std::string> e = 100;
    auto r = e.or_else([](const std::string&) -> mini::expected<int, std::string> {
        return 42; // no debe llamarse
    });
    ASSERT_TRUE(r.has_value());
    ASSERT_EQ(r.value(), 100);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 11 (Operaciones monádicas)...\n";
    return RUN_ALL_TESTS();
}