/* =============================================================================
   MÓDULO 11 - LECCIÓN 04: multiset / multimap y el criterio de orden
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
        set y map imponen "una clave, un elemento". Sus hermanos multiset<K>
        y multimap<K, V> hacen UNA concesión: aceptan claves REPETIDAS y, por
        lo demás, todo queda igual (árbol balanceado, O(log n), salida
        ordenada). Es la zona natural para:
          - multimap: índices con varias entradas por clave: "todas las
            entidades con este TAG", "todos los golpes de este enemigo",
            "todas las muertes en esa coordenada".
          - multiset: conteos con repetición y rangos ordenados: "los 5
            golpes más fuertes" salen del frente del contenedor sin pagar
            un sort: el orden ya viene del árbol.

        Pocas adiciones respecto a map/set:
          - count(k): ahora cuenta DE VERDAD (puede ser > 1).
          - equal_range(k): devuelve el PAR de iteradores [primero, después
            del último] de las entradas con clave k: lo recorrés y listo.
            (También existe en set/map: ahí como mucho delimita una sola
            posición; se usa con mayor intensidad en multimap.)
          - erase(clave) en multiset/multimap: borra TODAS las entradas de
            esa clave. erase(iterador) borra SOLO la de ese iterador.

        ¿Y el "criterio de orden" del título? map/set/multi_* están ordenados
        POR EL COMPARADOR que le pasás como tercer argumento: std::map<K, V,
        Comparador>. El default es std::less<K> (ascendente); cambiándolo
        elegís descendente, orden por otro campo, o una función comparadora
        propia. Lo único no negociable es que tu comparador cumpla el ORDEN
        TOTAL ESTRICTO (se detalla en la L05).
    2. ¿QUÉ PASA POR DEBAJO?
        Los mult_* guardan las entradas repetidas en el MISMO árbol; basta
        iterar: con equal_range delimitás exactamente los nodos de una clave
        sin recorrer el contenedor entero.

        COMO SIEMPRE: iteradores estables (los nodos no se reubican) y el
        costo de "un alloc por nodo" (la comparación con vectores ordenados
        va con detalle en L07-L08).

        Alternativa de diseño: si SIEMPRE tenés una lista por clave, conviene
        a veces map<K, vector<V>>: un nodo por clave y un vector contiguo
        adentro (menos alloc, más localidad) en lugar de N nodos de multimap.
    3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
        - erase(clave) borrando TODO cuando querías borrar UNO: en
          multiset/multimap erase(clave) = "borrar todas"; erase(iterador) =
          "borrar una". Leer la firma con cuidado.
        - Para recorrer las entradas de una clave: range-for sobre el par
          que devuelve equal_range, no count estático + find repetido.
        - map<K, vector<V>> vs multimap<K, V>: repetir la clave mil veces
          en multimap = mil nodos; en map<K, vector<V>> = un nodo y un
          vector que crece contiguo.
        - Los primeros N con multiset: ver en el main cómo topar el
          recorrido; con el comparador invertido "primero" significa
          "máximo": revisá qué leés antes de topar.
        - Cambiar el comparador sin pensarlo: con std::greater el orden se
          invierte gratis, pero "primero" pasa a significar "mayor".
          Leé el comparador antes de confiar en el frente del contenedor.
    4. PARA PROFUNDIZAR:
        https://en.cppreference.com/w/cpp/container/multimap
        https://en.cppreference.com/w/cpp/container/multiset
        https://en.cppreference.com/w/cpp/container/map/equal_range
    ============================================================================= */
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

int main() {
    // --- 1. multimap: todo lo que comparte una etiqueta ----------------------------
    std::multimap<std::string, int> eventos;
    eventos.insert({"combate", 14});    // (tag, tiempo_de_juego)
    eventos.insert({"soltar", 13});
    eventos.insert({"combate", 30});
    eventos.insert({"combate", 45});

    std::cout << "count(combate) = " << eventos.count("combate") << '\n';

    // equal_range: [primero, uno-despues-del-ultimo] de esa clave:
    {
        auto [ini, fin] = eventos.equal_range("combate");
        std::cout << "eventos de combate:";
        while (ini != fin) {
            std::cout << ' ' << ini->second;    // el par es <const string, int>
            ++ini;
        }
        std::cout << '\n';
    }

    // erase(iterador) borra UNA; erase(clave) borra TODAS:
    eventos.erase(eventos.find("combate"));    // solo esa
    std::cout << "tras erase(it): count(combate) = " << eventos.count("combate") << '\n';
    eventos.erase("soltar");
    std::cout << "tras erase(\"soltar\"): count = " << eventos.count("soltar") << '\n';

    // --- 2. multiset con comparador invertido: máximos al frente -------------------
    std::multiset<int, std::greater<int>> danios;   // orden: MAYOR primero
    for (int d : {12, 8, 6, 22, 15, 3, 22}) danios.insert(d);

    std::cout << "top 3 de golpes:";
    int n = 0;
    for (int d : danios) {
        std::cout << ' ' << d;
        if (++n == 3) break;
    }
    std::cout << "  (el 22 aparece dos veces: se permiten repetidos)\n";

    // --- 3. Recorrido inverso sin cambiar la copia: rbegin/rend ---------------------
    std::set<int> niveles{10, 20, 30};
    std::cout << "directo:";
    for (int v : niveles) std::cout << ' ' << v;
    std::cout << " | inverso:";
    for (auto rit = niveles.rbegin(); rit != niveles.rend(); ++rit)
        std::cout << ' ' << *rit;
    std::cout << '\n';

    // --- 4. Alternativa: map<K, vector<V>> en vez de multimap -----------------------
    std::map<std::string, std::vector<int>> porTag;
    porTag["combate"].push_back(30);
    porTag["combate"].push_back(45);
    porTag["soltar"].push_back(13);
    std::cout << "map<K, vector<V>> combate:";
    for (int t : porTag["combate"]) std::cout << ' ' << t;
    std::cout << "  (un nodo por clave, valores contiguos)\n";
    return 0;
}
