/* =============================================================================
   MÓDULO 10 - LECCIÓN 03: algoritmos de búsqueda lineal: find y count
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       La familia `find` busca QUÉ dónde (*dónde está un valor o el primero que
       cumple un predicado*) y la familia `count` cuenta CUÁNTOS. Cuatro del
       mismo paquete:
         - `std::find(primero, ultimo, valor)`: primer elemento IGUAL a valor.
           Devuelve iterador a él, o `ultimo` si no está.
         - `std::find_if(primero, ultimo, pred)`: primero que cumple el
           predicado (una función/lambda que devuelve bool).
         - `std::find_if_not(...)`: el primero que NO cumple (evita el
           `!(...)` en el lambda).
         - `std::count` / `std::count_if`: cuántos elementos (igual a valor /
           que cumplen el predicado). Devuelven un número (_difference_type).

       Dos patrones que vas a usar en TODO el módulo:
         1. Chequear existencia: `if (std::find(...) != fin)`. Nunca confundas
            "end" con "no encontró nada que usar": end es el estado NORMAL de
            no está, el contrato que viste en la L01.
         2. Buscar TODAS las apariciones: buscar con find, cuando aparece,
            seguir desde el siguiente (std::next del hallado). Es un bucle con
            find adentro: eso es una búsqueda lineal encadenada.
       `find_if` recibe un PREDICADO: por eso sobre structs/buscar por campo
       no hay `find`: es `find_if` con una lambda (módulo 04 enseñó sintaxis,
       la L12 del 10 se mete a fondo con capturas).
   2. ¿QUÉ PASA POR DEBAJO?
       Todo esto es un bucle for disfrazado: `for (; primero != ultimo;
       ++primero) if (*primero == valor) break;` — WORST_CASE O(n) sí o sí:
       un dato SIN ordenar no permite salto (la búsqueda binaria de la L09
       exige sí o sí datos ordenados). Mientras no te prometas orden y el
       contenedor sea no-ordenado, no hay manera más rápida que el paseo.
       Es exactamente el mismo costo que tu range-for con un if adentro: la
       diferencia es el nombre que LEE el código.
   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
       - Buscar en un vector ordenado con find O(n) cuando te toca usar
         binary_search/lower_bound O(log n) (L09): el orden no se aprovecha
         solo; tenés que cambiar de algoritmo.
       - Desreferenciar el resultado sin chequear contra end(): crash si no
         estaba (L01). ESTE chequeo es la parte 1 de nuestra LECCIÓN.
       - Preguntar "¿está?" con count e ignorar el número: find es el camino
         correcto para existencia y se corta apenas aparece; count recorre
         SIEMPRE todo el rango. count es para cuando el número sirve.
   4. PARA PROFUNDIZAR:
       https://en.cppreference.com/w/cpp/algorithm/find
       https://en.cppreference.com/w/cpp/algorithm/count
   ============================================================================= */
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

struct Jugador {
    std::string nombre;
    bool vivo;
};

// --- 1. CoCOCHEDE: encontrar todas las apariciones ---------------------------
// Patrón de "seguir desde el siguiente". Devuelve posición (size_t) de cada hit.
std::vector<std::size_t> posicionesDe(const std::string& texto, char objetivo) {
    std::vector<std::size_t> resultado;
    auto it = texto.begin();
    while (it != texto.end()) {
        it = std::find(it, texto.end(), objetivo);
        if (it != texto.end()) {
            resultado.push_back(static_cast<std::size_t>(
                std::distance(texto.begin(), it)));          // L02: como índice
            it = std::next(it);                              // seguir DESPUÉS del hit
        }
    }
    return resultado;
}

int main() {
    // --- 2. Equipo: buscar por campo con find_if -----------------------------
    std::vector<Jugador> equipo{
        {"Quesito", true}, {"Mordel", false}, {"Turbo", true}, {"Mordel", true},
    };

    // existe un jugador vivo llamado Turbo?
    const auto esta = std::find_if(equipo.begin(), equipo.end(),
        [](const Jugador& j) { return j.vivo && j.nombre == "Turbo"; });
    std::cout << "Turbo vivo: " << (esta != equipo.end() ? "si" : "no") << '\n';

    // el primer muerto se puede REVIVIR a traves del iterador:
    const auto muerto = std::find_if(equipo.begin(), equipo.end(),
        [](const Jugador& j) { return !j.vivo; });
    if (muerto != equipo.end()) {
        muerto->vivo = true;                            // escribo por el iterador
        std::cout << "revivi a " << muerto->nombre << '\n';
    }

    // --- 3. count: cuantas veces esta X --------------------------------------
    const int cuantos_vivos = static_cast<int>(
        std::count_if(equipo.begin(), equipo.end(),
            [](const Jugador& j) { return j.vivo; }));
    std::cout << "vivos: " << cuantos_vivos << " de " << equipo.size() << '\n';

    // --- 4. find_if_not: el primero que NO cumple (sin !(...) en el lambda) --
    const auto aRellenar = std::find_if_not(equipo.begin(), equipo.end(),
        [](const Jugador& j) { return j.nombre != "Pendiente"; });
    std::cout << "hay slot Pendiente? "
              << (aRellenar != equipo.end() ? "si" : "no") << '\n';

    // --- 5. El patrón encadenado sobre un texto ------------------------------
    const auto hits = posicionesDe("abracadabra", 'a');
    std::cout << "posiciones de 'a': ";
    for (std::size_t pos : hits) std::cout << pos << ' ';
    std::cout << "(total: " << std::count("abracadabra", "abracadabra" + 11, 'a')
              << ")\n";

    return 0;
}
