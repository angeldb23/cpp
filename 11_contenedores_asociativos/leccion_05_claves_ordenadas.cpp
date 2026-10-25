/* =============================================================================
   MÓDULO 11 - LECCIÓN 05: claves propias en contenedores ordenados: operator<
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
        Todas las lecciones anteriores usaron claves "listas para usar":
        std::string, int. El día que querés PUNTO, NPC o Config como clave,
        el compilador te para en seco: no sabe qué significa "menor" para tu
        tipo.

        Un contenedor ordenado pide un ORDEN TOTAL ESTRICTO sobre las claves.
        La forma más común de dárselo es operator< como miembro:

            bool operator<(const Clave& otra) const { ... }

        "Orden total estricto" significa tres cosas concretas (ahí está la
        mayoría de los bugs de map/set):
          1. Irreflexivo: c < c debe ser false.
          2. Asimétrico: si a < b, entonces NO b < a. (Un float con NaN no
             cumple nada de esto: NaN < NaN es false y NaN > NaN también:
             un map<float, ...> con NaN entre las claves es terreno minado.)
          3. Transitivo: a < b && b < c => a < c. Un comparador que dice
             "a<b y b<c pero a==b==c" rompe el árbol: insert entra por un
             lado y find sale por otro.

        La receta segura y canónica: comparar los campos del más
        discriminante al último, y en el primer campo distinto devolver su
        comparación; si son iguales, pasar al siguiente campo. En C++20 todo
        eso se reduce a comparar pares de campos con <=> o a dejar
        operator<=> como default. Acá lo hacemos a mano UNA vez para ver la
        técnica del desempate, que vas a necesitar igual cuando escribas
        comparadores a medida.
    2. ¿QUÉ PASA POR DEBAJO?
        Las rotaciones del árbol rojo-negro se hacen SEGÚN el comparador. Si
        el comparador no cumple los tres puntos, dos claves "iguales" pueden
        caer en ramas distintas: find recorre un camino distinto al que usó
        insert y no encuentra lo que vos mismo insertaste. No es que
        crashea: es peor, el contenedor queda incoherente y contesta
        cualquier cosa.

        Detalle de diseño: el comparador también puede ser un struct con
        operator() y pasarse como tercer parámetro del template. La forma
        miembro (operator<) y la forma libre (functor de comparación) son
        equivalentes para el contenedor. ¿Cuándo elegir una? Si el tipo tiene
        UN orden natural => operator< en el tipo. Si hay VARIOS órdenes
        posibles (por nivel, por nombre, por precio) => comparadores aparte,
        uno por cada orden (lección 04 ya usó std::greater<int>: es un
        functor de comparación de la biblioteca).
    3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
        - Devolver "diferencia de identidad" en lugar de "antes de": el
          comparador pide "va ANTES", no "es distinto". Si todos los campos
          pesan igual y no hay desempate, dos claves visualmente distintas
          se vuelven indistinguibles: el map las trae como UNA sola y el
          tamaño no cuadra con lo que insertaste.
        - Float como clave con valores "casi iguales": floats y orden/hash
          no se llevan bien; si podés, NO uses float como clave.
        - Escribir el comparador como lambda en cada uso: si la clave va en
          map/set/sort varias veces, la forma canónica es operator< o un
          alias del tipo del comparador.
        - Comparador caro: recorrer strings enteros o llamar funciones en
          cada comparación se multiplica por O(log n) operaciones de árbol;
          mantelo simple y barato.
    4. PARA PROFUNDIZAR:
        https://en.cppreference.com/w/cpp/named_req/Compare
        https://en.cppreference.com/w/cpp/language/default_comparisons
        https://en.cppreference.com/w/cpp/container/map (requisitos de Cmp)
    ============================================================================= */
#include <iostream>
#include <map>
#include <set>
#include <string>

struct NPC {
    std::string nombre;
    int         nivel;

    // Orden total a mano: PRIMERO nivel; desempate: nombre.
    bool operator<(const NPC& otra) const {
        if (nivel != otra.nivel) return nivel < otra.nivel;
        return nombre < otra.nombre;
    }
    // En C++20, si tienes todos los campos al pie, sería:
    //   friend auto operator<=>(const NPC&, const NPC&) = default;
};

int main() {
    // --- 1. Nuestro tipo como clave: map ordena por nivel, desempata nombre ---------
    std::map<NPC, std::string> tienda;
    tienda.emplace(NPC{"golem", 12}, "de piedra");
    tienda.emplace(NPC{"orco", 3},   "de acero");
    tienda.emplace(NPC{"troll", 3},  "de espinas");

    std::cout << "tienda (ordenada por NPC):\n";
    for (const auto& [npc, trono] : tienda)
        std::cout << "  nivel " << npc.nivel << " | " << npc.nombre
                  << " -> " << trono << '\n';

    // --- 2. Dos NPCs "iguales" para el comparador son UNA clave ---------------------
    const auto res = tienda.insert({NPC{"orco", 3}, "de huesos"});
    std::cout << "\ninsert de {orco, 3} ya presente: bool = " << res.second
              << " ; su trono sigue: " << tienda.at(NPC{"orco", 3}) << '\n';

    // Si la intención es "inserto o reescribo", existe insert_or_assign:
    auto [it2, reescrito] = tienda.insert_or_assign(NPC{"orco", 3}, "de huesos");
    std::cout << "insert_or_assign: reescribió = " << reescrito
              << " ; ahora el trono es: " << tienda.at(NPC{"orco", 3}) << '\n';

    // --- 3. "Equal" del comparador y find con otro objeto "equivalente" --------------
    auto it = tienda.find(NPC{"orco", 3});
    std::cout << "find de otro {orco, 3} construido al vuelo: "
              << (it != tienda.end() ? it->second : "NO") << '\n';

    // --- 4. Segundo orden posible: comparador aparte (otro "menor que") -------------
    struct MayorPorNombre {
        bool operator()(const NPC& a, const NPC& b) const {
            return a.nombre > b.nombre;    // orden alfabetico INVERSO
        }
    };
    std::set<NPC, MayorPorNombre> descendente;
    descendente.insert(NPC{"orco", 3});
    descendente.insert(NPC{"golem", 12});
    descendente.insert(NPC{"troll", 3});
    std::cout << "orden alfabetico inverso:";
    for (const NPC& npc : descendente) std::cout << " | " << npc.nombre;
    std::cout << '\n';
    return 0;
}
