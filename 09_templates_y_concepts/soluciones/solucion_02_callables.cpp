/* =============================================================================
   MÓDULO 09 - SOLUCIÓN 02: Algoritmos genéricos con callables
   =============================================================================
   POR QUÉ FUNCIONA:
     - Un parámetro de template puede ser cualquier cosa 'llamable': lambda, función o functor.
     - invoke_result_t calcula en compilación el tipo que devuelve f; así transformar cambia de int a string solo.
     - 'typename Contenedor::value_type' pide el tipo de los elementos (typename es obligatorio acá).
     - acumular es la idea de std::accumulate: un 'fold' genérico.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_02_callables.cpp -o sol02 && ./sol02
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <cstddef>
#include <type_traits>
#include "../../include/test_framework.hpp"

// 'Pred' puede ser una lambda, un puntero a función o un functor: el template
// no le importa cuál, solo que se pueda llamar como pred(x).
template <typename Contenedor, typename Pred>
std::size_t contarSi(const Contenedor& c, Pred pred) {
    std::size_t cuenta = 0;
    for (const auto& x : c) {
        if (pred(x)) ++cuenta;
    }
    return cuenta;
}

// invoke_result_t<F, X> es "el tipo que devuelve f al llamarla con un X".
// remove_cvref_t le saca const y & por si f devolviera una referencia.
template <typename Contenedor, typename F>
std::vector<std::remove_cvref_t<std::invoke_result_t<F, const typename Contenedor::value_type&>>>
transformar(const Contenedor& c, F f) {
    using R = std::remove_cvref_t<std::invoke_result_t<F, const typename Contenedor::value_type&>>;
    std::vector<R> resultado;
    resultado.reserve(c.size());
    for (const auto& x : c) {
        resultado.push_back(f(x));
    }
    return resultado;
}

template <typename Contenedor, typename T, typename F>
T acumular(const Contenedor& c, T inicial, F f) {
    for (const auto& x : c) {
        inicial = f(inicial, x);
    }
    return inicial;
}

// ============= TESTS (los mismos del ejercicio) =============
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
    std::cout << "Ejecutando pruebas para Solución 02 (Algoritmos genéricos con callables)...\n";
    return RUN_ALL_TESTS();
}
