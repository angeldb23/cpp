/* =============================================================================
   MÓDULO 1 - LECCIÓN 02: Bucles y Control de Flujo
   =============================================================================
   En esta lección aprenderemos a controlar el flujo de ejecución de nuestro
   programa: tomar decisiones y repetir acciones.

   Conceptos clave:
   1. Condicionales: if, else if, else.
   2. Selección múltiple: switch y la importancia del 'break'.
   3. Bucle 'for': cuando sabés cuántas veces querés repetir algo.
   4. Bucle 'while': cuando repetís mientras se cumpla una condición.
   5. Bucle 'do-while': se ejecuta AL MENOS una vez antes de verificar la condición.
   6. Control dentro de bucles: 'break' (sale del bucle) y 'continue' (salta a la siguiente vuelta).
   ============================================================================= */

#include <iostream>

using namespace std;

int main() {
    cout << "=== 1. CONDICIONALES (if, else if, else) ===" << endl;
    int vidaJugador = 45;

    if (vidaJugador <= 0) {
        cout << "Estado: El jugador ha muerto." << endl;
    } else if (vidaJugador < 50) {
        cout << "Estado: ¡Peligro! Vida baja (" << vidaJugador << " HP)." << endl;
    } else {
        cout << "Estado: Jugador saludable (" << vidaJugador << " HP)." << endl;
    }
    cout << endl;

    cout << "=== 2. SELECCIÓN MÚLTIPLE (switch) ===" << endl;
    // El 'switch' es ideal cuando evaluás una variable contra varios valores enteros o caracteres discretos.
    char opcionMenu = 'B';

    switch (opcionMenu) {
        case 'A':
        case 'a':
            cout << "Accion: Iniciar Partida." << endl;
            break; // ¡Importante! Sin 'break', la ejecución sigue de largo al siguiente case.
        case 'B':
        case 'b':
            cout << "Accion: Opciones de Configuracion." << endl;
            break;
        case 'C':
        case 'c':
            cout << "Accion: Salir del Juego." << endl;
            break;
        default:
            cout << "Opcion no reconocida." << endl;
            break;
    }
    cout << endl;

    cout << "=== 3. BUCLE FOR ===" << endl;
    // Estructura: for (inicializacion; condicion; incremento)
    cout << "Cuenta regresiva para despegar: ";
    for (int i = 5; i > 0; --i) {
        cout << i << " ";
    }
    cout << "¡DESPEGUE!" << endl << endl;

    cout << "=== 4. BUCLE WHILE Y CONTROL (break / continue) ===" << endl;
    // Imprimiremos solo los números impares del 1 al 10, pero frenaremos si llegamos a 9:
    int contador = 0;
    cout << "Numeros impares (frena si llega a 9): ";
    while (contador < 10) {
        contador++;

        // Si es par, saltamos esta vuelta con 'continue'
        if (contador % 2 == 0) {
            continue;
        }

        // Si llegamos a 9, rompemos el bucle antes de imprimirlo
        if (contador == 9) {
            cout << "[Break en 9] ";
            break;
        }

        cout << contador << " ";
    }
    cout << endl << endl;

    cout << "=== 5. BUCLE DO-WHILE ===" << endl;
    // A diferencia de 'while', 'do-while' evalúa la condición al final.
    // Esto garantiza que el bloque se ejecute al menos una vez (ideal para menús interactivos).
    int intentos = 0;
    do {
        cout << "Ejecutando do-while. Intento actual: " << intentos << endl;
        intentos++;
    } while (intentos < 1); // Aunque la condición ya no se cumple, se ejecutó una vez.

    return 0;
}
