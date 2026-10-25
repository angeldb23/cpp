/* =============================================================================
   MÓDULO 03 - EJERCICIO 07: Limpiar un Vector (erase-remove, unique, swap-and-pop)
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA:
     Practicá las tres formas clásicas de borrar elementos de un std::vector:
     1. eliminarValor(vector<int>&, int) -> int: borra todas las apariciones y devuelve cuántas borró
     2. deduplicar(vector<int>&): deja valores únicos y ordenados
     3. quitarEnIndice(vector<string>&, size_t): borra en O(1) sin conservar el orden
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_07_limpiar_vector.cpp -o ej07 && ./ej07
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <utility>
#include "../../include/test_framework.hpp"

// TODO 1: eliminarValor borra TODAS las apariciones de 'valor' y devuelve
//         cuántas borró. Pista: idioma erase-remove (std::remove + erase).
// TODO 2: deduplicar deja cada valor una sola vez y el vector ordenado.
//         Pista: std::sort + std::unique + erase.
// TODO 3: quitarEnIndice borra el elemento en la posición 'i' en O(1):
//         intercambiá con el último y hacé pop_back (no conserva el orden).
// TU CÓDIGO AQUÍ:

int eliminarValor(std::vector<int>& v, int valor) {
    (void)v; (void)valor;
    return 0; // placeholder
}

void deduplicar(std::vector<int>& v) {
    (void)v; // placeholder: no hace nada
}

void quitarEnIndice(std::vector<std::string>& v, std::size_t i) {
    (void)v; (void)i; // placeholder: no hace nada
}

// ============= TESTS =============
TEST_CASE(TestEliminarValor) {
    std::vector<int> v = {1, 2, 3, 2, 4, 2, 5};
    int borrados = eliminarValor(v, 2);
    ASSERT_EQ(borrados, 3);
    std::vector<int> esperado = {1, 3, 4, 5};
    ASSERT_TRUE(v == esperado);
}

TEST_CASE(TestEliminarValorInexistente) {
    std::vector<int> v = {1, 2, 3};
    int borrados = eliminarValor(v, 9);
    ASSERT_EQ(borrados, 0);
    ASSERT_EQ(static_cast<int>(v.size()), 3);
}

TEST_CASE(TestDeduplicar) {
    std::vector<int> v = {5, 3, 5, 1, 3, 3, 9, 1};
    deduplicar(v);
    std::vector<int> esperado = {1, 3, 5, 9};
    ASSERT_TRUE(v == esperado);
}

TEST_CASE(TestDeduplicarVacio) {
    std::vector<int> v;
    deduplicar(v);
    ASSERT_TRUE(v.empty());
}

TEST_CASE(TestQuitarEnIndice) {
    std::vector<std::string> inv = {"Espada", "Escudo", "Pocion", "Arco"};
    quitarEnIndice(inv, 1);
    ASSERT_EQ(static_cast<int>(inv.size()), 3);
    // el último ocupó el lugar del borrado (el orden NO se conserva)
    ASSERT_EQ(inv[1], std::string("Arco"));
    ASSERT_EQ(inv[0], std::string("Espada"));
    ASSERT_EQ(inv[2], std::string("Pocion"));
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 07 (Limpiar un Vector)...\n";
    return RUN_ALL_TESTS();
}
