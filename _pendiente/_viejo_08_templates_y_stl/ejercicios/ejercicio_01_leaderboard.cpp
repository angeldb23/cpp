/* =============================================================================
   MÓDULO 8 - EJERCICIO 01: Tabla de Puntuaciones (Leaderboard) con STL
   =============================================================================
   CONSIGNA:
   Crea una tabla de clasificación de jugadores utilizando 'std::vector', 'struct'
   y el algoritmo 'std::sort' con lambdas.

   1. Declara un 'struct JugadorScore':
      - 'string nombre'
      - 'int puntos'

   2. En 'main()':
      - Crea un 'vector<JugadorScore> leaderboard'.
      - Inserta al menos 5 jugadores con diferentes puntajes.
      - Usa 'std::sort' con una expresión Lambda para ordenar el vector de
        MAYOR a MENOR puntaje (descendente).
      - Imprime la tabla formateada en pantalla mostrando el Top 5 con su puesto:
        "1. [Nombre] - [Puntos] pts"
        "2. [Nombre] - [Puntos] pts", etc.

   COMPILACIÓN:
   g++ -std=c++17 -Wall ejercicio_01_leaderboard.cpp -o ejercicio_01_leaderboard
   ./ejercicio_01_leaderboard
   ============================================================================= */

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

// TODO 1: Declara el struct 'JugadorScore'
// TU CÓDIGO AQUÍ:


int main() {
    cout << "========================================" << endl;
    cout << "     TABLA DE PUNTUACIONES (TOP 5)      " << endl;
    cout << "========================================" << endl;

    // TODO 2: Declara el vector y agrega al menos 5 puntuaciones
    // TU CÓDIGO AQUÍ:


    // TODO 3: Ordena el vector de mayor a menor con std::sort y una lambda
    // TU CÓDIGO AQUÍ:


    // TODO 4: Imprime los resultados con su puesto (1, 2, 3...)
    // TU CÓDIGO AQUÍ:


    return 0;
}
