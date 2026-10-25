/* =============================================================================
   MÓDULO 13 - LECCIÓN 12: las monádicas: and_then, transform, transform_error, or_else
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       L02 contó el costo del canal (A): con códigos, cada paso del
       camino feliz se escribe TRES veces (chequear error, reenv Wolf).
       El chiste del expected ES su composición: una cadena de pasos
       con errores se escribe como UNA pipe, y el caso de error viaja
       solo por las cuatro fajas que corresponden:

         and_then(f)        si hay valor: f(valor) -> otro expected
                            si hay error: se pasa el error sin llamar f
           "el paso que TAMBIÉN puede fallar"
         transform(f)       si hay valor: f(valor) -> valor nuevo (el
                            expected lo re-envuelve); si falla: passa,
           "el paso seguro que CONVERTIDO el valor"
         transform_error(f) si hay error: f(error) -> error nuevo (del
                            tipo que la capa de arriba espera);
           "el paso de TRADUCIR" el error"
         or_else(f)         si hay error: f(error) -> expected de
                            "recuperación" (otro intento o default);
           "el paso de arreglo local"

       La pipe se lee de arriba a abajo: es la secuencia de pasos con
       valor; cuando algo falla, los CORTOS restantes no llaman a su
       callable y el error baja directo hasta el or_else/transform_error
       que toque (o hasta la variable final, donde el que llama la
       revisa una sola vez). Cada uno de los cuatro es de LAIZO:

         auto resultado = cargar(linea)          // expected<Config, E>
             .and_then(validar)                   // (paso que TAMBIÉN falla)
             .transform(aplicar)                  // (paso seguro que convierte)
             .transform_error(aErrorUI)           // (traducir el E)
             .or_else(usarDefault);               // (arreglo local)
       El or_else y el transform son los "cortes" de la cadena: si el
       error sigue (and_then no corre), la llamada "no corre"; y se
       cadena HASTA el punto que el FLUJO se interrumpe: en las
       excepciones, el "corte" es físico (unwinding) en la L02.
       versión A de la decisión; en el expected, es la RECOGNITION:
       el flujo NO se interrumpe: el error RECUE предваритель: la
       cadena sigue (con el error dentro).
    2. ¿QUÉ PASA POR DEBAJO?
        La monádica evalúa EAGER (no lazy: el callable correme inmedia-
        tos de la llamada). Con strings públicas: las lambdas de la
       .pipe (módulo 10: las lambdas genéricas) y el invoke_result_t
        (módulo 09) deducen los tipos nuevos: por eso la pipe compila
        SIN DECLARAR tipos esperados manualmente: cada step-que: es
        el proveedor de su tipo.
        _Inicio: parte de la cadena: las pipe: value_or-return?
        (parhered de la pipe? (el "CHECKED en medio: las value ()...)
    3. BUENAS PRÁCTICAS:
        - Pipeline con and_then/transform DE 2 a 5 pasos: la cadena
          larga se pierde de la vista; si la necesitas, separa y
          nombre cada fragmento (que es también qué hace la L05 del 08
          con las fases de compilación: CADA PIPE de una fase).
        - transform_error EN CADA CAPA: una vuelve claro: la REDUCE
          el backend, la UI agrega texto, la SERVIDORAحرس decide:
          el error pasa "acumulando lenguaje" (obj) de la capa más
          alta (y no un string interno de n�what استان).
        - or_else para la REPARACIÓN local: el default, el reintento
          o la otra fuente; y transform_error para la TRADUCCIÓN (prote-
          colo).
        - El nombre de la capa define el Mensaje: nunca pegar el string
          crudo del backend al usuario (la traducción es cosa de cada
          transform_error, no del main).
        - El orden de los pasos: la cadena es LINEAL: si el mismo error
          necesita dos o_else a la vez, está siendo usado en dos
          capas: separalas.
   ============================================================================= */
#include <iostream>
#include <string>

#include "../include/mini_expected.hpp"

namespace Leccion12 {

mini::expected<int, std::string> parseEntero(const std::string& s) {
    try {
        return std::stoi(s);
    } catch (const std::logic_error&) {
        return std::string("no es un numero: '" + s + "'");
    }
}

// El segundo paso QUE TAMBIÉN falla (and_next: otra capa de expected).
mini::expected<int, std::string> dividir(int num, int den) {
    if (den == 0) return std::string("división por cero");
    return num / den;
}

} // namespace Leccion12

int main() {
    std::cout << "== una pipe: parse -> dividir -> transform ==\n";
    auto happy = Leccion12::parseEntero("84")
                     .and_then([](int v) { return Leccion12::dividir(v, 2); })
                     .transform([](int half) { return half * 10; });
    std::cout << "84/2 * 10 = " << happy.value_or(-1) << '\n';

    std::cout << "== el mismo pipe con un dato que falla temprano ==\n";
    auto falla = Leccion12::parseEntero("basura")
                     .and_then([](int v) { return Leccion12::dividir(v, 2); })
                     .transform([](int half) { return half * 10; })
                     .transform_error([](std::string e) { return "capa UI: " + e; });
    std::cout << "error final: " << falla.error() << '\n';

    std::cout << "== or_else: el arreglo local ==\n";
    auto retinta = Leccion12::parseEntero("")
                       .or_else([](std::string e) -> mini::expected<int, std::string> {
                           std::cout << "  or_else corrió (el error: " << e << ")\n";
                           return Leccion12::parseEntero("100");     // segundo intento
                       })
                       .and_then([](int v) { return Leccion12::dividir(v, 4); });
    std::cout << "el segundo intento terminó en: " << retinta.value_or(-1) << '\n';

    std::cout << "== transform cambia el tipo del valor ==\n";
    mini::expected<int, std::string> base = Leccion12::parseEntero("7");
    auto par = base.transform([](int v) { return v % 2 == 0; });
    std::cout << "7 es par? " << (par.value() ? "true" : "false")
              << "; el tipo de par es expected<bool, string> (transform cambia T)\n";
    return 0;
}
