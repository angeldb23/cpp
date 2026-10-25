/* =============================================================================
   MÓDULO 05 - LECCIÓN 02: Linkage (static, extern, inline) y namespaces
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL
      Cada "unidad de traducción" (tu .cpp más sus includes) compila por
      separado, así que hay que decidir QUÉ nombres se comparten entre
      archivos. Eso es el linkage:

      a) External linkage (por defecto): el nombre es visible para el
         linker, se puede usar desde otros .cpp. Las funciones y variables
         globales de arriba lo tienen.

      b) Internal linkage: el nombre vive solo en su archivo. Se marca con
         `static` o con un namespace anónimo. Ideal para funciones auxiliares:
         otros .cpp pueden tener su propio `static void log(...)` sin chocar.

          static int contadorLocal = 0;   // privado de este .cpp
          int  compartida = 7;            // external: puede haber UNA sola

      c) Para una variable global definida en otro .cpp, no la redeclarás
         copiando la línea: la declarás `extern` (el linker encuentra la
         única definición que existe en algún otro .cpp).

      d) `inline` en una función no significa "hacela rápida": significa
         "esta definición puede aparecer en varias unidades de traducción
         y el linker se encarga de unirlas". Es la munición para definir
         funciones cortas en headers (junto con templates y constexpr).

      Namespaces: evitan la batalla de nombres entre librerías.
          namespace audio {
              void reiniciar();   // se invoca: audio::reiniciar();
          }
          using audio::reiniciar;              // solo esa función queda sin prefijo
          namespace audio2 = audio;            // alias corto
          namespace { void helper(); }         // internal linkage

   2. ¿QUÉ PASA POR DEBAJO?
      El linker maneja símbolos: cuando unidad A usa `compartida` y
      unidad B la define, el linker conecta la dirección. Si dos .cpp
      definen el mismo símbolo external, error de duplicado. Con `static`,
      cada unidad de traducción marca su copia como local al archivo y no
      se ven entre sí, de ahí el contador independiente de abajo.

      Los namespaces no generan código: son parte del nombre completo
      (mangled). `audio::reiniciar` y `motor::reiniciar` son símbolos
      distintos para el linker, aunque en tu editor se llamen igual.

   3. ERRORES COMUNES
      - Poner `using namespace std;` en un header: cada .cpp que lo
        incluya hereda decenas de miles de nombres (desde `count` hasta
        `swap`), preparado para chocar con los tuyos. En un .cpp propio
        se tolera si el archivo es corto; en un header, nunca.
      - Confundir `static` de esta lección con el `static` de miembros
        de clase (lección 18 el lo retoma).
      - `extern` en la definición: es solo para declarar algo que se
        define otro lado.
      - Creer que `inline` obliga a pegar el código: es una sugerencia;
        la garantía real es la de linkage.

   4. PARA PROFUNDIZAR
      https://en.cppreference.com/w/cpp/language/storage_duration
      https://en.cppreference.com/w/cpp/language/namespace
   ============================================================================= */
#include <iostream>

namespace audio {
void reiniciar() { std::cout << "  audio reiniciado\n"; }
void silenciar() { std::cout << "  audio silenciado\n"; }
}  // namespace audio

void motor() { std::cout << "motor arrancando\n"; }

// Variable de internal linkage: cada copia de este archivo tendría su cuenta.
static int llamadasAlAyudante = 0;

static void ayudante() {
    ++llamadasAlAyudante;
}

int main() {
    std::cout << "=== 1. NAMESPACES: SE INVOCAN CON :: ===\n";
    audio::reiniciar();
    audio::silenciar();

    std::cout << "\n=== 2. using DECLARATIVO: SOLO UNA FUNCIÓN ===\n";
    using audio::reiniciar;   // de acá en adelante, escribo reiniciar()
    reiniciar();

    std::cout << "\n=== 3. ALIAS DE NAMESPACE ===\n";
    namespace snd = audio;
    snd::silenciar();

    std::cout << "\n=== 4. LINKAGE INTERNO: contador local ===\n";
    ayudante();
    ayudante();
    ayudante();
    std::cout << "llamadas al ayudante: " << llamadasAlAyudante << "\n";
    std::cout << "(si este archivo se copiara a otro .cpp, allá se empezaría\n"
                 "en 0: son contadores distintos por archivo)\n";

    std::cout << "\n=== 5. SIN `using namespace std;`: prefijo explícito ===\n";
    std::string mensaje = "se escribe std::string y no string a secas";
    std::cout << mensaje << "\n";
    std::cout << "(en lecciones del curso se evita using namespace std para\n"
                 "que el hábito sea el del header real: prefijos claros)\n";
    return 0;
}
