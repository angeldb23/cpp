/* =============================================================================
   MÓDULO 10 - LECCIÓN 02: categorías de iteradores y cómo avanzar con cuidado
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       El contrato de la L01 (*it, ++it, !=) es el piso mínimo. Sobre ese piso
       hay CATEGORÍAS de iteradores, de la más débil a la más fuerte:
         - input/output: se recorre UNA vez, paso a paso (el L18 muestra los
           istream_iterator/ostream_iterator). No se puede volver atrás.
         - forward: lo de arriba + se puede recorrer MÁS DE UNA vez y guardar
           el iterador para retomar (unordered_map, unordered_set).
         - bidirectional: forward + `--it` hacia atrás (list, set, map).
         - random access: bidirectional + saltar de a N pasos en O(1)
           (`it + 5`, `it - it`, `it < otro`): vector, array, deque, punteros.

       ¿Por qué te importa? Porque cada algoritmo de la STL "cobra lo mínimo":
       `find` pide forward y sirve para casi todo; `sort` pide random access
       porque necesita saltar al medio y reordenar, y por eso `std::list` NO
       acepta `std::sort`: `list` tiene su propio `lst.sort()` (que recorre
       nodos). Cuando un algoritmo pide una categoría que tu contenedor no
       cumple, NO es error en runtime: directamente no compila.

       Las funciones para moverse que el resto del módulo usa todo el tiempo:
         - `std::distance(it1, it2)`: cuántos pasos hay entre los dos. O(n)
           en bidi/forward; O(1) en random access (es una resta).
         - `std::advance(it, n)`: avanza el iterador QUE LE PASASTE n pasos
           (n negativo si es bidi+). Modifica el original: in-place.
         - `std::next(it, n)` / `std::prev(it, n)`: tocan nada: devuelven
           una COPIA avanzada/retrocedida. Son la versión "no destructiva".
   2. ¿QUÉ PASA POR DEBAJO?
       Las categorías no son clases heredadas de runtime: son etiquetas de
       tipos. `iterator_traits<Iter>::iterator_category` la lee el algoritmo y
       el compilador elige la implementación que le alcanza a esa etiqueta
       (técnicamente usa SFINAE/cconceptos: la maquinaria que viste en el
       módulo 09). Con la etiqueta random_access, std::distance compila a una
       resta de punteros y std::advance a un `it += n`; con una más débil,
       quedan bucles de ++/--. Es el mismo espíritu del compilador eligiendo
       la mejor versión: la interfaz pública no cambia, el costo sí.
   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
       - advance/next con n NEGATIVO sobre un contenedor forward-only
         (unordered_*): no compila (la etiqueta no garantiza --).
       - distance dentro de un bucle sobre bidi: convertiste O(n log n) en
         O(n^2) sin darte cuenta. En random access no aplica (es O(1)).
       - `std::end(v)` guardado y luego advance negativo: `prev(begin)` es
         inválido (no hay nada antes del primero).
       - Para avanzar UN solo paso, `++it` directo es más claro que
         std::advance(it, 1).
   4. PARA PROFUNDIZAR:
       https://en.cppreference.com/w/cpp/iterator
       https://en.cppreference.com/w/cpp/iterator/distance
       https://en.cppreference.com/w/cpp/iterator/advance
   ============================================================================= */
#include <algorithm>
#include <iostream>
#include <list>
#include <vector>

// Devuelve el valor del índice i de la secuencia. Compila para CUALQUIER
// contenedor con iteradores avanzables, pero el COSTO depende de la
// categoría: random access salta en O(1), bidi avanza de a uno en O(n).
template <typename Cont>
int valorEn(const Cont& c, int i) {
    auto it = c.begin();
    std::advance(it, i);
    return *it;                                    // asume que i es válido
}

int main() {
    std::vector<int> v{10, 20, 30, 40, 50, 60};   // random access
    std::list<int> l{10, 20, 30, 40, 50, 60};     // bidirectional

    std::cout << "valorEn(v, 3) = " << valorEn(v, 3) << '\n';
    std::cout << "valorEn(l, 3) = " << valorEn(l, 3) << '\n';

    // --- 1. distance: cuántos pasos hay entre dos iteradores ----------------
    // En vector: una resta (O(1)). En list: un paseo contando (O(n)).
    std::cout << "distance(v.begin(), v.end()) = "
              << std::distance(v.begin(), v.end()) << '\n';
    std::cout << "distance(l.begin(), l.end()) = "
              << std::distance(l.begin(), l.end()) << '\n';

    // Es la respuesta sana a "¿qué índice es este iterador?":
    const auto en30 = std::find(v.begin(), v.end(), 30);
    std::cout << "el 30 esta en el indice "
              << std::distance(v.begin(), en30) << '\n';

    // --- 2. advance: mueve el iterador ORIGINAL ------------------------------
    auto li = l.begin();
    std::advance(li, 2);                       // li MOVIDO in-place: ya apunta al 30
    std::cout << "advance(+2) -> " << *li << '\n';

    // --- 3. next/prev: devuelven una copia, el original queda igual ----------
    const auto salto = std::next(li, 3);        // apunta al 60
    const auto atras = std::prev(li);           // apunta al 20
    std::cout << "next(+3) -> " << *salto << " | prev -> " << *atras
              << " | li sigue en " << *li << '\n';

    // --- 4. Retroceder en bidirectional: end() - n ---------------------------
    auto desdeAtras = l.end();
    std::advance(desdeAtras, -2);               // end-2 -> el 50
    std::cout << "end-2 en list -> " << *desdeAtras << '\n';

    // El patrón de "el último elemento" sin riesgo (validando antes):
    if (!v.empty()) {
        std::cout << "el ultimo de v es " << *std::prev(v.end()) << '\n';
    }

    return 0;
}
