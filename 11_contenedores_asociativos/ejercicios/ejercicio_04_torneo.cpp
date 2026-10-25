/* =============================================================================
   MÓDULO 11 - EJERCICIO 04: SCALA (juego de torneos) con claves propias
   (lección 05)
   =============================================================================
   DIFICULTAD: Medio
   CONSIGNA (tema 3 de TEMAS.md: claves con operator<):
     Una tabla de torneo usa std::map<NPC, int> (NPC -> puntos) y el
     struct NPC { string nombre; int nivel; } YA TIENE operator< por
     nivel y desempate por nombre (L05: está listo más abajo; NO lo
     cambies):
       1. registrar(puntajes, npc, puntos): si el npc NO estaba,
          lo inserta con sus puntos y devuelve true; si ya estaba
          (para el comparador: L05), NO CAMBIA nada y devuelve false.
       2. anotar(puntajes, npc, puntos): si el npc está, SUMA esos
          puntos y devuelve true; false si el npc no está.
       3. campeones(puntajes): el std::vector<NPC> con el/los npc de
          PUNTOS MÁS ALTO (puede haber empates; SIN sort: recordá que
          un map ya está ordenado POR CLAVE... acá conveniente no sirve
          para esto: usa un multiset con comparador de PUNTOS o recorré
          una vez y conservá los máximos).
       4. enNivel(puntajes, nivel): la CANTIDAD de NPCs con ese nivel
          (el orden del map es nivel primero, nombre después, L05).
   PISTA 4A: si el desempate del operator< se hace por nombre, "nivel==N"
   NO es un rango continuo en el map. La forma simple y correcta: UN
   recorrido contando.
   PISTA 4A: si el desempate del operator< se hace por nombre, "nivel==N"
   NO es un rango continuo en el map. La forma simple y correcta: UN
   recorrido contando (o un map<int, vector<NPC>> por nivel).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_04_torneo.cpp -o ej04 && ./ej04
   ============================================================================= */
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

struct NPC {
    std::string nombre;
    int         nivel;

    // Orden total canónico (L05): PRIMERO nivel, desempate: nombre:
    bool operator<(const NPC& otra) const {
        if (nivel != otra.nivel) return nivel < otra.nivel;
        return nombre < otra.nombre;
    }
};

using Puntajes = std::map<NPC, int>;

// TODO 1: registrar npc -> puntos; true si CREO la fila.
bool registrar([[maybe_unused]] Puntajes& tabla, [[maybe_unused]] const NPC& npc,
               [[maybe_unused]] int puntos) {
    return false;    // placeholder
}

// TODO 2: sumar puntos a un npc que YA está (true); false si no está.
bool anotar([[maybe_unused]] Puntajes& tabla, [[maybe_unused]] const NPC& npc,
            [[maybe_unused]] int puntos) {
    return false;    // placeholder
}

// TODO 3: los NPCs con los puntos mas altos (puede haber empates).
std::vector<NPC> campeones([[maybe_unused]] const Puntajes& tabla) {
    return {};    // placeholder
}

// TODO 4: CANTIDAD de NPCs de ese nivel.
int enNivel([[maybe_unused]] const Puntajes& tabla,
            [[maybe_unused]] int nivel) {
    return 0;    // placeholder
}

// ============= TESTS =============
TEST_CASE(RegistrarNoDuplicaNPC) {
    Puntajes tabla;
    ASSERT_TRUE(registrar(tabla, NPC{"ana", 3}, 10));
    ASSERT_FALSE(registrar(tabla, NPC{"ana", 3}, 50));   // misma clave
    ASSERT_EQ(tabla.at(NPC{"ana", 3}), 10);
}

TEST_CASE(DesempatePorNombreHaceClavesDistintas) {
    Puntajes tabla;
    ASSERT_TRUE(registrar(tabla, NPC{"ana", 3}, 10));
    ASSERT_TRUE(registrar(tabla, NPC{"bea", 3}, 12));    // nivel igual, otro nombre
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
    std::cout << "Ejecutando pruebas para Ejercicio 04 (torneo con claves NPC)...\n";
    return RUN_ALL_TESTS();
}
