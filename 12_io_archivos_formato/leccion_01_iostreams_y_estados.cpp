/* =============================================================================
   MÓDULO 12 - LECCIÓN 01: iostreams, estados del stream y qué es "formateado"
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       Los objects de E/S de C++ son STREAMS: un tubo por el que fluyen
       caracteres (o bytes) entre tu programa y "el mundo": la consola,
       un archivo, una GPU de texto, el programa que te invoca.

       std::cout, std::cerr y std::cin son ejemplos que ya usaste. Ahora se
       formaliza: son instancias de clases de la familia
         ostream (salida)  -> cout, cerr, clog
         istream (entrada) -> cin y (más adelante) ifstream/istringstream
       Concretamente cout es un ostream y cin es un istream; archivos y
       string-stream HOJAS de la misma familia: todo lo que aprendas acá
       reusa en fstream y sstream.

       Cinco estados que todo stream lleva, y que se combinan (bits):
         goodbit   todo bien (el default)
         eofbit    llegaste al FINAL de la entrada (no es un error por sí)
         failbit   la operación pedida NO se pudo (p.ej. int de "hola")
         badbit    corrupción irreparable: el stream ya no sirve
       Consultas: .good(), .eof(), .fail(), .bad() y el más práctico:
       el stream convierte a bool (`if (archivo)` = "sin problemas
       HASTA AHORA"). Clear para resetear: como los bits quedan puestos,
       un stream en fail NO sigue leyendo hasta que hagas .clear().

       Un stream es un objeto con estado OCULTO: cada lectura avanza una
       "posición" interna. Ese es el motivo de los bugs clásicos: quien
       lee 2 veces el mismo dato cree que es el mismo, pero el cursor ya
       avanzó (el módulo 03 lo llamó "iteradores que invalidan": acá el
       cursor es el propio stream).

       Formateado vs no formateado:
         - Formateado: `<<`/`>>` interpretan TIPOS (int, double, string)
           y deciden cómo se escriben como caracteres.
         - No formateado: get/put/read/write mueven BYTES crudos tal cual.
       En esta lección el foco es el estado; en la L02 abrimos el detalle
       del contraste formateado/no formateado.

       Continuidad (módulo 03+): el `>>` de un número salta blancos
       "actúa como un find_if": por eso en interfaces por consola podés
       tipear " 42 " y extrae el 42 igual.
   2. ¿QUÉ PASA POR DEBAJO?
       cout NO imprime directo: los caracteres entran en un buffer de
       memoria y el SO los escribe cuando el buffer se llena (o cuando
       alguien fuerce). Eso es la razón de "cout no aparece" si el
       programa crasea antes del flush: el buffer se pierde. El endl
       imprime '\n' Y HACE flush; el '\n' solo cambia de línea (mejor:
       flush es IO pesado). Un stream extrae al buffer entre llamadas;
       cerr y clog: cerr NO está bufferizado (los errores salen YA),
       clog sí (es para logs de larga absorción).
       El estado no es un número: son flags de bits (kind of un bitset
       de la STL). Las funciones good/fail/bad/eof son lectoras de esos
       bits; clear() reescribe el conjunto entero.
   3. BUENAS PRÁCTICAS:
       - `if (!stream)` después de CADA operación que importa (L12 lo
         convierte en un patrón completo).
       - '\n' por defecto; endl SOLO cuando querés forzar salida a esta
         altura.
       - No mezcles stderr/cout esperando orden: flujos van a destinos
         distintos y pueden salir entremezclados si no haces flush.
       - Nunca des por hecho que la lectura se logró: en C++ el `>>`
         no lanza: PONE failbit y te deja la variable EN CERO (o en
         lo que le alcanzó): validar es tu trabajo.
   ============================================================================= */
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

// Un "stream de estados" demo: las condiciones son las de CUALQUIER stream
void mostrar(const std::string& etiqueta, const std::istream& s) {
    std::cout << etiqueta
                  << " eof=" << (s.eof() ? 1 : 0)
                  << " fail=" << (s.fail() ? 1 : 0)
                  << " bad=" << (s.bad() ? 1 : 0) << "\n";
}

int main() {
    std::cout << "=== 1) Tres salidas canonicales ===\n";
    std::cout << "cout: salida con buffer, para EL DATO del programa\n";
    std::cerr << "cerr: salida SIN buffer (flush inmediato), para errores\n";
    std::clog  << "clog: salida CON buffer, para logs largos\n";

    std::cout << "\n=== 2) El estado tras cada operacion ===\n";
    std::istringstream datos("42 hola 99"); // string que "finge" stdin

    mostrar("estado inicial:      ", datos);

    int numero = 0;
    datos >> numero;
    mostrar("tras leer un int:    ", datos);

    // El problema clasico: pedirle un int al proximo dato QUE NO LO ES:
    std::string basura;
    datos >> basura;            // toma "hola" primero (>> descarta blancos)
    (void)basura;
    datos >> numero;            // intenta leer un int de "99": este SI es int,
    mostrar("tras el segundo int: ", datos);

    // Ahora forzamos el failure real: convertimos "hola" como numero:
    std::istringstream other("no-soy-numero");
    int valor = 0;
    other >> valor;             // falla: pone failbit y deja valor EN 0
    mostrar("tras int de texto:   ", other);
    std::cout << "valor quedo en: " << valor << " (>> no lanza: VALIDA TU) \n";
    std::cout << "notar: .fail()=1 y el stream NO lee mas sin clear()\n";

    other.clear();              // limpiar failbit: el stream vuelve a servir
    other >> valor;
    std::cout << "tras clear, reintentar da: false==true? logicamente limpio\n";
    (void)valor;

    std::cout << "\n=== 3) El stream como bool ===\n";
    if (datos) {
        std::cout << "datos esta bien: bool=true\n";
    }
    if (!other) {
        std::cout << "other en fail: bool=false (sin clear)\n";
    }

    std::cout << "\n=== 4) La trampa del >> que un numero le quede en 0 ===\n";
    // Un >> fallido tiene DOS efectos: failbit a 1 Y el valor de destino
    // paso a CERO (C++11 en adelante): molesto, pero al menos predecible.
    std::istringstream many("1 2 3 fin");
    int suma = 0, leido = 0;
    while (many >> leido) suma += leido;   // el while usa el bool del stream
    std::cout << "suma leida: " << suma << " (el loop corto cuando 'fin' no dio un int)\n";
    mostrar("estado del loop:     ", many);
    std::cout << "cheque de eof ahi arriba: eof nunca se seteo (no llegamos al final real)\n";
    std::cout << "es la forma idiomatica de 'leer hasta que no quede dato'\n";
    return 0;
}
