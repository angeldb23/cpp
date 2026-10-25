/* =============================================================================
   MÓDULO 04 - LECCIÓN 06: Aritmética de punteros, end y diferencia
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Sumar 1 a un puntero no suma un byte: suma sizeof(T) bytes, o sea, salta
      al siguiente elemento del mismo tipo.

          int* p = datos;      // p apunta a datos[0]
          p + 1                // apunta a datos[1]
          *(p + 1)             // datos[1]

      El puntero "end" (std::end o datos + n) apunta UNA POSICIÓN DESPUÉS del
      último elemento. No se puede desreferenciar, pero sí comparar: es el
      centinela natural de los bucles.

          for (int* it = begin; it != end; ++it) usar(*it);

      La diferencia entre dos punteros del mismo arreglo da un ptrdiff_t (un
      entero con signo): cuántos ELEMENTOS hay entre ellos.

          end - begin == n
          p - begin   == índice de p

      La resta solo está definida si ambos punteros pertenecen al mismo
      arreglo (o uno es end). Restar punteros de arreglos distintos es UB.

   2. ¿QUÉ PASA POR DEBAJO?
      La dirección se calcula como base + i * sizeof(T). Para int (4 bytes),
      p+3 avanza 12 bytes. El compilador escala automáticamente; por eso no se
      suma sizeof a mano. El tipo del puntero lleva esa escala incorporada.

   3. ERRORES COMUNES
      - Desreferenciar end (leer *end es UB).
      - Avanzar más allá de end o antes de begin.
      - Comparar punteros de arreglos distintos con < o >.
      - Usar int para diferencias grandes: corresponde std::ptrdiff_t.
      - Mezclar índices con punteros y equivocarse en el off-by-one.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/pointer#Pointer_arithmetic
   ============================================================================= */
#include <cstddef>
#include <iostream>
#include <iterator>

// Recorre [begin, end) y devuelve cuántos elementos hay.
std::ptrdiff_t contar(const int* begin, const int* end) {
    return end - begin;
}

// Devuelve el puntero al menor elemento, o end si el rango está vacío.
const int* minimo(const int* begin, const int* end) {
    if (begin == end) return end;
    const int* mejor = begin;
    for (const int* it = begin + 1; it != end; ++it) {
        if (*it < *mejor) mejor = it;
    }
    return mejor;
}

int main() {
    std::cout << "=== 1. AVANCE EN ELEMENTOS, NO EN BYTES ===\n";
    int datos[] = {10, 20, 30, 40, 50};
    int* p = datos;
    std::cout << "p        -> " << *p << "\n";
    std::cout << "p + 1    -> " << *(p + 1) << "\n";
    std::cout << "p + 4    -> " << *(p + 4) << "\n";
    std::cout << "bytes entre p y p+4: "
              << reinterpret_cast<const char*>(p + 4) - reinterpret_cast<const char*>(p)
              << " (4 * sizeof(int))\n";

    std::cout << "\n=== 2. begin, end Y EL BUCLE CANÓNICO ===\n";
    int* begin = std::begin(datos);
    int* end = std::end(datos);
    std::cout << "elementos: ";
    for (int* it = begin; it != end; ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";
    std::cout << "end - begin = " << (end - begin) << "\n";
    std::cout << "contar(begin, end) = " << contar(begin, end) << "\n";

    std::cout << "\n=== 3. DIFERENCIA = ÍNDICE ===\n";
    int* p30 = begin + 2;
    std::cout << "*p30 = " << *p30 << "\n";
    std::cout << "p30 - begin = " << (p30 - begin) << "\n";
    std::cout << "datos[p30 - begin] = " << datos[p30 - begin] << "\n";

    std::cout << "\n=== 4. RECORRER AL REVÉS ===\n";
    for (int* it = end; it != begin; ) {
        --it;  // primero retrocedo, después leo
        std::cout << *it << " ";
    }
    std::cout << "\n";

    std::cout << "\n=== 5. BUSCAR EL MÍNIMO CON PUNTEROS ===\n";
    const int* menor = minimo(begin, end);
    std::cout << "menor = " << *menor
              << " en el índice " << (menor - begin) << "\n";

    std::cout << "\n=== 6. RANGO VACÍO ===\n";
    const int* vacio = minimo(begin, begin);
    std::cout << "minimo de un rango vacío devuelve end: "
              << (vacio == begin ? "sí" : "no") << "\n";

    std::cout << "\n=== 7. std::distance ===\n";
    std::cout << "std::distance(begin, end) = " << std::distance(begin, end)
              << "\n";

    return 0;
}
