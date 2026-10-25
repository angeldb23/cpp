/* =============================================================================
   MÓDULO 10 - EJERCICIO 02: rastreo con find_if y count_if (lección 03)
   =============================================================================
   DIFICULTAD: Basico
   CONSIGNA (tema 2 de TEMAS.md: algoritmos no modificantes):
     Un equipo guardado como vector de struct Jugador. Con SOLO find /
     find_if / count_if:
       1. primerVivo(jugadores): iterador al primer jugador con !caido.
          Si no hay ninguno, el end del vector (el contrato del find).
       2. hayMuerto(jugadores): bool usando `find_if` (NO count_if: cortás
          apenas aparece).
       3. cuantosDe(archivo, nombre): cuántos elementos del
          `std::vector<std::string> archivo` son exactamente `nombre`
          (count de la L03: si el número sirve).
       4. posicionesDe(texto, c): TODAS las posiciones de `c` en `texto`
          (el patrón encadenado del ejemplo 1 de la lección: find y
          std::next para seguir).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_02_busqueda.cpp -o ej02 && ./ej02
   ============================================================================= */
#include <algorithm>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

namespace {
struct Jugador {
    std::string nombre;
    bool caido;
};
}  // namespace

// TODO 1: primerVivo: iterador al primero con !caido (o end()).
// TU CODIGO AQUI:
std::vector<Jugador>::iterator primerVivo(
    [[maybe_unused]] std::vector<Jugador>& jugadores) {
    return jugadores.end();    // placeholder: devuelve end (fallará el test limpio)
}

// TODO 2: hayMuerto: true apenas alguien tiene caido=true.
// TU CODIGO AQUI:
bool hayMuerto([[maybe_unused]] const std::vector<Jugador>& jugadores) {
    return false;    // placeholder
}

// TODO 3: cuantosDe: cuántas entradas exactas de `nombre` hay.
// TU CODIGO AQUI:
int cuantosDe([[maybe_unused]] const std::vector<std::string>& archivo,
              [[maybe_unused]] const std::string& nombre) {
    return 0;    // placeholder
}

// TODO 4: posicionesDe: todas las posiciones de c en texto.
// TU CODIGO AQUI:
std::vector<std::size_t> posicionesDe(
    [[maybe_unused]] const std::string& texto,
    [[maybe_unused]] char c) {
    return {};    // placeholder
}

// ============= TESTS =============
TEST_CASE(PrimerVivoSaltaLosCaidos) {
    std::vector<Jugador> equipo{
        {"Quesito", true}, {"Mordel", false}, {"Turbo", false},
    };
    auto it = primerVivo(equipo);
    ASSERT_TRUE(it != equipo.end());
    ASSERT_TRUE(it->nombre == "Mordel");   // apunta al segundo
}

TEST_CASE(PrimerVivoSinVivosEsEnd) {
    std::vector<Jugador> muertos{
        {"Lumen", true},
    };
    ASSERT_TRUE(primerVivo(muertos) == muertos.end());
}

TEST_CASE(HayMuertoCortaPorElPrimero) {
    const std::vector<Jugador> leer{
        {"A", false}, {"B", true}, {"C", true},
    };
    ASSERT_EQ(hayMuerto(leer), true);
    const std::vector<Jugador> sanos{
        {"A", false},
    };
    ASSERT_EQ(hayMuerto(sanos), false);
    const std::vector<Jugador> vacio;
    ASSERT_EQ(hayMuerto(vacio), false);
}

TEST_CASE(CuantosCuentaCoincidenciasExactas) {
    const std::vector<std::string> archivo{
        "pocion", "flecha", "pocion", "pocion", "flecha",
    };
    ASSERT_EQ(cuantosDe(archivo, "pocion"), 3);
    ASSERT_EQ(cuantosDe(archivo, "flecha"), 2);
    ASSERT_EQ(cuantosDe(archivo, "escudo"), 0);
}

TEST_CASE(PosicionesEncadenadas) {
    const std::vector<std::size_t> got = posicionesDe("abracadabra", 'a');
    ASSERT_EQ(got.size(), 5u);
    ASSERT_EQ(got[0], 0u);
    ASSERT_EQ(got[4], 10u);
    ASSERT_EQ(posicionesDe("xxx", 'y').size(), 0u);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 02 (busqueda)...\n";
    return RUN_ALL_TESTS();
}
