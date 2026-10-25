/* =============================================================================
   MÓDULO 05 - SOLUCIÓN 01: Namespaces, linkage y la idea de "header"
   =============================================================================
   POR QUÉ FUNCIONA:
     - `juego::golpear` hace el clamp con una comparación: si el
       resultado sería negativo, devuelve 0 (invariante de la vida).
     - `ceroVidas` es `static`: internal linkage, compartiendo nombre
       libre para otros .cpp que quieran su propia copia.
     - `using juego::vidas;` importa SOLO ese nombre: el hábito debe ser
       una importación quirúrgica, nunca using namespace en headers.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_01... -o sol01 && ./sol01
   ============================================================================= */
#include <iostream>
#include "../../include/test_framework.hpp"

namespace juego {
int vidas = 3;               // external: visible desde otros .cpp

int golpear(int danio, int vidaObjetivo) {
    int restante = vidaObjetivo - danio;
    if (restante < 0) {
        restante = 0;        // clamp
    }
    return restante;
}
}  // namespace juego

// static = internal linkage: existe UNA en este archivo y no pelea.
static bool ceroVidas(int vida) {
    return vida <= 0;
}

// ============= TESTS =============
TEST_CASE(TestGolpear) {
    ASSERT_EQ(juego::golpear(10, 40), 30);
    ASSERT_EQ(juego::golpear(45, 40), 0);
    ASSERT_EQ(juego::golpear(0, 9), 9);
}

TEST_CASE(TestCeroVidas) {
    ASSERT_TRUE(ceroVidas(0));
    ASSERT_TRUE(ceroVidas(-3));
    ASSERT_FALSE(ceroVidas(1));
}

TEST_CASE(TestNamespaceYGlobals) {
    ASSERT_EQ(juego::vidas, 3);
    juego::vidas = 2;
    ASSERT_EQ(juego::vidas, 2);
    juego::vidas = 3;
}

int main() {
    using juego::vidas;   // using declarativo: solo ese nombre
    std::cout << "vidas (sin prefijo): " << vidas << "\n";
    std::cout << "Ejecutando pruebas para Solución 01...\n";
    return RUN_ALL_TESTS();
}
