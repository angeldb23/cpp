/* =============================================================================
   MÓDULO 11 - LECCIÓN 06: claves propias en unordered: operator== y std::hash
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
        Con contenedores ordenados la única tarea era operator< (L05). Para
        unordered_map/unordered_set tu tipo necesita DOS piezas:

          1. operator==: el contenedor la usa para decidir si dos claves son
             "la misma" DENTRO del bucket. Dos claves caen en el mismo bucket
             si su hash coincide; adentro se las separa comparando con ==.
          2. UNA función de hash: transforma una clave en un std::size_t. La
             biblioteca ya trae std::hash para enteros, strings, punteros y
             enum; para tipos compuestos la forma estándar es ESPECIALIZAR
             std::hash para tu tipo:

                struct Punto { int x; int y; };

                template <>
                struct std::hash<Punto> {
                    std::size_t operator()(const Punto& p) const noexcept {
                        return std::hash<int>{}(p.x)
                             ^ (std::hash<int>{}(p.y) << 1);
                    }
                };

             Combinar hashes campo a campo (XOR + un desplazamiento) es la
             receta clásica; es la base de hash_combine de Boost. La idea:
             si combinás todo con puro XOR, dos claves simétricas give el
             mismo hash; el shift rompe esa simetría.

        Cómo se ve cada pieza cuando falta:
          - Sin operator==: error de compilación al instante (el
            contenedor "no puede comparar tu clave").
          - Sin la especialización de std::hash: igual, error de
            compilación ("no hay hash<Punto>").
          - Con un hash malo (por ejemplo, si devolviera 0 para todo): el
            código COMPILA y FUNCIONA, pero todas las claves quedan en un
            solo bucket y la tabla degrada a una búsqueda lineal: es el
            bug de performance más silencioso de este módulo.
    2. ¿QUÉ PASA POR DEBAJO?
        hash(k) % n_buckets elige el bucket. Un hash mal distribuido
        (por ejemplo, demasiado lineal con entradas vecinas) acumula todo
        en pocos buckets y el promedio O(1) se desploma hacia O(n). El
        shift + XOR del ejemplo mejora la distribución lo suficiente para
        las tablas del día a día.

        LA INVARIANTE que nunca se puede romper: si a == b, entonces
        hash(a) == hash(b). Si no, dos claves iguales terminan en buckets
        distintos y el contenedor jamás las encuentra. Al revés hay colisión
        permitida: hash igual NO significa claves iguales: para eso está
        el == dentro del bucket.

        Regla de mantenimiento: el == y la especialización de hash deben
        mirar LOS MISMOS campos. Si tocás un campo, actualizá ambas piezas
        juntas: quien solo cambia una rompe la invariante en silencio.
    3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
        - == que compara un campo y hash que usa otro conjunto distinto.
        - Hash "de mentira" (devolver 0, o los bytes de un contador) para
          "una prueba rápida": tu refactor de producción termina en una
          lista de colisiones y el O(1) se vuelve O(n) en silencio.
        - Cambiar un campo de la clave DESPUÉS de insertar: la tabla ya la
          archivó adentro de su bucket según el hash viejo: ya no vas a
          poder encontrarla. (Idéntico en map: por eso las claves son const
          en los elementos y el operator[] no puede devolverte la clave para
          arruinarla por accidente: por eso son const.) Si tenés que cambiar
          la clave: insert de la nueva + erase de la vieja.
        - Hash de floats con valores especiales (NaN): evitalo; el hash de
          NaN no se lleva bien con la invariante de igualdad.
    4. PARA PROFUNDIZAR:
        https://en.cppreference.com/w/cpp/utility/hash
        https://en.cppreference.com/w/cpp/language/default_comparisons
        (operator== default como parte de C++20)
        https://en.cppreference.com/w/cpp/container/unordered_map/erase
    ============================================================================= */
#include <cstddef>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>

struct Punto {
    int x;
    int y;

    bool operator==(const Punto&) const = default;
};

// La especialización de std::hash para Punto (combinación clásica):
template <>
struct std::hash<Punto> {
    std::size_t operator()(const Punto& p) const noexcept {
        return std::hash<int>{}(p.x) ^ (std::hash<int>{}(p.y) << 1);
    }
};

int main() {
    // --- 1. El tipo compuesto como clave --------------------------------------------
    std::unordered_map<Punto, std::string> niveles;
    niveles.emplace(Punto{2, 3}, "balacera en el torso");
    niveles.emplace(Punto{7, 0}, "campo abierto");

    std::cout << "niveles en {2,3}: " << niveles.at(Punto{2, 3}) << '\n';
    std::cout << "contains {7,0}? " << niveles.contains(Punto{7, 0}) << '\n';

    // --- 2. La invariante: claves iguales => hash igual ------------------------------
    Punto a{5, 5}, b{5, 5};
    std::size_t ha = std::hash<Punto>{}(a);
    std::size_t hb = std::hash<Punto>{}(b);
    std::cout << "\nhash({5,5}) == hash(copia {5,5})? " << (ha == hb) << '\n';

    // Hash igual NO implica claves iguales (colisión): para eso está el ==:
    std::cout << "hash({2,3}) == hash({7,0})? "
              << (std::hash<Punto>{}(Punto{2, 3}) == std::hash<Punto>{}(Punto{7, 0}))
              << " ; ¿y los puntos son ==? "
              << (Punto{2, 3} == Punto{7, 0} ? 1 : 0) << '\n';

    // --- 3. unordered_set con la misma clave ------------------------------------------
    std::unordered_set<Punto> visitados;
    visitados.insert(Punto{1, 1});
    visitados.insert(Punto{1, 1});    // duplicado: no entra
    std::cout << "\nvisitados.size() tras dos inserts de {1,1}: " << visitados.size()
              << '\n';

    // --- 4. La clave es const: hay que BORRAR + REINSERTAR para "cambiarla" -----------
    // (los elementos del unordered_set son const Punto)
    std::cout << "recorrido de visitados:";
    for (const Punto& p : visitados) std::cout << " (" << p.x << ',' << p.y << ')';
    std::cout << '\n';
    return 0;
}
