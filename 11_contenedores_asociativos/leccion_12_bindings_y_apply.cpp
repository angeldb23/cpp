/* =============================================================================
   MÓDULO 11 - LECCIÓN 12: structured bindings y std::apply
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
        La lección anterior desempaquetaba con std::tie y std::get. C++17
        trae la forma natural: STRUCTURED BINDINGS, la descomposición con
        nombres declarados al vuelo:

            auto [x, y] = par;              // declara x e y
            for (const auto& [clave, valor] : mapa) { ... }

        Tres reglas que importan:
          - El prefijo decide la relación: auto copia los slots, auto& los
            referencia modificables, const auto& los referencia de solo
            lectura.
          - Los "nombres" se atan a POSICIONES: en un par, los dos
            miembros; en un agregado, los miembros públicos EN SU ORDEN
            de declaración; en un array, las posiciones. Son posiciones,
            no etiquetas.
          - Solo hay tres objetivos: arrays C, agregados y par/tuple
            (los tres casos "sin constructor definido a medida").

        std::apply es la hermana "funcional" del tuple: pasa el tuple
        DESARROLLADO comoArgumentos a una función:

            std::tuple<int, int> delta{-5, 3};
            const int d = std::apply([](int dx, int dy) { return dx - dy; },
                                     delta);   // la lambda recibe -5 y 3

        Constd make_tuple, structured bindings y apply te queda un
        kit completo para datos agrupados sin struct a medida.
   2. ¿QUÉ PASA POR DEBAJO?
        Structured bindings son SINTAXIS del lenguaje (no de biblioteca):
        declaran variables locales atadas a los slots del objeto. La
        relación con el objeto ORIGINAL depende del prefijo (copia vs
        referencia); fijate bien cuál usás: auto& modifica el original,
        auto no.

        std::apply: interno: recorre el tuple con un std::index_sequence
        (módulo 09) para pasar cada elemento como argumento en orden. Si
        el TIPO de un elemento no calza con el parámetro de la función,
        el error de compilación sale en la LLAMADA, legible, no en el
        fondo de un template.
   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
        - Structured bindings sobre un tipo con miembros PRIVADOS no
          compila: la descomposición por miembros pide AGREGADO (módulo
          05); para clases encapsuladas van getters o std::tie.
        - auto& sobre un objeto TEMPORAL (el tuple que devuelve una
          función al vuelo): referencia colgante; usá auto o guardá el
          temporal primero.
        - Cambiar el ORDEN de los miembros de un struct: la
          descomposición sigue el ORDEN DECLARADO; si reordenás los
          miembros, tus bindings cambian de significado en silencio.
        - std::apply con el parámetro by-value y el elemento andando
          GREAT: aténdete: la copia extra: si el campo pesa, declará
          el parámetro como const referencia.
    4. PARA PROFUNDIZAR:
        https://en.cppreference.com/w/cpp/language/structured_binding
        https://en.cppreference.com/w/cpp/utility/apply
        https://en.cppreference.com/w/cpp/utility/tuple/index_sequence
   ============================================================================= */
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <utility>

struct Reporte {
    std::string nombre;
    int         golpes;
    float       precision;    // 0.0 - 1.0
};

int main() {
    // --- 1. Structured bindings sobre pair --------------------------------------------
    std::pair<int, std::string> inv{995, "espadas"};
    auto [cantidad, nombre] = inv;
    std::cout << "tengo " << cantidad << ' ' << nombre << '\n';

    // --- 2. Sobre agregado: los NOMBRES siguen el ORDEN de los miembros ----------------
    Reporte rep{"ana", 40, 0.85f};
    const auto& [jugadora, golpes, prec] = rep;
    std::cout << "reporte de " << jugadora << ": " << golpes << " golpes ("
              << prec << " de precision)\n";

    // --- 3. Sobre array C ---------------------------------------------------------------
    int coords[2] = {3, -7};
    auto [cx, cy] = coords;
    std::cout << "coordenadas: (" << cx << ", " << cy << ")\n";

    // --- 4. std::apply: el tuple se PLANTA como lista de argumentos ---------------------
    std::tuple<int, int> delta{-5, 3};
    const int resta =
        std::apply([](int dx, int dy) { return dx - dy; }, delta);
    std::cout << "\napply(-5, 3) => resta: " << resta << '\n';

    // range-for con bindings sobre map: la forma canonica de recorridos:
    std::map<std::string, int> kills{{"orco", 12}, {"troll", 4}};
    std::cout << "kills ordenados:\n";
    for (const auto& [enemigo, veces] : kills) {
        std::cout << "  " << enemigo << " x" << veces << '\n';
    }
    return 0;
}
