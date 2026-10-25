/* =============================================================================
   MÓDULO 02 - LECCIÓN 05: Argumentos con valores por defecto
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
      Los parámetros de una función pueden tener valores por defecto que el
      compilador usa cuando el llamador omite ese argumento. Los valores por
      defecto deben especificarse en la declaración (o definición) y deben ser
      constantes o expresiones constantes.
   2. ¿QUÉ PASA POR DEBAJO?
      El compilador genera varias sobrecargas implícitas: una versión de la
      función con menos parámetros y otro llamado con los valores por defecto
      insertados. En la ABI, el llamado sigue el mismo esquema que una función
      sin defaults; el código que inserta los valores ocurre en tiempo de
      compilación.
   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
      - Ningún parámetro con default debe preceder a uno sin default.
      - Evitar usar objetos pesados como defaults; prefiera `const&` o
        `constexpr`.
      - Cambiar el valor por defecto rompe la ABI solo si la función es parte
        de una librería binaria.
   4. PARA PROFUNDIZAR:
      - cppreference: https://es.cppreference.com/w/cpp/language/default_arguments
      - C++ Core Guidelines: D.30: Prefer defaults over overloads.
   ============================================================================= */

#include <iostream>

// Declaración de función con argumento por defecto
int sumaConDefault(int a, int b = 10) {
    // TU CÓDIGO AQUÍ: implementa la suma
    return a + b;
}

int main() {
    std::cout << "sumaConDefault(5) = " << sumaConDefault(5) << std::endl;
    std::cout << "sumaConDefault(5, 3) = " << sumaConDefault(5, 3) << std::endl;
    return 0;
}
