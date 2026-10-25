/* =============================================================================
   MÓDULO 01 - LECCIÓN 03: Inmutabilidad y Cómputo en Compilación (const, constexpr, consteval)
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
      En C++ moderno, cuanto más trabajo haga el compilador antes de que el programa
      se ejecute, más rápido y seguro será tu código (Zero-Cost Abstractions).
      
      - 'const': "Esta variable no se puede modificar una vez creada". Su valor puede
        calcularse en tiempo de ejecución (ej. entrada del usuario).
      - 'constexpr': "Esta expresión o función PUEDE evaluarse en tiempo de compilación
        si sus argumentos son conocidos de antemano".
      - 'consteval' (C++20): "Esta función DEBE OBLIGATORIAMENTE evaluarse en tiempo
        de compilación". Si no puede, el código no compila.

   2. ¿QUÉ PASA POR DEBAJO? (Memoria y CPU):
      Una variable 'const' normal vive en memoria RAM (en el Stack o en el segmento de solo
      lectura .rodata).
      Un valor 'constexpr' o 'consteval' evaluado en compilación se convierte en un
      valor inmediato (literal directo) incrustado en la instrucción de la CPU (ej. 'mov eax, 100').
      ¡Cero ciclos de CPU desperdiciados en runtime calculándolo!

   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
      - Error común: Usar '#define' para constantes. Nunca uses macros de preprocesador;
        no tienen tipo, no respetan namespaces ni ámbitos de C++.
      - Buena práctica moderna: Prefiere 'constexpr' por defecto para constantes conocidas,
        y 'const' para inmutabilidad de datos calculados dinámicamente.

   4. PARA PROFUNDIZAR:
      - cppreference constexpr: https://es.cppreference.com/w/cpp/language/constexpr
      - cppreference consteval: https://es.cppreference.com/w/cpp/language/consteval
   ============================================================================= */

#include <iostream>

// Función consteval (C++20): Garantiza evaluación pura en tiempo de compilación
consteval int calcularTamanoBuffer(int resolucionBase) {
    return resolucionBase * 4; // 4 bytes por píxel (RGBA)
}

// Función constexpr: Puede correr en compilación o en ejecución
constexpr double calcularAreaCirculo(double radio) {
    constexpr double PI = 3.141592653589793;
    return PI * radio * radio;
}

int main() {
    std::cout << "=== 1. CÓMPUTO EN TIEMPO DE COMPILACIÓN (C++20) ===\n";

    // Se calcula durante la compilación, en el binario ya está el número 7680:
    constexpr int tamano = calcularTamanoBuffer(1920);
    std::cout << "Tamaño de buffer calculado en compilación: " << tamano << " bytes\n";

    // Se evalúa en compilación porque el argumento (5.0) es constante:
    constexpr double areaFija = calcularAreaCirculo(5.0);
    std::cout << "Área calculada en compilación: " << areaFija << "\n";

    // También puede correr en tiempo de ejecución con variables dinámicas:
    double radioVariable = 3.5;
    double areaDinamica = calcularAreaCirculo(radioVariable);
    std::cout << "Área calculada en tiempo de ejecución: " << areaDinamica << "\n";

    return 0;
}
