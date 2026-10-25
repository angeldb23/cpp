/* =============================================================================
   MÓDULO 11 - LECCIÓN 15: nodos y algoritmos de contenedores: equal_range,
   extract y merge
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
        Tres piezas "de lun-ultimo" que completan el manejo de los
        contenedores de nodos (map/set y sus primos):

        equal_range(k): el PAR de iteradores [primero, uno después del
        último] de la clave k. En map/set delimita a lo sumo UNA
        posición, pero es LA herramienta cuando el contenedor es
        multimap/multiset (L04): delimitar-baline TODAS las entradas
        con esa clave sin recorrer todo el contenedor.

        extract(node): C++17. SACA el elemento completo (clave+valor) de
        un contenedor como NODE HANDLE, sin copiar el valor: la clave
        muta, los datos no. Del nodo se puede:
          - reinsertarlo en otro contenedor (usar cambia_clave no);
          - MIRAR/ MODIFICAR su valor (ohv: node.value()) ANTES de
            insertarlo: el "del TRABAJAR MUCHO".
          - La clave SÍ TOCA (¿en todosasions invariants): una clave
            "no constant"?

        merge(otro): movimientos de NODOS completos de un contenedor
        a otro (mismo tipo de clave) SIN asignación ni copia: los
        nodos sobreviven intactos; las claves que ya estaban se quedan
        en el contenedor de ORIGEN.

        ¿Para qué sirve el trío? Cambiar la CLAVE de un elemento del
        map: extraer el nodo, cambiar la clave fuera del árbol,
        reinsertarlo: sin copiar el valor y sin dejar el elemento al
        vacío durante el cambio.
   2. ¿QUÉ PASA POR DEBAJO?
        map/set = árboles de NODOS (un alloc por elemento: L01, L07). El
          node handle es un objeto RAII que "tomó" el nodo: la memoria
          perteneza a él hasta que decidís insertarlo (a otro
          contenedor!) o descartarlo (destruye nodo). Por eso extract
          es el re-namado: NO pide una copia del valor.

        Y esto conecta de lleno con los PRÓXIMOS temas: los nodos
        querés mover entre contenedores resultan la misma técnica con
        unordered_map y map ( ¡y el node-handlede asignación a CAJA de
        la SAME familia!).

        equal_range en multiset: lower_bound + upper_bound: los dos
        límites; pero en el árbol, un salto SOLICITADO. Qualquier
        performanio no de O(log n): la multiplicidad Sea Wild.
   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
        - extract devuelve node handle VACÍO si la clave no estaba (o
          sea que lo chequear): nh && a: ...
        - Modificar la clave ANTES de reinsertarla y NO: SOLO el valor:
          en el node_handle, key ES la parte const: la clave no se
          puede cambiar a MENOS que "sacar el valor, descartar el
          nodo, reinsertar con la clave nueva": manual (L05/L06).
        - merge lista de claves que YA están: se quedan en el ORIGEN
          (no fusiona valores: Cuidado con los "sumar" empezando así).
        - `extract` e insert: SIM, pero ES: el hit "re-namado"
          dealloc: el OLD key NO puede mutar: nuevo node key.
   4. PARA PROFUNDIZAR:
        https://en.cppreference.com/w/cpp/container/map/extract
        https://en.cppreference.com/w/cpp/container/map/merge
        https://en.cppreference.com/w/cpp/container/map/equal_range
   ============================================================================= */
#include <iostream>
#include <map>
#include <string>

int main() {
    // --- 1. extraer: el nodo completo_sale para mover y cambiar la CLAVE ---------------
    std::map<int, std::string> usuarios{
        {101, "ana"}, {205, "beto"}, {310, "carla"}};

    // cambio de ID (clave) sin copiar el nombre: extract + insert:
    auto nodo = usuarios.extract(205);
    if (nodo.empty()) {
        std::cout << "la clave no estaba\n";
    } else {
        nodo.key() = 900;          // LA clave, fuera del arbol: se puede
        usuarios.insert(std::move(nodo));   // va a nueva posicion O(log n)
    }
    std::cout << "tras extract+insert: ";
    for (const auto& [id, nombre] : usuarios) std::cout << id << ':' << nombre << ' ';
    std::cout << '\n';

    // --- 2. extract consulta: la clave faltante deja el handle VACIO -------------------
    auto vacio = usuarios.extract(999);
    std::cout << "extract de 999 vacio? " << vacio.empty() << '\n';

    // --- 3. merge: los nodos pasan SIN copiarse; los repetidos se quedan ----------------
    std::map<int, std::string> otras_cuentas{
        {400, "dani"}, {101, "ana_vieja"}};
    // (101 ya esta: se conserva en otras_cuentas)
    usuarios.merge(otras_cuentas);
    std::cout << "\ntras merge (usuarios):";
    for (const auto& [id, nombre] : usuarios) std::cout << ' ' << id << ':' << nombre;
    std::cout << "\ntras merge (quedaron otras_cuentas):";
    for (const auto& [id, nombre] : otras_cuentas) std::cout << ' ' << id << ':' << nombre;
    std::cout << '\n';

    // --- 4. equal_range en un map y su uso con arrays (L04) --------------------------------
    // (la version multimap: ver L04; aca el range delimita 0 o 1 elementos)
    for (auto [ini, fin] = usuarios.equal_range(900); ini != fin; ++ini)
        std::cout << "equal_range(900) encontro: " << ini->second << '\n';
    return 0;
}
