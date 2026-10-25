/* =============================================================================
   MÓDULO 2 - LECCIÓN 02: Paso por Valor vs Paso por Referencia (&)
   =============================================================================
   En C++, cómo pasás los argumentos a una función es CRUCIAL para el rendimiento
   y para saber si una variable original se modificará o no.

   1. PASO POR VALOR (Copia):
      La función crea una copia idéntica en memoria. Modificarla dentro de la
      función NO cambia la variable original afuera.
      - Ideal para: tipos primitivos chicos (int, float, bool, char).

   2. PASO POR REFERENCIA (Alias con '&'):
      La función NO crea una copia. Recibe el acceso directo a la variable original.
      Cualquier cambio dentro de la función afecta DIRECTAMENTE a la original.
      - Ideal para: cuando necesitás modificar la variable original.

   3. PASO POR REFERENCIA CONSTANTE ('const Tipo&'):
      Evita la copia (máxima velocidad) pero 'const' prohíbe modificarla.
      - Es la regla de oro para pasar objetos pesados (strings, vectores, clases).
   ============================================================================= */

#include <iostream>
#include <string>

using namespace std;

// 1. Paso por copia (no modifica el original)
void intentarCurarCopia(int vida) {
    vida += 50;
    cout << "  [Dentro de intentarCurarCopia] vida = " << vida << endl;
}

// 2. Paso por referencia con '&' (modifica la variable real)
void curarReal(int& vida) {
    vida += 50;
    cout << "  [Dentro de curarReal] vida = " << vida << endl;
}

// 3. Paso por referencia constante (rápido, sin copia, de solo lectura)
void mostrarEstadisticas(const string& nombre, const int& vida) {
    // nombre += "!"; // ERROR DE COMPILACIÓN: no se puede modificar porque es 'const'
    cout << "Jugador: " << nombre << " | Salud: " << vida << " HP" << endl;
}

// 4. Ejemplo clásico: Intercambiar dos variables (swap)
void intercambiar(int& a, int& b) {
    int temporal = a;
    a = b;
    b = temporal;
}

int main() {
    int vidaJugador = 100;
    string nombreJugador = "Guerrero";

    cout << "=== ESTADO INICIAL ===" << endl;
    mostrarEstadisticas(nombreJugador, vidaJugador);
    cout << endl;

    cout << "=== 1. PROBANDO PASO POR COPIA ===" << endl;
    intentarCurarCopia(vidaJugador);
    cout << "  [En main despues de intentarCurarCopia] vida = " << vidaJugador 
         << " (¡No cambio!)" << endl << endl;

    cout << "=== 2. PROBANDO PASO POR REFERENCIA (&) ===" << endl;
    curarReal(vidaJugador);
    cout << "  [En main despues de curarReal] vida = " << vidaJugador 
         << " (¡Si cambio!)" << endl << endl;

    cout << "=== 3. PROBANDO INTERCAMBIAR CON REFERENCIAS ===" << endl;
    int oro = 50;
    int gemas = 10;
    cout << "Antes del swap: Oro = " << oro << ", Gemas = " << gemas << endl;
    intercambiar(oro, gemas);
    cout << "Despues del swap: Oro = " << oro << ", Gemas = " << gemas << endl;

    return 0;
}
