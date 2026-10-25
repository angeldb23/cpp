/* =============================================================================
   MÓDULO 13 - LECCIÓN 01: la primera estrategia: códigos de retorno
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       Todo programa falla: se cae el disco, el usuario tipea mal, un archivo
       no está. La pregunta de este módulo no es SI hay errores, sino CÓMO
       los comunicamos de una función hacia afuera. Hay tres familias:
         a) códigos de retorno (acá y en L02),
         b) excepciones (el resto del módulo, ya anunciadas en el módulo 04),
         c) tipos de resultado explícitos como std::expected (L10-L12).
       Escribes todo el tiempo porque cada función RESPONDE una sola cosa
       cuando se termina: lo hice o no lo hice.

       La forma más vieja y simple: devolver el ESTADO por el valor de
       retorno y (si hace falta) el RESULTADO por el costado:
         - bool hacer(...)                     -> ¿sirvió? (ya lo usaste así)
         - enum class Estado hacer(...)        -> CÓMO falló, una lista corta
         - int* buscar(...)                    -> centinela: nullptr = no está
         - bool hacer(T& resultado)            -> código + resultado por parámetro
       Para casos donde el error puede tener VARIOS sabores, enum class es
       mejor que bool: el bool solo dice "no sirvió", el enum dice
       "no sirvió porque X". Bien hecho, el código puede quedarse en la
       firma: `Estado parse(const std::string&, T& valor)` — la firma te
       anuncia que hay dos cosas en juego (estado y resultado).

       El problema de familia es que el error viaja en el CANAL del éxito:
         - si la función devuelve int, hay que RESERVAR un número mágico
           (-1, nullptr...) y esa convención vive solo en la documentación.
         - nada te OBLIGA a leer el código: `parse(x, y);` compila, el
           compilador no se queja, y el error cae al piso silencioso.
       Por eso esta lección ordena una "jerarquía de fuerza": bool (débil,
       fácil de ignorar) -> enum (con razón) -> código + OUT-param ->
       optional (el "no está" es parte del tipo). También muy pronto este
       módulo suma las otras dos familias y las reglas de elección: L02
       arma la tabla completa; L10-L12 completan la foto con std::expected.

       Recuerde del módulo 04: allá el throw/catch fue uso práctico, sin
       profundizar la jerarquía. Acá se sistematiza: qué estrategia usar
       y por qué, según el tipo de error.
    2. ¿QUÉ PASA POR DEBAJO?
        Un bool de retorno suele viajar en un registro del CPU, así que
        verificarlo es casi gratis. La EXCEPCIÓN se paga distinto: en el
        camino común "sin error" la llamada no cuesta casi nada (zero
        cost hasta que algo lanza), pero cuando lanza hay que recorrer
        frames. Por eso la regla clásica: excepción para lo RARO, código
        para lo esperable. Si el proyecto se compila con -fno-exceptions,
        el código de retorno es el único camino disponible.
    3. BUENAS PRÁCTICAS:
        - enum class `con nombre`, nunca números crudos que nadie recuerda.
        - Never returning sirve para lo "casualidad" que ya es un valor
          en el tipo: `optional<int> buscar(...)` SI de IDETYPE: el
          "no está" es parte honesta de la firma (módulo 11, L13).
        - Si usas parámetro OUT, inicialízalo antes de salña: nadie debe
          leer basura si la función falla a mitad.
        - Devuelve el mismo tipo si poder: hacer confuso código en acción.
          La razón del error en un mensaje, no en una falta codes.
        - Si cada llamada es seguida de `if (ok)` pegado al nivel 4 y el
          error SÍ puede quedarse sin manejar, eso apunta a excepción
          (lo analizamos con calma en L02).
   ============================================================================= */
#include <cmath>
#include <iostream>
#include <optional>
#include <string>

namespace Leccion01 {

// (1) bool simple: vale para ¿sirvió o no?, no explica CÓMO falló.
bool esPar(int n) { return n % 2 == 0; }

// (2) enum class: falló, y te da la razón. Esta forma escala a firmas reales.
enum class CargaResultado { Ok, ArchivoInexistente, FormatoInvalido, SinPermisos };
CargaResultado cargarConfig(const std::string& nombreArchivo) {
    if (nombreArchivo.empty()) return CargaResultado::ArchivoInexistente;
    if (nombreArchivo == "viejo.txt") return CargaResultado::FormatoInvalido;
    return CargaResultado::Ok;
}

// (3) resultado por parámetro OUT + código: firma honesta, dos canales.
bool dividir(int numerador, int denominador, int& cociente) {
    cociente = 0;                       // siempre dejamos el OUT en un estado válido
    if (denominador == 0) return false; // el CASO ERROR también define el OUT
    cociente = numerador / denominador;
    return true;
}

// (4) centinela nullptr: clásico de C, sigue apareciendo por toda la STL.
const char* buscarEtiqueta(const char* const* tabla, const char* nombre) {
    for (const char* const* it = tabla; *it != nullptr; ++it) {
        if (std::string(*it) == nombre) return *it;
    }
    return nullptr;                     // "no está": el centinela ES el error
}

// (5) std::optional: el centinela como parte del TIPO, no como magia.
std::optional<int> raizCuadradaEntera(int n) {
    int r = static_cast<int>(std::sqrt(n));
    if (r * r != n) return std::nullopt;
    return r;
}

} // namespace Leccion01

int main() {
    using Leccion01::CargaResultado;
    std::cout << "== bool simple ==\n";
    std::cout << "esPar(4): " << (Leccion01::esPar(4) ? "sí" : "no") << '\n';

    std::cout << "== enum class: la razón del fallo viaja con el resultado ==\n";
    switch (Leccion01::cargarConfig("viejo.txt")) {
        case CargaResultado::Ok:                 std::cout << "cargó\n"; break;
        case CargaResultado::ArchivoInexistente: std::cout << "no existe\n"; break;
        case CargaResultado::FormatoInvalido:    std::cout << "formato viejo: avisa y salta\n"; break;
        case CargaResultado::SinPermisos:        std::cout << "permisos\n"; break;
    }

    std::cout << "== out-param: el resultado y el error por canales separados ==\n";
    int cociente = 0;
    if (!Leccion01::dividir(10, 0, cociente)) {
        std::cout << "No se divide por 0, cociente quedó en " << cociente << '\n';
    } else {
        std::cout << "10/2 = " << cociente << '\n';
    }
    Leccion01::dividir(10, 2, cociente);
    std::cout << "10/2 = " << cociente << '\n';

    std::cout << "== centinela ==\n";
    const char* etiquetas[] = {"vida", "mana", "exe", nullptr};
    std::cout << "buscar 'mana' da: " << (Leccion01::buscarEtiqueta(etiquetas, "mana") != nullptr ? "encontrada" : "null") << '\n';
    std::cout << "buscar 'hueso' da: " << (Leccion01::buscarEtiqueta(etiquetas, "hueso") == nullptr ? "null (no está)" : "?") << '\n';

    std::cout << "== optional: el 'no está' como parte de la firma ==\n";
    auto raiz = Leccion01::raizCuadradaEntera(49);
    std::cout << "raíz entera de 49: " << (raiz ? std::to_string(*raiz) : "no hay") << '\n';
    auto raizFail = Leccion01::raizCuadradaEntera(50);
    std::cout << "raíz entera de 50: " << (raizFail ? std::to_string(*raizFail) : "no hay") << '\n';
    return 0;
}
