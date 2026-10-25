/* =============================================================================
   MÓDULO 10 - LECCIÓN 11: partir en dos: partition/partition_point y min/max
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       `std::partition(prin, fin, pred)`: reordena el rango para que los que
       CUMPLEN el predicado queden todos a la IZQUIERDA y los que no, a la
       DERECHA. Devuelve el iterador al PRIMER elemento que NO cumple (el
       punto de corte). No ordena nada adentro de cada lado: es el
       "agrupar por un sí/no" barato: sanos/envenenados, vivos/muertos,
       jugables/no.

       La versión con garantías: `std::stable_partition`: igual pero
       conserva el orden relativo DENTRO de cada lado. Su costo es mayor:
       O(n log n) comparaciones y memoria temporal para lograr
       (típicamente Además de O(n) de memoria temporal).

       `std::is_partitioned(prin, fin, pred)`: bool: ¿el rango ya está
       cortado según pred? Sirve para un chequeo barato antes de confiar
       en partition_point.

       `std::partition_point(prin, fin, pred)`: sobre un rango YA
       particionado, encuentra el punto de corte CON BÚSQUEDA BINARIA en
       O(log n) en vez de recorrerlo todo otra vez. Es la lower_bound de
       los particionados: sirve cuando la condición genera "un tramo a la
       izquierda y otro a la derecha" (sin orden adentro de cada uno).

       La familia min/max en una pasada:
         - `min_element` / `max_element`: iterador al mínimo / al máximo.
           Devuelven el ITERADOR (no el valor): con el iterador podés
           borrar al más herido o saber su posición.
         - `std::minmax_element(prin, fin)`: AMBOS en una sola pasada: par
           de iteradores (min, max) y con menos comparaciones que dos
           llamadas separadas.
         - `min(a, b)`, `max(a, b)` por valores, y `std::minmax` para el
           par de valores.
   2. ¿QUÉ PASA POR DEBAJO?
       partition es UNA pasada O(n) con swaps (es la misma técnica de la
       partición de quicksort que se ve en L08/L09). stable_partition usa
       merge con memoria temporal para conservar el orden original de cada
       lado: por eso es más caro. partition_point es una búsqueda binaria
       sobre una secuencia que SABES que es "todos sí, luego todos no":
       con ese regla vale la binaria, igual que en la L09.
   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
       - Esperar una salida aparte: partition NO copia, reordena IN-PLACE
         el rango original. Si el original valioso, copiálo antes.
       - partition_point sobre un rango NO particionado: resultados sin
         sentido (mismo bug silencioso que la búsqueda binaria desordenada:
         el contrato del L09 se repite).
       - min/max_element sobre UNA secuencia vacía devuelve end(): chequeá
         antes de desreferenciar (L01).
       - Necesitar la POSICIÓN después de particionar: la función devuelve
         el ITERADOR de corte; con distance lo convertís en índice (L02).
   4. PARA PROFUNDIZAR:
       https://en.cppreference.com/w/cpp/algorithm/partition
       https://en.cppreference.com/w/cpp/algorithm/partition_point
       https://en.cppreference.com/w/cpp/algorithm/minmax_element
   ============================================================================= */
#include <algorithm>
#include <iostream>
#include <vector>

struct Guerrer {
    std::string nombre;
    int hp;
    bool llave;
};

int main() {
    std::vector<Guerrer> sala{
        {"Bulwark", 90, true}, {"Zarla", 15, false}, {"Tank", 60, true},
        {"Perro", 5, false},
    };

    // --- 1. partition: los CON llave a la izquierda --------------------------
    const auto corte = std::partition(sala.begin(), sala.end(),
                                      [](const Guerrer& g) { return g.llave; });
    std::cout << "con llave: ";
    for (auto it = sala.begin(); it != corte; ++it)
        std::cout << it->nombre << ' ';
    std::cout << "| sin llave: ";
    for (auto it = corte; it != sala.end(); ++it)
        std::cout << it->nombre << ' ';
    std::cout << "| indice de corte: "
              << std::distance(sala.begin(), corte) << '\n';

    // --- 2. is_partitioned + partition_point sobre lo ya cortado -------------
    std::cout << "esta particionado? "
              << (std::is_partitioned(sala.begin(), sala.end(),
                    [](const Guerrer& g) { return g.llave; }) ? "si" : "no")
              << '\n';
    // El corte actual también se puede re-encontrar en O(log n):
    const auto punto = std::partition_point(sala.begin(), sala.end(),
                                            [](const Guerrer& g) { return g.llave; });
    std::cout << "partition_point = indice "
              << std::distance(sala.begin(), punto) << '\n';

    // --- 3. min/max_element: iteradores, para poder tocar el elemento --------
    const auto masHerido = std::min_element(sala.begin(), sala.end(),
        [](const Guerrer& a, const Guerrer& b) { return a.hp < b.hp; });
    std::cout << "el mas herido es " << masHerido->nombre << " (hp "
              << masHerido->hp << ")\n";

    // --- 4. minmax_element: los DOS en una pasada ----------------------------
    const std::vector<int> hpTotal{90, 15, 60, 5, 72};
    const auto extremos = std::minmax_element(hpTotal.begin(), hpTotal.end());
    std::cout << "min " << *extremos.first << " | max " << *extremos.second
              << '\n';

    return 0;
}
