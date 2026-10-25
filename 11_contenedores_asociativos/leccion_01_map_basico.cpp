/* =============================================================================
   MÓDULO 11 - LECCIÓN 01: std::map, operator[] vs at y el recorrido ordenado
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       Hasta acá los contenedores que usaste son SECUENCIALES: tenés que recordar
       "está en la posición 3". Un contenedor ASOCIATIVO cambia el modelo: vos
       le pedís "el elemento cuya CLAVE es X" y el contenedor solito te lo
       encuentra, sin importar dónde esté guardado. La clave es la hoja de ruta,
       no la posición.

       std::map<K, V> guarda pares (K, V) ORDENADOS por la clave (ascendente,
       con std::less<K> por defecto). Lo que compra ese orden es búsqueda
       binaria: find/insert/borrado son O(log n) SIEMPRE, con caso peor
       contractual (exactamente ~log2(n) comparaciones, no "en promedio").

       Las tres formas de entrar:
         - operator[]: si la clave no existe, la CREA (con valor por defecto)
           y te devuelve una referencia mutable. Perfecto para contadores
           (m[key]++), peligroso para consultas: BUSCAR no puede ser const,
           porque puede escribir.
         - at(key): igual operator[] pero LANZA std::out_of_range si la clave
           no existe. Sirve cuando "la clave tiene que estar" y ausencia es un
           error de quien llama.
         - find(key): devuelve iterador a la entrada o end() si no está. Es la
           única forma de consulta que no introduce datos y sí es const.

       El recorrido está ordenado por clave: un range-for sobre un map te lo
       entrega en orden creciente de la clave. El tipo de cada elemento es
       std::pair<const K, V>: la clave es const porque cambiarla rompería el
       orden tal cual. El valor sí es mutable.

       Nota histórica: entender este contenedor es entender por qué los juegos mueven
       las tablas de entidades de "array de índices" a "tabla por ID": a cambio
       de renunciar a acceso aleatorio O(1), ganás búsquedas nombradas.

       Continuidad (módulo 10): los iteradores, find y los algoritmos que ya
       conocés funcionan igual acá; la diferencia es que el algoritmo corre sobre
       pares<const K, V> y las búsquedas especializadas del propio map son más
       rápidas que std::find (que es lineal).
   2. ¿QUÉ PASA POR DEBAJO?
       Un map NO es una tabla hash: es (casi siempre) un árbol binario
       BALANCEADO (una variante de red-black tree). Cada nodo guarda un par,
       punteros a izquierdo/derecho y un byte de color; el balance garantiza que
       la altura sea O(log n) hasta cuando insertas ordenado.

       Consecuencias por debajo:
         - ITERADORES: los iteradores de map son estables entre
           inserciones/borrados de OTRAS entradas (el nodo no se mueve; solo
           actualizan punteros). Ninguna inserción invalida nada salvo el
           iterador del borrado.
         - MEMORIA: al alocar un nodo se hace UN alloc independiente; no hay
           localidad entre nodos consecutivos en la memoria real. La localidad
           y su costo se ven a fondo en L07 y L08.
         - ORDEN: que tu tipo de clave cumpla un ORDEN TOTAL ESTRICTO
           (comparador = 'antes' siempre anti-simétricas) es requisito, no
           sugerencia. Cuándo escribís ese comparador lo ves en L05.
   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
       - Chequear si existe con m[k] != ... : ESO INSERTA k si no estaba. Para
         preguntar, usá m.count(k), m.find(k) o (C++20) m.contains(k).
       - Esperar que m.at("x") devuelva algo útil para una clave que no está:
         lanza; si la ausencia es NORMAL en tu lógica, find + un default.
       - operator[] const no compila, y no es un bug de tu código: la consulta
         que puede insertar no puede ser const por diseño. Alternativa: at o find.
       - Recorrer el map y olvidar que el elemento es pair<const K, V>: podés
         modificar p.second, nunca p.first.
       - operator[] para type con constructor POR DEFECTO caro: insert{...}
         pueden ahorrar una construcción vacía. La diferencia insert vs emplace
         la miramos en la próxima lección.
   4. PARA PROFUNDIZAR:
       https://en.cppreference.com/w/cpp/container/map
       https://en.cppreference.com/w/cpp/container/map/operator_at
       https://en.cppreference.com/w/cpp/container/map/contains
   ============================================================================= */
#include <iostream>
#include <map>
#include <string>

namespace limits {
template <typename K, typename V>
void imprime(const std::map<K, V>& tabla, const std::string& titulo) {
    std::cout << titulo << "\n";
    for (const auto& [clave, valor] : tabla) {
        std::cout << "  " << clave << " -> " << valor << '\n';
    }
}
} // namespace limits

int main() {
    // --- 1. operator[]: la consulta que ESCRIBE ---------------------------------
    std::map<std::string, int> oro;
    oro["lago"] = 15;                    // inserta y asigna
    std::cout << "lago tenia: " << oro["lago"] << '\n';

    // Efecto del operator[] sobre clave inexistente: la crea (con 0) e indexa:
    std::cout << "cantina NUNCA asignada: " << oro["cantina"] << '\n';
    std::cout << "ahora el mapa SI contiene cantina? "
              << oro.contains("cantina") << '\n';    // 1: la consulta inserto

    // Contador con operator[]: el caso de uso canonico:
    std::map<std::string, int> conteo;
    for (const char* muerte : {"orco", "orco", "mimico", "orco"}) {
        ++conteo[muerte];    // 0 -> 1 -> 2 -> 3
    }
    limits::imprime(conteo, "conteo (operator[] como contador):");

    // --- 2. at(): la consulta validar; y las formas de PREGUNTAR ------------------
    std::cout << "\nat(\"lago\") = " << oro.at("lago") << '\n';
    try {
        oro.at("cueva");
    } catch (const std::out_of_range& e) {
        std::cout << "at con clave inexistente lanza: " << e.what() << '\n';
    }
    // contains (C++20) es la pregunta mas barata si no necesitas el valor:
    std::cout << "contains cueva? " << oro.contains("cueva")
              << " | count orco? " << conteo.count("orco") << '\n';

    // find: cuando queres mirar el valor SIN correr riesgo de insertar
    // (o cuando #include at lanza y no es la ausencia esperada):
    auto it = conteo.find("mimico");
    if (it != conteo.end()) {
        std::cout << "encontre a " << it->first << " con " << it->second << '\n';
    }

    // --- 3. Recorridos ordenados por clave, no por insercion ----------------------
    std::map<std::string, std::string> inventario{
        {"espada", "de acero"},
        {"libro",  "de hechizos"},
        {"pocion", "menor"},
    };
    limits::imprime(inventario, "orden por salida: SECUENCIA en la declaracion? NO: por CLAVE:");

    // Mapas con rango de iteradores: corta por la mitad de claves:
    std::map<int, std::string> banco{{100, "plata"}, {25, "cobre"}, {50, "bronce"}, {10, "tin"}};
    for (auto it = banco.lower_bound(25); it != banco.end(); ++it) {
        std::cout << "  moneda >= 25: " << it->second << " (" << it->first << ")\n";
    }    // ejecuta para 25, 50, 100 exactamente.

    // El/los elementos son pair<const K, V>: clave inmutable, valor mutable:
    banco[50] = "bronce pulido";    // cambiar el valor esta bien
    // banco.begin()->first = 500;  // NO compila: la clave es const

    // --- 4. Iteradores que NO se invalidan al insertar -----------------------------
    auto tn = banco.find(100);
    banco[1] = "casi sin valor";    // insercion: tn sigue apuntando a 100
    std::cout << "iterador guardado antes de insertar: " << tn->second << '\n';
    return 0;
}
