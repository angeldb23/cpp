/* =============================================================================
   MÓDULO 04 - LECCIÓN 05: Punteros y arrays (decaimiento)
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Un arreglo de C es un bloque contiguo de elementos del mismo tipo. En la
      mayoría de las expresiones, el nombre del arreglo se convierte
      ("decae") a un puntero a su primer elemento:

          int datos[4] = {10, 20, 30, 40};
          int* p = datos;        // p == &datos[0]

      Por eso arr[i] y *(arr + i) son equivalentes. El compilador traduce
      ambas formas a la misma dirección.

      Pero un arreglo NO es un puntero:
          sizeof(datos)   -> 4 * sizeof(int) = 16
          sizeof(p)       -> tamaño de un puntero (8 en 64 bits)
      Además, &datos tiene tipo int (*)[4] (puntero a arreglo), no int*.

      El decaimiento ocurre al pasar el arreglo a una función:
          void f(int* p);       // recibe un puntero, perdió el tamaño
      Por eso hace falta pasar la cantidad de elementos por separado, o usar
      std::span (módulo 03) / std::array / std::vector.

   2. ¿QUÉ PASA POR DEBAJO?
      No hay una copia al asignar a un puntero: solo se copia la dirección del
      primer elemento. El arreglo sigue viviendo en el stack (o donde se haya
      declarado). Los elementos están uno al lado del otro, sin huecos, y por
      eso la aritmética de punteros funciona.

   3. ERRORES COMUNES
      - Usar sizeof dentro de una función que recibió int*: devuelve el tamaño
        del puntero, no del arreglo.
      - Escribir fuera de los límites: no hay chequeo automático.
      - Confundir &datos con datos (misma dirección, distinto tipo).
      - Creer que un arreglo se puede asignar o devolver por valor.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/array
   ============================================================================= */
#include <array>
#include <iostream>
#include <iterator>

// Recibe un puntero: el tamaño del arreglo se perdió.
int sumar(int* datos, int n) {
    int total = 0;
    for (int i = 0; i < n; ++i) total += datos[i];
    return total;
}

// Forma más segura: std::span (C++20) conserva tamaño y no copia.
#include <span>
int sumarSpan(std::span<const int> datos) {
    int total = 0;
    for (int x : datos) total += x;
    return total;
}

int main() {
    std::cout << "=== 1. EL NOMBRE DECAE A PUNTERO ===\n";
    int datos[4] = {10, 20, 30, 40};
    int* p = datos;
    std::cout << "datos        = " << datos << "\n";
    std::cout << "p            = " << p << "\n";
    std::cout << "&datos[0]    = " << &datos[0] << "\n";
    std::cout << "p == datos: " << (p == datos ? "true" : "false") << "\n";

    std::cout << "\n=== 2. ARREGLO NO ES PUNTERO ===\n";
    std::cout << "sizeof(datos) = " << sizeof(datos) << " bytes (4 int)\n";
    std::cout << "sizeof(p)     = " << sizeof(p) << " bytes (un puntero)\n";
    std::cout << "std::size(datos) = " << std::size(datos) << "\n";
    std::cout << "&datos tiene tipo int (*)[4], no int*\n";

    std::cout << "\n=== 3. LAS DOS FORMAS DE INDEXAR ===\n";
    for (int i = 0; i < 4; ++i) {
        std::cout << "datos[" << i << "] = " << datos[i]
                  << "  |  *(datos + " << i << ") = " << *(datos + i) << "\n";
    }

    std::cout << "\n=== 4. PASAR A UNA FUNCIÓN ===\n";
    std::cout << "sumar(datos, 4) = " << sumar(datos, 4) << "\n";
    std::cout << "sumarSpan(datos) = " << sumarSpan(datos) << "\n";

    std::cout << "\n=== 5. ITERAR CON PUNTEROS begin/end ===\n";
    int* inicio = std::begin(datos);
    int* fin = std::end(datos);  // uno más allá del último
    for (int* it = inicio; it != fin; ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";

    std::cout << "\n=== 6. std::array NO DECAE TAN FÁCIL ===\n";
    std::array<int, 4> moderno = {1, 2, 3, 4};
    std::cout << "sizeof(moderno) = " << sizeof(moderno) << " bytes\n";
    std::cout << "moderno.size()  = " << moderno.size() << "\n";
    std::cout << "sumar(moderno.data(), 4) = " << sumar(moderno.data(), 4) << "\n";

    return 0;
}
