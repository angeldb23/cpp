/* =============================================================================
   MÓDULO 10 - SOLUCIÓN 02: rastreo con find_if y count_if (lección 03)
   =============================================================================
   Resolución directa de los cuatro TODOs con los algoritmos no
   modificantes:
       - primerVivo: find_if con ese predicado (!caido).
       - hayMuerto: `find_if(...) != end()`: el corta apenas aparece
         (o sea "hay algo": evitamos count_if que recorrería TODO).
       - cuantosDe: count_if con `s == nombre` (o count con el texto).
       - posicionesDe: el patrón encadenado de la lección: find y
         seguir desde std::next(it).
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

std::vector<Jugador>::iterator primerVivo(std::vector<Jugador>& jugadores) {
    return std::find_if(jugadores.begin(), jugadores.end(),
                        [](const Jugador& j) { return !j.caido; });
}

bool hayMuerto(const std::vector<Jugador>& jugadores) {
    return std::find_if(jugadores.begin(), jugadores.end(),
                        [](const Jugador& j) { return j.caido; })
           != jugadores.end();
}

int cuantosDe(const std::vector<std::string>& archivo,
              const std::string& nombre) {
    return static_cast<int>(
        std::count(archivo.begin(), archivo.end(), nombre));
}

std::vector<std::size_t> posicionesDe(const std::string& texto, char c) {
    std::vector<std::size_t> resultado;
    auto it = std::find(texto.begin(), texto.end(), c);
    while (it != texto.end()) {
        resultado.push_back(static_cast<std::size_t>(
            std::distance(texto.begin(), it)));
        it = std::find(std::next(it), texto.end(), c);
    }
    return resultado;
}

// ============= TESTS =============
TEST_CASE(PrimerVivoSaltaLosCaidos) {
    std::vector<Jugador> equipo{
        {"Quesito", true}, {"Mordel", false}, {"Turbo", false},
    };
    auto it = primerVivo(equipo);
    ASSERT_TRUE(it != equipo.end());
    ASSERT_TRUE(it->nombre == "Mordel");
}

TEST_CASE(PrimerVivoSinVivosEsEnd) {
    std::vector<Jugador> muertos{{"Lumen", true}};
    ASSERT_TRUE(primerVivo(muertos) == muertos.end());
}

TEST_CASE(HayMuertoCortaPorElPrimero) {
    const std::vector<Jugador> leer{
        {"A", false}, {"B", true}, {"C", true},
    };
    ASSERT_EQ(hayMuerto(leer), true);
    const std::vector<Jugador> sanos{{"A", false}};
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
    std::cout << "Solución 02 (busqueda)...\n";
    return RUN_ALL_TESTS();
}
