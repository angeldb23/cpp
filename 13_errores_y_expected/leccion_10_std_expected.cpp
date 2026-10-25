/* =============================================================================
   MÓDULO 13 - LECCIÓN 10: std::expected<T, E>: el error en el tipo
   =============================================================================
   C++23: esta lección muestra el std::expected REAL: compilar con
     g++ -std=c++23 -Wall -Wextra leccion_10_std_expected.cpp -o l10 && ./l10
   (el resto del curso compila con c++20: la implementación usable en
   C++20 está en la L11 y en include/mini_expected.hpp).
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       std::expected<T, E> es el "resultado con razón": o TRAE el T del
       éxito o el E del error, y le pasa el batón a la llamada con la
       razón completa. Resuelve los tres problemas vistos:
         - L01: el código de retorno se come el valor útil: expected
           guarda AMBAS cosas a la vez (valor y razón, en su unión).
         - L01: el error s'e puede ignorar -> expected NO deja otra
           opción: para leer el T tenés que pasar por has_value() (o
           value_or: la elección de "con default" es explícita).
         - L02: entre optional (solo "no está") y errores que tienen
           razón pero viajan por el canal lateral de las excepciones ->
           expected es el término medio: la razón viaja EN el tipo,
           sin nada oculto en la pila.
       API básica (la que empezamos):
         construction:  expected<int, Codigo> r = 5;  como ok
                        expected<int, Codigo> e = Codigo::Formato;
         consultas:     bool(r) / r.has_value(); r.value(); r.error();
                        r.value_or(-1);
         "unexpect":    construcción explícita como ERROR:
                        std::expected<int, Codigo>(std::unexpect, Codigo::X)
       Expected vs optional: el optional NO diferencia "no está" de
       "falla": expected lleva el E: "por qué" está disponible.
       Expected vs excepciones: una función firma su error (compila y
       documenta); no hay "paso lateral" en la pila (sin unwinding) y el
       que llama ve la elección de manejar el caso o propagarla
       (en la revisión del código, el manejo se ve: no está oculto).
    2. ¿QUÉ PASA POR DEBAJO?
        El T y el E viven en una UNIÓN: solo uno está "activo"; el bool
        dice cuál. Por eso el costo es el tamaño de max(T, E) + 1 byte
        alineado, sin alocación en heap y sin tablas de unwinding:
        el retorno suele viajar en registros del CPU cuando el tipo
        entra (el análisis del módulo 08 sobre abstracciones baratas
        aplica: no hay motivo para evitarlo).
        El std::expected también se combinan con la "in_place"
        (módulo 05: la misma lógica del optional, módulo 11) para
        construir el T sin mover/copiar (con clases grandes).
        
        std::bad_expected_access<E> (ella sí es una excepción cuando
        vos la pedís: la decisión es tuya, no sube por sí sola).
        En el std::expected el E también es la plantilla con la que
        type_traits (del módulo 09) mira si las operaciones OK.
    3. BUENAS PRÁCTICAS:
        - Elegí el E BIEN: un enum class con nombres (L01) o una
          estructura de dominio; el string está bien para UI, y algo
          más rico para sumar contexto (el módulo 13 traía temas de la jerarquía).
        - value() TAMBIÉN lanza: value() ES una promesa "sé que va a
          estar bien" (y si te equivóc, es std::bad_expected_access:
          es un throw "a la carta"): si no estás seguro, usa
          has_value() + value(), o value_or para el default.
        - No confundas "ignorar el error" con el value_or: el value_or
          al pasar "0" es una política explícita (es un patrón válido).
        - Preferí `if (!r) {  ... r.error() ... }` y retornar el
          VALUE de r (no una referencia): el error es un único ID
          (el módulo 11: optional: L13 y su compatribuida, optional del
          "detalles de reg").
        - NO launches ... std::expected<int, ...> (el error es la
          manera de fallar lógicamente (opcional del 11, pero LA
          RAZÓN (falta el punto del error)).
   ============================================================================= */
#include <cmath>
#include <expected>
#include <iostream>
#include <optional>
#include <string>
#include <variant>

namespace Leccion10 {

enum class Codigo { Ok, FormatoInvalido, ArchivoInexistente };

// El tipo de resultado con la razón de cada capa:
std::expected<int, Codigo> parseEntero(const std::string& s) {
    if (s.empty()) return std::unexpected(Codigo::FormatoInvalido);
    try {
        return std::stoi(s);              // un expected "por ok"
    } catch (const std::logic_error&) {
        return std::unexpected(Codigo::FormatoInvalido);
    }
}

// El patrón "encadenar chequeos 2 y fallar temprano":
std::expected<int, Codigo> numeroPorSesión(const std::string& archivo, const std::string& clave) {
    if (archivo != "config.ini") return std::unexpected(Codigo::ArchivoInexistente);
    if (clave.empty()) return std::unexpected(Codigo::FormatoInvalido);
    return parseEntero("42");
}

} // namespace Leccion10

int main() {
    std::cout << "== construcción implícita (ok y error) ==\n";
    std::expected<int, Leccion10::Codigo> ok = 7;
    std::expected<int, Leccion10::Codigo> err = std::unexpected(Leccion10::Codigo::FormatoInvalido);
    std::cout << "ok:  bool = " << static_cast<bool>(ok) << ", value = " << ok.value() << '\n';
    std::cout << "err: bool = " << static_cast<bool>(err) << ", has_value = " << err.has_value() << '\n';

    std::cout << "== value_or: el default con UNA línea ==\n";
    std::cout << "ok.value_or(0)   = " << ok.value_or(0) << '\n';
    std::cout << "err.value_or(0)  = " << err.value_or(0) << '\n';

    std::cout << "== value() sobre error lanza (bad_expected_access) ==\n";
    try {
        std::cout << err.value() << '\n';
    } catch (const std::bad_expected_access<Leccion10::Codigo>& e) {
        std::cout << "atrapada: el error guardado viaja adentro de la excepción\n";
        (void)e;
    }

    std::cout << "== el uso real: leé lo que cada expected decía ==\n";
    auto r = Leccion10::numeroPorSesión("config.ini", "ancho");
    std::cout << "config.ini \"ancho\": "
              << (r ? "ancho = " + std::to_string(r.value()) : "error: la razón viaja en el tipo") << '\n';
    auto r2 = Leccion10::numeroPorSesión("otro.cfg", "ancho");
    std::cout << "otro.cfg: bool = " << static_cast<bool>(r2)
              << " (la firma declaró el error y el que llama revisa)\n";
    (void)r2;
    return 0;
}
