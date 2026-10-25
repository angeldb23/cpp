/* =============================================================================
   MÓDULO 11 - SOLUCIÓN 04: torneo con claves NPC (ejercicio 04)
   =============================================================================
   L05 es la lección de respaldo: map con claves propias (operator< por
   nivel + nombre como desempate), try_emplace/insert con su bool,
   y recorrido para agregados (los máximos con una pasada o multiset).
   ============================================================================= */
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

struct NPC {
    std::string nombre;
    int         nivel;

    // Orden total canónico (L05): PRIMERO nivel; desempate: nombre.
    bool operator<(const NPC& otra) const {
        if (nivel != otra.nivel) return nivel < otra.nivel;
        return nombre < otra.nombre;
    }
};

using Puntajes = std::map<NPC, int>;

// 1) con try_emplace: insertar SI FALTA sin tocar el valor presente:
bool registrar(Puntajes& tabla, const NPC& npc, int puntos) {
    const auto [pos, nueva] = tabla.try_emplace(npc, puntos);
    (void)pos;
    return nueva;
}

// 2) find primero (L01): si no está, NADA pasa:
bool anotar(Puntajes& tabla, const NPC& npc, int puntos) {
    auto it = tabla.find(npc);
    if (it == tabla.end()) return false;
    it->second += puntos;
    return true;
}

// 3) los máximos: multiset con comparador de puntos (inverse: max primero):
std::vector<NPC> campeones(const Puntajes& tabla) {
    std::multiset<int, std::greater<int>> ranking;
    for (const auto& [npc, puntos] : tabla) {
        (void)npc;
        ranking.insert(puntos);
    }
    if (ranking.empty()) return {};
    int maximo = *ranking.begin();     // el "mayor primero" del multiset

    std::vector<NPC> aureo;
    for (const auto& [npc, puntos] : tabla) {
        if (puntos == maximo) aureo.push_back(npc);
    }
    return aureo;
}

int enNivel(const Puntajes& tabla, int nivel) {
    int cuantos = 0;
    for (const auto& [npc, puntos] : tabla) {
        (void)puntos;
        if (npc.nivel == nivel) ++cuantos;
    }
    return cuantos;
}

// ============= TESTS =============
TEST_CASE(RegistrarNoDuplicaNPC) {
    Puntajes tabla;
    ASSERT_TRUE(registrar(tabla, NPC{"ana", 3}, 10));
    ASSERT_FALSE(registrar(tabla, NPC{"ana", 3}, 50));
    ASSERT_EQ(tabla.at(NPC{"ana", 3}), 10);
}

TEST_CASE(DesempatePorNombreHaceClavesDistintas) {
    Puntajes tabla;
    ASSERT_TRUE(registrar(tabla, NPC{"ana", 3}, 10));
    ASSERT_TRUE(registrar(tabla, NPC{"bea", 3}, 12));
    ASSERT_EQ(tabla.size(), 2u);
}

TEST_CASE(AnotarSumaSoloSiExiste) {
    Puntajes tabla{{NPC{"ana", 3}, 10}};
    ASSERT_TRUE(anotar(tabla, NPC{"ana", 3}, 5));
    ASSERT_EQ(tabla.at(NPC{"ana", 3}), 15);
    ASSERT_FALSE(anotar(tabla, NPC{"x", 1}, 5));
    ASSERT_EQ(tabla.size(), 1u);
}

TEST_CASE(CampeonesConEmpate) {
    Puntajes tabla{{NPC{"ana", 3}, 10}, {NPC{"bea", 9}, 22}, {NPC{"caro", 5}, 22}};
    const std::vector<NPC> top = campeones(tabla);
    ASSERT_EQ(top.size(), 2u);
    bool hayBea = false;
    bool hayCaro = false;
    for (const NPC& npc : top) {
        hayBea = hayBea || npc.nombre == "bea";
        hayCaro = hayCaro || npc.nombre == "caro";
    }
    ASSERT_TRUE(hayBea);
    ASSERT_TRUE(hayCaro);
}

TEST_CASE(CuentaPorNivel) {
    Puntajes tabla{{NPC{"ana", 3}, 10}, {NPC{"bea", 9}, 22}, {NPC{"caro", 3}, 22}};
    ASSERT_EQ(enNivel(tabla, 3), 2);
    ASSERT_EQ(enNivel(tabla, 9), 1);
    ASSERT_EQ(enNivel(tabla, 5), 0);
}

int main() {
    std::cout << "Ejecutando pruebas para Solucion 04 (torneo con claves NPC)...\n";
    return RUN_ALL_TESTS();
}
