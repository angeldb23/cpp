/* =============================================================================
   MÓDULO 11 - LECCIÓN 03: set / unordered_set: el contenedor de "¿lo tengo?"
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       Un map de clave=valor tiene un hermano sin la parte del valor:
       std::set<K> guarda SOLO claves, exactamente una de cada, y responde a
       una sola pregunta en O(log n): "¿está X?". Es el contenedor de los
       "ya los visité", "los activos", "los prohibidos".

       Sirve para tres cosas de siempre:
         - MEMBERSHIP: contains/find, sin valor: en un map tenés que
           fingir un valor (map<K, bool> con todo true: es un set mal escrito).
         - UNICIDAD: insert() solo mete un elemento si no estaba; si
           almacenás con insert, el "contiene duplicados" se resuelve solo.
         - ORDEN: el recorrido va de las claves de menor a mayor: iterar un
           set es recorrer los elementos ordenados (como en el map, el
           árbol los lleva anotados desde el principio).

       std::unordered_set<K> es el mismo contrato sin orden: hash + buckets,
       O(1) promedio. Misma regla que en la lección del map: ¿te sirve saber
       el orden o hace falta rango? set. ¿Solo pregunta "está"? unordered_set.

       La API: insert (devuelve par<iterador, bool>), find/contains, erase,
       size. Sin operator[] ni at: no hay "valor", así que la única forma de
       diálogo es el iterador que devuelven find (o count).

       Continuidad (módulo 10): remove/erase, algorithmos con rango... acá
       "borrar un set entero mientras lo recorres" ES LO MISMO que en el map:
       la forma seguro es tomar el iterador ANTES.
   2. ¿QUÉ PASA POR DEBAJO?
       La estructura número-a-número del set es la del map: un árbol
       balanceado con las claves como nodos (sin los "valores"). Todo lo que
       dijiste de iteradores estables, O(log n) con caso peor contractual y
       localidad floja se repite; simplemente el par<K,V> se convierte en K.

       El unordered_set usa hash + buckets como el unordered_map (L02) y cae
       en la misma regla: tu tipo de clave tiene que proveer operator== y
       std::hash (o un hasher a medida: lección 06).

       Buena noticia para el uso en juegos: para tablinas pequeñas (decenas
       a algunos miles), una BÚSQUEDA LINEAL en un std::vector ordenado
       suele ganar a un set: es lo que llamamos "vector ordenado como set
       pobre" (luego vemos el porqué exacto en la lección 08: localidad de
       caché). set/unordered_set gana cuando baja DE SER MUCHO el n.
   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
       - map<K, bool> o map<K, unit> como set manual: correcto pero raro;
         desde ya cuesta más memoria y deja un valor dummy feo.
       - "remove-erase" de módulo 10 sobre set: erase por CLAVE es O(log n)
         y hace falta (más barato); no hay que correr std::remove acá.
       - Insertar en un set en la FIRMA "set<Monster>": cada inserción COPIA
         el objeto entero a un nodo propio; para objetos pesados guardá
         pointers/ids y lista en el orden del objeto. (Claves por puntero =
         tema específico de la L06: ojo al ciclo.)
       - Chequear si el set ya lo tiene: con contains/find, NUNCA con
         insert + bool (eso ya crea el nodo).
       - Recorrer set esperando orden: sí, está prometido. Recorrer
         unordered_set esperando orden: NO, no hay contrato.
   4. PARA PROFUNDIZAR:
       https://en.cppreference.com/w/cpp/container/set
       https://en.cppreference.com/w/cpp/container/unordered_set
       https://en.cppreference.com/w/cpp/container/set/insert
   ============================================================================= */
#include <iostream>
#include <set>
#include <unordered_set>
#include <vector>

int main() {
    // --- 1. Un informe de encuesta: set como "ya están en el mundo" ---------------
    std::vector<int> lluvia_de_entity_ids{3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};

    std::set<int> ids_vistos;
    for (int id : lluvia_de_entity_ids) ids_vistos.insert(id);
    std::cout << "ids distintos (set ordenado):";
    for (int id : ids_vistos) std::cout << ' ' << id;
    std::cout << '\n';

    // --- 2. Problemas: contains, count y find en bando ----------------------------
    std::set<int> prohibidos{2, 4, 6};
    std::cout << "prohibidos tiene 4? " << prohibidos.contains(4) << " | 7? "
              << prohibidos.contains(7) << " | count(2) = " << prohibidos.count(2) << '\n';

    // find devuelve iterador sobre el elemento; set no tiene at ni []:
    auto it = prohibidos.find(4);
    if (it != prohibidos.end()) std::cout << "encontrado: " << *it << '\n';

    // --- 3. insert te dice si insertó (y NO duplica) -------------------------------
    const auto res = prohibidos.insert(6);
    std::cout << "insert(6) que ya estaba: bool = " << res.second << '\n';

    // --- 4. unordered_set: misma pregunta, mecanismo hash, sin orden ----------------
    std::unordered_set<std::string> klausa{"orco", "troll", "golem"};
    klausa.insert("titán");
    std::cout << "¿hay orco? " << klausa.contains("orco")
              << " ; hay trasgo? " << klausa.contains("trasgo") << '\n';

    // --- 5. Borrado durasi el recorrido (patrón seguro con erase(it++)) ------------
    std::set<int> supervisados{1, 2, 3, 4, 5, 6, 7, 8};
    for (auto i = supervisados.begin(); i != supervisados.end();) {
        if (*i % 2 == 0) i = supervisados.erase(i);   // erase devuelve el siguiente
        else ++i;
    }
    std::cout << "impares restantes:";
    for (int n : supervisados) std::cout << ' ' << n;
    std::cout << '\n';
    return 0;
}
