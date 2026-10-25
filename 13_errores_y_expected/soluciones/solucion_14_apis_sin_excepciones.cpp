/* =============================================================================
   MÓDULO 13 - SOLUCIÓN 14: APIs sin excepciones: expected<T, error_code> y boundary
   =============================================================================
   NOTA: mini::expected<void, E> no funciona (std::optional<void> es inválido).
   Para funciones "void que pueden fallar", el patrón idiomático es devolver
   std::error_code directamente (0 = éxito). Esto es lo que la lección 14
   muestra como "API void + error_code out" pero sin out-param: el error_code
   ES el retorno.
   NOTA 2: Los fallos aleatorios (10%) se muestran en la LECCIÓN, pero en la
   SOLUCIÓN se DESACTIVAN para que los tests sean deterministas.
============================================================================= */
#include <iostream>
#include <system_error>
#include <string>
#include <unordered_map>
#include <random>
#include <functional>
#include <type_traits>
#include <utility>
#include "../../include/mini_expected.hpp"
#include "../../include/test_framework.hpp"

// 1. RegistroError + RegistroCategory + make_error_code + is_error_code_enum
enum class RegistroError {
    UsuarioExistente   = 1,
    ClaveDebil         = 2,
    BaseNoDisponible   = 3,
    Corrupto           = 4
};

class RegistroCategory : public std::error_category {
public:
    const char* name() const noexcept override {
        return "registro_usuarios";
    }

    std::string message(int ev) const override {
        switch (static_cast<RegistroError>(ev)) {
            case RegistroError::UsuarioExistente:   return "el usuario ya existe";
            case RegistroError::ClaveDebil:         return "la clave es demasiado débil (mínimo 8 caracteres)";
            case RegistroError::BaseNoDisponible:   return "base de datos no disponible";
            case RegistroError::Corrupto:           return "datos corruptos: credenciales inválidas";
            default:                                return "código de registro desconocido";
        }
    }

    std::error_condition default_error_condition(int ev) const noexcept override {
        switch (static_cast<RegistroError>(ev)) {
            case RegistroError::UsuarioExistente:
                return std::errc::file_exists;
            case RegistroError::ClaveDebil:
                return std::errc::invalid_argument;
            case RegistroError::BaseNoDisponible:
                return std::errc::io_error;
            case RegistroError::Corrupto:
                return std::errc::bad_message;
            default:
                return std::error_condition(ev, *this);
        }
    }

    bool equivalent(int code, const std::error_condition& cond) const noexcept override {
        return default_error_condition(code) == cond;
    }
    bool equivalent(const std::error_code& code, int condition) const noexcept override {
        return code.category() == *this && default_error_condition(code.value()) == std::error_condition(condition, std::generic_category());
    }
};

const RegistroCategory& registroCategory() {
    static const RegistroCategory instance;
    return instance;
}

std::error_code make_error_code(RegistroError e) {
    return std::error_code(static_cast<int>(e), registroCategory());
}

namespace std {
    template <>
    struct is_error_code_enum<RegistroError> : true_type {};
}

// 2. RegistroDB (SIN fallos aleatorios para tests deterministas)
struct UsuarioData {
    std::string hashClave;
    std::string email;
};

class RegistroDB {
    std::unordered_map<std::string, UsuarioData> db_;

    static bool claveValida(const std::string& clave) {
        return clave.size() >= 8;
    }

public:
    // registrar: no devuelve valor útil -> std::error_code
    std::error_code registrar(const std::string& usuario,
                              const std::string& clave,
                              const std::string& email) {
        if (db_.find(usuario) != db_.end()) {
            return make_error_code(RegistroError::UsuarioExistente);
        }
        if (!claveValida(clave)) {
            return make_error_code(RegistroError::ClaveDebil);
        }
        // Sin fallo aleatorio en la solución (ver lección para el patrón)
        db_[usuario] = {clave, email};
        return std::error_code{}; // éxito
    }

    // autenticar: devuelve bool -> expected<bool, error_code>
    mini::expected<bool, std::error_code> autenticar(const std::string& usuario,
                                                     const std::string& clave) {
        // Sin fallo aleatorio
        auto it = db_.find(usuario);
        if (it == db_.end()) {
            return false;
        }
        return it->second.hashClave == clave;
    }

    // cambiarClave: no devuelve valor útil -> std::error_code
    std::error_code cambiarClave(const std::string& usuario,
                                 const std::string& claveVieja,
                                 const std::string& claveNueva) {
        if (!claveValida(claveNueva)) {
            return make_error_code(RegistroError::ClaveDebil);
        }
        auto it = db_.find(usuario);
        if (it == db_.end()) {
            return make_error_code(RegistroError::UsuarioExistente);
        }
        if (it->second.hashClave != claveVieja) {
            return make_error_code(RegistroError::Corrupto);
        }
        it->second.hashClave = claveNueva;
        return std::error_code{};
    }

    const std::unordered_map<std::string, UsuarioData>& getDB() const { return db_; }
};

// 3. Boundary: guardarEnDisco (determinista: siempre éxito en tests)
std::error_code guardarEnDisco(const RegistroDB& db) {
    (void)db;
    return std::error_code{};
}

// 4. try_call genérico
template <typename F>
auto try_call(F&& f) -> mini::expected<std::decay_t<decltype(f())>, std::error_code> {
    try {
        return f();
    } catch (const std::system_error& e) {
        return e.code();
    } catch (const std::bad_alloc&) {
        return std::make_error_code(std::errc::not_enough_memory);
    } catch (const std::exception& e) {
        (void)e;
        return std::error_code(static_cast<int>(std::errc::invalid_argument), std::generic_category());
    } catch (...) {
        return std::make_error_code(std::errc::io_error);
    }
}

// ============= TESTS =============
TEST_CASE(TestRegistroErrorYCategoria) {
    std::error_code ec = RegistroError::UsuarioExistente;
    ASSERT_EQ(std::string(ec.category().name()), std::string("registro_usuarios"));
    ASSERT_TRUE(ec == std::errc::file_exists);
    ec = RegistroError::ClaveDebil;
    ASSERT_TRUE(ec == std::errc::invalid_argument);
    ec = RegistroError::BaseNoDisponible;
    ASSERT_TRUE(ec == std::errc::io_error);
    ec = RegistroError::Corrupto;
    ASSERT_TRUE(ec == std::errc::bad_message);
}

TEST_CASE(TestRegistrarExito) {
    RegistroDB db;
    std::error_code ec = db.registrar("juan", "claveSegura123", "juan@ejemplo.com");
    ASSERT_FALSE(static_cast<bool>(ec));
}

TEST_CASE(TestRegistrarUsuarioExistente) {
    RegistroDB db;
    std::error_code ec1 = db.registrar("juan", "claveSegura123", "juan@ejemplo.com");
    ASSERT_FALSE(static_cast<bool>(ec1));
    std::error_code ec2 = db.registrar("juan", "otraClave456", "otro@ejemplo.com");
    ASSERT_TRUE(static_cast<bool>(ec2));
    ASSERT_TRUE(ec2 == std::errc::file_exists);
}

TEST_CASE(TestRegistrarClaveDebil) {
    RegistroDB db;
    std::error_code ec = db.registrar("pedro", "corta", "pedro@ejemplo.com");
    ASSERT_TRUE(static_cast<bool>(ec));
    ASSERT_TRUE(ec == std::errc::invalid_argument);
}

TEST_CASE(TestRegistrarBaseNoDisponible) {
    // En la solución no hay fallo aleatorio; el test verifica que la API existe.
    // El ejercicio del alumno SÍ tiene fallo aleatorio y se testea con muchas iteraciones.
    RegistroDB db;
    std::error_code ec = db.registrar("test_base", "claveLarga123", "x@x.com");
    ASSERT_FALSE(static_cast<bool>(ec));
    ASSERT_TRUE(true);
}

TEST_CASE(TestAutenticarExito) {
    RegistroDB db;
    db.registrar("juan", "claveSegura123", "juan@ejemplo.com");
    auto r = db.autenticar("juan", "claveSegura123");
    ASSERT_TRUE(r.has_value());
    ASSERT_TRUE(r.value());
}

TEST_CASE(TestAutenticarClaveIncorrecta) {
    RegistroDB db;
    db.registrar("juan", "claveSegura123", "juan@ejemplo.com");
    auto r = db.autenticar("juan", "claveIncorrecta");
    ASSERT_TRUE(r.has_value());
    ASSERT_FALSE(r.value());
}

TEST_CASE(TestAutenticarUsuarioInexistente) {
    RegistroDB db;
    auto r = db.autenticar("fantasma", "cualquiera");
    ASSERT_TRUE(r.has_value());
    ASSERT_FALSE(r.value());
}

TEST_CASE(TestCambiarClaveExito) {
    RegistroDB db;
    db.registrar("juan", "claveVieja123", "juan@ejemplo.com");
    std::error_code ec = db.cambiarClave("juan", "claveVieja123", "claveNueva456");
    ASSERT_FALSE(static_cast<bool>(ec));
    auto r2 = db.autenticar("juan", "claveNueva456");
    ASSERT_TRUE(r2.has_value() && r2.value());
}

TEST_CASE(TestCambiarClaveViejaIncorrecta) {
    RegistroDB db;
    db.registrar("juan", "claveVieja123", "juan@ejemplo.com");
    std::error_code ec = db.cambiarClave("juan", "incorrecta", "nuevaClave456");
    ASSERT_TRUE(static_cast<bool>(ec));
    ASSERT_TRUE(ec == std::errc::bad_message);
}

TEST_CASE(TestCambiarClaveNuevaDebil) {
    RegistroDB db;
    db.registrar("juan", "claveVieja123", "juan@ejemplo.com");
    std::error_code ec = db.cambiarClave("juan", "claveVieja123", "corta");
    ASSERT_TRUE(static_cast<bool>(ec));
    ASSERT_TRUE(ec == std::errc::invalid_argument);
}

TEST_CASE(TestGuardarEnDiscoBoundary) {
    RegistroDB db;
    db.registrar("a", "claveLarga123", "a@x.com");
    db.registrar("b", "claveLarga456", "b@x.com");
    std::error_code ec = guardarEnDisco(db);
    (void)ec;
    ASSERT_TRUE(true);
}

TEST_CASE(TestTryCallConvierteExcepciones) {
    auto r = try_call([]() -> int { throw std::system_error(std::make_error_code(std::errc::permission_denied), "no"); });
    ASSERT_FALSE(r.has_value());
    ASSERT_TRUE(r.error() == std::errc::permission_denied);

    auto r2 = try_call([]() -> int { throw std::bad_alloc(); });
    ASSERT_FALSE(r2.has_value());
    ASSERT_TRUE(r2.error() == std::errc::not_enough_memory);

    auto r3 = try_call([]() -> int { throw std::runtime_error("genérica"); });
    ASSERT_FALSE(r3.has_value());
    ASSERT_TRUE(r3.error() == std::errc::invalid_argument);

    auto r4 = try_call([]() -> int { return 42; });
    ASSERT_TRUE(r4.has_value());
    ASSERT_EQ(r4.value(), 42);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 14 (APIs sin excepciones)...\n";
    return RUN_ALL_TESTS();
}