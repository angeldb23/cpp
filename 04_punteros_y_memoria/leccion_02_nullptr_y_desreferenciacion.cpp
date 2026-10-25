/* =============================================================================
   MÓDULO 04 - LECCIÓN 02: nullptr y desreferenciación segura
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Un puntero que no apunta a ningún objeto válido debe valer nullptr.
      nullptr es un literal de tipo std::nullptr_t que se convierte a cualquier
      tipo de puntero, pero no a un entero. Reemplaza al viejo NULL y al 0.

      Regla práctica: un puntero siempre se inicializa.
          int* p = nullptr;          // todavía no apunta a nada
          int* q = &algunaVariable;  // apunta a algo vivo

      Antes de desreferenciar, comprobá que no sea nulo:
          if (p != nullptr) { usar(*p); }

      Un puntero nulo es válido como valor: podés compararlo, copiarlo y
      pasarlo. Lo que no podés hacer es leer o escribir a través de él.

   2. ¿QUÉ PASA POR DEBAJO?
      nullptr suele representarse como la dirección 0. El sistema operativo no
      mapea esa página, así que desreferenciarlo produce una falta de página y
      el proceso muere con SIGSEGV. El chequeo if (p) no es una formalidad: es
      la diferencia entre un error controlado y un crash.

      Un puntero no nulo tampoco garantiza que sea válido. Puede apuntar a
      memoria liberada (dangling) o a otra variable que ya no existe. nullptr
      detecta el caso "no apunta a nada", no el caso "apunta a basura".

   3. ERRORES COMUNES
      - Inicializar con NULL o 0: funciona, pero pierde el tipado de nullptr.
      - Comprobar p == 0 en lugar de p == nullptr (estilo viejo, menos claro).
      - Asumir que un puntero recibido por parámetro es válido.
      - Devolver la dirección de una variable local: al volver, esa variable
        muere y el puntero queda colgando.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/nullptr
   ============================================================================= */
#include <iostream>

// Esta función puede no encontrar nada: devuelve nullptr en ese caso.
int* buscarPositivo(int* datos, int n) {
    for (int i = 0; i < n; ++i) {
        if (datos[i] > 0) return &datos[i];
    }
    return nullptr;
}

// El llamador puede pasar nullptr: la función debe aceptarlo.
void imprimirSiExiste(const int* p) {
    if (p == nullptr) {
        std::cout << "  (no hay valor para imprimir)\n";
        return;
    }
    std::cout << "  valor = " << *p << "\n";
}

int main() {
    std::cout << "=== 1. PUNTERO NULO EXPLÍCITO ===\n";
    int* p = nullptr;
    std::cout << "p = " << p << "\n";
    std::cout << "p == nullptr: " << (p == nullptr ? "true" : "false") << "\n";

    // Comprobación segura antes de desreferenciar.
    if (p != nullptr) {
        std::cout << *p << "\n";  // no se ejecuta
    } else {
        std::cout << "p es nulo: no se desreferencia\n";
    }

    std::cout << "\n=== 2. ASIGNAR UNA DIRECCIÓN VÁLIDA ===\n";
    int valor = 21;
    p = &valor;
    if (p) {  // equivalente a p != nullptr
        std::cout << "ahora *p = " << *p << "\n";
    }

    std::cout << "\n=== 3. REASIGNAR Y VOLVER A NULO ===\n";
    int otro = 5;
    p = &otro;
    std::cout << "*p = " << *p << "\n";
    p = nullptr;  // ya no apunta a otro
    std::cout << "p volvió a nullptr: " << (p == nullptr ? "sí" : "no") << "\n";

    std::cout << "\n=== 4. FUNCIÓN QUE PUEDE DEVOLVER nullptr ===\n";
    int numeros[] = {-4, -1, 0, 8, 3};
    int* encontrado = buscarPositivo(numeros, 5);
    imprimirSiExiste(encontrado);

    int soloNegativos[] = {-2, -7, -9};
    imprimirSiExiste(buscarPositivo(soloNegativos, 3));

    std::cout << "\n=== 5. nullptr NO ES UN ENTERO ===\n";
    // int x = nullptr;  // ERROR: nullptr no se convierte a int
    std::nullptr_t nulo = nullptr;
    int* q = nulo;  // sí se convierte a cualquier puntero
    std::cout << "q == nullptr: " << (q == nullptr ? "true" : "false") << "\n";

    std::cout << "\n=== 6. PUNTERO COLGANTE (SOLO EXPLICADO) ===\n";
    // El siguiente código sería un error; queda comentado a propósito:
    //   int* malo() { int local = 3; return &local; }
    // Al volver de malo(), local ya no existe y el puntero queda colgando.
    // La solución es devolver por valor, usar un contenedor o reservar en el
    // heap con un dueño claro (lecciones 09 y 17).
    std::cout << "nunca devuelvas la dirección de una variable local\n";

    return 0;
}
