/* =============================================================================
   MÓDULO 10 - EJERCICIO 12: ranges::sort con proyección + inserción/stream
   (lecciones 17 y 18)
   =============================================================================
   DIFICULTAD: Intermedio-Avanzado
   CONSIGNA (temas 9 y 10 de TEMAS.md): proyecciones y salida.
     1. ordenarPorCampo(personajes): UNA llamada a ranges::sort con
        comparador + proyección juntas: comp = hp DESC (greater) y
        proyección = el campo hp (&Pet::hp, la L17).
     2. masEntrenado(personajes): ranges::max_element CON proyección por
        nivel; devolver la referencia al máximo (una sola llamada).
     3. materializarNombres(personajes): un transform que proyecte el
        nombre y un back_inserter (L18) a un vector VACÍO: SOLO los
        nombres, en el orden que llega.
     4. puntosProducidos(texto): "10 20 30" -> vector<int> con
        istream_iterator + back_inserter (el copy de streams del L18:
        sin bucles ni push_back manual).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_12_rangos.cpp -o ej12 && ./ej12
   ============================================================================= */
#include <algorithm>
#include <functional>
#include <iterator>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

struct Pet {
    std::string nombre;
    int hp;
    int nivel;
};

// TODO 1: ordenarPorCampo: hp DESC con comp+proyeccion EN LA MISMA llamada.
// TU CODIGO AQUI:
void ordenarPorCampo([[maybe_unused]] std::vector<Pet>& personajes) {
}

// TODO 2: masEntrenado: el max por nivel con proyeccion (una llamada).
// TU CODIGO AQUI (asumí non-empty):
const Pet& masEntrenado([[maybe_unused]] const std::vector<Pet>& personajes) {
    static const Pet vacio{"", 0, 0};
    return vacio;    // placeholder
}

// TODO 3: materializarNombres: solo los nombres, a un vector VACIO
//         (transform + back_inserter).
// TU CODIGO AQUI:
std::vector<std::string> materializarNombres(
    [[maybe_unused]] const std::vector<Pet>& personajes) {
    return {};    // placeholder
}

// TODO 4: puntosProducidos: parse de un texto "10 20 30" a vector<int>
//         con istream_iterator y back_inserter (sin bucles).
// TU CODIGO AQUI:
std::vector<int> puntosProducidos([[maybe_unused]] const std::string& texto) {
    return {};    // placeholder
}

// ============= TESTS =============
TEST_CASE(OrdenarPorHpDescen) {
    std::vector<Pet> roster{
        {"Quesito", 90, 3}, {"Mordel", 60, 7}, {"Turbo", 120, 2},
    };
    ordenarPorCampo(roster);
    ASSERT_EQ(roster[0].nombre, "Turbo");       // 120
    ASSERT_EQ(roster[1].nombre, "Quesito");     // 90
    ASSERT_EQ(roster[2].nombre, "Mordel");      // 60
}

TEST_CASE(MasEntrenadoPorNivel) {
    const std::vector<Pet> roster{
        {"Quesito", 90, 3}, {"Mordel", 60, 7}, {"Turbo", 120, 2},
    };
    ASSERT_EQ(masEntrenado(roster).nombre, "Mordel");   // nivel 7
    ASSERT_EQ(masEntrenado(roster).hp, 60);
}

TEST_CASE(NombresNeto) {
    const std::vector<Pet> roster{
        {"Quesito", 90, 3}, {"Mordel", 60, 7},
    };
    const auto nj = materializarNombres(roster);
    ASSERT_EQ(nj.size(), 2u);
    ASSERT_EQ(nj[0], "Quesito");
    ASSERT_EQ(nj[1], "Mordel");
    ASSERT_TRUE(materializarNombres({}).empty());
}

TEST_CASE(PuntosDelStream) {
    ASSERT_TRUE((puntosProducidos("10 20 30") == std::vector<int>{10, 20, 30}));
    ASSERT_TRUE((puntosProducidos("7") == std::vector<int>{7}));
    ASSERT_TRUE(puntosProducidos("").empty());
    ASSERT_TRUE((puntosProducidos("1 2 3 4 5") ==
                 std::vector<int>{1, 2, 3, 4, 5}));
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 12 (rangos)...\n";
    return RUN_ALL_TESTS();
}
