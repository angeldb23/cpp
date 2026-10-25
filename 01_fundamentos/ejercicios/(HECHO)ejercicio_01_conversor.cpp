/* =============================================================================
   MÓDULO 1 - EJERCICIO 01: Conversor de Temperatura y Precisión
   =============================================================================
   CONSIGNA:
   Escribe un programa que:
   1. Le pida al usuario una temperatura en grados Celsius (puede tener
   decimales).
   2. Convierta esa temperatura a Fahrenheit usando la fórmula:
         Fahrenheit = (Celsius * 9.0 / 5.0) + 32.0
   3. Convierta esa temperatura a Kelvin usando la fórmula:
         Kelvin = Celsius + 273.15
   4. Muestre ambos resultados por pantalla formateados con 2 decimales usando
      fixed y setprecision(2) de <iomanip>.
   5. Como desafío extra: Si la temperatura en Celsius es menor a -273.15,
      imprimir un mensaje de advertencia: "¡Esa temperatura está por debajo del
   cero absoluto!".

   COMPILACIÓN:
   g++ -std=c++17 -Wall ejercicio_01_conversor.cpp -o ejercicio_01_conversor
   ./ejercicio_01_conversor
   =============================================================================
 */

#include <iomanip>
#include <iostream>

using namespace std;

int main() {
  cout << "========================================" << endl;
  cout << "    CONVERSOR DE TEMPERATURA EN C++     " << endl;
  cout << "========================================" << endl;

  // TODO 1: Declara una variable de tipo double llamada 'celsius'
  // TU CÓDIGO AQUÍ:
  double celsius;

  // TODO 2: Pídele al usuario que ingrese la temperatura en Celsius con cout y
  // cin
  // TU CÓDIGO AQUÍ:

  cout << "Introduce la temperatura en celsius: ";
  cin >> celsius;

  // TODO 3: Valida con un 'if' si celsius es menor a -273.15
  // TU CÓDIGO AQUÍ:

  if (celsius < -273.15) {
    cout << "Error, la temperatura no puede estar por debajo del 0 absoluto."
         << endl;
    return 1;
  }

  // TODO 4: Declara y calcula las variables 'fahrenheit' y 'kelvin' usando las
  // fórmulas
  // TU CÓDIGO AQUÍ:

  double fahrenheit = (celsius * 9.0 / 5.0) + 32.0;
  double kelvin = celsius + 273.15;

  // TODO 5: Muestra los resultados con fixed y setprecision(2)
  // TU CÓDIGO AQUÍ:

  cout << fixed << setprecision(2);
  cout << "Temperatura en Fahrenheit: " << fahrenheit << " °F" << endl;
  cout << "Temperatura en Kelvin: " << kelvin << " K" << endl;

  return 0;
}
