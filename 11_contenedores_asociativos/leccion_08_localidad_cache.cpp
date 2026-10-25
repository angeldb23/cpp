/* =============================================================================
   MÓDULO 11 - LECCIÓN 08: localidad de caché: el vector frente a list/deque/map
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
        En L07 la tabla de complejidades sugería list/deque/map para casos
        concretos. Esta lección agrega la segunda mitad del criterio: la
        LOCALIDAD DE CACHÉ. La RAM es lenta frente a la CPU; el hardware
        baja bloques (caché lines de ~64 bytes) desde RAM a la caché L1/L2
        cuando accedés a un dato. Consecuencia directa:

          - std::vector: los elementos están contiguos. Recorrer 10.000
            elementos es hacerles funcionar a las cache lines SEQUENCIAS:
            cada línea de 64 bytes trae varios elementos de un golpe y vos
            los usás todos.
          - std::list o map/unordered_map: cada nodo es una alocación
            independiente, dispersa por el heap. Recorrer 10.000 elementos
            son 10.000 saltos a direcciones "al azar": cada salto es un
            cache miss probable (y al vecino de un nodo, el "vecino" lógico,
            NO lo bajó el hardware: no está pegado en memoria).

        El resumen para tu día a día en un motor:
          - "recorro todo, con frecuencia" => el contenedor no necesita
            nodos: vector (o vector ordenado) gana el recorrido casi
            siempre, por más que la tabla diga O(n).
          - "edición frecuente por iterador estable + recorrido raro" =>
            ahí el contenedor de nodos paga lo cobra.
          - unordered_map: una línea de caché por acceso es probable de
            todos modos (la tabla dispersa los nodos), PERO el acceso por
            hash es UNA trayectoria, no un recorrido.

        El caso "set/map chico que se consulta MUCHO": una práctica común
        en motores es usar un VECTOR ORDENADO como set/map: buscar con
        búsqueda binaria (std::lower_bound, O(log n), módulo 10) y ganar
        el trayecto por caché del recorrido/orden. Es el patrón que se
        llama "sorted vector".
    2. ¿QUÉ PASA POR DEBAJO?
        Un acceso a RAM tarda ~100x un acceso a caché L1 (~100 ns vs ~1
        ns). Una caché line de 64 bytes son 8 doubles: al tocar el
        primero, los otros 7 ya están en caché: recorrido de vector =
        casi todos hits. En una lista enlazada lo que baja es EL nodo
        (y un pedazo del heap ajeno): casi todos misses.

        No es que "los árboles son malos": son lentos de recorrido PERO
        estables ante la edición (iteradores que no se invalidan, insert
        O(log n) sin mover a NADIE). Las dos mitades (L07-L08) son las
        partes de la misma decision: ¿cuánto editás vs cuánto recorres?

        Detalle útil para el futuro: std::map y std::set pueden dar
        trayectos de caché IGUALmente moderados para datos GRANDES
        (estructuras que ocupan varias cache lines: un nodo armado con
        menos alineación "ya vale una línea"); el problema aguda es el
        de elementos PEQUEÑOS contiguos (ints, floats, vertxs).
    3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
        - Elegir mientras se ve SOLO la tabla de O(...): la localidad es
          parte del costo real; medí SIEMPRE con el compilador en -O2
          cuando la pregunta es performance.
        - "Cross-arquetipo" entre pequeño n y grande n: para decenas de
          elementos, un vector con búsqueda lineal bate a un map: n
          escala PERO tu complejidad no la puteás por la pantalla.
        - Estructuras GRANDES por elemento: si cada elemento ocupa
          su (varias) cache lines, la localidad pesa menos: el nodo o el
          elemento contiguo casi tienen el mismo costo (casing).
        - Vector ordenado como set: OJO: no deduplicación graciosa: si
          insertás sin chequear, hay duplicados: usá lower_bound ANTES
          de insertar y comprobará que no estaba.
    4. PARA PROFUNDIZAR:
        https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rperf
        https://en.cppreference.com/w/cpp/algorithm/lower_bound
        https://en.wikipedia.org/wiki/Cache-oblivious_algorithm
   ============================================================================= */
#include <algorithm>
#include <iostream>
#include <list>
#include <utility>
#include <vector>

int main() {
    // --- 1. El patrón "sorted vector": un vector ordenado se comporta como set ------
    std::vector<int> idsSet{5, 12, 33, 40, 41};
    const bool esta = std::binary_search(idsSet.begin(), idsSet.end(), 33);
    std::cout << "vector-ordenado como set: esta el 33? " << esta << '\n';

    // Insertar manteniendo el orden: O(log n) buscar + O(n) mover (el insert de un
    // set es O(log n) y NO mueve nada: la contraparte se paga en mover datos):
    auto spot = std::lower_bound(idsSet.begin(), idsSet.end(), 20);
    idsSet.insert(spot, 20);
    std::cout << "tras insertar 20:";
    for (int id : idsSet) std::cout << ' ' << id;
    std::cout << '\n';

    // --- 2. El insert "como set" con chequeo de duplicado ----------------------------
    auto spot2 = std::lower_bound(idsSet.begin(), idsSet.end(), 33);
    if (spot2 == idsSet.end() || *spot2 != 33) {
        idsSet.insert(spot2, 33);        // solo si no estaba
    }
    std::cout << "tras intentar insertar 33 de nuevo:";
    for (int id : idsSet) std::cout << ' ' << id;
    std::cout << '\n';

    // --- 3. list en el heap: cada nodo es una alocacion suelta ------------------------
    std::list<int> l{5, 12, 33, 40, 41};
    (void)l;    // demostrativo: los nodos NO son contiguos: no hay lógica de caché

    // --- 4. "sorted vector" con registros (campo clave + valor) -----------------------
    std::vector<std::pair<int, int>> registros{{3, 300}, {1, 100}, {2, 200}};
    std::sort(registros.begin(), registros.end());
    const auto it = std::lower_bound(
        registros.begin(), registros.end(), 2,
        [](const std::pair<int, int>& r, int clave) { return r.first < clave; });
    std::cout << "registro con id 2: " << it->second << '\n';
    return 0;
}
