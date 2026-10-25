/* =============================================================================
   MÓDULO 04 - LECCIÓN 14: Referencias vs punteros
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Una referencia es un alias: otro nombre para un objeto que ya existe.

          int x = 10;
          int& r = x;    // r es otro nombre para x
          r = 20;        // modifica x
          std::cout << x; // 20

      Diferencias clave con un puntero:
        - Una referencia se inicializa al declararse y no puede quedar sin
          objeto: no existe una referencia nula.
        - No se puede "reasignar" para que nombre otro objeto; asignar a la
          referencia asigna al objeto.
        - No tiene su propia dirección observable: &r es la dirección de x.
        - No se puede hacer aritmética de referencias.

      Las referencias son ideales para parámetros que deben existir y para
      evitar copias costosas:

          void escalar(std::vector<int>& v, int factor);        // modifica
          void imprimir(const std::vector<int>& v);             // solo lee

      Los punteros son mejores cuando el valor puede faltar (nullptr), cuando
      hay que recorrer memoria, o cuando se administra propiedad.

   2. ¿QUÉ PASA POR DEBAJO?
      En la práctica una referencia suele implementarse como un puntero
      constante: el compilador guarda la dirección y la usa en cada acceso. No
      ocupa necesariamente memoria propia si se optimiza, y no es un objeto
      direccionable. Una referencia a un objeto destruido sigue siendo un
      puntero colgante, aunque no haya nullptr para detectarlo.

   3. ERRORES COMUNES
      - Devolver una referencia a una variable local: queda colgando.
      - Guardar una referencia a un elemento de un vector y luego hacer
        push_back (puede realocar e invalidarla).
      - Usar una referencia como si se pudiera reasignar.
      - Elegir referencia cuando el parámetro es opcional: ahí conviene T*.
      - Pasar por valor objetos grandes sin necesidad.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/reference
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>

void intercambiarRef(int& a, int& b) {
    int temporal = a;
    a = b;
    b = temporal;
}

void intercambiarPtr(int* a, int* b) {
    if (a == nullptr || b == nullptr) return;
    int temporal = *a;
    *a = *b;
    *b = temporal;
}

void escalar(std::vector<int>& v, int factor) {
    for (int& x : v) x *= factor;
}

// const& evita copiar y garantiza que no se modifica.
std::size_t longitud(const std::string& s) {
    return s.size();
}

int main() {
    std::cout << "=== 1. UNA REFERENCIA ES UN ALIAS ===\n";
    int x = 10;
    int& r = x;
    r = 20;
    std::cout << "x = " << x << ", r = " << r << "\n";
    std::cout << "&x == &r: " << (&x == &r ? "true" : "false") << "\n";

    int y = 99;
    r = y;  // NO rebindea: copia el valor de y en x
    std::cout << "después de r = y: x = " << x << ", y = " << y << "\n";

    std::cout << "\n=== 2. INTERCAMBIO CON REFERENCIA Y CON PUNTERO ===\n";
    int a = 1, b = 2;
    intercambiarRef(a, b);
    std::cout << "ref: a = " << a << ", b = " << b << "\n";
    intercambiarPtr(&a, &b);
    std::cout << "ptr: a = " << a << ", b = " << b << "\n";

    std::cout << "\n=== 3. REFERENCIA COMO PARÁMETRO DE SALIDA/ENTRADA ===\n";
    std::vector<int> datos = {1, 2, 3};
    escalar(datos, 10);
    std::cout << "datos: ";
    for (int n : datos) std::cout << n << " ";
    std::cout << "\n";

    std::cout << "\n=== 4. const& PARA LEER SIN COPIAR ===\n";
    std::string texto = "un string largo que no queremos copiar";
    std::cout << "longitud = " << longitud(texto) << "\n";
    std::cout << "longitud de un literal = " << longitud("hola") << "\n";

    std::cout << "\n=== 5. REFERENCIA A PUNTERO ===\n";
    int valor = 5;
    int* p = &valor;
    int*& refP = p;  // refP es otro nombre para el puntero p
    int otro = 8;
    refP = &otro;    // cambia p, no el valor apuntado
    std::cout << "*p = " << *p << "\n";

    std::cout << "\n=== 6. CUÁNDO USAR CADA UNO ===\n";
    std::cout << "referencia: parámetro obligatorio, sin copia, API clara\n";
    std::cout << "puntero: puede faltar (nullptr), arreglos, propiedad, aritmética\n";
    // int& nula = *static_cast<int*>(nullptr);  // UB: una referencia no es nula
    std::cout << "nunca crees una referencia a partir de un puntero nulo\n";

    std::cout << "\n=== 7. PELIGRO: REFERENCIA A LOCAL ===\n";
    // const int& malo() { int local = 3; return local; }  // colgante
    std::cout << "devolver una referencia a una local es UB al usarla\n";

    return 0;
}
