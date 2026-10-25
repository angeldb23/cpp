/* =============================================================================
   MÓDULO 11 - LECCIÓN 13: std::optional: "hay valor o no hay"
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
        ¿Cómo devolvés "a veces no hay resultado"?
          - un número centinela (-1, 0, nullptr): mentira silenciosa: el
            centinela SE CONFUNDE con un resultado válido.
          - un par de valor + bool: el bool vive aparte y se olvida.
          - excepciones: para lógica de negocio NORMAL (el jugador no
            tiene equipo) es caro y abortivo.

        std::optional<T> es "un T que puede estar vacío". Es un valor por
        valor (no heap: el T vive EN el optional, con un byte aparte que
        dice si está):

            std::optional<int> buscarLlave(int id);
            if (auto llave = buscarLlave(7)) { usa la llave: es un }

        API núcleo:
          - tiene Buenos constructs: nullopt (vacio explícito), T, y
            conversiones; emplace() re-llena sin borrarte el old.
          - has_value() / operator bool: "hay o no hay".
          - value(): el T, pero LANZA bad_optional_access si no hay.
          - value_or(X): el T... o X si está vacío: "dámelo o usa esto".
          - *opt / opt->: desreferencia (sin chequeo: es UB si vacío);
            value() es la versión que chequera.

        Truco C++20: optional-like={has_value()} + bool. Cuando NO va:
          para referencias: std::optional<T&> no existe (al menos en
          C++23: optional<ref> has no value no puede ser propia: usá
          puntero: uno referencia punterizé está bien).
   2. ¿QUÉ PASA POR DEBAJO?
        sizeof(optional<T>) no es "sizeof(T) + algo chico": es
        sizeof(T) más un estado de "hay/no hay" (bytes y padding). NO
        aloca memoria: el T vive DENTRO del optional. La copia depende
        del T (copiable si T es copiable); reset() vacía (llama al
        destructor del T si estaba lleno) y emplace() llena de nuevo.
        Es RAII chico: sin new, sin fugas, sin casos especiales.

        Ojo con optional de un objeto GRANDE: vive en la pila como
        cualquier variable local; para "grandes que a veces no están",
        optional no deja de construir el T cuando está.
   3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
        - optional<bool> es UNA TRAMPA: optional<bool>{false} TIENE un
          valor (false), pero el operator bool del optional pregunta
          "¿hay valor?", no "¿el valor es true?": if(opt) es true aunque
          bool interno sea false. Devolvé un bool o un enum, no envuelto.
        - Usar *opt sin chequear has_value(): UB si está vacío. Para
          lectura segura: value() (que lanza) o value_or(default).
        - value() en un optional vacío: lanza bad_optional_access; si
          "vacío" es parte normal de tu flujo, value_or.
        - optional<T> para "encontrado o no" con strings/floats: bien;
          para "ID o no existe" también es un candidato elegante
          (y std::variant lo alterna: L14).
   4. PARA PROFUNDIZAR:
        https://en.cppreference.com/w/cpp/utility/optional
        https://en.cppreference.com/w/cpp/utility/optional/value_or
        https://en.cppreference.com/w/cpp/utility/optional/bad_optional_access
   ============================================================================= */
#include <iostream>
#include <optional>
#include <string>
#include <vector>

std::optional<int> buscarEnEquipamiento(const std::vector<std::string>& bolsa,
                                        const std::string& nombre,
                                        int id) {
    for (const std::string& item : bolsa) {
        if (item == nombre) return id;    // lo encontró: el optional se llena
    }
    return std::nullopt;                  // no hay: VACIO EXPLICITO
}

int main() {
    std::vector<std::string> bolsa{"espada", "pocion", "lupa"};

    // --- 1. El flujo canonico: if (auto x = f()) --------------------------------------
    if (std::optional<int> llave = buscarEnEquipamiento(bolsa, "lupa", 3)) {
        std::cout << "encontrada: id " << *llave << '\n';
    }

    std::optional<int> tiempo = buscarEnEquipamiento(bolsa, "capa", 5);
    std::cout << "capa corre? " << (tiempo.has_value() ? "si" : "no") << '\n';

    // --- 2. value_or: dámelo o el default ----------------------------------------------
    std::cout << "\nllave del baston (no existe): "
              << buscarEnEquipamiento(bolsa, "baston", -1).value_or(0) << '\n';

    // --- 3. value() LANZA si está vacío; nullopt lo vacía -------------------------------
    std::optional<int> anchura;
    try {
        (void)anchura.value();
    } catch (const std::bad_optional_access& e) {
        std::cout << "value() en vacio lanza: " << e.what() << '\n';
    }
    anchura = 42;
    std::cout << "ahora: " << anchura.value() << '\n';
    anchura.reset();
    std::cout << "tras reset: " << (anchura.has_value() ? "algo" : "vacio") << '\n';

    // --- 4. opcional de struct: sigue siendo "por valor" ---------------------------------
    std::optional<std::string> chat{"llego el refuerzo"};
    std::cout << "size del buffer antes: " << chat->size() << '\n';
    chat = std::nullopt;    // vaciar sin dejar basura
    std::cout << "chat vacio? " << (!chat.has_value()) << '\n';
    return 0;
}
