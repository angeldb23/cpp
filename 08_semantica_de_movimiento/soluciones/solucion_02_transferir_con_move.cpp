/* =============================================================================
   MÓDULO 08 - SOLUCIÓN 02: Transferir un vector con std::move
   =============================================================================
   EXPLICACIÓN:
     `std::move(origen)` convierte el lvalue en rvalue (L07: es solo
     un cast); la ASIGNACIÓN de movimiento de std::vector hace el
     trabajo real: intercambia/roba las tres palabras internas
     (puntero a datos, tamaño, capacidad). No se copia NINGÚN
     elemento: transferencia O(1) pase lo que pese el vector.

     El origen queda "válido pero indeterminado"; para std::vector la
     garantía del estándar es QUE QUEDA VACÍO (size 0), y sigue siendo
     completamente utilizable: en los tests ahí lo rellenamos con
     push_back para mostrar el reuso (L07).
   ============================================================================= */

#include <cstddef>
#include <iostream>
#include <utility>
#include <vector>
#include "../../include/test_framework.hpp"

// Una línea hace todo el trabajo: le pedimos permiso y el vector se roba.
void moverVector(std::vector<int>& origen, std::vector<int>& destino) {
    destino = std::move(origen);
}

TEST_CASE(TransfiereEnO1) {
    std::vector<int> src = {1, 2, 3};
    std::vector<int> dst;
    moverVector(src, dst);
    ASSERT_EQ(dst.size(), std::size_t(3));
    ASSERT_EQ(dst[0], 1);
    ASSERT_EQ(dst[1], 2);
    ASSERT_EQ(dst[2], 3);
}

TEST_CASE(OrigenQuedaMovidoFrom) {
    std::vector<int> src = {4, 5, 6};
    std::vector<int> dst;
    moverVector(src, dst);
    ASSERT_EQ(src.size(), std::size_t(0));   // garantía estándar: vacío

    src.push_back(7);                        // moved-from: reasignable (L07)
    ASSERT_EQ(src.size(), std::size_t(1));
    ASSERT_EQ(src[0], 7);
}

TEST_CASE(OrigenYaLleno) {
    std::vector<int> src = {9};
    std::vector<int> dst = {8, 8};
    moverVector(src, dst);
    ASSERT_EQ(dst.size(), std::size_t(1));
    ASSERT_EQ(dst[0], 9);
}

int main() {
    std::cout << "[SOLUCIÓN] Ejecutando tests de Solución 02...\n";
    return RUN_ALL_TESTS();
}
