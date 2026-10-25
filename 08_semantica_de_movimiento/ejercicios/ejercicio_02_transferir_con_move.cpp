/* =============================================================================
   MÓDULO 08 - EJERCICIO 02: Transferir un vector con std::move
   (migrado y reescrito desde el módulo 02, práctica inicial)
   =============================================================================
   DIFICULTAD: Básico
   PRACTICA: lecciones 03 y 04 (semántica de movimiento: std::move en
   asignación, y el estado moved-from del vector, que queda VACÍO).

   CONSIGNA:
   Implementá `moverVector`: transfiere el contenido de `origen` a
   `destino` en O(1) (sin copiar ningún elemento). Después del
   movimiento:
      - `destino` tiene TODOS los datos que tenía `origen`.
      - `origen` quedó VACÍO (size == 0): garantía estándar (L07).

   PISTA: una sola línea. El constructor de movimiento de
   std::vector ya sabe robar; vos solo tenés que pedir el PERMISO.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_02_transferir_con_move.cpp -o ej02 && ./ej02
   =============================================================================
 */
#include <cstddef>
#include <iostream>
#include <utility>
#include <vector>
#include "../../include/test_framework.hpp"

// TODO: transferir los datos de `origen` a `destino` en O(1).
void moverVector(std::vector<int>& origen, std::vector<int>& destino) {
    (void)origen;                                  // PLACEHOLDER
    (void)destino;
}

// ============= TESTS =============
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
    std::cout << "Ejecutando tests de Ejercicio 02...\n";
    return RUN_ALL_TESTS();
}
