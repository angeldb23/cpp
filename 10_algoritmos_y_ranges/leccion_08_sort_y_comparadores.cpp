/* =============================================================================
   MÓDULO 10 - LECCIÓN 08: ordenar: sort, comparadores y stable_sort
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       `std::sort(prin, fin)` ordena todo el rango con `<` (orden natural:
       números crecientes, strings alfabéticamente). `std::sort(prin, fin,
       comp)` te deja elegir el criterio con un COMPARADOR callable.

       La regla del comparador es una sola: `comp(a, b)` devuelve true si `a`
       debe ir ANTES que `b` (es un `<` con tu criterio). De ahí salen los
       dos modos clásicos:
         - descendente: el `<` volteado. Con números, `std::greater<int>{}`:
           "a va antes que b si a > b".
         - orden por campo/score: lambda que compara los campos.
       Para orden por VARIOS criterios (primero score, desempate por
       nombre) la construcción directa es `std::tie`: compará un tuple de
       campos; `tie(a, b) < tie(c, d)` es orden lexicográfico: primero el
       primer campo, y si empata (`==`) pasa al segundo, y así.

       `std::stable_sort`: igual que sort (mismo comparador) pero GARANTIZA
       que los elementos que comparan igual (`comp` dice ni antes ni
       después) conservan su orden relativo ORIGINAL. `sort` es más rápido y
       no lo garantiza: si empate no importa (o desempatás vos), sort; si el
       "orden de llegada" es parte de tu negocio, stable_sort.

       EL COSTO: sort es O(n log n) comparaciones, cuando n se duplica, el
       trabajo crece apenas un poco más (n log n). Y "ordenar a demanda" es el costo
       cuando los datos cambian seguido: si los datos son constantes y
       preguntás mucho, mejor ordenar UNA vez y usar búsqueda binaria (L09).
   2. ¿QUÉ PASA POR DEBAJO?
       sort es introsort (quicksort + heapsort si la recursión se enreda +
       inserción en tramos cortos) y NO es estable: el orden final de los
       elementos que comparan igual no está garantizado. stable_sort se
       apoya en merge y necesita memoria temporal extra: ése es su costo
       por conservar el orden de llegada. El comparador se llama O(n log n)
       veces: que sea barato (no aloque strings dentro de cada llamada).

   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
       - El comparador con `<=`: en vez de `<` rompe el orden débil
         estricto que la STL exige; la regla mínima es que comp(a, a) sea
         SIEMPRE false. En builds de debug la STL suele detectarlo; en
         release es comportamiento indefinido.
       - Tu comparador SIEMPRE devuelve bool y responde UNA pregunta:
         ¿a va antes que b? El "empate" se maneja con stable_sort o
         desempatando con un segundo criterio (tie).
       - Ordenar cada frame un vector que cambia apenas: si solo necesitás
         el máximo de cada ronda, min_element (L11) es O(n) sin ordenar todo.
       - El orden de los empates: sort NO lo preserva; stable_sort sí.
   4. PARA PROFUNDIZAR:
       https://en.cppreference.com/w/cpp/algorithm/sort
       https://en.cppreference.com/w/cpp/algorithm/stable_sort
       https://en.cppreference.com/w/cpp/utility/tuple/tie
   ============================================================================= */
#include <algorithm>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

struct Entrada {
    int puntaje;
    std::string nombre;
};

// comparador como FUNCIÓN común (la forma más explícita):
bool porNombre(const Entrada& a, const Entrada& b) {
    return a.nombre < b.nombre;                    // ¿a antes que b?
}

int main() {
    // --- 1. sort básico: orden natural y descendiente ------------------------
    std::vector<int> hp{45, 80, 12, 92, 30};
    std::sort(hp.begin(), hp.end());                          // creciente
    std::cout << "hp creciente: ";
    for (int x : hp) std::cout << x << ' ';
    std::sort(hp.begin(), hp.end(), std::greater<int>{});     // u el lambda volteado
    std::cout << "| decreciente: ";
    for (int x : hp) std::cout << x << ' ';
    std::cout << '\n';

    // --- 2. sort por campo con lambda ----------------------------------------
    std::vector<Entrada> tabla{
        {90, "Quesito"}, {90, "Mordel"}, {120, "Turbo"}, {45, "Lumen"},
    };
    std::sort(tabla.begin(), tabla.end(),
              [](const Entrada& a, const Entrada& b) {
                  return a.puntaje > b.puntaje;          // score de arriba
              });
    std::cout << "tabla por puntaje: ";
    for (const Entrada& e : tabla) std::cout << e.nombre << '=' << e.puntaje << ' ';
    std::cout << '\n';

    // --- 3. multiples criterios con std::tie ---------------------------------
    // Primero puntaje DESC, desempate por nombre ASC (un tie con el orden
    // combados: el puntaje "reversado", el nombre no):
    std::sort(tabla.begin(), tabla.end(),
              [](const Entrada& a, const Entrada& b) {
                  const int pa = -a.puntaje;             // truco: negarlo
                  const int pb = -b.puntaje;
                  return std::tie(pa, a.nombre) < std::tie(pb, b.nombre);
              });
    std::cout << "score desc + nombre asc: ";
    for (const Entrada& e : tabla) std::cout << e.nombre << ' ';
    std::cout << '\n';

    // --- 4. comparator como función / functor --------------------------------
    std::sort(tabla.begin(), tabla.end(), porNombre);      // puntero a función
    std::cout << "alfabetico: ";
    for (const Entrada& e : tabla) std::cout << e.nombre << ' ';
    std::cout << '\n';

    // --- 5. stable_sort: los empates conservan el orden de llegada ----------
    std::vector<Entrada> llegadas{
        {90, "Quesito"}, {90, "Mordel"}, {120, "Turbo"}, {90, "Lumen"},
    };
    std::stable_sort(llegadas.begin(), llegadas.end(),
                     [](const Entrada& a, const Entrada& b) {
                         return a.puntaje > b.puntaje;
                     });
    std::cout << "stable (Quesito antes de Mordel/Lumen): ";
    for (const Entrada& e : llegadas) std::cout << e.nombre << ' ';
    std::cout << '\n';

    return 0;
}
