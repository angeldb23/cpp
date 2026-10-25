/* =============================================================================
   MÓDULO 08 - SOLUCIÓN 05: El estado moved-from: válido pero indeterminado
   =============================================================================
   EXPLICACIÓN:
     El ciclo completo de un moved-from (L07):

       1. MOVER: `std::vector<int> viajero = std::move(ruta);`: desde
          acá `ruta` es válido pero indeterminado. Se PUEDE...
          destruir, y asignar; NO se puede confiar en su contenido.
       2. REASIGNAR: `ruta = std::vector<int>{1, 2, 3};` es la forma
          pulida de reusarlo: escribimos encima de las garantías y
          el objeto vuelve a tener un estado conocido.
       3. USAR: recién después es sanísimo leer `ruta`.

     La parte del string funciona igual: mover a un local y, de
     inmediato, REESCRIBIR al origen para poder seguir usándola
     (quedó "legajos viejos/enalcesada").
   ============================================================================= */

#include <cstddef>
#include <iostream>
#include <utility>
#include <vector>
#include "../../include/test_framework.hpp"

inline int acumuladorMovido = 0;

int transferirYReciclar(std::vector<int>& ruta) {
    std::vector<int> viajero = std::move(ruta);    // 1) robo: ruta = moved-from
    acumuladorMovido += static_cast<int>(viajero.size());
    ruta = std::vector<int>{1, 2, 3};              // 2) escribo encima: reuso seguro
    return 10 * static_cast<int>(viajero.size())
                + static_cast<int>(ruta.size());   // 3) ambos usos sanos
}

std::string reformatear(std::string& cadena) {
    std::string viajero = std::move(cadena);
    cadena = "enalcesada";                         // reutilizamos escribiendo
    return viajero + "/" + cadena;
}

TEST_CASE(TransfiereYReasigna) {
    std::vector<int> ruta = {5, 10, 15};
    const int vuelto = transferirYReciclar(ruta);
    ASSERT_EQ(acumuladorMovido, 3);
    ASSERT_EQ(vuelto, 33);
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
    ASSERT_EQ(original, "enalcesada");
    ASSERT_EQ(lista, "legajos viejos/enalcesada");
}

int main() {
    std::cout << "[SOLUCIÓN] Ejecutando tests de Solución 05...\n";
    return RUN_ALL_TESTS();
}
