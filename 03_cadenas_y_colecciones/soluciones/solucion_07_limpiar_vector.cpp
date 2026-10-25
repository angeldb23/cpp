/* =============================================================================
   MÓDULO 03 - SOLUCIÓN 07: Limpiar un Vector (erase-remove, unique, swap-and-pop)
   =============================================================================
   POR QUÉ FUNCIONA:
     - erase-remove: remove reordena, erase corta. Sin el erase el vector conserva su tamaño.
     - unique necesita datos ordenados, o los duplicados no adyacentes sobreviven.
     - swap-and-pop es O(1) pero cambia el orden: sirve para inventarios, no para listas ordenadas.
     - Chequeamos i >= size() para no tocar memoria inválida.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_07_limpiar_vector.cpp -o sol07 && ./sol07
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <utility>
#include "../../include/test_framework.hpp"

// std::remove NO borra: mueve los elementos que se quedan hacia adelante y
// devuelve el nuevo "final lógico". Recién erase() achica el vector de verdad.
int eliminarValor(std::vector<int>& v, int valor) {
    auto nuevoFin = std::remove(v.begin(), v.end(), valor);
    int borrados = static_cast<int>(std::distance(nuevoFin, v.end()));
    v.erase(nuevoFin, v.end());
    return borrados;
}

// unique solo colapsa duplicados ADYACENTES, por eso primero ordenamos.
void deduplicar(std::vector<int>& v) {
    std::sort(v.begin(), v.end());
    v.erase(std::unique(v.begin(), v.end()), v.end());
}

// swap-and-pop: O(1) en vez de O(n) de erase en el medio. Cuesta el orden.
void quitarEnIndice(std::vector<std::string>& v, std::size_t i) {
    if (i >= v.size()) return;
    std::swap(v[i], v.back());
    v.pop_back();
}

// ============= TESTS (los mismos del ejercicio) =============
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
    std::cout << "Ejecutando pruebas para Solución 07 (Limpiar un Vector)...\n";
    return RUN_ALL_TESTS();
}
