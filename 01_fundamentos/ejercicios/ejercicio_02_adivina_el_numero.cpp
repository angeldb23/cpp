/* =============================================================================
   MÓDULO 1 - EJERCICIO 02: Mini Juego "Adivina el Número Secreto"
   =============================================================================
   CONSIGNA:
   Crea un mini juego interactivo donde la computadora genera un número secreto
   entre 1 y 100, y el usuario tiene que adivinarlo.

   Requisitos:
   1. Usar un bucle (while o do-while) que siga pidiendo números hasta que el
      usuario adivine el número secreto.
   2. En cada intento, si el número ingresado es menor al secreto, decir:
      "¡El número secreto es MÁS ALTO! Intenta de nuevo."
   3. Si el número ingresado es mayor al secreto, decir:
      "¡El número secreto es MÁS BAJO! Intenta de nuevo."
   4. Llevar la cuenta de la cantidad de intentos realizados.
   5. Al adivinar, felicitar al jugador y mostrar en cuántos intentos lo logró.

   COMPILACIÓN:
   g++ -std=c++17 -Wall ejercicio_02_adivina_el_numero.cpp -o ejercicio_02_adivina_el_numero
   ./ejercicio_02_adivina_el_numero
   ============================================================================= */

#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

int main() {
    // Inicializamos el generador de números aleatorios con la hora actual
    srand(static_cast<unsigned int>(time(nullptr)));

    // Genera un número aleatorio entre 1 y 100
    const int numeroSecreto = (rand() % 100) + 1;

    int intentoUsuario = 0;
    int cantidadIntentos = 0;

    cout << "========================================" << endl;
    cout << "   ¡BIENVENIDO A ADIVINA EL NÚMERO!    " << endl;
    cout << "  He elegido un número entre 1 y 100.  " << endl;
    cout << "========================================" << endl << endl;

    // TODO: Crea el bucle que interactúe con el usuario.
    // Pistas:
    // - Usa 'do-while' o 'while (intentoUsuario != numeroSecreto)'
    // - Cada vez que el usuario ingresa un número, incrementa 'cantidadIntentos++'
    // - Usa 'if', 'else if' y 'else' para comparar intentoUsuario con numeroSecreto
    
    // TU CÓDIGO AQUÍ:


    return 0;
}
