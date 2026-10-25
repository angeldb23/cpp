/* =============================================================================
   MÓDULO 11 - SOLUCIÓN 02: tabla de entidades (ejercicio 02)
   =============================================================================
   L02 es la lección de respaldo: try_emplace para "insertar si falta sin
   tocar lo existente", contains/find para el resto, y el recorrido
   determinista (el unordered_map NO está ordenado: la igualdad es
   búsqueda lineal de ID).
   ============================================================================= */
#include <iostream>
#include <string>
#include <unordered_map>
#include "../../include/test_framework.hpp"

using Entidades = std::unordered_map<int, std::string>;

// 1) try_emplace: los datos son SOLO si no existe; el bool del par cuenta:
bool spawn(Entidades& mundo, int id, const std::string& nombre) {
    const auto [pos, nueva] = mundo.try_emplace(id, nombre);
    (void)pos;
    return nueva;
}

// 2) find primero: si no existe NO se agrega nada:
bool renombrar(Entidades& mundo, int id, const std::string& nuevo) {
    auto it = mundo.find(id);
    if (it == mundo.end()) return false;
    it->second = nuevo;
    return true;
}

bool despawn(Entidades& mundo, int id) {
    return mundo.erase(id) > 0;    // erase devuelve la CANTIDAD de eliminados
}

// 4) recorrido completo: unordered_map NO esta ordenado ni indexado:
int idAlfanumerico(const Entidades& mundo, const std::string& patron) {
    for (const auto& [id, nombre] : mundo) {
        if (nombre.rfind(patron, 0) == 0) return id;   // arranca con patron
    }
    return -1;
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
    ASSERT_EQ(mundo.size(), 1u);                 // el 99 nunca se agregó
}

TEST_CASE(DespawnBorraDeVerdad) {
    Entidades mundo{{7, "orco"}, {8, "troll"}};
    ASSERT_TRUE(despawn(mundo, 7));
    ASSERT_FALSE(despawn(mundo, 7));             // ya no estaba
    ASSERT_FALSE(mundo.contains(7));
    ASSERT_TRUE(mundo.contains(8));
}

TEST_CASE(BusquedaPorPrefijoEnTablaNoOrdenada) {
    Entidades mundo{{10, "torre alta"}, {11, "orco veral"}};
    ASSERT_EQ(idAlfanumerico(mundo, "torre"), 10);
    ASSERT_EQ(idAlfanumerico(mundo, "orc"), 11);
    ASSERT_EQ(idAlfanumerico(mundo, "dragon"), -1);
}

int main() {
    std::cout << "Ejecutando pruebas para Solucion 02 (tabla de entidades)...\n";
    return RUN_ALL_TESTS();
}
