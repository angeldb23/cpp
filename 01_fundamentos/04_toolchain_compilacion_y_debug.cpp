/* =============================================================================
   MÓDULO 01 - LECCIÓN 04: El Toolchain de C++, Fases de Compilación y Sanitizers
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
      C++ no es interpretado ni usa una máquina virtual (como Python o JVM).
      Tu código pasa por cuatro etapas antes de ser un ejecutable en Linux:

      [Tu Código .cpp]
            │
            ▼  1. Preprocesador (cpp): Expande #include, procesa #define y macros.
      [Código Expandido]
            │
            ▼  2. Compilador (g++ / clang++): Genera código ensamblador (.s).
      [Ensamblador .s]
            │
            ▼  3. Ensamblador (as): Traduce a código máquina en código objeto (.o).
      [Código Objeto .o]
            │
            ▼  4. Enlazador / Linker (ld): Une tus .o con la biblioteca estándar (libc, libstdc++).
      [Ejecutable ELF de Linux]

   2. ¿QUÉ PASA POR DEBAJO? (Flags del Compilador y Sanitizers):
      - '-std=c++20': Activa el estándar moderno.
      - '-Wall -Wextra -Wpedantic': Enciende todas las advertencias que salvan vidas.
      - '-O2 / -O3': Optimizador de la CPU (vectorización, desenrollado de bucles, inlining).
      - '-fsanitize=address,undefined': Las herramientas más poderosas de C++. Inyectan
        detectores en tiempo de ejecución que te avisan si tocás memoria indebida o
        provocas un Comportamiento Indefinido (Undefined Behavior).

   3. PARA PROFUNDIZAR:
      - Compiler Explorer (Godbolt): https://godbolt.org/
      - Guía de GCC Flags: https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
   ============================================================================= */

#include <iostream>

int main() {
    std::cout << "=== FASES DE COMPILACIÓN EN C++ ===\n";
    std::cout << "1. Preprocesamiento (expandir cabeceras)\n";
    std::cout << "2. Compilacion (C++ -> Ensamblador)\n";
    std::cout << "3. Ensamblado (Ensamblador -> Codigo Objeto .o)\n";
    std::cout << "4. Linkeo (Objetos + Librerias -> Binario ejecutable)\n";
    std::cout << "\nTip: Podes ver el ensamblador generado en tu terminal con:\n";
    std::cout << "  g++ -S -O2 -fverbose-asm 04_toolchain_compilacion_y_debug.cpp\n";
    return 0;
}
