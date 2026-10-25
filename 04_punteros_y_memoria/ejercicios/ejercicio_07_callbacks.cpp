/* =============================================================================
   MÓDULO 04 - EJERCICIO 07: Callbacks con punteros a función
   =============================================================================
   DIFICULTAD: Intermedio
   PRACTICA: lección 11 (punteros a función, callbacks, tablas, nullptr).

   CONSIGNA:
     1. Implementá las operaciones básicas:
          sumar(a, b), restar(a, b), multiplicar(a, b), maximo(a, b).
     2. reducirRango(const int* datos, std::size_t n, int (*op)(int, int),
        int inicial): aplicá op de izquierda a derecha:
          acumulado = op(acumulado, datos[i])
        Si n es 0, devolvé inicial. Si datos es nullptr, devolvé inicial.
     3. buscarSi(const int* datos, std::size_t n, bool (*pred)(int)):
        devolvé el puntero al primer elemento que cumpla pred, o nullptr.
     4. contarSi(const int* datos, std::size_t n, bool (*pred)(int)):
        contá cuántos elementos cumplen pred.
     5. elegirOperacion(char simbolo): devolvé el puntero a la función
        correspondiente ('+' -> sumar, '-' -> restar, '*' -> multiplicar,
        'M' -> maximo). Cualquier otro carácter -> nullptr.

   `esPar` y `esPositivo` ya están implementadas para que las uses como
   predicados en las pruebas.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra ejercicio_07_callbacks.cpp -o ej07 && ./ej07
   ============================================================================= */
#include <cstddef>
#include <iostream>
#include "../../include/test_framework.hpp"

// TODO: implementá estas cuatro operaciones.
int sumar(int a, int b) { (void)a; (void)b; return 0; }
int restar(int a, int b) { (void)a; (void)b; return 0; }
int multiplicar(int a, int b) { (void)a; (void)b; return 0; }
int maximo(int a, int b) { (void)a; (void)b; return 0; }

using Operacion = int (*)(int, int);
using Predicado = bool (*)(int);

// TODO: implementá estas cuatro funciones.
int reducirRango(const int* datos, std::size_t n, Operacion op, int inicial) {
    (void)datos; (void)n; (void)op; (void)inicial;
    return 0;  // placeholder
}

const int* buscarSi(const int* datos, std::size_t n, Predicado pred) {
    (void)datos; (void)n; (void)pred;
    return nullptr;  // placeholder
}

std::size_t contarSi(const int* datos, std::size_t n, Predicado pred) {
    (void)datos; (void)n; (void)pred;
    return 0;  // placeholder
}

Operacion elegirOperacion(char simbolo) {
    (void)simbolo;
    return nullptr;  // placeholder
}

// Predicados de apoyo (ya implementados).
bool esPar(int x) { return x % 2 == 0; }
bool esPositivo(int x) { return x > 0; }
bool siempreFalso(int) { return false; }

// ============= TESTS =============
TEST_CASE(TestOperaciones) {
    ASSERT_EQ(sumar(7, 2), 9);
    ASSERT_EQ(restar(7, 2), 5);
    ASSERT_EQ(multiplicar(7, 2), 14);
    ASSERT_EQ(maximo(7, 2), 7);
    ASSERT_EQ(maximo(2, 7), 7);
}

TEST_CASE(TestReducirRango) {
    int datos[] = {1, 2, 3, 4};
    ASSERT_EQ(reducirRango(datos, 4, sumar, 0), 10);
    ASSERT_EQ(reducirRango(datos, 4, multiplicar, 1), 24);
    ASSERT_EQ(reducirRango(datos, 4, maximo, 0), 4);
    ASSERT_EQ(reducirRango(datos, 0, sumar, 7), 7);
    ASSERT_EQ(reducirRango(nullptr, 4, sumar, 7), 7);
}

TEST_CASE(TestBuscarSi) {
    int datos[] = {1, 3, 4, 7};
    ASSERT_TRUE(buscarSi(datos, 4, esPar) == datos + 2);
    ASSERT_TRUE(buscarSi(datos, 4, esPositivo) == datos);
    ASSERT_TRUE(buscarSi(datos, 4, siempreFalso) == nullptr);
}

TEST_CASE(TestContarSi) {
    int datos[] = {-1, 2, -3, 4, 5};
    ASSERT_EQ(contarSi(datos, 5, esPar), 2);
    ASSERT_EQ(contarSi(datos, 5, esPositivo), 3);
    ASSERT_EQ(contarSi(datos, 0, esPar), 0);
}

TEST_CASE(TestElegirOperacion) {
    ASSERT_TRUE(elegirOperacion('+') == sumar);
    ASSERT_TRUE(elegirOperacion('-') == restar);
    ASSERT_TRUE(elegirOperacion('*') == multiplicar);
    ASSERT_TRUE(elegirOperacion('M') == maximo);
    ASSERT_TRUE(elegirOperacion('?') == nullptr);
}

TEST_CASE(TestTablaDeOperaciones) {
    Operacion tabla[] = {elegirOperacion('+'), elegirOperacion('*')};
    ASSERT_TRUE(tabla[0] != nullptr);
    ASSERT_TRUE(tabla[1] != nullptr);
    ASSERT_EQ(tabla[0](3, 4), 7);
    ASSERT_EQ(tabla[1](3, 4), 12);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 07...\n";
    return RUN_ALL_TESTS();
}
