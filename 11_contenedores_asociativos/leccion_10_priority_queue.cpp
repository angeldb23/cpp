/* =============================================================================
   MÓDULO 11 - LECCIÓN 10: priority_queue y comparadores
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
        std::stack y std::queue (L09) respetan el orden de llegada.
        std::priority_queue<T> impone otro modelo: sale SIEMPRE el
        elemento "mayor" según un COMPARADOR (con el default std::less<T>,
        sale el valor máximo, "al revés" de lo que el nombre less insinúa).
        Como en set/multiset, le podés pasar un comparador propio...
        PERO con una trampa de orden de parámetros: en el adaptador el
        tipo va PRIMERO, después el contenedor de fondo y al final el
        comparador: std::priority_queue<T, Contenedor, Comparador>.

        ¿Para qué sirve? Para el caso "encargate siempre del más
        importante de muchos" sin ordenar todos:
          - prioridad de enemigos, requests con pesos, turnos.
          - top-K en streaming: metés, sacás, quedan los mejores.
          - algoritmos tipo Dijkstra/A*: la frontera de nodos pendientes
            vive en una priority_queue.

        El modelo: push O(log n), pop O(log n), top O(1). Es lo MÁS
        barato que "mantener un set ordenado" si solo te interesa el
        frente.

        Comparador propio, forma canónica:
            struct MenorDinero {
                bool operator()(const Pedido& a, const Pedido& b) const {
                    return a.monto < b.monto;   // true => b sale antes que a
                }
            };
            std::priority_queue<Pedido, std::vector<Pedido>, MenorDinero> q;
        La semántica es la misma que en sort (módulo 10): el comparador
        dice lo que significa "va antes". Como el montículo saca el
        "mayor", con un comparador "menor monto va antes" sale el pedido
        de mayor monto primero; si querés los más baratos primero,
        invertís el comparador (std::greater o tu propio functor).
    2. ¿QUÉ PASA POR DEBAJO?
        Un montículo binario (binary heap): un árbol completo guardado
        IN SITU sobre un vector (la posición i tiene hijos 2i+1 y 2i+2).
        push hace "burbujeo hacia arriba"; pop mueve la raíz y burbujea
        hacia abajo. Ventaja fuerte respecto de set: la memoria ES UN
        VECTOR contiguo: pura localidad de caché (L08) y ninguna
        alocación por nodo. Por eso su O(log n) suele medir más rápido
        que el O(log n) del set.

        Contraste con el módulo 10: std::nth_element es la consulta
        "los k mayores, UNA vez, en O(n)". La priority_queue es para la
        MISMA consulta repetida mientras la muestra cambia (top-K en
        streaming, cada frame): reordenar todo con sort en cada frame
        sería lp co sparse; el montículo paga O(log n) por evento.
    3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
        - priority_queue<int> sale de máximos (¡less => máximo!): si te
          salió al revés de lo que esperabas, no es un bug: es la
          convención; usá greater para invertir.
        - No tiene begin()/end() (como L09): la única ventana es top();
          si necesitas recorrer, no era un adaptador lo que querías.
        - Actualizar la prioridad de un elemento YA dentro: no existe
          en la API estándar. El truco usual: "lazy deletion" (enviás
          otra entrada con la prioridad nueva; la vieja se saca cuando
          entra por top y la detectás desactualizada).
        - Comparador con estado mutable: el montículo asume que la
          relación entre dos elementos nunca cambia mientras viven ahí;
          un comparador que "opina distinto según el momento" desordena
          el heap en silencio. Mantenelo puro.
    4. PARA PROFUNDIZAR:
        https://en.cppreference.com/w/cpp/container/priority_queue
        https://en.cppreference.com/w/cpp/named_req/Compare
        https://en.wikipedia.org/wiki/Binary_heap
   ============================================================================= */
#include <functional>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

struct Pedido {
    int         monto;
    std::string cliente;
};

struct MenorDinero {
    bool operator()(const Pedido& a, const Pedido& b) const {
        return a.monto < b.monto;    // b "mayor" sale antes que a
    }
};

int main() {
    // --- 1. priority_queue<int>: sale el MAYOR primero (comparador por defecto) ------
    std::priority_queue<int> q;
    q.push(3);
    q.push(25);
    q.push(10);

    std::cout << "sale en este orden:";
    while (!q.empty()) {
        std::cout << ' ' << q.top();
        q.pop();
    }
    std::cout << "  (de mayor a menor)\n";

    // --- 2. Tipo propio con comparador: sale el pedido de MAYOR monto ----------------
    std::priority_queue<Pedido, std::vector<Pedido>, MenorDinero> urgente;
    urgente.push({10, "ana"});
    urgente.push({85, "sofia"});
    urgente.push({40, "luis"});

    std::cout << "\npedidos por monto:\n";
    while (!urgente.empty()) {
        std::cout << "  " << urgente.top().monto << " (" << urgente.top().cliente
                  << ")\n";
        urgente.pop();
    }

    // --- 3. Con std::greater: los MENORES salen primero ------------------------------
    std::priority_queue<int, std::vector<int>, std::greater<int>> baratos;
    baratos.push(30);
    baratos.push(5);
    baratos.push(20);
    std::cout << "\nel menor primero:";
    while (!baratos.empty()) {
        std::cout << ' ' << baratos.top();
        baratos.pop();
    }
    std::cout << "  (el 5 sale primero)\n";
    return 0;
}
