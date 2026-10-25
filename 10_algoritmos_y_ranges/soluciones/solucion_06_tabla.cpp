/* =============================================================================
   MÓDULO 10 - SOLUCIÓN 06: sort multi-criterio y stable_sort (lección 08)
   =============================================================================
   Resolución:
       - tablaFinal: UN sort donde el comparador compara std::tie con el
         puntaje NEGADO (el truco de la lección para "desc" dentro de tie):
         es exactamente lo mismo que comparar campo por campo.
       - ordenAlfa: sort con a.nombre < b.nombre.
       - ordenDeLlegada: stable_sort: los empates conservan el orden
         original (el punto diferencial vs sort).
       - podium: copiar el vector, ordenarlo con la MISMA clave y devolver
         los primeros 3.
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

namespace {

// la clave compartida: puntaje DESC y nombre ASC (el `>` va invertido):
bool antesEnTabla(const Resultado& a, const Resultado& b) {
    const int pa = -a.puntaje;          // negar para "desc" dentro del tie
    const int pb = -b.puntaje;
    return std::tie(pa, a.nombre) < std::tie(pb, b.nombre);
}

}  // namespace

// SOLUCIÓN 1: la tabla con UN solo sort y la clave común:
void tablaFinal(std::vector<Resultado>& tabla) {
    std::sort(tabla.begin(), tabla.end(), antesEnTabla);
}

// SOLUCIÓN 2: alfabeto, el orden natural:
void ordenAlfa(std::vector<Resultado>& tabla) {
    std::sort(tabla.begin(), tabla.end(),
              [](const Resultado& a, const Resultado& b) {
                  return a.nombre < b.nombre;
              });
}

// SOLUCIÓN 3: STABLE: el orden de llegada de los empates queda:
void ordenDeLlegada(std::vector<Resultado>& tabla) {
    std::stable_sort(tabla.begin(), tabla.end(),
                     [](const Resultado& a, const Resultado& b) {
                         return a.puntaje > b.puntaje;   // desc, sin desempate
                     });
}

// SOLUCIÓN 4: copia + la clave común + los primeros 3:
std::vector<Resultado> podium(std::vector<Resultado> tabla) {
    std::sort(tabla.begin(), tabla.end(), antesEnTabla);
    tabla.resize(3);
    return tabla;
}

// ============= TESTS =============
TEST_CASE(TablaPorPuntajeYNombre) {
    std::vector<Resultado> tabla{
        {90, "Quesito"}, {90, "Mordel"}, {120, "Turbo"}, {90, "Lumen"},
    };
    tablaFinal(tabla);
    ASSERT_EQ(tabla[0].nombre, "Turbo");
    ASSERT_EQ(tabla[1].nombre, "Lumen");
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
    ASSERT_EQ(tabla[1].nombre, "Quesito");
    ASSERT_EQ(tabla[2].nombre, "Mordel");
    ASSERT_EQ(tabla[3].nombre, "Lumen");
}

TEST_CASE(PodiumSonLosTresMayores) {
    const std::vector<Resultado> tabla{
        {10, "A"}, {70, "B"}, {50, "C"}, {90, "D"}, {60, "E"}, {30, "F"},
    };
    const auto p = podium(tabla);
    ASSERT_EQ(p.size(), 3u);
    ASSERT_EQ(p[0].nombre, "D");
    ASSERT_EQ(p[1].nombre, "B");
    ASSERT_EQ(p[2].nombre, "E");
    ASSERT_EQ(tabla[0].nombre, "A");
}

int main() {
    std::cout << "Solución 06 (tabla)...\n";
    return RUN_ALL_TESTS();
}
