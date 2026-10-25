/* =============================================================================
   MÓDULO 17 - EJERCICIO 03: Tint — LerpColor y Flash de Daño
   =============================================================================
   DIFICULTAD: Básico
   APOYO: Lección 02 (Tint y efectos de color)

   CONSIGNA:
   1) Implementa lerpColor(a, b, t): interpola por canal r/g/b/a entre dos
      colores con t en [0,1]. FUERA DE RANGO: clamp (t<0 => 0, t>1 => 1).
   2) Implementa DamageFlash(current, t): con t en [0,1] como "intensidad del
      flash", mezcla el color actual con ROJO puro: t=0 => sin flash,
      t=1 => todo rojo (alpha intacta).

   El tint multiplicativo de raylib (de la lección 02) es:
     pixel_final.r = pixel_textura.r * tint.r / 255
   En esta lección el flash se simula creando el TINT correcto.

   TESTS (no tocar): interpolación cero/uno/mitad y el flash.

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_03_tint_flash.cpp -o ej && ./ej
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

// ============================ TU CÓDIGO AQUÍ ================================
// TODO 1: interpolación por canal con clamp de t
Color lerpColor(Color a, Color b, float t) {
    // Pista: canal por canal; devuelve unsigned char (con round si querés precisión)
    return {};   // placeholder
}

// TODO 2: flash de daño hacia color ROJO
Color damageFlash(Color current, float t) {
    return current;   // placeholder (sin t, nunca cambia)
}

// ============================ TESTS (no tocar) ==============================
TEST_CASE(LerpEnTeroEsElOrigen) {
    Color c = lerpColor(WHITE, RED, 0.0f);
    ASSERT_EQ(int(c.r), int(WHITE.r));
    ASSERT_EQ(int(c.g), int(WHITE.g));
    ASSERT_EQ(int(c.a), int(WHITE.a));
}

TEST_CASE(LerpEnUnoEsElDestino) {
    Color c = lerpColor(WHITE, RED, 1.0f);
    ASSERT_EQ(int(c.r), 255);      // RED.r
    ASSERT_EQ(int(c.g), 0);
    ASSERT_EQ(int(c.a), 255);
}

TEST_CASE(LerpEnMitadSonValoresMedios) {
    // WHITE → PLAYER_BLUE a t=0.5: r=(255+40)/2=147.5 → ~147 o 148
    Color c = lerpColor(WHITE, PLAYER_BLUE, 0.5f);
    ASSERT_TRUE(std::abs(int(c.r) - 147) <= 1);
    ASSERT_TRUE(std::abs(int(c.g) - 187) <= 1);   // (255+120)/2 = 187
    ASSERT_TRUE(std::abs(int(c.b) - 255) <= 1);
}

TEST_CASE(LerpClampaTFueradeRango) {
    Color c1 = lerpColor(WHITE, RED, -0.5f);   // t<0 => t=0
    ASSERT_EQ(int(c1.r), int(WHITE.r));
    Color c2 = lerpColor(WHITE, RED, 2.0f);    // t>1 => t=1
    ASSERT_EQ(int(c2.r), 255);
}

TEST_CASE(FlashNuloYMaximo) {
    // t=0: color intacto
    ASSERT_TRUE(damageFlash(PLAYER_BLUE, 0.0f) == PLAYER_BLUE);
    // t=1: totalmente rojo pero mismo alpha
    Color flash = damageFlash(PLAYER_BLUE, 1.0f);
    ASSERT_EQ(int(flash.r), 255);
    ASSERT_EQ(int(flash.b), 0);
    ASSERT_EQ(int(flash.a), int(PLAYER_BLUE.a));
}

TEST_CASE(FlashMitadEsRojoParcial) {
    Color mid = damageFlash(PLAYER_BLUE, 0.5f);
    // b del player (255) tiende a 0 con la mitad
    ASSERT_TRUE(std::abs(int(mid.b) - 127) <= 1);
    // r del player (40) tiende a 255 con la mitad
    ASSERT_TRUE(std::abs(int(mid.r) - 147) <= 1);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 03 (Tint/Flash)...\n";
    return RUN_ALL_TESTS();
}
