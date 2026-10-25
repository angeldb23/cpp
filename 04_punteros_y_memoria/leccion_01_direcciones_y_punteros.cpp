/* =============================================================================
   MÓDULO 04 - LECCIÓN 01: Direcciones, & y *
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Toda variable ocupa una caja de memoria. Esa caja tiene una dirección
      (un número) y un contenido (el valor). El operador & obtiene la dirección
      de un objeto; el operador * accede al objeto que hay en una dirección.

          int x = 42;       // caja llamada x, contenido 42
          int* p = &x;      // p guarda la DIRECCIÓN de x
          *p = 99;          // escribe 99 en la caja apuntada por p (o sea, x)

      Un puntero es una variable más: también tiene su propia dirección y su
      propio tamaño. Lo que lo distingue es que su VALOR se interpreta como la
      dirección de un objeto de un tipo determinado.

      El tipo del puntero importa por dos motivos:
        - Define qué operaciones son válidas (no es lo mismo int* que double*).
        - Define cuántos bytes avanza al sumarle 1 (lo vemos en la lección 06).

   2. ¿QUÉ PASA POR DEBAJO?
      En una máquina de 64 bits, un puntero suele ocupar 8 bytes. La dirección
      es un número entero, pero el lenguaje no te deja usarla como entero sin
      una conversión explícita. Desreferenciar carga o guarda en la dirección
      que contiene el puntero; si esa dirección no está mapeada, el programa
      recibe una señal (segmentation fault) y termina.

   3. ERRORES COMUNES
      - Declarar un puntero sin inicializarlo y desreferenciarlo: su valor es
        basura, no necesariamente nullptr.
      - Confundir el valor del puntero con el valor apuntado: p != *p.
      - Creer que imprimir un puntero muestra el contenido; muestra la dirección.
      - Usar & sobre una expresión temporal (no se puede tomar la dirección de
        un rvalue).

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/pointer
   ============================================================================= */
#include <cstdint>
#include <iostream>

int main() {
    std::cout << "=== 1. UNA VARIABLE TIENE DIRECCIÓN Y CONTENIDO ===\n";
    int vida = 100;
    std::cout << "vida          = " << vida << "\n";
    std::cout << "&vida         = " << &vida << "\n";
    std::cout << "sizeof(vida)  = " << sizeof(vida) << " bytes\n";
    std::cout << "sizeof(&vida) = " << sizeof(&vida) << " bytes (un puntero)\n";

    std::cout << "\n=== 2. UN PUNTERO GUARDA ESA DIRECCIÓN ===\n";
    int* ptr = &vida;
    std::cout << "ptr           = " << ptr << "  (la dirección de vida)\n";
    std::cout << "&ptr          = " << &ptr << "  (la dirección del propio puntero)\n";
    std::cout << "*ptr          = " << *ptr << "  (el contenido de la caja apuntada)\n";
    std::cout << "ptr == &vida  = " << (ptr == &vida ? "true" : "false") << "\n";

    std::cout << "\n=== 3. ESCRIBIR A TRAVÉS DEL PUNTERO ===\n";
    *ptr = 75;  // equivale a: vida = 75;
    std::cout << "después de *ptr = 75, vida = " << vida << "\n";

    // Cambiar el puntero no cambia el objeto: ahora apunta a otra caja.
    int otra = 7;
    ptr = &otra;
    std::cout << "después de ptr = &otra, *ptr = " << *ptr
              << " y vida sigue valiendo " << vida << "\n";

    std::cout << "\n=== 4. TIPOS DE PUNTERO Y TAMAÑO ===\n";
    double precio = 19.99;
    double* pd = &precio;
    char letra = 'A';
    char* pc = &letra;
    std::cout << "sizeof(int*)    = " << sizeof(int*) << "\n";
    std::cout << "sizeof(double*) = " << sizeof(double*) << "\n";
    std::cout << "sizeof(char*)   = " << sizeof(char*) << "\n";
    std::cout << "pd apunta a " << *pd << " y pc a '" << *pc << "'\n";

    std::cout << "\n=== 5. DIRECCIÓN COMO NÚMERO (SOLO PARA MIRAR) ===\n";
    // Convertir el puntero a entero es explícito: sirve para depurar, no para
    // hacer aritmética normal con direcciones.
    auto direccion = reinterpret_cast<std::uintptr_t>(ptr);
    std::cout << "dirección de otra en hexadecimal: 0x" << std::hex << direccion
              << std::dec << "\n";

    return 0;
}
