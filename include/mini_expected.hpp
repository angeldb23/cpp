// mini_expected.hpp: la implementación propia de expected<T, E> para C++20.
// -----------------------------------------------------------------------------
// La API es la de std::expected (C++23) reducida a la API que el tramo
// del curso necesita: valores copiables y errores copiables, con las
// cuatro operaciones monádicas (and_then, transform, transform_error y
// or_else) y el mismo espíritu de etiquetas (unexpect) y acceso
// (has_value/value/error/value_or).
//
// Compilar: es un header puro C++20 (sin <expected>): sirve para
// practicar expected en el módulo 13 sin el flag -std=c++23.
// El std::expected real usa una unión; acá usamos dos optionales para
// que el código sea legible (la lección 11 cuenta la diferencia).
#pragma once
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace mini {

// La etiqueta de "construir como ERROR" (como std::unexpect de C++23).
struct unexpected_t { explicit unexpected_t() = default; };
inline constexpr unexpected_t unexpect{};

template <typename T, typename E>
class expected {
public:
    // Construcción implícita "como ok" y "como error": la firma
    // expected<Punto, Codigo> se alimenta con Punto{} o con Codigo::x.
    // Usamos SFINAE para desambiguar cuando T y E son el mismo tipo.
    template <typename U = T, std::enable_if_t<!std::is_same_v<std::decay_t<U>, E>, int> = 0>
    expected(U&& valor) : valor_(std::forward<U>(valor)) {}
    
    template <typename U = E, std::enable_if_t<!std::is_same_v<std::decay_t<U>, T>, int> = 0>
    expected(U&& error) : error_(std::forward<U>(error)) {}
    
    // Cuando T == E, necesitamos constructores explícitos o tag dispatch.
    // Para simplificar, si T==E, la construcción implícita no está disponible
    // y se debe usar mini::unexpect para errores.

    // Copy/move: los optionales ya lo traen (es un T y un E en una caja).
    expected(const expected&) = default;
    expected(expected&&) = default;

    // Construcción explícita como error: expected<int, Codigo>(
    // mini::unexpect, Codigo::FormatoInvalido)
    template <typename E2>
    expected(unexpected_t, E2 error) : error_(std::move(error)) {}

    // ... y el par de consultas que definen a un expected:
    bool has_value() const noexcept { return valor_.has_value(); }
    explicit operator bool() const noexcept { return has_value(); }

    const T& value() const {
        if (!has_value()) {
            throw std::runtime_error("value() sobre un expected con error: "
                                     + descripcionError());
        }
        return *valor_;
    }
    T& value() {
        if (!has_value()) {
            throw std::runtime_error("value() sobre un expected con error: "
                                     + descripcionError());
        }
        return *valor_;
    }

    const E& error() const& { return *error_; }
    E error() && { return std::move(*error_); }

    // value_or: el "con fallback" que evita un if en el llamador.
    T value_or(const T& porDefecto) const {
        return has_value() ? *valor_ : porDefecto;
    }

    // Monádicas (L12). Firma "const&" simplificada: los mini tipos del
    // curso (int, string, structs) son copiables, acá se apuesta a eso.
    // and_then:-success -> otro expected<U,E> (encadenar operaciones que
    // TAMBIÉN fallan). Solo corre si hay valor.
    template <typename F>
    auto and_then(F&& onSuccess) const& {
        using Res = std::decay_t<std::invoke_result_t<F&, const T&>>;
        if (has_value()) return std::invoke(std::forward<F>(onSuccess), *valor_);
        return Res(unexpect, *error_);
    }

    // transform: éxito -> un VALOR del nuevo tipo (lo "envuelve" acá
    // en el expected nuevo). Solo corre si hay valor.
    template <typename F>
    auto transform(F&& onSuccess) const& {
        using U = std::decay_t<std::invoke_result_t<F&, const T&>>;
        if (has_value()) return expected<U, E>(std::invoke(std::forward<F>(onSuccess), *valor_));
        return expected<U, E>(unexpect, *error_);
    }

    // transform_error: cambia el TIPO del error. Solo corre si hubo error.
    template <typename F>
    auto transform_error(F&& onError) const& {
        using E2 = std::decay_t<std::invoke_result_t<F&, const E&>>;
        if (has_value()) return expected<T, E2>(*valor_);
        return expected<T, E2>(std::invoke(std::forward<F>(onError), *error_));
    }

    // or_else: corre UNA vegA solo si hubo error (el "arreglo local").
    template <typename F>
    auto or_else(F&& onError) const& {
        using Res = std::decay_t<std::invoke_result_t<F&, const E&>>;
        if (has_value()) return Res(*valor_);
        return std::invoke(std::forward<F>(onError), *error_);
    }

private:
    std::string descripcionError() const {
        // mini: si el E no imprime (p.ej. un enum), queda la descripción plana:
        if constexpr (std::is_convertible_v<E, std::string>) {
            return *error_;
        }
        return "el error de la clase no imprime";
    }

    std::optional<T> valor_;
    std::optional<E> error_;
};

} // namespace mini
