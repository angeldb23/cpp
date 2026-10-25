/* =============================================================================
   MÓDULO 10 - SOLUCIÓN 12: ranges::sort con proyección + inserción/stream
   (lecciones 17 y 18)
   =============================================================================
   Resolución:
       - ordenarPorCampo: ranges::sort acepta comp + proyección en la MISMA
         llamada (L17): el comparador greater aplicado sobre el campo
         hp via proyección: queda hp DESC.
       - masEntrenado: ranges::max_element con SOLO proyección (comp por
         defecto): devuelve un iterador; *iter es la referencia al max.
       - materializarNombres: la proyección &Pet::nombre dentro de un
         transform (L17) + back_inserter (L18): solo los nombres, tal cual
         llegan.
       - puntosProducidos: el copy entre streams de la L18: istream_iterator
         lee del istringstream hasta agotarlo y el back_inserter apila.
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

// SOLUCIÓN 1: comp + proyección en la MISMA llamada:
void ordenarPorCampo(std::vector<Pet>& personajes) {
    std::ranges::sort(personajes, std::greater<int>{}, &Pet::hp);
}

// SOLUCIÓN 2: el máximo por nivel, una sola llamada con proyección:
const Pet& masEntrenado(const std::vector<Pet>& personajes) {
    return *std::ranges::max_element(personajes, {}, &Pet::nivel);
}

// SOLUCIÓN 3: transform con proyección + back_inserter a un vector vacío:
std::vector<std::string> materializarNombres(const std::vector<Pet>& personajes) {
    std::vector<std::string> nombres;
    std::ranges::transform(personajes, std::back_inserter(nombres),
                           &Pet::nombre);
    return nombres;
}

// SOLUCIÓN 4: parse de un stream con istream_iterator + back_inserter:
std::vector<int> puntosProducidos(const std::string& texto) {
    std::istringstream entrada(texto);
    const std::vector<int> valores(
        std::istream_iterator<int>{entrada},
        std::istream_iterator<int>{});
    return valores;
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
    std::cout << "Solución 12 (rangos)...\n";
    return RUN_ALL_TESTS();
}
