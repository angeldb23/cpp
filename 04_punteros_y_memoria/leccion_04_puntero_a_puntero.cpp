/* =============================================================================
   MÓDULO 04 - LECCIÓN 04: Puntero a puntero y paso por puntero
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Si un puntero guarda una dirección, un puntero a puntero guarda la
      dirección de esa variable puntero:

          int x = 10;
          int* p = &x;      // p -> x
          int** pp = &p;    // pp -> p -> x

          *pp  == p         // el puntero intermedio
          **pp == x         // el valor final

      ¿Para qué sirve? Para que una función pueda CAMBIAR el puntero del
      llamador, no solo el objeto apuntado. Ese es el patrón de salida:

          bool crearBuffer(int** salida, int n);

      La función recibe la dirección del puntero del llamador y escribe en
      *salida la dirección del buffer nuevo. Si solo recibiera int*, podría
      modificar los datos, pero no podría hacer que el puntero del llamador
      apunte a otro sitio.

      En C++ moderno muchas veces se prefiere devolver el puntero por valor o
      usar una referencia a puntero (int*&, lección 14). El puntero a puntero
      sigue apareciendo en APIs de C y en arreglos de punteros.

   2. ¿QUÉ PASA POR DEBAJO?
      No hay magia: pp es una variable que contiene la dirección de p. Cada
      nivel de indirección es una carga más. El compilador no puede saber si
      pp es nulo; si lo desreferenciás sin comprobar, el crash es tu
      responsabilidad.

   3. ERRORES COMUNES
      - Pasar &p cuando la función espera int* (o al revés): el compilador
        avisa, pero con casts se puede silenciar y romper todo.
      - No inicializar *salida antes de usarla.
      - Perder el puntero original al reasignar dentro de la función.
      - Confundir **pp con *pp: son niveles distintos.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/pointer
   ============================================================================= */
#include <iostream>

// Devuelve por salida el puntero al mayor de dos enteros.
void apuntarAlMayor(int** salida, int* a, int* b) {
    if (salida == nullptr) return;
    if (a == nullptr) { *salida = b; return; }
    if (b == nullptr) { *salida = a; return; }
    *salida = (*a >= *b) ? a : b;
}

// Intercambia los punteros del llamador, no los valores apuntados.
void intercambiarPunteros(int** a, int** b) {
    if (a == nullptr || b == nullptr) return;
    int* temporal = *a;
    *a = *b;
    *b = temporal;
}

// Recorre un arreglo de punteros: cada elemento es un int*.
int sumarApuntados(int* const* datos, int n) {
    int total = 0;
    for (int i = 0; i < n; ++i) {
        if (datos[i] != nullptr) total += *datos[i];
    }
    return total;
}

int main() {
    std::cout << "=== 1. DOS NIVELES DE INDIRECCIÓN ===\n";
    int x = 10;
    int* p = &x;
    int** pp = &p;

    std::cout << "x    = " << x << "\n";
    std::cout << "*p   = " << *p << "\n";
    std::cout << "**pp = " << **pp << "\n";
    std::cout << "pp == &p: " << (pp == &p ? "true" : "false") << "\n";

    **pp = 42;  // llega hasta x
    std::cout << "después de **pp = 42, x = " << x << "\n";

    std::cout << "\n=== 2. CAMBIAR EL PUNTERO DEL LLAMADOR ===\n";
    int a = 3;
    int b = 8;
    int* elegido = nullptr;
    apuntarAlMayor(&elegido, &a, &b);
    std::cout << "elegido apunta a " << *elegido << "\n";
    std::cout << "elegido == &b: " << (elegido == &b ? "true" : "false") << "\n";

    std::cout << "\n=== 3. INTERCAMBIAR PUNTEROS ===\n";
    int* pa = &a;
    int* pb = &b;
    std::cout << "antes:  *pa = " << *pa << ", *pb = " << *pb << "\n";
    intercambiarPunteros(&pa, &pb);
    std::cout << "después: *pa = " << *pa << ", *pb = " << *pb << "\n";
    std::cout << "los valores a y b no se tocaron: a = " << a << ", b = " << b << "\n";

    std::cout << "\n=== 4. ARREGLO DE PUNTEROS ===\n";
    int* arreglo[] = {&a, &b, nullptr, &x};
    std::cout << "suma de los apuntados = " << sumarApuntados(arreglo, 4) << "\n";

    std::cout << "\n=== 5. POR QUÉ NO ALCANZA CON int* ===\n";
    // Si una función recibe int* puede cambiar *p, pero no puede hacer que el
    // puntero del llamador apunte a otra variable. Para eso necesita int**.
    std::cout << "int* modifica el objeto; int** modifica el puntero\n";

    return 0;
}
