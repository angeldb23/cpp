/* =============================================================================
   MÓDULO 13 - SOLUCIÓN 09: Jerarquías de error propias
   =============================================================================
   POR QUÉ FUNCIONA:
     - enum class (L01) es bueno para códigos simples, pero no lleva contexto.
     - Una jerarquía de clases (polimorfismo) permite adjuntar datos ricos:
       ruta, línea, errno, host, puerto, stack trace, etc.
     - std::unique_ptr<ErrorBase> como tipo E en expected permite guardar
       CUALQUIER error derivado (slicing evitado por puntero).
     - El que recibe el expected puede:
       - error()->que() para log técnico
       - error()->mensajeUsuario() para mostrar al usuario
       - dynamic_cast si necesita manejar tipos específicos distinto.
     - virtual destructor en la base es OBLIGATORIO para delete correcto
       a través de puntero base (unique_ptr<ErrorBase>).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../include solucion_09_jerarquias.cpp -o sol09 && ./sol09
   ============================================================================= */
#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <cerrno>
#include <fstream>
#include <sstream>
#include "../../include/mini_expected.hpp"
#include "../../include/test_framework.hpp"

struct Config {
    int puerto = 8080;
    std::string host = "localhost";
};

class ErrorBase {
public:
    virtual ~ErrorBase() = default;
    virtual std::string que() const = 0;
    virtual std::string mensajeUsuario() const = 0;
};

class ErrorArchivo : public ErrorBase {
public:
    ErrorArchivo(std::string ruta, int errnum) : ruta_(std::move(ruta)), errno_(errnum) {}

    std::string que() const override {
        return "Error archivo '" + ruta_ + "': " + std::strerror(errno_);
    }

    std::string mensajeUsuario() const override {
        return "No se pudo leer el archivo '" + ruta_ + "'";
    }

    const std::string& ruta() const noexcept { return ruta_; }
    int errnoNum() const noexcept { return errno_; }

private:
    std::string ruta_;
    int errno_;
};

class ErrorFormato : public ErrorBase {
public:
    ErrorFormato(std::string ruta, std::size_t linea, std::string detalle)
        : ruta_(std::move(ruta)), linea_(linea), detalle_(std::move(detalle)) {}

    std::string que() const override {
        return "Error formato en '" + ruta_ + "' línea " + std::to_string(linea_) + ": " + detalle_;
    }

    std::string mensajeUsuario() const override {
        return "El archivo '" + ruta_ + "' tiene formato inválido (línea " + std::to_string(linea_) + ")";
    }

    const std::string& ruta() const noexcept { return ruta_; }
    std::size_t linea() const noexcept { return linea_; }
    const std::string& detalle() const noexcept { return detalle_; }

private:
    std::string ruta_;
    std::size_t linea_;
    std::string detalle_;
};

class ErrorRed : public ErrorBase {
public:
    ErrorRed(std::string host, int puerto, std::string detalle)
        : host_(std::move(host)), puerto_(puerto), detalle_(std::move(detalle)) {}

    std::string que() const override {
        return "Error red conectando a " + host_ + ":" + std::to_string(puerto_) + ": " + detalle_;
    }

    std::string mensajeUsuario() const override {
        return "No se pudo conectar a " + host_ + ":" + std::to_string(puerto_);
    }

    const std::string& host() const noexcept { return host_; }
    int puerto() const noexcept { return puerto_; }
    const std::string& detalle() const noexcept { return detalle_; }

private:
    std::string host_;
    int puerto_;
    std::string detalle_;
};

// Helper para construir expected con error polimórfico
template<typename Derived, typename... Args>
auto make_error(Args&&... args) {
    return std::unique_ptr<ErrorBase>(std::make_unique<Derived>(std::forward<Args>(args)...));
}

mini::expected<Config, std::unique_ptr<ErrorBase>> parseConfig(const std::string& ruta) {
    std::ifstream archivo(ruta);
    if (!archivo) {
        return make_error<ErrorArchivo>(ruta, errno);
    }

    Config cfg;
    std::string linea;
    std::size_t num_linea = 0;
    bool tiene_puerto = false, tiene_host = false;

    while (std::getline(archivo, linea)) {
        ++num_linea;
        if (linea.empty() || linea[0] == '#') continue;

        auto pos = linea.find('=');
        if (pos == std::string::npos) {
            return make_error<ErrorFormato>(ruta, num_linea, "falta '=' en la línea");
        }

        std::string clave = linea.substr(0, pos);
        std::string valor = linea.substr(pos + 1);

        // Trim simple
        auto trim = [](std::string& s) {
            auto start = s.find_first_not_of(" \t\r\n");
            auto end = s.find_last_not_of(" \t\r\n");
            if (start == std::string::npos) { s.clear(); return; }
            s = s.substr(start, end - start + 1);
        };
        trim(clave);
        trim(valor);

        if (clave == "puerto") {
            try {
                cfg.puerto = std::stoi(valor);
                tiene_puerto = true;
            } catch (...) {
                return make_error<ErrorFormato>(ruta, num_linea, "puerto inválido: " + valor);
            }
        } else if (clave == "host") {
            cfg.host = valor;
            tiene_host = true;
        } else {
            return make_error<ErrorFormato>(ruta, num_linea, "clave desconocida: " + clave);
        }
    }

    if (!tiene_puerto) return make_error<ErrorFormato>(ruta, num_linea, "falta clave 'puerto'");
    if (!tiene_host) return make_error<ErrorFormato>(ruta, num_linea, "falta clave 'host'");

    return cfg;
}

// ============= TESTS =============
TEST_CASE(TestErrorArchivoQue) {
    auto err = std::make_unique<ErrorArchivo>("/etc/passwd", ENOENT);
    ASSERT_TRUE(err->que().find("passwd") != std::string::npos);
    ASSERT_TRUE(err->que().find("No such file") != std::string::npos);
}

TEST_CASE(TestErrorArchivoMensajeUsuario) {
    auto err = std::make_unique<ErrorArchivo>("/etc/passwd", ENOENT);
    ASSERT_TRUE(err->mensajeUsuario().find("leer") != std::string::npos);
}

TEST_CASE(TestErrorFormatoQue) {
    auto err = std::make_unique<ErrorFormato>("config.ini", 5, "falta =");
    ASSERT_TRUE(err->que().find("config.ini") != std::string::npos);
    ASSERT_TRUE(err->que().find("5") != std::string::npos);
    ASSERT_TRUE(err->que().find("falta =") != std::string::npos);
}

TEST_CASE(TestErrorFormatoMensajeUsuario) {
    auto err = std::make_unique<ErrorFormato>("config.ini", 5, "falta =");
    ASSERT_TRUE(err->mensajeUsuario().find("inválido") != std::string::npos);
    ASSERT_TRUE(err->mensajeUsuario().find("5") != std::string::npos);
}

TEST_CASE(TestErrorRedQue) {
    auto err = std::make_unique<ErrorRed>("localhost", 8080, "timeout");
    ASSERT_TRUE(err->que().find("localhost") != std::string::npos);
    ASSERT_TRUE(err->que().find("8080") != std::string::npos);
}

TEST_CASE(TestParseConfigExitoso) {
    // Crear archivo temporal
    std::ofstream tmp("test_config.ini");
    tmp << "puerto=9000\nhost=mihost\n";
    tmp.close();

    auto r = parseConfig("test_config.ini");
    ASSERT_TRUE(r.has_value());
    ASSERT_EQ(r.value().puerto, 9000);
    ASSERT_EQ(r.value().host, "mihost");

    std::remove("test_config.ini");
}

TEST_CASE(TestParseConfigErrorArchivo) {
    auto r = parseConfig("/no/existe.ini");
    ASSERT_FALSE(r.has_value());
    ASSERT_TRUE(dynamic_cast<ErrorArchivo*>(r.error().get()) != nullptr);
}

TEST_CASE(TestParseConfigErrorFormato) {
    std::ofstream tmp("bad_config.ini");
    tmp << "puerto=abc\n";  // inválido
    tmp.close();

    auto r = parseConfig("bad_config.ini");
    ASSERT_FALSE(r.has_value());
    ASSERT_TRUE(dynamic_cast<ErrorFormato*>(r.error().get()) != nullptr);

    std::remove("bad_config.ini");
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 09 (Jerarquías de error)...\n";
    return RUN_ALL_TESTS();
}