/* =============================================================================
   MÓDULO 2 - LECCIÓN 01: Funciones Básicas, Parámetros y Retorno
   =============================================================================
   Una función es un bloque de código reutilizable que realiza una tarea
   específica. En C++, las funciones nos permiten modularizar y aplicar el
   principio DRY (Don't Repeat Yourself).

   Conceptos clave:
   1. Declaración (prototipo) vs Definición.
   2. Parámetros de entrada y valor de retorno.
   3. Funciones 'void' (que no retornan nada).
   4. Ámbito (Scope) de variables locales.
   ============================================================================= */

#include <iostream>

using namespace std;

// 1. PROTOTIPO O DECLARACIÓN
// Le avisa al compilador que esta función existe antes de que la llame main()
int sumar(int a, int b);
void saludar(string nombre);
bool esPar(int numero);

int main() {
    cout << "=== 1. LLAMANDO A FUNCIONES ===" << endl;
    saludar("Angel");

    int resultado = sumar(15, 25);
    cout << "Resultado de sumar(15, 25): " << resultado << endl;

    int valor = 8;
    if (esPar(valor)) {
        cout << "El numero " << valor << " es PAR." << endl;
    } else {
        cout << "El numero " << valor << " es IMPAR." << endl;
    }

    return 0;
}

// 2. DEFINICIÓN DE LAS FUNCIONES

// Función con retorno int y dos parámetros int pasados por valor (copia)
int sumar(int a, int b) {
    return a + b;
}

// Función que no devuelve nada (void)
void saludar(string nombre) {
    cout << "¡Hola, " << nombre << "! Bienvenido a la leccion de funciones." << endl;
}

// Función que devuelve un booleano (true/false)
bool esPar(int numero) {
    return (numero % 2 == 0);
}
