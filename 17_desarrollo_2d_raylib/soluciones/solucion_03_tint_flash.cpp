/* =============================================================================
   MÓDULO 17 - SOLUCIÓN 03: Tint — LerpColor y Flash de Daño
   =============================================================================
   Resuelve ejercicio_03_tint_flash.cpp. El canal float→unsigned char se
   redondea con lround para que el error de acumulación no se acumula en el
   test a mitad de camino.

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_03_tint_flash.cpp -o sol && ./sol
   ============================================================================= */
#include <iostream>
#include <algorithm>
#include <cmath>
#include "../../include/test_framework.hpp"

// ============================ STUBS (no tocar) ===============================
struct Color { unsigned char r, g, b, a; };
inline bool operator==(const Color& a, const Color& b) {
    return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a;
}
const Color WHITE{255,255,255,255};
const Color RED{255, 0, 0, 255};
const Color PLAYER_BLUE{40,120,255,255};

// ============================ SOLUCIÓN =======================================
// TODO 1: interpolación por canal con clamp de t
static unsigned char lerpChannel(unsigned char a, unsigned char b, float t) {
    // Interpola en float y redondea al cast: evita el sesgo hacia abajo del
    // truncamiento (que romperia el test a mitad de camino)
    const float v = float(a) + (float(b) - float(a)) * std::clamp(t, 0.0f, 1.0f);
    return static_cast<unsigned char>(std::lround(v));
}

Color lerpColor(Color a, Color b, float t) {
    return Color{
        lerpChannel(a.r, b.r, t),
        lerpChannel(a.g, b.g, t),
        lerpChannel(a.b, b.b, t),
        lerpChannel(a.a, b.a, t)
    };
}

// TODO 2: flash de daño hacia ROJO (r=255, g=0, b=0), alpha intacto
Color damageFlash(Color current, float t) {
    const Color flashTarget{255, 0, 0, current.a};
    return lerpColor(current, flashTarget, std::clamp(t, 0.0f, 1.0f));
}

// ============================ TESTS (iguales) ================================
TEST_CASE(LerpEnTeroEsElOrigen) {
    Color c = lerpColor(WHITE, RED, 0.0f);
    ASSERT_EQ(int(c.r), int(WHITE.r));
    ASSERT_EQ(int(c.g), int(WHITE.g));
    ASSERT_EQ(int(c.a), int(WHITE.a));
}

TEST_CASE(LerpEnUnoEsElDestino) {
    Color c = lerpColor(WHITE, RED, 1.0f);
    ASSERT_EQ(int(c.r), 255);
    ASSERT_EQ(int(c.g), 0);
    ASSERT_EQ(int(c.a), 255);
}

TEST_CASE(LerpEnMitadSonValoresMedios) {
    Color c = lerpColor(WHITE, PLAYER_BLUE, 0.5f);
    ASSERT_TRUE(std::abs(int(c.r) - 147) <= 1);
    ASSERT_TRUE(std::abs(int(c.g) - 187) <= 1);
    ASSERT_TRUE(std::abs(int(c.b) - 255) <= 1);
}

TEST_CASE(LerpClampaTFueradeRango) {
    Color c1 = lerpColor(WHITE, RED, -0.5f);
    ASSERT_EQ(int(c1.r), int(WHITE.r));
    Color c2 = lerpColor(WHITE, RED, 2.0f);
    ASSERT_EQ(int(c2.r), 255);
}

TEST_CASE(FlashNuloYMaximo) {
    ASSERT_TRUE(damageFlash(PLAYER_BLUE, 0.0f) == PLAYER_BLUE);
    Color flash = damageFlash(PLAYER_BLUE, 1.0f);
    ASSERT_EQ(int(flash.r), 255);
    ASSERT_EQ(int(flash.b), 0);
    ASSERT_EQ(int(flash.a), int(PLAYER_BLUE.a));
}

TEST_CASE(FlashMitadEsRojoParcial) {
    Color mid = damageFlash(PLAYER_BLUE, 0.5f);
    ASSERT_TRUE(std::abs(int(mid.b) - 127) <= 1);
    ASSERT_TRUE(std::abs(int(mid.r) - 147) <= 1);
}

int main() {
    std::cout << "Solución 03: Tint/Flash\n";
    return RUN_ALL_TESTS();
}
