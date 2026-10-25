/* =============================================================================
   MÓDULO 2 - LECCIÓN 03: Sobrecarga de Funciones y Argumentos por Defecto
   =============================================================================
   1. Sobrecarga de funciones (Function Overloading):
      En C++ podés tener varias funciones con el MISMO NOMBRE, siempre y cuando
      tengan diferente cantidad o tipo de parámetros. El compilador sabe cuál
      llamar según los argumentos que le pases.

   2. Parámetros por Defecto:
      Podés darle un valor predeterminado a los últimos parámetros de una función.
      Si el usuario no los pasa al llamarla, se usa ese valor.
   ============================================================================= */

#include <iostream>

using namespace std;

// 1. Sobrecarga: misma función, distintos tipos
int multiplicar(int a, int b) {
    cout << "[Llamando a version INT]: ";
    return a * b;
}

double multiplicar(double a, double b) {
    cout << "[Llamando a version DOUBLE]: ";
    return a * b;
}

// 2. Parámetros por defecto (siempre se colocan al final de la lista)
void aplicarDanio(int& vida, int danio, float multiplicadorCritico = 1.0f) {
    int danioFinal = static_cast<int>(danio * multiplicadorCritico);
    vida -= danioFinal;
    if (vida < 0) vida = 0;

    cout << "Danio infligido: " << danioFinal 
         << " (Critico x" << multiplicadorCritico << ") -> Vida restante: " 
         << vida << endl;
}

int main() {
    cout << "=== 1. SOBRECARGA DE FUNCIONES ===" << endl;
    cout << multiplicar(4, 5) << endl;         // Llama a la versión de int
    cout << multiplicar(2.5, 3.2) << endl;     // Llama a la versión de double
    cout << endl;

    cout << "=== 2. PARÁMETROS POR DEFECTO ===" << endl;
    int vidaEnemigo = 100;

    // Usando el valor por defecto (multiplicadorCritico = 1.0f):
    aplicarDanio(vidaEnemigo, 20);

    // Sobrescribiendo el valor por defecto:
    aplicarDanio(vidaEnemigo, 20, 2.5f);

    return 0;
}
