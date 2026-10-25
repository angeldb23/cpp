/* =============================================================================
   MÓDULO 02 - SOLUCIÓN 01: Sistema de Combate y Paso por Referencia
   =============================================================================
   Implementación de las funciones solicitadas en el ejercicio de combate.
   ============================================================================= */

#include <iostream>
#include "../../include/test_framework.hpp"

// Reduce la vida del objetivo según daño y defensa, mínimo 1 daño.
void recibirAtaque(int& vidaObjetivo, int danio, int defensa) {
    int dmg = danio - defensa;
    if (dmg < 1) dmg = 1;
    vidaObjetivo -= dmg;
    if (vidaObjetivo < 0) vidaObjetivo = 0;
}

// Incrementa nivel, ataque máximo (15%) y vida máxima (+20).
void subirDeNivel(int& nivel, int& ataqueMaximo, int& vidaMaxima) {
    ++nivel;
    ataqueMaximo = static_cast<int>(ataqueMaximo * 1.15);
    vidaMaxima += 20;
}

// Indica si la vida es mayor que 0.
bool estaVivo(int vida) {
    return vida > 0;
}

TEST_CASE(TestCombate) {
    int vida = 100;
    recibirAtaque(vida, 30, 5);
    ASSERT_EQ(vida, 75); // 30-5=25 dmg
    ASSERT_TRUE(estaVivo(vida));
    int nivel = 1, ataque = 20, vidaMax = 100;
    subirDeNivel(nivel, ataque, vidaMax);
    ASSERT_EQ(nivel, 2);
    ASSERT_EQ(ataque, static_cast<int>(20 * 1.15));
    ASSERT_EQ(vidaMax, 120);
}

int main() {
    std::cout << "[SOLUCIÓN] Ejecutando tests de Solución 01..." << std::endl;
    return RUN_ALL_TESTS();
}
