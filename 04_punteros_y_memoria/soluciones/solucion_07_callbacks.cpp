/* =============================================================================
   MÓDULO 04 - SOLUCIÓN 07: Callbacks con punteros a función
   =============================================================================
   POR QUÉ FUNCIONA:
     - Cada operación respeta la firma int(int, int), así todas son
       intercambiables como Operacion.
     - reducirRango acumula aplicando el callback; no conoce la operación.
     - buscarSi/contarSi reciben un predicado y lo llaman por cada elemento.
     - elegirOperacion devuelve nullptr para un símbolo desconocido; el
       llamador debe comprobarlo antes de invocar.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_07_callbacks.cpp -o sol07 && ./sol07
   ============================================================================= */
#include <cstddef>
#include <iostream>
#include "../../include/test_framework.hpp"

int sumar(int a, int b) { return a + b; }
int restar(int a, int b) { return a - b; }
int multiplicar(int a, int b) { return a * b; }
int maximo(int a, int b) { return a > b ? a : b; }

using Operacion = int (*)(int, int);
using Predicado = bool (*)(int);

int reducirRango(const int* datos, std::size_t n, Operacion op, int inicial) {
    if (datos == nullptr || op == nullptr) return inicial;
    int acumulado = inicial;
    for (std::size_t i = 0; i < n; ++i) {
        acumulado = op(acumulado, datos[i]);
    }
    return acumulado;
}

const int* buscarSi(const int* datos, std::size_t n, Predicado pred) {
    if (datos == nullptr || pred == nullptr) return nullptr;
    for (std::size_t i = 0; i < n; ++i) {
        if (pred(datos[i])) return datos + i;
    }
    return nullptr;
}

std::size_t contarSi(const int* datos, std::size_t n, Predicado pred) {
    if (datos == nullptr || pred == nullptr) return 0;
    std::size_t total = 0;
    for (std::size_t i = 0; i < n; ++i) {
        if (pred(datos[i])) ++total;
    }
    return total;
}

Operacion elegirOperacion(char simbolo) {
    switch (simbolo) {
        case '+': return sumar;
        case '-': return restar;
        case '*': return multiplicar;
        case 'M': return maximo;
        default:  return nullptr;
    }
}

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
    ASSERT_EQ(contarSi(datos, 5, esPar), 2u);
    ASSERT_EQ(contarSi(datos, 5, esPositivo), 3u);
    ASSERT_EQ(contarSi(datos, 0, esPar), 0u);
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
    std::cout << "Ejecutando pruebas para Solución 07...\n";
    return RUN_ALL_TESTS();
}
