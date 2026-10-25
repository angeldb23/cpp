/* =============================================================================
   MÓDULO 09 - EJERCICIO 02: Algoritmos genéricos con callables
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA:
     Escribí tres algoritmos genéricos que reciben cualquier contenedor y cualquier función:
     1. contarSi(c, pred): cuántos elementos cumplen el predicado
     2. transformar(c, f): vector nuevo con f aplicada a cada elemento
     3. acumular(c, inicial, f): reduce el contenedor con f(acumulado, elemento)
     Las firmas ya vienen armadas: fijate cómo se calcula el tipo de retorno de transformar.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_02_callables.cpp -o ej02 && ./ej02
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <cstddef>
#include <type_traits>
#include "../../include/test_framework.hpp"

// TODO 1: contarSi devuelve cuántos elementos de 'c' hacen que pred(x) sea true
// TU CÓDIGO AQUÍ:
template <typename Contenedor, typename Pred>
std::size_t contarSi([[maybe_unused]] const Contenedor& c, [[maybe_unused]] Pred pred) {
    return 0;   // placeholder
}

// TODO 2: transformar devuelve un vector con f(x) para cada x de 'c', en orden.
//         El tipo de retorno ya está calculado con invoke_result_t.
// TU CÓDIGO AQUÍ:
template <typename Contenedor, typename F>
std::vector<std::remove_cvref_t<std::invoke_result_t<F, const typename Contenedor::value_type&>>>
transformar([[maybe_unused]] const Contenedor& c, [[maybe_unused]] F f) {
    return {};   // placeholder
}

// TODO 3: acumular parte de 'inicial' y hace acc = f(acc, x) por cada x de 'c'
// TU CÓDIGO AQUÍ:
template <typename Contenedor, typename T, typename F>
T acumular([[maybe_unused]] const Contenedor& c, T inicial, [[maybe_unused]] F f) {
    return inicial;   // placeholder
}

// ============= TESTS =============
TEST_CASE(TestContarSiEnteros) {
    std::vector<int> v = {1, 2, 3, 4, 5, 6};
    auto esPar = [](int x) { return x % 2 == 0; };
    ASSERT_EQ(contarSi(v, esPar), 3u);
    auto mayorA10 = [](int x) { return x > 10; };
    ASSERT_EQ(contarSi(v, mayorA10), 0u);
}

TEST_CASE(TestContarSiStrings) {
    std::vector<std::string> palabras = {"sol", "luna", "estrella", "mar"};
    auto larga = [](const std::string& s) { return s.size() > 3; };
    ASSERT_EQ(contarSi(palabras, larga), 2u);
}

TEST_CASE(TestTransformarEnteros) {
    std::vector<int> v = {1, 2, 3};
    auto porDos = [](int x) { return x * 2; };
    auto r = transformar(v, porDos);
    ASSERT_EQ(static_cast<int>(r.size()), 3);
    ASSERT_EQ(r[0], 2);
    ASSERT_EQ(r[1], 4);
    ASSERT_EQ(r[2], 6);
}

TEST_CASE(TestTransformarCambiaDeTipo) {
    std::array<int, 3> a = {7, 8, 9};
    auto aTexto = [](int x) { return std::to_string(x); };
    auto r = transformar(a, aTexto);          // vector<std::string>
    ASSERT_EQ(static_cast<int>(r.size()), 3);
    ASSERT_EQ(r[0], std::string("7"));
    ASSERT_EQ(r[2], std::string("9"));
}

TEST_CASE(TestAcumular) {
    std::vector<int> v = {1, 2, 3, 4};
    auto suma = [](int acc, int x) { return acc + x; };
    auto producto = [](int acc, int x) { return acc * x; };
    ASSERT_EQ(acumular(v, 0, suma), 10);
    ASSERT_EQ(acumular(v, 1, producto), 24);
    std::vector<std::string> palabras = {"a", "b", "c"};
    auto unir = [](std::string acc, const std::string& x) { return acc + x; };
    ASSERT_EQ(acumular(palabras, std::string(), unir), std::string("abc"));
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 02 (Algoritmos genéricos con callables)...\n";
    return RUN_ALL_TESTS();
}
