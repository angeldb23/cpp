/* =============================================================================
   MÓDULO 10 - LECCIÓN 01: iteradores, begin/end y el contrato de los tres
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       Un iterador es un objeto que señala UN elemento de una secuencia y sabe
       tres cosas: a qué apunta (`*it`), cómo avanzar al siguiente (`++it`) y
       cuándo llegó al final (`it != fin`). Es la abstracción que hizo posible
       escribir los algoritmos STL UNA sola vez para cualquier contenedor:
       `std::find` no sabe si le pasaste un `vector`, una `list` o un `array`;
       solo usa ese contrato.

       Todos los contenedores están pensados como un RANGO semiabierto:
       [begin, end). begin señala el primer elemento y end señala "uno después
       del último", el lugar donde dejaría de haber contenido nuevo. Ese end
       diseñado así tiene dos consecuencias que se aprovechan todo el tiempo:
         - el rango vacío es válido (begin == end) y no necesita caso especial;
         - "pasa el final como límite" y "devuelve el final cuando no encontré"
         son la MISMA convención en todos los algoritmos.

       El range-for del módulo 03 es azúcar sobre exactamente ese contrato: el
       compilador lo expande a un bucle con begin()/end(). En esta lección
       trabajamos los iteradores a mano, porque TODO el módulo 10 se apoya en
       ellos: los algoritmos los reciben y los devuelven.

       Continuidad (módulo 03): ya viste las reglas de invalidación (un
       push_back puede realocar el vector y matar los iteradores que guardaste;
       erase devuelve el iterador al siguiente para poder seguir). Acá no se
       repite la teoría: solo suma que lo que devuelve un algoritmo STL es
       invalidable exactamente igual que un iterador que sacaste vos.
   2. ¿QUÉ PASA POR DEBAJO?
       Para `vector`, el iterador SUELE ser directamente `T*`: un puntero crudo
       que cumple el contrato (*it, ++it, !=). Por eso recorrer un vector con
       iteradores es tan barato y por eso lo del módulo 04 se traduce
       directo: `*(v.begin() + 2)` y `v[2]` son la misma cosa para un vector.
       Para una `list`, en cambio, el iterador es un puntero a nodo que avanza
       con `next`: la INTERFAZ es la misma, el COSTO por paso es otro (un salto
       de puntero por ++, sin aritmética directa).
       El range-for `for (int x : v)` se expande aproximadamente a:
           {
             auto __it = v.begin();
             auto __fin = v.end();
             for (; __it != __fin; ++__it) { int x = *__it; cuerpo; }
           }
       Por eso cualquier tipo que defina begin/end usable se puede recorrer con
       range-for, aunque no sea un contenedor: lo vas a ver cuando lleguen los
       views (L14).
   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
       - Desreferenciar end(): no apunta a un elemento, es "el justo después".
         Antes de usar `*it`, SIEMPRE chequeá `it != fin`.
       - El caso borde que más se olvida: un contenedor VACÍO tiene
         begin() == end(); un bucle manual no entra ni una vez, y si en tu
         lógica esperabas al menos un elemento, validalo ANTES.
       - Calcular índices restando iteradores en contenedores que no son de
         acceso aleatorio: la resta no existe ahí; usá `std::distance` (L02).
       - Cuando solo leés o solo modificás valores y no necesitás el
         iterador, preferí range-for: menos código, mismas garantías.
   4. PARA PROFUNDIZAR:
       https://en.cppreference.com/w/cpp/iterator/begin
       https://en.cppreference.com/w/cpp/language/range-for
       https://en.cppreference.com/w/cpp/container/vector/end
   ============================================================================= */
#include <algorithm>
#include <iostream>
#include <list>
#include <vector>

// --- 1. El bucle a mano: el range-for sin azúcar -----------------------------
// Misma secuencia, dos formas de recorrerla. Fijate que el cuerpo es idéntico:
// esa es la promesa del contrato de iterador.
int main() {
    std::vector<int> vida{100, 45, 0, 80, 12};

    std::cout << "range-for (azucar):\n  ";
    for (int hp : vida) std::cout << hp << ' ';

    std::cout << "\nbucle con iteradores (lo que realmente pasa):\n  ";
    for (auto it = vida.begin(); it != vida.end(); ++it) {
        std::cout << *it << ' ';                  // *it: a qué apunto
    }

    // --- 2. El iterador no solo lee: puede VOLVER A ESCRIBIR el elemento
    // Un iterador de un vector no-const permite modificar lo que apunta,
    // igual que un puntero a T no-const (L02 del módulo 04).
    std::cout << "\nrevivir a los caidos (+50 si hp < 50):\n  ";
    for (auto it = vida.begin(); it != vida.end(); ++it) {
        if (*it < 50) *it += 50;                  // escribo A TRAVES del iterador
        std::cout << *it << ' ';
    }

    // --- 3. El end() es un límite, no un elemento ----------------------------
    // std::find devuelve end() cuando no encontró. Ese "sin valor especial
    // para 'no encontré'" es la convención de TODO el módulo.
    const auto caido = std::find(vida.begin(), vida.end(), 0);
    if (caido == vida.end()) {
        std::cout << "\nno quedan hp en 0 (find devolvio end)\n";
    }

    // caso borde/planteado a proposito: contenedor VACIO
    std::vector<int> vacia;
    std::cout << "vacia: begin == end? " << (vacia.begin() == vacia.end()) << '\n';

    // --- 4. La interfaz es la misma en TODA la STL ---------------------------
    // list también tiene begin/end: el mismo find corre sin tocar nada.
    std::list<int> lista_vida{100, 45, 80};
    const auto primero = std::find(lista_vida.begin(), lista_vida.end(), 45);
    // distancia entre begin y lo encontrado: lo cuenta (más en L02)
    std::cout << "en la lista, el hp 45 esta en la posicion "
              << std::distance(lista_vida.begin(), primero) << '\n';

    return 0;
}
