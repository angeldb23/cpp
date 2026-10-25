/* =============================================================================
   MÓDULO 11 - SOLUCIÓN 11: mover nodos entre tablas (ejercicio 11)
   =============================================================================
   L15 es la lección de respaldo: extract devuelve un node handle
   (RAII), no copia nada; insert(std::move(nodo)) lo reubica;
   lower_bound/upper_bound delimitan el rango; merge mueve los nodos
   no-repetidos y deja los repetidos en el origen.
   ============================================================================= */
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

using Cuentas = std::map<int, std::string>;

// 1) extract + (a veces "volver") + insert: el nodo a su dst:
bool migrarCuenta(Cuentas& dst, Cuentas& src, int id) {
    auto nodo = src.extract(id);           // L15: el nodo completo (o vacío)
    if (nodo.empty()) return false;
    if (dst.contains(id)) {
        src.insert(std::move(nodo));       // no se toca: vuelve a su lugar
        return false;
    }
    dst.insert(std::move(nodo));           // el nodo entero, sin copia
    return true;
}

// 2) erase por iterador dentro del recorrido seguro (L03/L15):
int limpiarAgua(Cuentas& tabla) {
    int borradas = 0;
    for (auto it = tabla.begin(); it != tabla.end();) {
        if (it->second == "agua") {
            it = tabla.erase(it);
            ++borradas;
        } else {
            ++it;
        }
    }
    return borradas;
}

// 3) rango de claves: lower_bound + upper_bound = equal_range (L15):
std::vector<int> idsEnRango(const Cuentas& tabla, int min, int max) {
    std::vector<int> ids;
    for (auto it = tabla.lower_bound(min); it != tabla.upper_bound(max); ++it) {
        ids.push_back(it->first);
    }
    return ids;
}

// 4) merge: mueve los nodos NO repetidos y deja los repetidos en src (L15):
int juntar(Cuentas& dst, Cuentas& src) {
    const int antes = static_cast<int>(dst.size());
    dst.merge(src);
    return static_cast<int>(dst.size()) - antes;
}

// ============= TESTS =============
TEST_CASE(MigrarMueveNodoDeVerdad) {
    Cuentas urgencias{{101, "ana"}};
    Cuentas historial;
    ASSERT_TRUE(migrarCuenta(historial, urgencias, 101));
    ASSERT_FALSE(urgencias.contains(101));
    ASSERT_EQ(historial.at(101), "ana");
}

TEST_CASE(MigrarConDestinoOcupadoDevuelveElNodo) {
    Cuentas urgencias{{101, "ana"}};
    Cuentas historial{{101, "ana_previa"}};
    ASSERT_FALSE(migrarCuenta(historial, urgencias, 101));
    ASSERT_EQ(urgencias.at(101), "ana");
    ASSERT_EQ(historial.at(101), "ana_previa");
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
    ASSERT_EQ(juntar(destino, otras), 2);
    ASSERT_EQ(destino.at(101), "ana");
    ASSERT_EQ(otras.size(), 1u);
    ASSERT_EQ(otras.at(101), "ana_vieja");
}

int main() {
    std::cout << "Ejecutando pruebas para Solucion 11 (nodos y migración)...\n";
    return RUN_ALL_TESTS();
}
