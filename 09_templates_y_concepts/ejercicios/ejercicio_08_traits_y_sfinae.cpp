/* =============================================================================
   MÓDULO 09 - EJERCICIO 08: Traits, if constexpr y SFINAE clásico
   =============================================================================
   DIFICULTAD: Avanzado
   CONSIGNA:
     Practicá las herramientas de antes de C++20 (todavía muy presentes en código real):
     1. Limpio<T>: alias que saca const, volátil y referencia de T (std::remove_cv_t, std::remove_reference_t)
     2. TieneSize<T>: trait con el detection idiom (std::void_t y decltype) que dice si T tiene .size()
     3. clasificar(x): con if constexpr devuelve "entero", "decimal", "puntero", "coleccion" u "otro"
     4. mitad(x): dos sobrecargas con std::enable_if_t; enteros dividen en 2 (división entera), decimales multiplican por 0.5
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_08_traits_y_sfinae.cpp -o ej08 && ./ej08
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include <type_traits>
#include <utility>
#include "../../include/test_framework.hpp"

// TODO 1: Limpio<T> debe valer T sin const, sin volatile y sin referencia.
//         Ej: Limpio<const int&> es int.
// TU CÓDIGO AQUÍ:
template <typename T>
using Limpio = T;   // placeholder

// TODO 2: dejá la primaria como está y agregá la especialización parcial que
//         valga true_type cuando decltype(std::declval<T>().size()) sea válido.
// TU CÓDIGO AQUÍ:
template <typename T, typename = void>
struct TieneSize : std::false_type {};

// TODO 3: clasificar con if constexpr:
//         is_integral_v -> "entero", is_floating_point_v -> "decimal",
//         is_pointer_v -> "puntero", TieneSize<T>::value -> "coleccion", si no "otro"
// TU CÓDIGO AQUÍ:
template <typename T>
std::string clasificar([[maybe_unused]] const T& x) {
    return "otro";   // placeholder
}

// TODO 4: mitad con DOS sobrecargas y enable_if_t en el tipo de retorno.
//         Para integrales: x / 2.  Para decimales: x * 0.5.
//         Borrá este placeholder y escribí las dos sobrecargas.
// TU CÓDIGO AQUÍ:
template <typename T>
T mitad(T x) {
    return x;   // placeholder
}

// ============= TESTS =============
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
    std::cout << "Ejecutando pruebas para Ejercicio 08 (Traits, if constexpr y SFINAE clásico)...\n";
    return RUN_ALL_TESTS();
}
