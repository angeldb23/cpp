/* =============================================================================
   MÓDULO 13 - EJERCICIO 14: APIs sin excepciones: expected<T, error_code> y boundary
   =============================================================================
   DIFICULTAD: Intermedio-Avanzado
   CONSIGNA:
     Implementá una mini-biblioteca de "registro de usuarios" que NO usa excepciones.
     Toda la API devuelve mini::expected<T, std::error_code> o std::error_code.

     1. Definí un enum class RegistroError { UsuarioExistente = 1, ClaveDebil = 2,
        BaseNoDisponible = 3, Corrupto = 4 } con su categoría propia
        (RegistroCategory) que mapea en default_error_condition:
          UsuarioExistente   -> errc::file_exists
          ClaveDebil         -> errc::invalid_argument
          BaseNoDisponible   -> errc::io_error
          Corrupto           -> errc::bad_message

     2. Implementá una "base de datos" en memoria: class RegistroDB
        - mapa: usuario -> (hash_clave, email)
        - métodos (todos devuelven expected<..., error_code> o error_code):
            * registrar(usuario, clave, email) -> expected<void, error_code>
              - falla UsuarioExistente si el usuario ya está
              - falla ClaveDebil si clave.length() < 8
              - falla BaseNoDisponible si falla la "escritura" (simulado: 10% aleatorio)
              - en éxito: guarda y devuelve void (expected vacío ok)
            * autenticar(usuario, clave) -> expected<bool, error_code>
              - falla BaseNoDisponible (10% aleatorio)
              - devuelve true si coincide, false si no (éxito con valor false)
            * cambiarClave(usuario, claveVieja, claveNueva) -> expected<void, error_code>
              - falla BaseNoDisponible (10%)
              - falla ClaveDebil si nueva < 8
              - falla UsuarioExistente si usuario no existe (reusar el código)
              - falla Corrupto si claveVieja no coincide

     3. Implementá un "boundary" que simula llamar a una librería C que usa errno:
        std::error_code guardarEnDisco(const RegistroDB&) -> error_code
        - Itera la DB y "escribe" (simula con 5% de fallo aleatorio por usuario)
        - En fallo: devuelve error_code(errno, system_category()) con EIO
        - En éxito: error_code{}

     4. Implementá un wrapper try_call_genérico (como en la lección 14) que
        atrapa CUALQUIER excepción y la convierte a error_code:
        template <typename F> auto try_call(F&& f) -> expected<decay_t<decltype(f())>, error_code>

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../include ejercicio_14_apis_sin_excepciones.cpp -o ej14 && ./ej14
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

// TODO 1: RegistroError + RegistroCategory + make_error_code + is_error_code_enum
// enum class RegistroError { ... };
// class RegistroCategory : public std::error_category { ... };
// const RegistroCategory& registroCategory();
// std::error_code make_error_code(RegistroError e);
// namespace std { template <> struct is_error_code_enum<RegistroError> : true_type {}; }

// TODO 2: class RegistroDB
// struct UsuarioData { std::string hashClave; std::string email; };
// class RegistroDB {
//     std::unordered_map<std::string, UsuarioData> db_;
//     std::mt19937 rng_{std::random_device{}()};
//     bool fallaAleatoria(double prob) { return std::uniform_real_distribution<>(0.0, 1.0)(rng_) < prob; }
// public:
//     mini::expected<void, std::error_code> registrar(const std::string& usuario,
//                                                     const std::string& clave,
//                                                     const std::string& email);
//     mini::expected<bool, std::error_code> autenticar(const std::string& usuario,
//                                                      const std::string& clave);
//     mini::expected<void, std::error_code> cambiarClave(const std::string& usuario,
//                                                        const std::string& claveVieja,
//                                                        const std::string& claveNueva);
// };

// TODO 3: boundary guardarEnDisco
// std::error_code guardarEnDisco(const RegistroDB& db);

// TODO 4: try_call genérico
// template <typename F>
// auto try_call(F&& f) -> mini::expected<std::decay_t<decltype(f())>, std::error_code>;

// ============= TESTS =============
TEST_CASE(TestRegistroErrorYCategoria) {
    // std::error_code ec = RegistroError::UsuarioExistente;
    // ASSERT_EQ(ec.category().name(), std::string("registro_usuarios"));
    // ASSERT_EQ(ec, std::errc::file_exists);
    // ec = RegistroError::ClaveDebil;
    // ASSERT_EQ(ec, std::errc::invalid_argument);
    // ec = RegistroError::BaseNoDisponible;
    // ASSERT_EQ(ec, std::errc::io_error);
    // ec = RegistroError::Corrupto;
    // ASSERT_EQ(ec, std::errc::bad_message);
    ASSERT_TRUE(false);
}

TEST_CASE(TestRegistrarExito) {
    // RegistroDB db;
    // auto r = db.registrar("juan", "claveSegura123", "juan@ejemplo.com");
    // ASSERT_TRUE(r.has_value());
    ASSERT_TRUE(false);
}

TEST_CASE(TestRegistrarUsuarioExistente) {
    // RegistroDB db;
    // db.registrar("juan", "claveSegura123", "juan@ejemplo.com");
    // auto r = db.registrar("juan", "otraClave456", "otro@ejemplo.com");
    // ASSERT_FALSE(r.has_value());
    // ASSERT_EQ(r.error(), std::errc::file_exists); // vía default_error_condition
    ASSERT_TRUE(false);
}

TEST_CASE(TestRegistrarClaveDebil) {
    // RegistroDB db;
    // auto r = db.registrar("pedro", "corta", "pedro@ejemplo.com");
    // ASSERT_FALSE(r.has_value());
    // ASSERT_EQ(r.error(), std::errc::invalid_argument);
    ASSERT_TRUE(false);
}

TEST_CASE(TestRegistrarBaseNoDisponible) {
    // RegistroDB db;
    // // Forzar fallo aleatorio: registrar muchas veces hasta que falle
    // // (el test puede ser flaky; en la práctica se mockea el RNG)
    // // Para el test: asumimos que la implementación permite inyectar fallo
    // // o corremos varias veces. Aquí placeholder.
    // ASSERT_TRUE(false);
    ASSERT_TRUE(false);
}

TEST_CASE(TestAutenticarExito) {
    // RegistroDB db;
    // db.registrar("juan", "claveSegura123", "juan@ejemplo.com");
    // auto r = db.autenticar("juan", "claveSegura123");
    // ASSERT_TRUE(r.has_value());
    // ASSERT_TRUE(r.value());
    ASSERT_TRUE(false);
}

TEST_CASE(TestAutenticarClaveIncorrecta) {
    // RegistroDB db;
    // db.registrar("juan", "claveSegura123", "juan@ejemplo.com");
    // auto r = db.autenticar("juan", "claveIncorrecta");
    // ASSERT_TRUE(r.has_value()); // éxito: devuelve false como valor
    // ASSERT_FALSE(r.value());
    ASSERT_TRUE(false);
}

TEST_CASE(TestAutenticarUsuarioInexistente) {
    // RegistroDB db;
    // auto r = db.autenticar("fantasma", "cualquiera");
    // ASSERT_TRUE(r.has_value());
    // ASSERT_FALSE(r.value());
    ASSERT_TRUE(false);
}

TEST_CASE(TestCambiarClaveExito) {
    // RegistroDB db;
    // db.registrar("juan", "claveVieja123", "juan@ejemplo.com");
    // auto r = db.cambiarClave("juan", "claveVieja123", "claveNueva456");
    // ASSERT_TRUE(r.has_value());
    // // Verificar que la nueva funciona
    // auto r2 = db.autenticar("juan", "claveNueva456");
    // ASSERT_TRUE(r2.has_value() && r2.value());
    ASSERT_TRUE(false);
}

TEST_CASE(TestCambiarClaveViejaIncorrecta) {
    // RegistroDB db;
    // db.registrar("juan", "claveVieja123", "juan@ejemplo.com");
    // auto r = db.cambiarClave("juan", "incorrecta", "nuevaClave456");
    // ASSERT_FALSE(r.has_value());
    // ASSERT_EQ(r.error(), std::errc::bad_message); // Corrupto -> bad_message
    ASSERT_TRUE(false);
}

TEST_CASE(TestCambiarClaveNuevaDebil) {
    // RegistroDB db;
    // db.registrar("juan", "claveVieja123", "juan@ejemplo.com");
    // auto r = db.cambiarClave("juan", "claveVieja123", "corta");
    // ASSERT_FALSE(r.has_value());
    // ASSERT_EQ(r.error(), std::errc::invalid_argument);
    ASSERT_TRUE(false);
}

TEST_CASE(TestGuardarEnDiscoBoundary) {
    // RegistroDB db;
    // db.registrar("a", "claveLarga123", "a@x.com");
    // db.registrar("b", "claveLarga456", "b@x.com");
    // std::error_code ec = guardarEnDisco(db);
    // // Puede fallar aleatoriamente; si no falla, ec == 0
    // // El test verifica que compila y devuelve error_code
    // (void)ec;
    // ASSERT_TRUE(true); // placeholder: que compile y corra
    ASSERT_TRUE(false);
}

TEST_CASE(TestTryCallConvierteExcepciones) {
    // auto r = try_call([]() -> int { throw std::system_error(std::make_error_code(std::errc::permission_denied), "no"); });
    // ASSERT_FALSE(r.has_value());
    // ASSERT_EQ(r.error(), std::errc::permission_denied);

    // auto r2 = try_call([]() -> int { throw std::bad_alloc(); });
    // ASSERT_FALSE(r2.has_value());
    // ASSERT_EQ(r2.error(), std::errc::not_enough_memory);

    // auto r3 = try_call([]() -> int { throw std::runtime_error("genérica"); });
    // ASSERT_FALSE(r3.has_value());
    // ASSERT_EQ(r3.error(), std::errc::invalid_argument);

    // auto r4 = try_call([]() -> int { return 42; });
    // ASSERT_TRUE(r4.has_value());
    // ASSERT_EQ(r4.value(), 42);
    ASSERT_TRUE(false);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 14 (APIs sin excepciones)...\n";
    return RUN_ALL_TESTS();
}