/* =============================================================================
   MÓDULO 08 - EJERCICIO 05: El estado moved-from: válido pero indeterminado
   =============================================================================
   DIFICULTAD: Intermedio
   PRACTICA: lección 07 (std::move es un cast; el origen queda "válido
   pero indeterminado": se le puede asignar de nuevo, no se puede
   confiar en su contenido).

   CONSIGNA:
   Implementá `transferirYReciclar`, que hace el ciclo COMPLETO del
   buen uso de un moved-from:

      1. Roba el contenido de `ruta` a un vector local `viajero`
         (con std::move) y suma su tamaño al acumulador de la clase.
      2. REASIGNA `ruta` con un vector nuevo `{1, 2, 3}` (el uso del
         moved-from que SÍ es seguro: escribir).
      3. Devuelve 10 * (el tamaño que tenía `viajero` después de
         robar) + (el tamaño de la `ruta` recién reasignada).

   PISTA: no leas nada de `ruta` entre el std::move y la
   reasignación: esa es LA regla.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra -I../../include \
         ejercicio_05_estado_moved_from.cpp -o ej05 && ./ej05
   =============================================================================
 */
#include <cstddef>
#include <iostream>
#include <utility>
#include <vector>
#include "../../include/test_framework.hpp"

inline int acumuladorMovido = 0;   // suma de elementos que pasaron por movimiento

// TODO: implementar los tres pasos de la consigna.
int transferirYReciclar(std::vector<int>& ruta) {
    (void)ruta;
    return 0;                                      // PLACEHOLDER
}

// Función ya escrita (guía): mueve, reasigna y usa.
std::string reformatear(std::string& cadena) {
    std::string viajero = std::move(cadena);       // roba (cadena: moved-from)
    cadena = "enalcesada";                         // REUTILIZACION valida: escribir
    return viajero + "/" + cadena;
}

// ============= TESTS =============
TEST_CASE(TransfiereYReasigna) {
    std::vector<int> ruta = {5, 10, 15};
    const int vuelto = transferirYReciclar(ruta);
    ASSERT_EQ(acumuladorMovido, 3);        // se movieron 3 elementos
    ASSERT_EQ(vuelto, 33);                 // 10 * viajero.size() + ruta.size()
}

TEST_CASE(RutaQuedaReasignada) {
    std::vector<int> ruta = {7, 7};
    transferirYReciclar(ruta);
    ASSERT_EQ(ruta.size(), std::size_t(3));
    ASSERT_EQ(ruta[0], 1);
    ASSERT_EQ(ruta[2], 3);
}

TEST_CASE(StringMovedFromReutilizable) {
    std::string original("legajos viejos");
    const std::string lista = reformatear(original);
    ASSERT_EQ(original, "enalcesada");             // fue REASIGNADO (no leido)
    ASSERT_EQ(lista, "legajos viejos/enalcesada");
}

int main() {
    std::cout << "Ejecutando tests de Ejercicio 05...\n";
    return RUN_ALL_TESTS();
}
