/* =============================================================================
   MÓDULO 10 - EJERCICIO 01: iteradores de a pie (lecciones 01 y 02)
   =============================================================================
   DIFICULTAD: Basico
   CONSIGNA (tema 1 de TEMAS.md: iteradores, begin/end y avance):
     Armá un puñado de utilidades que trabajan con ITERADORES (sin
     índices de `[]`):
       1. indiceDe(cont, valor): la POSICION (std::size_t) del primer
          elemento que es igual a valor. Si no esta, devuelve
          `noEncontrado` (una constante que se define abajo).
          Pista: find (L03) y convertí el iterador en índice con distance
          sobre `cont.begin()` (L01/L02).
       2. ultimoDe(cont): el VALOR del último elemento (sin usar
          cont.back(); usá prev sobre end). Asumí que el contenedor NO
          está vacío, pero usá el patrón seguro valido por vos (L01: el
          end es un límite, no un elemento).
       3. desdeElFinal(cont, n): el VALOR del elemento que está a `n`
          posiciones desde el final (n=0 es el último). Pista: prev(end, n+1).
       4. Usá la MISMA funciones con un std::vector<int> y con una
          std::list<int> (la interfaz de la L01: no cambia nada; el TEST
          lo demuestra con ambas).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_01_iteradores.cpp -o ej01 && ./ej01
   ============================================================================= */
#include <algorithm>
#include <iterator>
#include <list>
#include <vector>
#include "../../include/test_framework.hpp"

// Constante "no esta" para indiceDe:
constexpr std::size_t noEncontrado = static_cast<std::size_t>(-1);

// TODO 1: indiceDe devuelve la posicion del primer elemento igual a valor
//         (o noEncontrado).
// TU CODIGO AQUI:
std::size_t indiceDe([[maybe_unused]] const std::vector<int>& datos,
                     [[maybe_unused]] int valor) {
    return noEncontrado;    // placeholder
}

// (y la version genérica para list)
std::size_t indiceDe([[maybe_unused]] const std::list<int>& datos,
                     [[maybe_unused]] int valor) {
    return noEncontrado;    // placeholder
}

// TODO 2: ultimoDe devuelve el valor del ultimo elemento.
// TU CODIGO AQUI (version vector):
int ultimoDe([[maybe_unused]] const std::vector<int>& datos) {
    return 0;    // placeholder
}

// TODO 3: desdeElFinal devuelve el valor a `n` del final (0 = ultimo).
// TU CODIGO AQUI (version vector):
int desdeElFinal([[maybe_unused]] const std::vector<int>& datos,
                 [[maybe_unused]] std::size_t n) {
    return 0;    // placeholder
}

// ============= TESTS =============
TEST_CASE(IndiceEncuentraLaPrimeraCoincidencia) {
    const std::vector<int> v{10, 20, 30, 20};
    ASSERT_EQ(indiceDe(v, 20), 1u);          // el PRIMERO
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
    std::cout << "Ejecutando pruebas para Ejercicio 01 (iteradores)...\n";
    return RUN_ALL_TESTS();
}
