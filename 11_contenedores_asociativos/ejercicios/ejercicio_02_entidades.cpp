/* =============================================================================
   MÓDULO 11 - EJERCICIO 02: tabla de entidades (lección 02)
   =============================================================================
   DIFICULTAD: Basico
   CONSIGNA (tema 1 de TEMAS.md: unordered_map + insert/emplace/try_emplace):
     Una partida registra entidades por ID en std::unordered_map<int,
     std::string> (id -> nombre):
       1. spawn(map, id, nombre): si el id NO estaba, lo inserta con
          try_emplace (L02) y devuelve true. Si ya estaba, NO toca el
          nombre y devuelve false (las entidades "ya viven": no hay
          re-spawn silencioso).
       2. renombrar(map, id, nuevo): reescribe el nombre si el id existe
          (true); false si no existe (y no agrega nada).
       3. despawn(map, id): borra la entidad y devuelve true; false si
          no estaba.
       4. idAlfanumerico(map, patron): el primer id cuyo nombre ARRANCA
          con el patron (el map NO está ordenado: recorrerlo es la única
          forma de buscarlo). Si no hay, devuelve -1.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_02_entidades.cpp -o ej02 && ./ej02
   ============================================================================= */
#include <iostream>
#include <string>
#include <unordered_map>
#include "../../include/test_framework.hpp"

using Entidades = std::unordered_map<int, std::string>;

// TODO 1: inserta si falta (true) sin tocar si ya estaba (false).
bool spawn([[maybe_unused]] Entidades& mundo, [[maybe_unused]] int id,
           [[maybe_unused]] const std::string& nombre) {
    return false;    // placeholder
}

// TODO 2: reescribe si el id existe (true); false si no existe.
bool renombrar([[maybe_unused]] Entidades& mundo, [[maybe_unused]] int id,
               [[maybe_unused]] const std::string& nuevo) {
    return false;    // placeholder
}

// TODO 3: borra y devuelve true; false si no estaba.
bool despawn([[maybe_unused]] Entidades& mundo, [[maybe_unused]] int id) {
    return false;    // placeholder
}

// TODO 4: primer id cuyo nombre arranca con patron, o -1.
int idAlfanumerico([[maybe_unused]] const Entidades& mundo,
                   [[maybe_unused]] const std::string& patron) {
    return -1;    // placeholder
}

// ============= TESTS =============
TEST_CASE(SpawnNoDuplica) {
    Entidades mundo;
    ASSERT_TRUE(spawn(mundo, 7, "orco"));
    ASSERT_FALSE(spawn(mundo, 7, "troll"));      // ya estaba: no toca nombre
    ASSERT_EQ(mundo.at(7), "orco");              // el original sobrevive
}

TEST_CASE(RenombrarSoloSiExiste) {
    Entidades mundo{{7, "orco"}};
    ASSERT_TRUE(renombrar(mundo, 7, "orco mayor"));
    ASSERT_FALSE(renombrar(mundo, 99, "fantasma"));
    ASSERT_EQ(mundo.at(7), "orco mayor");
    ASSERT_EQ(mundo.size(), 1u);                  // el 99 nunca se agregó
}

TEST_CASE(DespawnBorraDeVerdad) {
    Entidades mundo{{7, "orco"}, {8, "troll"}};
    ASSERT_TRUE(despawn(mundo, 7));
    ASSERT_FALSE(despawn(mundo, 7));              // ya no estaba
    ASSERT_FALSE(mundo.contains(7));
    ASSERT_TRUE(mundo.contains(8));
}

TEST_CASE(BusquedaPorPrefijoEnTablasQueNoSeOrdenan) {
    Entidades mundo{{10, "torre alta"}, {11, "orco veral"}};
    ASSERT_EQ(idAlfanumerico(mundo, "torre"), 10);
    ASSERT_EQ(idAlfanumerico(mundo, "orc"), 11);
    ASSERT_EQ(idAlfanumerico(mundo, "dragon"), -1);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 02 (tabla de entidades)...\n";
    return RUN_ALL_TESTS();
}
