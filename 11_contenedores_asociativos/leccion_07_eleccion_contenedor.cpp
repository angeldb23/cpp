/* =============================================================================
   MÓDULO 11 - LECCIÓN 07: elegir el contenedor: complejidad como criterio
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
        Con map/set y sus primos hash ya acumulás SEIS contenedores
        asociativos de la biblioteca estándar... y básicos como vector,
        deque y list del módulo 03. Elegir mal no te da un error: te da un
        programa lento o con memoria desordenada. Criterios de complejidad,
        en TODO el módulo:
                                              buscar     insertar/borrar
          std::vector (ordenado, bsearch)     O(log n)   O(n) (mover todo)
          std::vector                         O(n)       O(1) amort. al final
          std::deque                          O(n)       O(1) en ambos bordes
          std::list                           O(n)       O(1) con iterador
          std::map / std::set                 O(log n)   O(log n)
          std::unordered_map / _set           O(1) prom  O(1) prom

        El criterio que sigue a la TABLA es de uso (¿qué hace este dato?):
          - "pregunto por cada uno una vez" => lo recorres igual: sirve
            cualquier; elegí el más simple (vector).
          - "el orden de las claves me da los primeros/últimos": map/set.
          - "memo buscar por clave mil veces por frame": unordered_map.
          - "agrego y saco del frente y del fondo": deque.
          - "solo entro/salgo por un lado o uso atrás": vector es el rey.

        Otra categoría NECESARIA: si tu n es chico (decenas, cientos),
        la complejidad asintótica no te cambia la vida: el vector con
        búsqueda lineal gana igual. La complejidad monta cuando n escala.
   2. ¿QUÉ PASA POR DEBAJO?
        Los O(...) de la tabla no son todo: es la complejidad ASINTÓTICA
        (comparaciones, movimientos de datos). Nada te dice el costo
        CONSTANTE de cada una: un push_back de un vector escribe al final
        del buffer (1 escritura); un insert de un map repara el árbol
        (rotaciones + enlaces de puntero) y hace un alloc nuevo (mucho más
        caro que la escritura). Por eso "O(log n)" no es "todo bien": es
        "comportamiento a escala garantizado".

        En ordenados, O(log n) de búsqueda suele ser aceptable en la
        práctica: 20 comparaciones con n = 1.000.000. Con unordered, es
        "hash + 1 comparación": O(1) con caso peor O(n) (todas colisionan).
        El caso peor IMPORTA en juegos por el "hitch": tu presupuesto de
        trabajo por frame es fijo, y late un frame nunca: por eso hay
        equipos que evitan la tabla hash pura (y prefieren map con su
        garantía de caso peor).
   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
        - map<int, T> cuando el índice es un int denso: los enteros son
          tan bucols / casi son de detalle: ahí lo correcto es un VECTOR.
          Pero map<int, T> con IDs dispersos (300, 7200, 90000) sí pide
          un contenedor asociativo (o tu tabla dispersa).
        - list<T> "porque voy a borrar del medio": borra rápido SI ya tenés
          el iterador, pero la búsqueda es O(n) y cada nodo es un alloc:
          en general un vector con la pieza en la mano gana.
        - Dejarte llevar por O(...) sin mirar el uso real: "O(n) iterar" en
          un vector contiguo suele bater a "O(log n) buscar" en un árbol
          cuando las llamadas siguen y el n es chico: la localidad pesa
          (es el tema de la próximo lección).
        - deque con insertion en el medio "porque es rápido en los
          extremos": no lo es en el medio (y es tu caso).
   4. PARA PROFUNDIZAR:
        https://en.cppreference.com/w/cpp/container (la tabla completa de
          complejidades por cada contenedor sigue la página índice)
        https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#es-stl-size
   ============================================================================= */
#include <algorithm>
#include <deque>
#include <iostream>
#include <list>
#include <map>
#include <unordered_map>
#include <vector>

int main() {
    // --- 1. ¿IDs densos o dispersos? primer criterio gold ----------------------------
    std::vector<std::string> porIndice{"reparto", "menu", "sombra"};
    // (IDs 0,1,2 densos: el index ES el contenedor: nadie hace falta un map)

    std::unordered_map<int, std::string> Disperso;
    Disperso[300]   = "bandido a distinta fila";
    Disperso[7200]  = "torre lejana";
    Disperso[900500] = "dragon del fondo";
    for (const auto& [id, nombre] : Disperso)
        std::cout << "id " << id << " -> " << nombre << '\n';

    // --- 2. eleccion por complejidad: en un map<int, float> la busqueda es O(log n) ---
    std::map<int, float> aleatorio;
    for (int i = 0; i < 5; ++i) aleatorio.emplace(i * 1000, 0.1f * i);
    auto lo = aleatorio.lower_bound(2000);
    std::cout << "\nlower_bound(2000) = " << lo->first << '\n';

    // --- 3. list vs vector vs deque: ¿dónde meto/saco? --------------------------------
    std::list<int> eventos_frecuente{5, 6, 7};
    auto middleIt = std::next(eventos_frecuente.begin());
    eventos_frecuente.insert(middleIt, 99);   // O(1): ya tengo el iterador

    std::deque<int> dobleBorde{1, 2, 3};
    dobleBorde.push_front(0);                 // O(1)
    dobleBorde.push_back(4);                  // O(1)
    std::cout << "deque:";
    for (int v : dobleBorde) std::cout << ' ' << v;
    std::cout << '\n';
    return 0;
}
