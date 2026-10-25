/* =============================================================================
   MÓDULO 02 - LECCIÓN 04: Recursividad, Stack Frames y Optimización Tail Call
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
      Una función recursiva es aquella que se llama a sí misma para resolver un
      subproblema más pequeño, hasta alcanzar un CASO BASE.

   2. ¿QUÉ PASA POR DEBAJO? (El Stack de Llamadas):
      Cada llamada a función crea un "Stack Frame" en memoria RAM con sus variables
      locales, argumentos y la dirección de retorno.
      - Si la recursión es demasiado profunda, el Stack se agota -> Crash (Stack Overflow).
      - Optimización Tail Call (TCO): Si la llamada recursiva es la ÚLTIMA instrucción
        pura sin operaciones pendientes, los compiladores modernos con -O2 la convierten
        internamente en un simple bucle 'jump', reutilizando el mismo Stack Frame.

   3. PARA PROFUNDIZAR:
      - C++ Core Guidelines F.4: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rf-tail
   ============================================================================= */

#include <iostream>

// Recursión de cola (Tail Recursion)
unsigned long long factorialTail(unsigned int n, unsigned long long acumulador = 1) {
    if (n <= 1) return acumulador;
    return factorialTail(n - 1, n * acumulador); // Última operación es la llamada pura
}

int main() {
    std::cout << "=== RECURSIVIDAD Y FACTORIAL TAIL-CALL ===\n";
    std::cout << "Factorial de 5:  " << factorialTail(5) << "\n";
    std::cout << "Factorial de 10: " << factorialTail(10) << "\n";
    std::cout << "Factorial de 20: " << factorialTail(20) << "\n";
    return 0;
}
