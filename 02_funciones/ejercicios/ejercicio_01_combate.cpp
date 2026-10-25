/* =============================================================================
   MÓDULO 2 - EJERCICIO 01: Sistema de Combate y Paso por Referencia
   =============================================================================
   CONSIGNA:
   Implementa un sistema de simulación de combate utilizando funciones y paso
   por referencia.

   Debes programar las siguientes funciones:
   1. 'void recibirAtaque(int& vidaObjetivo, int danio, int defensa)'
      - El daño recibido real es: danio - defensa.
      - Si el daño resultante es menor que 1, el daño mínimo siempre es 1.
      - Restar el daño a 'vidaObjetivo' (usando referencia para que modifique la variable real).
      - Si la vida cae por debajo de 0, ajustarla a 0.

   2. 'void subirDeNivel(int& nivel, int& ataqueMaximo, int& vidaMaxima)'
      - Aumenta 'nivel' en 1.
      - Aumenta 'ataqueMaximo' en un 15%.
      - Aumenta 'vidaMaxima' en 20 puntos.

   3. 'bool estaVivo(int vida)'
      - Devuelve 'true' si vida > 0, o 'false' en caso contrario.

   COMPILACIÓN:
   g++ -std=c++17 -Wall ejercicio_01_combate.cpp -o ejercicio_01_combate
   ./ejercicio_01_combate
   ============================================================================= */

#include <iostream>

using namespace std;

// TODO 1: Declara e implementa la función 'recibirAtaque'
void recibirAtaque(int& vidaObjetivo, int danio, int defensa) {
    int dmg = danio - defensa;
    if (dmg < 1) dmg = 1;
    vidaObjetivo -= dmg;
    if (vidaObjetivo < 0) vidaObjetivo = 0;
}


// TODO 2: Declara e implementa la función 'subirDeNivel'
void subirDeNivel(int& nivel, int& ataqueMaximo, int& vidaMaxima) {
    ++nivel;
    ataqueMaximo = static_cast<int>(ataqueMaximo * 1.15);
    vidaMaxima += 20;
}


// TODO 3: Declara e implementa la función 'estaVivo'
bool estaVivo(int vida) {
    return vida > 0;
}


int main() {
    int nivel = 1;
    int vidaMax = 100;
    int vidaActual = 100;
    int ataque = 25;
    int defensa = 5;

    cout << "=== ESTADO INICIAL DEL HEROE ===" << endl;
    cout << "Nivel: " << nivel << " | Vida: " << vidaActual << "/" << vidaMax << " | Ataque: " << ataque << endl;
    cout << endl;

    cout << "=== SIMULANDO COMBATE ===" << endl;
    cout << "Un orco ataca con 30 de poder!" << endl;
    
    // TODO 4: Llama a 'recibirAtaque' pasando vidaActual, 30 de daño y la defensa
    // TU CÓDIGO AQUÍ:


    cout << "Vida restante: " << vidaActual << endl;

    // TODO 5: Verifica si sigue vivo usando 'estaVivo' y muestra un mensaje
    // TU CÓDIGO AQUÍ:


    cout << endl << "=== SUBIENDO DE NIVEL ===" << endl;
    // TODO 6: Llama a 'subirDeNivel'
    // TU CÓDIGO AQUÍ:


    cout << "Nuevo Nivel: " << nivel << " | Nueva Vida Max: " << vidaMax << " | Nuevo Ataque: " << ataque << endl;

    return 0;
}
