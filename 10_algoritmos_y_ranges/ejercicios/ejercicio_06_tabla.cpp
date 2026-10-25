/* =============================================================================
   MÓDULO 10 - EJERCICIO 06: sort multi-criterio y stable_sort (lección 08)
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA (tema 5 de TEMAS.md: ordenamiento): la tabla de la temporada.
     Un `Resultado` guarda puntaje (más es mejor) y el nombre del equipo.
       1. tablaFinal(tabla): UN solo sort: puntaje DESC y, en empate,
          nombre ASC (con std::tie: negar el primer campo o un
          comparador que decida campo por campo).
       2. ordenAlfa(tabla): sort por nombre (el orden natural).
       3. ordenDeLlegada(tabla): stable_sort por puntaje DESC: los
          EMPATADOS deben conservar el orden en que estaban (el sort
          común no garantiza eso: es el punto del stable).
       4. podium(tabla): CÓPIA el vector, ordenalo con la MISMA regla de
          tablaFinal y devolvé solo los primeros 3.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_06_tabla.cpp -o ej06 && ./ej06
   ============================================================================= */
#include <algorithm>
#include <string>
#include <tuple>
#include <vector>
#include "../../include/test_framework.hpp"

struct Resultado {
    int puntaje;        // mas es mejor
    std::string nombre;
};

// TODO 1: tablaFinal: puntaje DESC, desempate nombre ASC (un solo sort).
// TU CODIGO AQUI:
void tablaFinal([[maybe_unused]] std::vector<Resultado>& tabla) {
}

// TODO 2: ordenAlfa: sort por nombre (el ASC default).
// TU CODIGO AQUI:
void ordenAlfa([[maybe_unused]] std::vector<Resultado>& tabla) {
}

// TODO 3: ordenDeLlegada: stable_sort por puntaje DESC conservando empates.
// TU CODIGO AQUI:
void ordenDeLlegada([[maybe_unused]] std::vector<Resultado>& tabla) {
}

// TODO 4: podium: devuelve los TRES primero (asumí tamaño >= 3 si te piden),
//         con puntaje DESC (reutilizando el mismo comparador de tablaFinal).
// TU CODIGO AQUI:
std::vector<Resultado> podium(
    [[maybe_unused]] std::vector<Resultado> tabla) {   // una copia: libre
    return {};    // placeholder
}

// ============= TESTS =============
TEST_CASE(TablaPorPuntajeYNombre) {
    std::vector<Resultado> tabla{
        {90, "Quesito"}, {90, "Mordel"}, {120, "Turbo"}, {90, "Lumen"},
    };
    tablaFinal(tabla);
    ASSERT_EQ(tabla[0].nombre, "Turbo");        // 120 arriba
    ASSERT_EQ(tabla[1].nombre, "Lumen");        // los 90: alfa
    ASSERT_EQ(tabla[2].nombre, "Mordel");
    ASSERT_EQ(tabla[3].nombre, "Quesito");
}

TEST_CASE(OrdenAlfa) {
    std::vector<Resultado> tabla{
        {90, "Quesito"}, {120, "Turbo"}, {45, "Lumen"},
    };
    ordenAlfa(tabla);
    ASSERT_EQ(tabla[0].nombre, "Lumen");
    ASSERT_EQ(tabla[1].nombre, "Quesito");
    ASSERT_EQ(tabla[2].nombre, "Turbo");
}

TEST_CASE(StableConservaLosEmpates) {
    std::vector<Resultado> tabla{
        {90, "Quesito"}, {90, "Mordel"}, {120, "Turbo"}, {90, "Lumen"},
    };
    ordenDeLlegada(tabla);
    ASSERT_EQ(tabla[0].nombre, "Turbo");
    ASSERT_EQ(tabla[1].nombre, "Quesito");      // llego antes
    ASSERT_EQ(tabla[2].nombre, "Mordel");       // su orden se conserva
    ASSERT_EQ(tabla[3].nombre, "Lumen");
}

TEST_CASE(PodiumSonLosTresMayores) {
    const std::vector<Resultado> tabla{
        {10, "A"}, {70, "B"}, {50, "C"}, {90, "D"}, {60, "E"}, {30, "F"},
    };
    const auto p = podium(tabla);
    ASSERT_EQ(p.size(), 3u);
    ASSERT_EQ(p[0].nombre, "D");                // 90
    ASSERT_EQ(p[1].nombre, "B");                // 70
    ASSERT_EQ(p[2].nombre, "E");                // 60
    ASSERT_EQ(tabla[0].nombre, "A");            // la original no cambio
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 06 (tabla)...\n";
    return RUN_ALL_TESTS();
}
