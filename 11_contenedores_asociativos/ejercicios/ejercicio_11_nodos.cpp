/* =============================================================================
   MÓDULO 11 - EJERCICIO 11: mover nodos entre tablas (lección 15)
   =============================================================================
   DIFICULTAD: Difícil
   CONSIGNA (tema 8 de TEMAS.md: equal_range, extract y merge):
     Dos tablas de cuentas: la de URGENCIAS (id -> paciente) y la de
     HISTORIAL (id -> paciente). Al migrar una cuenta NO se copia su
     paciente: se MUEVE su NODO (extract + insert: L15):
       1. bool migrarCuenta(historial& dst, urgencias& src, int id):
          extraer el nodo de src; si la clave NO estaba, false; si
          estaba pero EN dst YA EXISTE un id igual, devolver el nodo a
          su lugar en src y false. Si todo OK, Va al dst (el nodo entero,
          sin copia) y true.
       2. int limpiarAgua(mapa): sacá TODAS las cuentas con nombre
          "agua" y devolvé cuántas fuero. (erase por iterador dentro
          del bucle: erase(it++): L03/L15.)
       3. std::vector<int> idsEnRango(tabla, min, max): los ids del
          rango [min, max] EN ORDEN (lower_bound/upper_bound o
          equal_range: L15), como ranged range (el map está ordenado).
       4. int juntar(historial& dst, const urgencias& src): migrar
          TODAS las cuentas de src que no existan en dst y devolver
          el CUENTO migrado (merge de la L15 ya lo hace: fijate que
          las repetidas quedan en src).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_11_nodos.cpp -o ej11 && ./ej11
   ============================================================================= */
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

using Cuentas = std::map<int, std::string>;

// TODO 1: migrar el NODO (extract + insert). La clave existente dst
//         o missing src (devuelve el nodo: L15 extract vacío):
bool migrarCuenta([[maybe_unused]] Cuentas& dst,
                  [[maybe_unused]] Cuentas& src,
                  [[maybe_unused]] int id) {
    return false;    // placeholder
}

// TODO 2: borrar las entradas con valor "agua" y devolver cuántas eran.
int limpiarAgua([[maybe_unused]] Cuentas& tabla) {
    return 0;    // placeholder
}

// TODO 3: los ids del rango [min, max] en orden (L15: igual_range):
std::vector<int> idsEnRango([[maybe_unused]] const Cuentas& tabla,
                            [[maybe_unused]] int min,
                            [[maybe_unused]] int max) {
    return {};    // placeholder
}

// TODO 4: juntar TODAS las cuentas de src que no estén en dst;
//         devolver cuántas entraron (merge: L15).
int juntar([[maybe_unused]] Cuentas& dst, [[maybe_unused]] Cuentas& src) {
    return 0;    // placeholder
}

// ============= TESTS =============
TEST_CASE(MigrarMueveNodoDeVerdad) {
    Cuentas urgencias{{101, "ana"}};
    Cuentas historial;
    ASSERT_TRUE(migrarCuenta(historial, urgencias, 101));
    ASSERT_FALSE(urgencias.contains(101));            // salió de src
    ASSERT_EQ(historial.at(101), "ana");              // yendo al dst
}

TEST_CASE(MigrarConDestinoOcupadoDevuelveElNodo) {
    Cuentas urgencias{{101, "ana"}};
    Cuentas historial{{101, "ana_previa"}};
    ASSERT_FALSE(migrarCuenta(historial, urgencias, 101));
    ASSERT_EQ(urgencias.at(101), "ana");              // no se perdió
    ASSERT_EQ(historial.at(101), "ana_previa");       // no se tocó
}

TEST_CASE(LimpiarAgua) {
    Cuentas tabla{{3, "agua"}, {7, "lava"}, {9, "agua"}, {1, "hielo"}};
    ASSERT_EQ(limpiarAgua(tabla), 2);
    ASSERT_FALSE(tabla.contains(3));
    ASSERT_FALSE(tabla.contains(9));
    ASSERT_EQ(tabla.size(), 2u);
}

TEST_CASE(idsEnRangoOrdenado) {
    Cuentas tabla{{10, "a"}, {20, "b"}, {30, "c"}, {40, "d"}};
    const std::vector<int> ids = idsEnRango(tabla, 15, 35);
    ASSERT_TRUE(ids.size() == 2);
    ASSERT_EQ(ids[0], 20);
    ASSERT_EQ(ids[1], 30);
    const std::vector<int> vacia = idsEnRango(tabla, 100, 200);
    ASSERT_TRUE(vacia.empty());
}

TEST_CASE(JuntarContraPersistenciaDeRepetidos) {
    Cuentas destino{{101, "ana"}};
    Cuentas otras{{101, "ana_vieja"}, {205, "beto"}, {310, "carla"}};
    ASSERT_EQ(juntar(destino, otras), 2);             // 205 y 310 migran
    ASSERT_EQ(destino.at(101), "ana");                // la k es del destino
    ASSERT_EQ(otras.size(), 1u);                      // el repetido queda
    ASSERT_EQ(otras.at(101), "ana_vieja");
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 11 (nodos y migración)...\n";
    return RUN_ALL_TESTS();
}
