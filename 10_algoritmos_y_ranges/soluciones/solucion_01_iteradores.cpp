/* =============================================================================
   MÓDULO 10 - SOLUCIÓN 01: iteradores de a pie (lecciones 01 y 02)
   =============================================================================
   La idea de fondo: resolver TODO con la interfaz de iteradores de la
   L01/L02 (find + prev) y convertir el iterador en posición con
   std::distance (la jugada de la L02).
       - indiceDe: find + distance contra begin().
       - ultimoDe: *std::prev(end): el patrón "el último sin back()".
       - desdeElFinal: prev(end, n + 1): n=0 es el último, o sea el
         end movido n+1 pasos hacia atrás.
   ============================================================================= */
#include <algorithm>
#include <iterator>
#include <list>
#include <vector>
#include "../../include/test_framework.hpp"

constexpr std::size_t noEncontrado = static_cast<std::size_t>(-1);

// SOLUCIÓN 1: find y la conversión iterador->índice con distance (L01+L02).
std::size_t indiceDe(const std::vector<int>& datos, int valor) {
    const auto it = std::find(datos.begin(), datos.end(), valor);
    if (it == datos.end()) return noEncontrado;
    return static_cast<std::size_t>(std::distance(datos.begin(), it));
}

// el MISMO código sirve para list: es la promesa de la L01.
std::size_t indiceDe(const std::list<int>& datos, int valor) {
    const auto it = std::find(datos.begin(), datos.end(), valor);
    if (it == datos.end()) return noEncontrado;
    return static_cast<std::size_t>(std::distance(datos.begin(), it));
}

// SOLUCIÓN 2: prev sobre end (el último está "una sí" antes del límite).
int ultimoDe(const std::vector<int>& datos) {
    // (los tests garantizan datos no vacio: el prev es valido)
    return *std::prev(datos.end());
}

// SOLUCIÓN 3: n=0 es el último: prev(end, n+1).
int desdeElFinal(const std::vector<int>& datos, std::size_t n) {
    return *std::prev(datos.end(), static_cast<long>(n) + 1);
}

// ============= TESTS =============
TEST_CASE(IndiceEncuentraLaPrimeraCoincidencia) {
    const std::vector<int> v{10, 20, 30, 20};
    ASSERT_EQ(indiceDe(v, 20), 1u);
    ASSERT_EQ(indiceDe(v, 30), 2u);
    ASSERT_EQ(indiceDe(v, 99), noEncontrado);
}

TEST_CASE(IndiceTambienAndaEnList) {
    const std::list<int> l{7, 8, 9};
    ASSERT_EQ(indiceDe(l, 9), 2u);
    ASSERT_EQ(indiceDe(l, 7), 0u);
    ASSERT_EQ(indiceDe(l, 1), noEncontrado);
}

TEST_CASE(UltimoDeSinBack) {
    const std::vector<int> v{5, 6, 7};
    ASSERT_EQ(ultimoDe(v), 7);
    const std::vector<int> solo{42};
    ASSERT_EQ(ultimoDe(solo), 42);
}

TEST_CASE(DesdeElFinalCuentaHaciaAtras) {
    const std::vector<int> v{10, 20, 30, 40, 50};
    ASSERT_EQ(desdeElFinal(v, 0), 50);
    ASSERT_EQ(desdeElFinal(v, 1), 40);
    ASSERT_EQ(desdeElFinal(v, 4), 10);
}

int main() {
    std::cout << "Solución 01 (iteradores)...\n";
    return RUN_ALL_TESTS();
}
