/* =============================================================================
   MÓDULO 04 - LECCIÓN 11: Punteros a función y callbacks
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Una función también tiene una dirección. Un puntero a función guarda esa
      dirección y permite llamarla indirectamente, pasarla como parámetro o
      guardarla en una tabla.

          int sumar(int a, int b);
          int (*op)(int, int) = sumar;   // op apunta a sumar
          int r = op(2, 3);              // llama a sumar

      La sintaxis se lee: op es un puntero a una función que recibe dos int y
      devuelve int. using la vuelve legible:

          using Operacion = int (*)(int, int);
          Operacion op = sumar;

      El uso típico es el callback: una función genérica recibe la operación
      que debe aplicar, y el llamador decide el comportamiento:

          int reducir(const int* datos, int n, int (*op)(int, int), int inicio);

      También se pueden hacer arreglos de punteros a función (una tabla de
      operaciones) y devolver punteros a función desde otra función.

   2. ¿QUÉ PASA POR DEBAJO?
      Un puntero a función guarda la dirección del código. Llamarlo es una
      llamada indirecta: la CPU salta a la dirección guardada. Cuesta un poco
      más que una llamada directa porque no se puede inlinear con facilidad.
      Los punteros a función no capturan estado: solo la función. Para llevar
      estado se pasa un parámetro extra o se usa una lambda con captura
      (lección 12).

   3. ERRORES COMUNES
      - Olvidar los paréntesis: int* f(int) es una función que devuelve int*,
        no un puntero a función.
      - Devolver un puntero a una función local (las funciones no son locales,
        pero sí se puede devolver un puntero a algo que ya no existe).
      - Usar un puntero a función nulo sin comprobarlo.
      - Confundir el tipo de retorno o los parámetros: el compilador rechaza
        la asignación si no coinciden exactamente.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/pointer#Pointers_to_functions
   ============================================================================= */
#include <iostream>

int sumar(int a, int b) { return a + b; }
int restar(int a, int b) { return a - b; }
int multiplicar(int a, int b) { return a * b; }
int maximo(int a, int b) { return a > b ? a : b; }

using Operacion = int (*)(int, int);

// Callback: la operación se decide en el llamador.
int reducir(const int* datos, int n, Operacion op, int inicio) {
    int acumulado = inicio;
    for (int i = 0; i < n; ++i) {
        acumulado = op(acumulado, datos[i]);
    }
    return acumulado;
}

// Aplica un predicado a cada elemento y cuenta cuántos lo cumplen.
int contarSi(const int* datos, int n, bool (*predicado)(int)) {
    int total = 0;
    for (int i = 0; i < n; ++i) {
        if (predicado(datos[i])) ++total;
    }
    return total;
}

bool esPar(int x) { return x % 2 == 0; }
bool esPositivo(int x) { return x > 0; }

// Devuelve un puntero a función según un carácter.
Operacion elegirOperacion(char c) {
    switch (c) {
        case '+': return sumar;
        case '-': return restar;
        case '*': return multiplicar;
        default:  return nullptr;
    }
}

int main() {
    std::cout << "=== 1. DECLARAR Y LLAMAR ===\n";
    int (*op)(int, int) = sumar;
    std::cout << "op(2, 3) = " << op(2, 3) << "\n";
    std::cout << "(*op)(2, 3) = " << (*op)(2, 3) << " (la misma llamada)\n";

    std::cout << "\n=== 2. CAMBIAR DE FUNCIÓN ===\n";
    op = multiplicar;
    std::cout << "op(2, 3) ahora multiplica: " << op(2, 3) << "\n";

    std::cout << "\n=== 3. CALLBACK GENÉRICO ===\n";
    int datos[] = {1, 2, 3, 4};
    std::cout << "suma con reducir:      "
              << reducir(datos, 4, sumar, 0) << "\n";
    std::cout << "producto con reducir:  "
              << reducir(datos, 4, multiplicar, 1) << "\n";
    std::cout << "máximo con reducir:    "
              << reducir(datos, 4, maximo, datos[0]) << "\n";

    std::cout << "\n=== 4. PREDICADOS ===\n";
    std::cout << "pares:    " << contarSi(datos, 4, esPar) << "\n";
    std::cout << "positivos:" << contarSi(datos, 4, esPositivo) << "\n";

    std::cout << "\n=== 5. TABLA DE OPERACIONES ===\n";
    Operacion tabla[] = {sumar, restar, multiplicar};
    const char* nombres[] = {"sumar", "restar", "multiplicar"};
    for (int i = 0; i < 3; ++i) {
        std::cout << nombres[i] << "(7, 2) = " << tabla[i](7, 2) << "\n";
    }

    std::cout << "\n=== 6. ELEGIR POR CARÁCTER Y COMPROBAR nullptr ===\n";
    Operacion elegida = elegirOperacion('*');
    if (elegida) std::cout << "'*' -> " << elegida(6, 7) << "\n";
    if (elegirOperacion('?') == nullptr) {
        std::cout << "'?' no tiene operación: puntero nulo\n";
    }

    return 0;
}
