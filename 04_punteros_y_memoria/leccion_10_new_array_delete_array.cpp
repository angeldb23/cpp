/* =============================================================================
   MÓDULO 04 - LECCIÓN 10: new[], delete[] y arreglos dinámicos
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Para reservar N objetos contiguos se usa new[]:

          int* datos = new int[n];      // n enteros sin inicializar
          int* ceros = new int[n]();    // n enteros en cero
          delete[] datos;               // libera TODO el arreglo

      El arreglo dinámico no guarda su tamaño de forma accesible: si necesitás
      n después, lo llevás en una variable aparte. Esa es una de las razones
      por las que std::vector es preferible: guarda size y capacity y libera
      solo en su destructor.

      new[] y delete[] son una pareja distinta de new y delete:
          new T      <-> delete
          new T[n]   <-> delete[]
      Cruzarlos es comportamiento indefinido.

   2. ¿QUÉ PASA POR DEBAJO?
      new T[n] reserva n * sizeof(T) bytes (más posible metadata del runtime),
      construye cada elemento y devuelve el puntero al primero. delete[] tiene
      que saber cuántos destructores llamar; por eso el runtime guarda el
      número de elementos en algún lado. Con tipos triviales como int el
      efecto es solo liberar, pero la regla no cambia.

      Los elementos son contiguos: datos[i] == *(datos + i). El bloque puede
      ser enorme; el límite es la memoria disponible, no el stack.

   3. ERRORES COMUNES
      - Hacer delete (sin corchetes) sobre un new[]: UB.
      - Hacer delete[] sobre un new simple: UB.
      - Olvidar delete[]: fuga de todo el bloque.
      - Acceder a datos[n] o más allá.
      - Usar el arreglo después de delete[].
      - Copiar el puntero y liberar dos veces.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/new
   ============================================================================= */
#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

int main() {
    std::cout << "=== 1. RESERVAR, USAR Y LIBERAR ===\n";
    const int n = 5;
    int* datos = new int[n]{10, 20, 30, 40, 50};

    std::cout << "datos: ";
    for (int i = 0; i < n; ++i) std::cout << datos[i] << " ";
    std::cout << "\n";
    std::cout << "suma = " << std::accumulate(datos, datos + n, 0) << "\n";

    delete[] datos;
    datos = nullptr;

    std::cout << "\n=== 2. SIN INICIALIZAR VS CON CERO ===\n";
    int* crudo = new int[3];
    int* cero = new int[3]();
    for (int i = 0; i < 3; ++i) crudo[i] = i + 1;  // hay que escribirlos
    std::cout << "crudo (asignado a mano): " << crudo[0] << " " << crudo[1]
              << " " << crudo[2] << "\n";
    std::cout << "cero (value-initialized): " << cero[0] << " " << cero[1]
              << " " << cero[2] << "\n";
    delete[] crudo;
    delete[] cero;

    std::cout << "\n=== 3. EL TAMAÑO NO VIAJA CON EL PUNTERO ===\n";
    int* v = new int[n];
    std::cout << "sizeof(v) = " << sizeof(v) << " bytes (solo el puntero)\n";
    std::cout << "los " << n << " elementos no se pueden recuperar desde v\n";
    delete[] v;

    std::cout << "\n=== 4. ARREGLO DINÁMICO DE OBJETOS ===\n";
    std::string* nombres = new std::string[3]{"Ana", "Beto", "Carla"};
    for (int i = 0; i < 3; ++i) std::cout << nombres[i] << " ";
    std::cout << "\n";
    delete[] nombres;  // llama al destructor de cada string

    std::cout << "\n=== 5. LA ALTERNATIVA SEGURA: std::vector ===\n";
    {
        std::vector<int> vec(n);
        std::iota(vec.begin(), vec.end(), 1);
        std::cout << "vector: ";
        for (int x : vec) std::cout << x << " ";
        std::cout << "\n";
        std::cout << "size = " << vec.size() << ", capacity = " << vec.capacity()
                  << "\n";
    }  // libera solo, sin delete[]

    std::cout << "\n=== 6. REGLA ===\n";
    std::cout << "new[] con delete[]; si podés, usá vector y olvidate del delete\n";

    return 0;
}
