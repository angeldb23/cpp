/* =============================================================================
   MÓDULO 12 - LECCIÓN 06: validación de entrada: leer datos con RESPONSABILIDAD
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       Toda entrada del mundo real puede venir basura: el jugador tipea
       mal, el archivo trae números en texto, el buffer de la red se
       corta. Validar la entrada NO es cosmético: es la única forma de
       escribir código que no cuelga cuando el mundo no coopera.

       La receta canon de "leer un dato de un stream sanamente" (los
       TRES pasos en orden: la L01 y la L02 combinadas):
         1) INTENTAR la lectura (>> para números).
         2) SI FALLA, la señal ES el bool del stream. Y SI ESTÁ en
            failbit/badbit, el stream no sirve más hasta:
               in.clear();                      // reinicia los bits
               in.ignore(max, '\n');           // desecha la porción pegada
         3) REPORTAR: devolver false (o optional vacío) y avisar QUÉ se
            esperaba: el error que no se ve no existe.

       Las tres variante útiles que se programan en esta lección:
         - leerEntero(in, &out): el parse estricto (después del >> el
           próximo char no puede ser una letra (peek lo decide).
         - pedirEnteroConReintentos(in, max): el loop con TOPE de
           reintentos, devolviendo optional<int> vacío al agotarse.
         - leerEnRango(in, &out, min, max): hay que aceptar un int
           Y que esté en rango (el menú 1..3).

       ¿Por qué reintentos con TOPE? En un juego real el input puede
       venir de un stream NO interactivo (red, demo grabado): un
       "reintenta hasta que sea válido" SIN tope es un loop infinito
       que no puedes matar. Con tope, el flujo devuelve nullopt y el
       caller decide (p.ej. "volvé al menú principal").

       LA TRAMPA del cuadrada menú: si el stream quedó en failbit y NO
       clear(), el próximo `>>` es no-op: el menu entra en un
       while(true) de "opción inválida" infinita en medio de un
       stream muerto: la sección 4 del main lo demuestra.
   2. ¿QUÉ PASA POR DEBAJO?
       El estado del stream NO se aleja solo: una vez failbit, siempre
       failbit: todo `>>` posterior no hace nada (la trampa clásica del
       "while (cin >> x) { ... }" que corta UNA VUELTA tarde). El
       clear() + ignore() son los DOS pasos (uno devuelve el estado,
       el otro desecha el residuo del parse).
       El bool del stream en un `while (in >> x)` es "estado no-fail":
       para el parse ESTRICTO se apoya en get/peek después del `>>`
       (la receta leerEntero de abajo).
   3. BUENAS PRÁCTICAS:
       - Devolver bool/optional y NO meter cin dentro de la función: el
         parseo contra `std::istream&` es TESTEABLE con istringstream.
       - Mensaje de error EXPLICATIVO: no "error", sino "no es un entero":
         un texto que explica por qué se rechazó y qué se espera.
       - Limitar reintentos (el maxIntentos de_optional): si el input
         viene de un stream no interactivo, un loop infinito es un
         bug garantizado: con tope, vos decidís.
       - NO leas `while (!in.eof())`: eof() se enciende solo DESPUÉS de
         una lectura que cruzó el final: ese loop agrega UNA
         vuelta extra con el dato fallado: el canon es `while
         (lectura_exitosa)`: la L01 ya lo mostró.
   ============================================================================= */

#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <string>

namespace entrada {
    // (receta) leer un int de "lo que quede en la linea actual".
    // False si lo que resta no es un entero SOLO (peek ha de ser blanco/EOF).
    bool leerEntero(std::istream& in, int& destino) {
        if (!(in >> destino)) return false;     // no es int (o ya en fail)
        // parse "estricto": el proximo char no puede 'ser' una letra suelta:
        char c = 0;
        if (in.get(c) && !std::isspace(static_cast<unsigned char>(c))) return false;
        if (in) {                // tomó un char: lo devolvemos al stream
            in.putback(c);
        }
        return true;             // peek() == white/EOF y el int sale OK
    }

    // (receta) loop de reintentos con tope; devuelve optional<int> vacio si
    // SE AGOTAN los reintentos con datos @@basura en toda la fila restante.
    std::optional<int> pedirEnteroConReintentos(std::istream& in, int maxIntentos) {
        int valor = 0;
        for (int intento = 0; intento < maxIntentos; ++intento) {
            if (leerEntero(in, valor)) return valor;
            in.clear();
            // desechamos el RESTO de la linea (basura en la mitad del parse)
            in.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
        return std::nullopt;    // no es optional que el jugador estafe infinita
    }

    bool leerEnRango(std::istream& in, int& destino, int min, int max) {
        if (!leerEntero(in, destino)) return false;
        return destino >= min && destino <= max;
    }
} // namespace entrada

int main() {
    std::cout << "=== 1) leerEntero acepta el bien y rechaza el mal ===\n";
    std::istringstream bien("42");
    int v = 0;
    std::cout << "input '42'      -> " << entrada::leerEntero(bien, v)
              << " (valor " << v << ")\n";
    std::istringstream basura("hola");
    v = 7;
    std::cout << "input 'hola'    -> " << (entrada::leerEntero(basura, v) ? "true" : "false")
              << " (valor sigue " << v << ")\n";
    std::istringstream semi("12abc");
    std::cout << "input '12abc'   -> "
              << (entrada::leerEntero(semi, v) ? "true" : "false")
              << " <- rechazado por el peek del parse estricto\n";

    std::cout << "\n=== 2) reintentos con tope: el menu que no cuelga ===\n";
    std::istringstream torpe("azul\nverde\n2007\n");
    try {
        auto dato = entrada::pedirEnteroConReintentos(torpe, 3);
        std::cout << "tercer intento dio: " << dato.value() << "\n";
    } catch (const std::bad_optional_access&) {
        std::cout << "con maxIntentos agotados: nullopt (el programa NO cuelga)\n";
    }
    std::istringstream agostado("aza\narbol\n");
    const auto agotado = entrada::pedirEnteroConReintentos(agostado, 3);
    if (!agotado) {
        std::cout << "2 datos malos y fin de entrada: se devolvio nullopt\n";
    }

    std::cout << "\n=== 3) rango: el dato tiene que SER int Y estar en rango ===\n";
    std::istringstream opciones("2 9 3 0");
    std::cout << "input: 2 9 3 0 (rango 1..3):\n";
    int opc = 0;
    while (entrada::leerEntero(opciones, opc)) {
        if (entrada::leerEnRango(opciones, opc, 1, 3)) {
            std::cout << "   opcion " << opc << ": valida\n";
        } else {
            std::cout << "   opcion " << opc << ": fuera de rango (1..3)\n";
        }
    }
    std::cout << "(validar rango es un paso DESPUES del parse: dos chequeos)\n";

    std::cout << "\n=== 4) LA TRAMPA del while(!eof) ===\n";
    std::istringstream pocos("1 2");
    int leido = 0, suma = 0, vueltas = 0;
    // MAL: eof se activa DESPUES de fallar una vez ya:
    while (!pocos.eof()) {
        pocos >> leido;
        suma += leido;           // la vuelta extra suma basura
        ++vueltas;
    }
    std::cout << "vueltas: " << vueltas << " con 2 datos: sobra 1 (la vuelta extra)\n";
    pocos.clear();
    pocos.str("1 2");            // recargar para el caso fino
    leido = 0; suma = 0;
    while (pocos >> leido) suma += leido;
    std::cout << "canon (while >> ): suma " << suma << ", vueltas exactas\n";
    return 0;
}
