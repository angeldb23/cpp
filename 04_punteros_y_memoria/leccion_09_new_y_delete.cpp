/* =============================================================================
   MÓDULO 04 - LECCIÓN 09: new y delete para objetos individuales
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      new crea un objeto en el heap y devuelve un puntero a él. delete destruye
      ese objeto y libera su memoria. Son operaciones emparejadas:

          int* p = new int(42);   // reserva + inicializa
          std::cout << *p;
          delete p;               // destruye + libera
          p = nullptr;            // evita un puntero colgante

      Formas de inicialización:
          new int          -> valor indeterminado (no lo leas)
          new int{}        -> cero
          new int(42)      -> 42
          new int{42}      -> 42 (inicialización uniforme)

      new no es malloc: además de reservar, llama al constructor del tipo. Y
      delete no es free: llama al destructor antes de liberar.

   2. ¿QUÉ PASA POR DEBAJO?
      new llama a operator new (que suele terminar en malloc), y si falla lanza
      std::bad_alloc. Luego construye el objeto en esa memoria. delete llama al
      destructor y a operator delete. El compilador genera código distinto para
      delete y delete[]; mezclarlos es comportamiento indefinido.

      Si el constructor lanza una excepción, la memoria reservada por new se
      libera automáticamente: no queda a medias. Eso no vale para recursos que
      el constructor ya haya adquirido por su cuenta.

   3. ERRORES COMUNES
      - Olvidar delete: fuga de memoria.
      - Hacer delete dos veces sobre el mismo puntero: doble liberación.
      - Usar el puntero después de delete: use-after-free.
      - Hacer delete de un puntero que no vino de new.
      - Mezclar new con free o malloc con delete.
      - Perder el único puntero al bloque (p = otro) sin liberarlo.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/new
   ============================================================================= */
#include <iostream>
#include <new>
#include <string>

struct Ficha {
    std::string nombre;
    int nivel;

    Ficha(const std::string& n, int l) : nombre(n), nivel(l) {
        std::cout << "  constructor de " << nombre << "\n";
    }
    ~Ficha() {
        std::cout << "  destructor de " << nombre << "\n";
    }
};

int main() {
    std::cout << "=== 1. new CON INICIALIZACIÓN ===\n";
    int* a = new int(42);
    int* b = new int{};       // cero
    int* c = new int;         // indeterminado: no leer sin asignar
    *c = 7;
    std::cout << "*a = " << *a << ", *b = " << *b << ", *c = " << *c << "\n";
    delete a;
    delete b;
    delete c;
    a = b = c = nullptr;

    std::cout << "\n=== 2. new LLAMA AL CONSTRUCTOR; delete AL DESTRUCTOR ===\n";
    Ficha* f = new Ficha("Ana", 3);
    std::cout << "  f->nombre = " << f->nombre << ", f->nivel = " << f->nivel << "\n";
    delete f;
    f = nullptr;

    std::cout << "\n=== 3. PUNTERO COLGANTE DESPUÉS DE delete ===\n";
    int* p = new int(5);
    delete p;
    // *p = 10;  // UB: la memoria ya no es nuestra
    p = nullptr;  // así una comprobación posterior funciona
    if (p == nullptr) std::cout << "  p quedó en nullptr: seguro\n";

    std::cout << "\n=== 4. SI NO HAY MEMORIA ===\n";
    // El new normal lanza std::bad_alloc cuando no puede reservar. La sintaxis
    // try/catch se ve en la lección 16; acá alcanza con saber que la excepción
    // existe. La variante nothrow devuelve nullptr en lugar de lanzar.
    int* q = new (std::nothrow) int(9);
    if (q != nullptr) {
        std::cout << "  reserva nothrow OK: " << *q << "\n";
        delete q;
    } else {
        std::cout << "  la reserva nothrow devolvió nullptr\n";
    }

    std::cout << "\n=== 5. new Y delete NO SON malloc/free ===\n";
    std::cout << "  new construye; delete destruye; no los mezcles con free\n";

    std::cout << "\n=== 6. REGLA DE ORO ===\n";
    std::cout << "  cada new tiene exactamente un delete\n";
    return 0;
}
