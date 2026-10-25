/* =============================================================================
   MÓDULO 10 - LECCIÓN 09: búsqueda binaria sobre ordenado + nth_element
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       La familia de búsqueda binaria pide UNA condición previa: el rango
       TIENE que estar ordenado según el mismo criterio con el que vas a
       buscar. Sobre el rango ordenado hay cuatro algoritmos:
         - `std::lower_bound(prin, fin, x)`: iterador al PRIMER elemento
           >= x. Es el más útil de todos.
         - `std::upper_bound(prin, fin, x)`: iterador al PRIMER elemento
           > x (estrictamente mayor). lower y upper delimitan el tramo de
           los que SON x (si los hay).
         - `std::equal_range(prin, fin, x)`: devuelve el PAR (lower, upper)
           en una llamada: el tramo completo de los iguales a x.
         - `std::binary_search(prin, fin, x)`: bool puro, ¿está o no está?
           (por dentro es un lower_bound + un chequeo). Para solo saber si
           existe, alcanza; si querés el elemento, lower_bound.

       El caso de uso del lower_bound MÁS ALLÁ de buscar: INSERTAR
       manteniendo el orden: `pos = lower_bound(...); v.insert(pos, x);`
       La incorporación queda bien ubicada sin reordenar todo (el search
       es O(log n); el propio insert es O(n) por el corrimiento).

       Y el as bajo la manga: `std::nth_element(prin, nth, fin)`. Coloca en
       `nth` el elemento que quedaría ahí si el rango estuviera ORDENADO,
       deja ANTES de él todo lo menor y DESPUÉS todo lo mayor, pero NO
       ordena el resto: hace menos trabajo que sort. Es la jugada para la
       MEDIANA (nth = medio) y para el TOP-K (nth = fin - k: los k mayores
       ya quedan agrupados en la cola, para ordenarlos recién si hace falta).

       Y la ADVERTENCIA del módulo: búsqueda binaria sobre datos SIN
       ordenar no compila-error ni da crash: devuelve POSICIONES SIN SENTIDO
       peor tipo de bug: silencioso). El orden es TU contrato, no del
       algoritmo.
   2. ¿QUÉ PASA POR DEBAJO?
       La binaria salta al medio: compara con el del medio y descarta la
       mitad que no puede contener a x; y así hasta llegar. log2(1.000.000)
       son ~20 comparaciones contra 1.000.000 del find de la L03: por eso
       para un lote que se consulta muchísimo vale ordenar una vez.
       lower_bound con repetidos devuelve el primero. nth_element funciona
       como una partición (L11) alrededor del n-ésimo: es O(n) promedio.
   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
       - lower_bound sobre desordenado: bug silencioso. Si no estás seguro
         del orden, validalo (lección: la invariante es de TU estructura).
       - binary_search es solo el sí/no; si
         querés el ITERADOR (para mirar, modificar o borrar), lower_bound.
       - Confusión lower/upper: lower = primer >= x; upper = primer > x.
         Sin repetidos dan el MISMO lugar (el lugar donde iría x); con
         repetidos delimitan el tramo de los iguales.
       - nth_element pide iteradores random access (vector/array/deque).
   4. PARA PROFUNDIZAR:
       https://en.cppreference.com/w/cpp/algorithm/lower_bound
       https://en.cppreference.com/w/cpp/algorithm/equal_range
       https://en.cppreference.com/w/cpp/algorithm/nth_element
   ============================================================================= */
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

int main() {
    // --- 1. El rango ordenado (el contrato) ----------------------------------
    std::vector<int> tabla{10, 20, 30, 30, 30, 40, 50};   // ya ordenada!

    // lower_bound: primer >= x
    const auto primer30 = std::lower_bound(tabla.begin(), tabla.end(), 30);
    // upper_bound: primer > x
    const auto fin30 = std::upper_bound(tabla.begin(), tabla.end(), 30);
    std::cout << "primer >= 30 en el indice "
              << std::distance(tabla.begin(), primer30)
              << "; los iguales a 30 ocupan "
              << std::distance(primer30, fin30) << " lugares\n";

    // --- 2. equal_range: el tramo completo de los iguales --------------------
    const auto tramo = std::equal_range(tabla.begin(), tabla.end(), 30);
    std::cout << "equal_range: ["
              << std::distance(tabla.begin(), tramo.first) << ", "
              << std::distance(tabla.begin(), tramo.second) << ")\n";

    // --- 3. binary_search: solo el booleano ----------------------------------
    std::cout << "esta el 35? "
              << (std::binary_search(tabla.begin(), tabla.end(), 35) ? "si" : "no")
              << '\n';

    // --- 4. lower_bound para INSERTAR en orden --------------------------------
    std::vector<int> guardado{12, 45, 78};              // ordenada
    const int nuevo = 50;
    const auto pos = std::lower_bound(guardado.begin(), guardado.end(), nuevo);
    guardado.insert(pos, nuevo);                        // queda ordenada
    std::cout << "insertado el 50: ";
    for (int x : guardado) std::cout << x << ' ';
    std::cout << '\n';

    // --- 5. nth_element: mediana sin ordenar todo ----------------------------
    std::vector<int> tiempos{320, 90, 150, 700, 240, 60, 180};
    const std::size_t medio = tiempos.size() / 2;
    std::nth_element(tiempos.begin(), tiempos.begin() + static_cast<long>(medio),
                     tiempos.end());
    std::cout << "mediana (elemento del medio): " << tiempos[medio] << '\n';
    std::cout << "el resto NO quedo ordenado: ";
    for (int x : tiempos) std::cout << x << ' ';
    std::cout << '\n';

    // top-3: los mayores quedan en la cola del rango:
    std::vector<int> scores{320, 90, 150, 700, 240, 60};
    std::nth_element(scores.begin(), scores.end() - 3, scores.end());
    std::vector<int> top({scores[scores.size() - 3], scores[scores.size() - 2],
                          scores[scores.size() - 1]});
    std::sort(top.begin(), top.end(), std::greater<int>{});   // ahora si
    std::cout << "top 3: ";
    for (int x : top) std::cout << x << ' ';
    std::cout << '\n';

    return 0;
}
