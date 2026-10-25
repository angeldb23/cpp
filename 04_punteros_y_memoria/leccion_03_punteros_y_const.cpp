/* =============================================================================
   MÓDULO 04 - LECCIÓN 03: Punteros y const
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      const puede aplicarse a tres cosas distintas cuando hay un puntero:

          const int* p;        // puntero a int constante: no podés escribir *p
          int* const q = &x;   // puntero constante: no podés cambiar q, sí *q
          const int* const r;  // ambos constantes

      Leé la declaración de derecha a izquierda:
          const int* p   -> "p es un puntero a un int const"
          int* const q   -> "q es un puntero const a un int"

      El const del objeto apuntado es el que más se usa en APIs: una función
      que solo lee recibe const T* y así puede aceptar tanto T* como const T*.
      El const del propio puntero casi nunca aparece en parámetros, porque el
      parámetro es una copia local; en la firma se ignora.

   2. ¿QUÉ PASA POR DEBAJO?
      const no cambia el tamaño ni la representación del puntero. Es una
      restricción de compilación: el compilador rechaza las escrituras y puede
      optimizar sabiendo que el objeto no cambia por esa vía. Un const_cast
      puede quitar el const, pero si el objeto original era realmente const y
      lo modificás, el comportamiento es indefinido.

   3. ERRORES COMUNES
      - Creer que const int* impide modificar el original por otra vía: no lo
        impide, solo prohíbe hacerlo a través de ese puntero.
      - Intentar asignar const int* a int* (el compilador lo rechaza).
      - Usar const_cast para "arreglar" una API mal diseñada.
      - Olvidar que un puntero const puede apuntar a un objeto no const.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/cv
   ============================================================================= */
#include <iostream>

// Lee a través del puntero: no necesita modificarlo.
int sumar(const int* datos, int n) {
    int total = 0;
    for (int i = 0; i < n; ++i) {
        total += datos[i];  // solo lectura
    }
    return total;
}

// Recibe un puntero cuyo VALOR no cambia, pero el objeto sí puede cambiar.
void duplicarConPunteroFijo(int* const p) {
    if (p != nullptr) *p *= 2;
}

int main() {
    std::cout << "=== 1. PUNTERO A DATO CONSTANTE ===\n";
    int x = 10;
    const int* p = &x;  // puedo leer *p, no escribir
    std::cout << "*p = " << *p << "\n";
    // *p = 20;         // ERROR de compilación
    x = 20;             // el objeto original no es const: se puede cambiar
    std::cout << "x cambió por otra vía; *p = " << *p << "\n";

    int y = 30;
    p = &y;             // el puntero sí puede apuntar a otro lado
    std::cout << "p ahora apunta a y: *p = " << *p << "\n";

    std::cout << "\n=== 2. PUNTERO CONSTANTE ===\n";
    int z = 5;
    int* const q = &z;  // q no puede apuntar a otro objeto
    *q = 6;             // pero el objeto sí cambia
    std::cout << "z = " << z << "\n";
    // q = &x;          // ERROR de compilación

    std::cout << "\n=== 3. AMBOS CONSTANTES ===\n";
    const int* const r = &x;
    std::cout << "*r = " << *r << "\n";
    // *r = 1;  // ERROR
    // r = &y;  // ERROR

    std::cout << "\n=== 4. const EN PARÁMETROS DE LECTURA ===\n";
    int datos[] = {1, 2, 3, 4};
    const int* vista = datos;
    std::cout << "sumar(datos, 4)        = " << sumar(datos, 4) << "\n";
    std::cout << "sumar(vista, 4)        = " << sumar(vista, 4) << "\n";
    // sumar acepta int* y const int*: el const es más general para leer.

    std::cout << "\n=== 5. const EN EL PROPIO PUNTERO COMO PARÁMETRO ===\n";
    int n = 7;
    duplicarConPunteroFijo(&n);
    std::cout << "n después de duplicar = " << n << "\n";
    // En la firma, int* const es equivalente a int*: el const es local.

    std::cout << "\n=== 6. CONVERSIÓN PELIGROSA ===\n";
    const int* soloLectura = &x;
    // int* sinConst = soloLectura;  // ERROR: se perdería el const
    int* sinConst = const_cast<int*>(soloLectura);
    *sinConst = 99;  // x no era const, así que es válido; si lo fuera, sería UB
    std::cout << "x = " << x << " (modificado con const_cast, solo porque x no era const)\n";

    return 0;
}
