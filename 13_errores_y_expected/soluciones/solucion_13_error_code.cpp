/* =============================================================================
   MÓDULO 13 - SOLUCIÓN 13: std::error_code, std::errc y categoría propia
   ============================================================================= */
#include <iostream>
#include <system_error>
#include <string>
#include "../../include/test_framework.hpp"

// 1. Enum class RedError
enum class RedError {
    ConexionRechazada = 1,
    Timeout           = 2,
    ProtocoloInvalido = 3,
    ServidorOcupado   = 4
};

// 2. Especialización de is_error_code_enum y make_error_code (forward)
namespace std {
    template <>
    struct is_error_code_enum<RedError> : true_type {};
}

// 3. Categoría propia
class RedCategory : public std::error_category {
public:
    const char* name() const noexcept override {
        return "red_cliente";
    }

    std::string message(int ev) const override {
        switch (static_cast<RedError>(ev)) {
            case RedError::ConexionRechazada:  return "conexión rechazada por el servidor";
            case RedError::Timeout:            return "tiempo de espera agotado";
            case RedError::ProtocoloInvalido:  return "protocolo inválido: handshake fallido";
            case RedError::ServidorOcupado:    return "servidor ocupado: reintente más tarde";
            default:                           return "código de red desconocido";
        }
    }

    std::error_condition default_error_condition(int ev) const noexcept override {
        switch (static_cast<RedError>(ev)) {
            case RedError::ConexionRechazada:
                return std::errc::connection_refused;
            case RedError::Timeout:
                return std::errc::timed_out;
            case RedError::ProtocoloInvalido:
                return std::errc::protocol_error;
            case RedError::ServidorOcupado:
                return std::errc::resource_unavailable_try_again;
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

// Instancia singleton de la categoría
const RedCategory& redCategory() {
    static const RedCategory instance;
    return instance;
}

// make_error_code para RedError
std::error_code make_error_code(RedError e) {
    return std::error_code(static_cast<int>(e), redCategory());
}

// 4. Función conectar
std::error_code conectar(const std::string& host, int puerto, RedError simularError) {
    (void)host; (void)puerto; // no usados en la simulación
    switch (simularError) {
        case RedError::ConexionRechazada:
            return std::error_code(ECONNREFUSED, std::system_category());
        case RedError::Timeout:
            return std::error_code(ETIMEDOUT, std::system_category());
        case RedError::ProtocoloInvalido:
            return make_error_code(RedError::ProtocoloInvalido);
        case RedError::ServidorOcupado:
            return make_error_code(RedError::ServidorOcupado);
        default:
            return std::error_code{}; // éxito
    }
}

// ============= TESTS =============
TEST_CASE(TestRedErrorEnumValores) {
    static_assert(static_cast<int>(RedError::ConexionRechazada) == 1);
    static_assert(static_cast<int>(RedError::Timeout) == 2);
    static_assert(static_cast<int>(RedError::ProtocoloInvalido) == 3);
    static_assert(static_cast<int>(RedError::ServidorOcupado) == 4);
    ASSERT_TRUE(true);
}

TEST_CASE(TestIsErrorCodeEnum) {
    std::error_code ec = RedError::Timeout;
    ASSERT_EQ(ec.value(), 2);
    ASSERT_EQ(std::string(ec.category().name()), std::string("red_cliente"));
}

TEST_CASE(TestRedCategoryName) {
    const RedCategory& cat = redCategory();
    ASSERT_EQ(std::string(cat.name()), "red_cliente");
}

TEST_CASE(TestRedCategoryMessages) {
    const RedCategory& cat = redCategory();
    ASSERT_EQ(cat.message(static_cast<int>(RedError::ConexionRechazada)), "conexión rechazada por el servidor");
    ASSERT_EQ(cat.message(static_cast<int>(RedError::Timeout)), "tiempo de espera agotado");
    ASSERT_EQ(cat.message(static_cast<int>(RedError::ProtocoloInvalido)), "protocolo inválido: handshake fallido");
    ASSERT_EQ(cat.message(static_cast<int>(RedError::ServidorOcupado)), "servidor ocupado: reintente más tarde");
}

TEST_CASE(TestDefaultErrorConditionMapping) {
    const RedCategory& cat = redCategory();
    ASSERT_EQ(cat.default_error_condition(static_cast<int>(RedError::ConexionRechazada)).value(), static_cast<int>(std::errc::connection_refused));
    ASSERT_EQ(cat.default_error_condition(static_cast<int>(RedError::Timeout)).value(), static_cast<int>(std::errc::timed_out));
    ASSERT_EQ(cat.default_error_condition(static_cast<int>(RedError::ProtocoloInvalido)).value(), static_cast<int>(std::errc::protocol_error));
    ASSERT_EQ(cat.default_error_condition(static_cast<int>(RedError::ServidorOcupado)).value(), static_cast<int>(std::errc::resource_unavailable_try_again));
}

TEST_CASE(TestConectarExito) {
    std::error_code ec_ok = conectar("localhost", 8080, static_cast<RedError>(99));
    ASSERT_FALSE(static_cast<bool>(ec_ok));
}

TEST_CASE(TestConectarConexionRechazadaSystemCategory) {
    std::error_code ec = conectar("localhost", 8080, RedError::ConexionRechazada);
    ASSERT_TRUE(static_cast<bool>(ec));
    ASSERT_EQ(std::string(ec.category().name()), std::string("system"));
    ASSERT_EQ(ec.value(), ECONNREFUSED);
}

TEST_CASE(TestConectarTimeoutSystemCategory) {
    std::error_code ec = conectar("localhost", 8080, RedError::Timeout);
    ASSERT_TRUE(static_cast<bool>(ec));
    ASSERT_EQ(std::string(ec.category().name()), std::string("system"));
    ASSERT_EQ(ec.value(), ETIMEDOUT);
}

TEST_CASE(TestConectarProtocoloInvalidoPropia) {
    std::error_code ec = conectar("localhost", 8080, RedError::ProtocoloInvalido);
    ASSERT_TRUE(static_cast<bool>(ec));
    ASSERT_EQ(std::string(ec.category().name()), std::string("red_cliente"));
    ASSERT_EQ(ec.value(), static_cast<int>(RedError::ProtocoloInvalido));
    ASSERT_TRUE(ec == std::errc::protocol_error);
}

TEST_CASE(TestConectarServidorOcupadoPropia) {
    std::error_code ec = conectar("localhost", 8080, RedError::ServidorOcupado);
    ASSERT_TRUE(static_cast<bool>(ec));
    ASSERT_EQ(std::string(ec.category().name()), std::string("red_cliente"));
    ASSERT_TRUE(ec == std::errc::resource_unavailable_try_again);
}

TEST_CASE(TestComparacionConErrc) {
    std::error_code ec = make_error_code(RedError::ConexionRechazada);
    ASSERT_TRUE(ec == std::errc::connection_refused);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 13 (error_code y categoría propia)...\n";
    return RUN_ALL_TESTS();
}