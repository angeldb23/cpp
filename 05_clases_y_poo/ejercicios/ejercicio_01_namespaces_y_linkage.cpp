/* =============================================================================
   MÓDULO 05 - EJERCICIO 01: Namespaces, linkage y la idea de "header"
   =============================================================================
   DIFICULTAD: Básico
   PRACTICA: lecciones 01-02 (ODR, static/extern, namespaces sin
   `using namespace std`).

   CONSIGNA:
     En un solo archivo (simulando lo que iría al header y al .cpp):

     1. Armá un namespace `juego` que contenga:
        - una VARIABILE global del juego: `int vidas = 3;` (external)
        - la función `int golpear(int danio, int vidaObjetivo)` que
          devuelve la vida restante, clampeada a 0 como mínimo.
     2. Definí una función de utilidad con linkage INTERNO:
        `static bool ceroVidas(int vida)` → true si vida <= 0.
     3. En el main: usá `juego::golpear` y, con un `using` declarativo,
        importá solo `juego::vidas` para leerla sin prefijo.

     Descomentá y completá los TODO. El resto está armado.

   COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra ejercicio_01_namespaces_y_linkage.cpp -o ej01 && ./ej01
   ============================================================================= */
#include <iostream>
#include "../../include/test_framework.hpp"

namespace juego {
// TODO 1a: variable `vidas` inicializada en 3 (dentro de este namespace);
// reemplazá la línea placeholder por la tuya.
int vidas = -1;  // placeholder: valor a reemplazar

// TODO 1b: función `golpear(danio, vidaObjetivo)`: devolvé
// max(vidaObjetivo - danio, 0). Sin usar <algorithm>: a mano.
int golpear(int danio, int vidaObjetivo) {
    (void)danio; (void)vidaObjetivo;
    return -1;  // placeholder
}
}  // namespace juego

// TODO 2: función con INTERNAL linkage (static) `ceroVidas(int vida)`;
// reemplazá el cuerpo placeholder por el tuyo.
static bool ceroVidas(int vida) {
    (void)vida;
    return true;  // placeholder: siempre true (el test de vida 1 debe fallar)
}

// ============= TESTS =============
TEST_CASE(TestGolpear) {
    ASSERT_EQ(juego::golpear(10, 40), 30);
    ASSERT_EQ(juego::golpear(45, 40), 0);    // clamp a 0
    ASSERT_EQ(juego::golpear(0, 9), 9);
}

TEST_CASE(TestCeroVidas) {
    ASSERT_TRUE(ceroVidas(0));
    ASSERT_TRUE(ceroVidas(-3));
    ASSERT_FALSE(ceroVidas(1));
}

TEST_CASE(TestNamespaceYGlobals) {
    ASSERT_EQ(juego::vidas, 3);
    juego::vidas = 2;            // variable global mutable del juego
    ASSERT_EQ(juego::vidas, 2);  // (tests NO revierten: el siguiente
                                 // test la vuelve a fijar)
    juego::vidas = 3;
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 01...\n";
    return RUN_ALL_TESTS();
}
