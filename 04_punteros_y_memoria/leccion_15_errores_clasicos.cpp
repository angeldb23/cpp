/* =============================================================================
   MÓDULO 04 - LECCIÓN 15: Errores clásicos de memoria
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Cuatro errores explican casi todos los bugs de memoria en C++:

      a) FUGA (leak): reservás con new y perdés el puntero sin delete.
             int* p = new int(5);
             p = nullptr;              // el bloque quedó inaccesible
         El programa sigue, pero la memoria no se recupera hasta que termina.

      b) DOBLE LIBERACIÓN (double free): dos delete sobre el mismo puntero.
             int* p = new int(5);
             delete p;
             delete p;                 // UB
         El allocator puede corromper sus estructuras internas.

      c) PUNTERO COLGANTE (dangling): el objeto murió pero el puntero sigue.
             int* p;
             { int local = 3; p = &local; }
             *p;                       // UB: local ya no existe

      d) USE-AFTER-FREE: usás la memoria después de delete.
             int* p = new int(5);
             delete p;
             *p = 7;                   // UB

      La raíz común es no tener claro QUIÉN ES EL DUEÑO del recurso. Si hay un
      solo dueño y libera una sola vez, los cuatro errores desaparecen.

   2. ¿QUÉ PASA POR DEBAJO?
      El allocator no sabe si un puntero sigue siendo válido. delete marca el
      bloque como libre; la memoria puede reutilizarse en cualquier momento.
      Por eso un use-after-free a veces "funciona" en una prueba y falla en
      producción. Valgrind y ASan (lección 17) detectan estos casos.

      El doble delete puede parecer inofensivo si el bloque todavía está libre,
      pero es UB: el optimizador puede asumir que no ocurre y generar código
      sorprendente.

   3. CÓMO EVITARLOS
      - Inicializá siempre los punteros y poné nullptr después de delete.
      - Asigná un único dueño; los demás usan referencias o punteros no dueños.
      - No devuelvas direcciones de locales.
      - Preferí std::vector, std::string y (módulo 07) smart pointers.
      - Si una función reserva, documentá quién libera.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/new#Deallocation
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>

// Ejemplo de API con dueño claro: devuelve un bloque nuevo y el llamador
// debe liberarlo con delete[].
int* crearSecuencia(int n) {
    int* datos = new int[n];
    for (int i = 0; i < n; ++i) datos[i] = i + 1;
    return datos;
}

int main() {
    std::cout << "=== 1. FUGA (QUÉ NO HACER) ===\n";
    // int* perdido = new int(5);
    // perdido = nullptr;  // fuga: el bloque sigue reservado
    std::cout << "si perdés el último puntero, ya no podés liberar\n";

    std::cout << "\n=== 2. LIBERAR UNA SOLA VEZ ===\n";
    int* p = new int(5);
    delete p;
    p = nullptr;  // así un segundo delete sería inocuo
    delete p;     // delete nullptr no hace nada (válido)
    std::cout << "delete nullptr es seguro; delete dos veces el mismo bloque no\n";

    std::cout << "\n=== 3. PUNTERO COLGANTE ===\n";
    // int* colgante;
    // { int local = 3; colgante = &local; }
    // std::cout << *colgante;  // UB
    std::cout << "un puntero a una local no sobrevive al bloque\n";

    std::cout << "\n=== 4. USE-AFTER-FREE ===\n";
    // int* uaf = new int(7);
    // delete uaf;
    // *uaf = 9;  // UB
    std::cout << "después de delete, el bloque ya no es tuyo\n";

    std::cout << "\n=== 5. PATRÓN CORRECTO CON UN DUEÑO ===\n";
    int* secuencia = crearSecuencia(5);
    std::cout << "secuencia: ";
    for (int i = 0; i < 5; ++i) std::cout << secuencia[i] << " ";
    std::cout << "\n";
    delete[] secuencia;  // el llamador conoce el contrato
    secuencia = nullptr;

    std::cout << "\n=== 6. LA MISMA IDEA CON std::vector ===\n";
    {
        std::vector<int> v = {1, 2, 3, 4, 5};
        std::cout << "vector: ";
        for (int n : v) std::cout << n << " ";
        std::cout << "\n";
    }  // no hay delete: el destructor libera

    std::cout << "\n=== 7. CHECKLIST MENTAL ===\n";
    std::cout << "¿quién reservó? ¿quién libera? ¿puede fallar en el medio?\n";

    return 0;
}
