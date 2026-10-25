/* =============================================================================
   MÓDULO 04 - LECCIÓN 08: Duración, alcance y tamaño de la memoria
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      No confundas alcance (scope) con duración (lifetime):

        - Alcance: dónde es visible el nombre en el código.
        - Duración: cuánto tiempo existe el objeto en memoria.

      Las duraciones principales son:
        - automática: variables locales; nacen al entrar al bloque y mueren al
          salir. Van al stack.
        - estática: globales, static de función y static de clase; viven todo
          el programa. Van a data/bss.
        - dinámica: las crea new; viven hasta delete. Van al heap.
        - de hilo: thread_local (módulo 14); una por hilo.

      El stack se reserva por frames: cada llamada guarda parámetros, retorno,
      variables locales y temporales. Al volver, el frame se descarta de golpe.
      Por eso el costo de entrar y salir de una función es bajísimo.

   2. ¿QUÉ PASA POR DEBAJO?
      El compilador calcula el tamaño del frame en tiempo de compilación y
      ajusta el stack pointer. No hay un allocator por variable local. En
      cambio, new negocia con el allocator del heap (malloc/arenas), que busca
      un bloque libre y puede pedir páginas al sistema operativo.

      El stack tiene un límite (ulimit -s, típicamente 8 MB en Linux). El heap
      crece hasta agotar memoria; si new no consigue, lanza std::bad_alloc.

   3. ERRORES COMUNES
      - Reservar buffers grandes en el stack y provocar stack overflow.
      - Usar un puntero después de que su variable automática murió.
      - Confiar en que una variable local conserva su valor entre llamadas.
      - Olvidar que static se inicializa una sola vez y es compartido.
      - Medir memoria dinámica con sizeof: solo mide el puntero.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/storage_duration
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>

int contadorLlamadas() {
    static int veces = 0;  // se inicializa una sola vez
    ++veces;
    return veces;
}

void alcanceDeBloque() {
    int exterior = 1;
    {
        int interior = 2;
        std::cout << "  dentro: exterior = " << exterior
                  << ", interior = " << interior << "\n";
    }  // interior muere acá
    // std::cout << interior;  // ERROR: fuera de alcance
    std::cout << "  fuera: exterior sigue vivo = " << exterior << "\n";
}

// Simula frames: cada llamada tiene su propia copia de n.
int sumaRecursiva(int n) {
    int local = n;  // vive en el frame de esta llamada
    if (n <= 1) return local;
    return local + sumaRecursiva(n - 1);
}

int main() {
    std::cout << "=== 1. STATIC LOCAL: UNA SOLA INSTANCIA ===\n";
    std::cout << contadorLlamadas() << " " << contadorLlamadas() << " "
              << contadorLlamadas() << "\n";

    std::cout << "\n=== 2. ALCANCE DE BLOQUE ===\n";
    alcanceDeBloque();

    std::cout << "\n=== 3. FRAMES Y RECURSIÓN ===\n";
    std::cout << "sumaRecursiva(4) = " << sumaRecursiva(4) << "\n";
    std::cout << "cada llamada tiene su propio 'local'; al volver, su frame se descarta\n";

    std::cout << "\n=== 4. VECTOR: OBJETO AUTOMÁTICO, BUFFER DINÁMICO ===\n";
    {
        std::vector<int> v = {1, 2, 3};
        std::cout << "size = " << v.size() << ", capacity = " << v.capacity() << "\n";
        v.push_back(4);  // puede realocar el buffer del heap
        std::cout << "después de push_back: size = " << v.size()
                  << ", capacity = " << v.capacity() << "\n";
    }  // el vector libera su buffer automáticamente

    std::cout << "\n=== 5. DURACIÓN DINÁMICA ===\n";
    int* p = new int(7);
    std::cout << "el int del heap vale " << *p << " y sigue vivo hasta delete\n";
    delete p;
    p = nullptr;

    std::cout << "\n=== 6. TAMAÑO DEL FRAME (APROXIMADO) ===\n";
    std::cout << "sizeof(std::string) = " << sizeof(std::string) << " bytes\n";
    std::cout << "sizeof(std::vector) = " << sizeof(std::vector<int>) << " bytes\n";
    std::cout << "los buffers internos no están en el stack\n";

    return 0;
}
