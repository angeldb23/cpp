/* =============================================================================
   MÓDULO 09 - SOLUCIÓN 08: Traits, if constexpr y SFINAE clásico
   =============================================================================
   POR QUÉ FUNCIONA:
     - Los traits de <type_traits> responden preguntas sobre tipos en tiempo de compilación.
     - void_t<expr> es 'void si expr es válida, error de sustitución si no': base del detection idiom.
     - if constexpr descarta las ramas no elegidas; un if común las compilaría todas y fallaría.
     - enable_if_t en el retorno hace que solo una sobrecarga de mitad sea candidata por tipo (SFINAE).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_08_traits_y_sfinae.cpp -o sol08 && ./sol08
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include <type_traits>
#include <utility>
#include "../../include/test_framework.hpp"

// remove_reference_t saca el &; remove_cv_t saca const y volatile.
// El orden importa: "const int&" es una referencia a const int, así que
// primero hay que sacar la referencia y recién después el const.
template <typename T>
using Limpio = std::remove_cv_t<std::remove_reference_t<T>>;

// Detection idiom: si decltype(declval<T>().size()) es inválido, la
// especialización se descarta (SFINAE) y queda la primaria (false_type).
template <typename T, typename = void>
struct TieneSize : std::false_type {};

template <typename T>
struct TieneSize<T, std::void_t<decltype(std::declval<T>().size())>> : std::true_type {};

// if constexpr: las ramas que no corresponden NO se compilan para ese T.
template <typename T>
std::string clasificar(const T&) {
    if constexpr (std::is_integral_v<T>) {
        return "entero";
    } else if constexpr (std::is_floating_point_v<T>) {
        return "decimal";
    } else if constexpr (std::is_pointer_v<T>) {
        return "puntero";
    } else if constexpr (TieneSize<T>::value) {
        return "coleccion";
    } else {
        return "otro";
    }
}

// SFINAE clásico: para cada T solo UNA de las dos firmas es válida.
template <typename T>
std::enable_if_t<std::is_integral_v<T>, T> mitad(T x) {
    return x / 2;
}

template <typename T>
std::enable_if_t<std::is_floating_point_v<T>, T> mitad(T x) {
    return x * static_cast<T>(0.5);
}

// ============= TESTS (los mismos del ejercicio) =============
struct Vacio {};

TEST_CASE(TestLimpio) {
    ASSERT_TRUE((std::is_same_v<Limpio<const int&>, int>));
    ASSERT_TRUE((std::is_same_v<Limpio<int&&>, int>));
    ASSERT_TRUE((std::is_same_v<Limpio<volatile double>, double>));
    ASSERT_TRUE((std::is_same_v<Limpio<int>, int>));
}

TEST_CASE(TestTieneSize) {
    ASSERT_TRUE(TieneSize<std::vector<int>>::value);
    ASSERT_TRUE(TieneSize<std::string>::value);
    ASSERT_FALSE(TieneSize<int>::value);
    ASSERT_FALSE(TieneSize<Vacio>::value);
}

TEST_CASE(TestClasificar) {
    std::vector<int> v = {1, 2};
    const char* texto = "hola";
    ASSERT_EQ(clasificar(5), std::string("entero"));
    ASSERT_EQ(clasificar(2.5), std::string("decimal"));
    ASSERT_EQ(clasificar(texto), std::string("puntero"));
    ASSERT_EQ(clasificar(v), std::string("coleccion"));
    ASSERT_EQ(clasificar(std::string("x")), std::string("coleccion"));
    ASSERT_EQ(clasificar(Vacio{}), std::string("otro"));
}

TEST_CASE(TestMitad) {
    ASSERT_EQ(mitad(9), 4);          // división entera
    ASSERT_EQ(mitad(10), 5);
    ASSERT_NEAR(mitad(9.0), 4.5, 1e-9);
    ASSERT_NEAR(mitad(3.0f), 1.5, 1e-6);
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 08 (Traits, if constexpr y SFINAE clásico)...\n";
    return RUN_ALL_TESTS();
}
