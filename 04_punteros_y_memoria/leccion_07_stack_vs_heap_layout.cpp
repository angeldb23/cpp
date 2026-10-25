/* =============================================================================
   MÓDULO 04 - LECCIÓN 07: Stack vs heap y layout del proceso
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Un programa en ejecución ve varias regiones de memoria con propósitos
      distintos. Los nombres exactos dependen del sistema, pero el modelo
      clásico es:

        +----------------------+  direcciones altas
        | stack (pila)         |  variables locales, parámetros, retorno
        |   crece hacia abajo  |
        +----------------------+
        | ...                  |
        +----------------------+
        | heap (montón)        |  new / malloc; crece según se pida
        |   crece hacia arriba |
        +----------------------+
        | bss                  |  globales/static sin inicializar (cero)
        +----------------------+
        | data                 |  globales/static inicializados
        +----------------------+
        | rodata               |  literales y constantes de solo lectura
        +----------------------+
        | text                 |  código máquina de las funciones
        +----------------------+  direcciones bajas

      El stack es automático y rapidísimo: entrar a una función reserva un
      frame y salir lo libera. Su tamaño es limitado (típicamente 1-8 MB). El
      heap es grande (toda la RAM disponible, más swap) pero más lento de
      asignar y liberar, y no se limpia solo.

   2. ¿QUÉ PASA POR DEBAJO?
      El sistema operativo mapea páginas virtuales; las direcciones que ves no
      son físicas. ASLR (Address Space Layout Randomization) cambia las bases
      en cada ejecución, así que no compares direcciones entre corridas. El
      stack suele crecer hacia direcciones menores en x86-64, pero el estándar
      no lo garantiza: no escribas código que dependa de ello.

      "Stack overflow" no es una metáfora: si la pila supera el límite, el
      proceso muere. Pasa con recursión profunda o arreglos locales enormes.

   3. ERRORES COMUNES
      - Crear arreglos gigantes como variable local (van al stack).
      - Devolver un puntero a una variable local (su frame desaparece).
      - Creer que delete libera el stack: solo libera lo pedido con new.
      - Asumir que el heap y el stack están en un orden fijo.
      - Medir el uso de memoria con sizeof de un puntero.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/storage_duration
   ============================================================================= */
#include <iostream>
#include <string>

int global_inicializada = 42;   // data
int global_sin_iniciar;         // bss (se inicializa a 0)
const int constante_global = 7; // suele ir a rodata

void funcion() {
    int local = 1;              // stack
    static int contador = 0;    // data (persiste entre llamadas)
    ++contador;
    std::cout << "  funcion: local en " << &local
              << ", static en " << &contador
              << ", llamada " << contador << "\n";
}

int main() {
    std::cout << "=== 1. DIRECCIONES DE CADA REGIÓN ===\n";
    std::cout << "main()          = " << reinterpret_cast<const void*>(&main) << "\n";
    std::cout << "global inicial  = " << &global_inicializada << "\n";
    std::cout << "global sin init = " << &global_sin_iniciar << "\n";
    std::cout << "const global    = " << &constante_global << "\n";

    int local_main = 5;
    std::cout << "local main      = " << &local_main << "\n";

    int* en_heap = new int(99);
    std::cout << "objeto en heap  = " << en_heap << "\n";

    std::cout << "\n(las direcciones exactas cambian por ASLR; lo importante es\n"
                 " que cada región cumple un rol distinto)\n";

    std::cout << "\n=== 2. STACK: VIDA ATADA AL BLOQUE ===\n";
    funcion();
    funcion();
    std::cout << "el static sobrevive entre llamadas; el local no\n";

    std::cout << "\n=== 3. HEAP: VIVE HASTA QUE LO LIBERES ===\n";
    {
        int* temporal = new int(123);
        std::cout << "dentro del bloque: " << *temporal << "\n";
        delete temporal;  // sin esto, fuga
    }
    std::cout << "el bloque terminó; el heap se liberó a mano\n";

    std::cout << "\n=== 4. OBJETOS DE STL: CABEZA EN STACK, DATOS EN HEAP ===\n";
    std::string texto = "un texto suficientemente largo para no usar SSO";
    std::cout << "&texto (stack)   = " << &texto << "\n";
    std::cout << "texto.data()     = " << static_cast<const void*>(texto.data())
              << " (posiblemente heap)\n";
    std::cout << "sizeof(texto)    = " << sizeof(texto)
              << " bytes (la cabeza, no el buffer)\n";

    std::cout << "\n=== 5. TAMAÑOS TÍPICOS ===\n";
    std::cout << "sizeof(int)    = " << sizeof(int) << "\n";
    std::cout << "sizeof(int*)   = " << sizeof(int*) << "\n";
    std::cout << "un arreglo local de 1M de int ocuparía "
              << (1000000 * sizeof(int)) / (1024 * 1024) << " MB de stack\n";

    delete en_heap;
    en_heap = nullptr;

    return 0;
}
