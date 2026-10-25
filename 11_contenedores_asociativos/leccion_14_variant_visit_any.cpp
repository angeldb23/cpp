/* =============================================================================
   MÓDULO 11 - LECCIÓN 14: std::variant, std::visit y std::any
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
        std::optional (L13) resuelve "un tipo, a veces ausente".
        std::variant<A, B, C> resuelve el escalón siguiente: una variable
        contiene UNA de varias alternativas (un "enemigo es humano,
        fantasma o torreta"):

            std::variant<int, std::string> celda;
            celda = 42;        // ahora es int
            celda = "texto";   // ahora es string (destruyó al int)

        Consultar Qué Hay:
          - std::holds_alternative<T>(v): ¿hay un T?
          - std::get<T>(v): el T, o LANZA bad_variant_access si no.
          - std::get_if<T>(&v): puntero al T, o nullptr: sin excepción.
          - v.index(): cuál alternativa está activa.

        La joya es std::visit: aplicar una llamada a "lo que haya":

            std::visit([](const auto& valor) { cout << valor; }, celda);

        la lambda genérica (módulo 10) se instancia por CADA
        alternativa y visit elige: es el switch de tipos hecho derecho.
        Cuando cada tipo necesita su lógica propia, la técnica estándar
        es el "overloaded" pattern con lambdas sobrecargadas:

            auto proc = [](const int& n) { ... };
            auto procs = Overloads{proc, [](const std::string& s) { ... }};
            std::visit(procs, celda);

        std::any: el último escalafón: "cualquier tipo", conocido solo en
        runtime. Es type erasure como std::function (módulo 09), pero de
        valor; cuando el tipo puede copiar, guarda una copia; cuando no,
        puede alocar. Usalo POCO: casi siempre variant (con las
        alternativas QUE YA CONOCES) o un tipo concreto es mejor diseño:
        any es para los bordes de "vengo sin esquema" (bindings de
        scripts, plugins).
    2. ¿QUÉ PASA POR DEBAJO?
        sizeof(variant<A, B, C>) ≈ max(sizeof(A), sizeof(B), sizeof(C))
        más un índice pequeño: guarda el valor en algún almacenamiento
        alineado y un índice que dice qué alternativa está activa.
        Cambiar de alternativa destruye lo viejo y construye lo nuevo
        (RAII dentro de variant: sin fugas).

        std::visit es un "dispatch": salta al índice y ejecuta la rama
        correcta; como la lista de alternativas es CONOCIDA en
        compilación, el compilador puede INLINEAR y optimizar mejor que
        una llamada virtual o un std::function (mod. 09).

        estado "valueless_by_exception": si construir la nueva
        alternativa lanza en el medio, el variant queda "sin valor"
        (raro, pero existe); por eso get lanza si estuviera así.
    3. ERRORES COMUNES Y BUENAS PRÁCTICAS:
        - variant con alternativas artificiales: no inventes un "caso
          nulo" para llenar el variant si tu primer estado es "vacío":
          eso es optional (L13). Las alternativas deben ser estados
          reales de tu modelo.
        - std::get<T>(v) esperando otro tipo: lanza al vuelo; para
          flujo normal usá get_if o visit: las excepciones van para
          "esto NO debió pasar".
        - visit con una lambda genérica que hace cosas distintas por
          tipo: el error aparece COMPILANDO (una instancia por
          alternativa): si una alternativa no compila en el cuerpo
          común, se ve ahí, no en runtime.
        - std::any para tipos que YA conocés: un variant es más barato
          y autodocumentado; any paga type erasure con alocación
          posible: guardalo para los bordes dinámicos (scripts,
          plugins).
    4. PARA PROFUNDIZAR:
        https://en.cppreference.com/w/cpp/utility/variant
        https://en.cppreference.com/w/cpp/utility/variant/visit
        https://en.cppreference.com/w/cpp/utility/any
   ============================================================================= */
#include <any>
#include <iostream>
#include <string>
#include <variant>

// El "overloaded" pattern para visit con una lambda por alternativa:
// solo funciona en C++20 (lambdas en deducción no declarada con <auto>):
template <class... Lambdas>
struct EnCadaTipo : Lambdas... {
    using Lambdas::operator()...;
};
// guía de deduccion: dejar que el compilador vea las lambdas:
template <class... Lambdas>
EnCadaTipo(Lambdas...) -> EnCadaTipo<Lambdas...>;

int main() {
    // --- 1. variant: UNA variable con tipos alternativos ------------------------------
    std::variant<int, std::string> celda;
    celda = 42;
    std::cout << "es int? " << std::holds_alternative<int>(celda)
              << " | get<int>: " << std::get<int>(celda) << '\n';

    celda = "texto";    // cambio de alternativa: el int se destruyo
    std::cout << "es string? " << std::holds_alternative<std::string>(celda)
              << " | get<string>: " << std::get<std::string>(celda) << '\n';

    // --- 2. get_if: consulta SIN excepsiones ---------------------------------------------
    if (const int* n = std::get_if<int>(&celda)) {
        std::cout << "int: " << *n << '\n';
    } else {
        std::cout << "no hay int en la celda (index = " << celda.index() << ")\n";
    }

    // --- 3. visit: una lambda por CADA alternativa (overloaded pattern) ------------------
    std::variant<int, std::string> entidad{std::string{"luz"}};
    const auto procesar = EnCadaTipo{
        [](const int& vida) {
            std::cout << "visit (int): vida " << vida << '\n';
        },
        [](const std::string& tipo) {
            std::cout << "visit (string): tipo " << tipo << '\n';
        },
    };
    std::visit(procesar, entidad);

    // --- 4. any: valor de tipo conocido SOLO en runtime -----------------------------------
    std::any caja = 3.14;
    caja = std::string{"nada"};
    std::cout << "\nany typeid: " << caja.type().name() << '\n';
    std::cout << "any_cast a string: " << std::any_cast<std::string>(caja) << '\n';
    try {
        (void)std::any_cast<int>(caja);
    } catch (const std::bad_any_cast& e) {
        std::cout << "any_cast malo lanza: " << e.what() << '\n';
    }
    return 0;
}
