/* =============================================================================
   MÓDULO 03 - SOLUCIÓN 03: Tokenizador de Comandos
   =============================================================================
   POR QUÉ FUNCIONA:
     - find(separador, inicio) busca desde una posición; si no encuentra devuelve npos.
     - Cuando no hay más separadores, el token llega hasta el final (fin = size()).
     - Con texto vacío el bucle corre una vez, no agrega nada y termina: resultado {}.
     - emplace_back construye el std::string directo dentro del vector.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_03_tokenizador.cpp -o sol03 && ./sol03
   ============================================================================= */
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include "../../include/test_framework.hpp"

// Trabajamos sobre string_view para no copiar la línea entera; solo copiamos
// cada token a un std::string al guardarlo en el vector.
// Decisión de diseño: los tokens vacíos (separadores seguidos) se descartan.
std::vector<std::string> tokenizar(std::string_view linea, char separador) {
    std::vector<std::string> tokens;
    std::size_t inicio = 0;
    while (inicio <= linea.size()) {
        std::size_t fin = linea.find(separador, inicio);
        if (fin == std::string_view::npos) fin = linea.size();
        if (fin > inicio) {
            tokens.emplace_back(linea.substr(inicio, fin - inicio));
        }
        inicio = fin + 1;
    }
    return tokens;
}

// ============= TESTS (los mismos del ejercicio) =============
TEST_CASE(TestTokenizarComa) {
    auto r = tokenizar("mover,10,20", ',');
    ASSERT_EQ(static_cast<int>(r.size()), 3);
    ASSERT_EQ(r[0], std::string("mover"));
    ASSERT_EQ(r[1], std::string("10"));
    ASSERT_EQ(r[2], std::string("20"));
}

TEST_CASE(TestTokenizarEspacio) {
    auto r = tokenizar("hola mundo fin", ' ');
    ASSERT_EQ(static_cast<int>(r.size()), 3);
    ASSERT_EQ(r[0], std::string("hola"));
    ASSERT_EQ(r[2], std::string("fin"));
}

TEST_CASE(TestTokenizarVacio) {
    auto r = tokenizar("", ',');
    ASSERT_EQ(static_cast<int>(r.size()), 0);
}

TEST_CASE(TestTokenizarUno) {
    auto r = tokenizar("solo", ',');
    ASSERT_EQ(static_cast<int>(r.size()), 1);
    ASSERT_EQ(r[0], std::string("solo"));
}

int main() {
    std::cout << "Ejecutando pruebas para Solución 03 (Tokenizador de Comandos)...\n";
    return RUN_ALL_TESTS();
}
