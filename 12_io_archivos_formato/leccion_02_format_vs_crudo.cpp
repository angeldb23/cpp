/* =============================================================================
   MÓDULO 12 - LECCIÓN 02: formateado vs no formateado; saltar datos en la entrada
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       Toda E/S de streams se divide en dos mundos:

       FORMATEADO (`<<` y `>>`):
         - "Traducí este int a caracteres" / "leé caracteres y convertilos
           a int".
         - La gracia: los operadores son EXTERNOS a la clase y se pueden
           sobrecargar para TUS tipos (L14-L15 lo explotan); es cómodo
           PERO deja casos de borde en tu cara (con `>>` un "12abc" lee
           12 y deja "abc" en el stream: parseo parcial, no error).
         - Toda operación formateada salta blancos ANTES de leer (skipws):
           la entrada " 42" y la "42" son lo mismo para `>>`.

       NO FORMATEADO (get, put, read, write, ignore, peek, putback,
       gcount):
         - "Moveme estos bytes tal cual, sin interpretar nada". Es lo
           que usan los archivos binarios (L08) y los parsers finos
           (L05).
         - La razón de existir: el formateado NO te dice CUÁNTO leés ni
           por dónde se quedó: si tu parser necesita "mirar el próximo
           char SIN comerlo" (peek), "comer la línea entera" (ignore) o
           leer char a char (get), no hay `<<` o `>>` que salga barato.

       La estrella de la entrada sana: ignore(n, delim). Come hasta n
       caracteres o hasta encontrar delim (el delim TAMBIÉN se come);
       con std::numeric_limits<std::streamsize>::max() y '\n' comés
       "toda la línea restante": es el estándar para limpiar una
       entrada fallida, junto con clear() (L06 lo arma como receta).

       gcount: cuántos caracteres leyó la ÚLTIMA operación NO formateada.
       El formateado no reporta cuántos consumió (te quedaste mirando el
       stream a ciegas): otra razón para preferir el crudo en parsers serios.

       Continuidad: getline (L05) es una operación semi-formateada que UNE
       los dos mundos: recorre crudo hasta '\n' pero te entrega std::string.
   2. ¿QUÉ PASA POR DEBAJO?
       `stream >> x` equivale, grosso modo, a:
         1) saltar blancos (ctype.is(space) en loop crudo),
         2) llamar a la faceta num_get/ctype del locale para parsear,
         3) escribir en x solo si el parse completo funcionó.
       El costo de ese paso 3 es que x queda TRUNCADO en el fallo (viste
       el "quedar en 0" de la L01): por eso la técnica sanadora:
       clear() + ignore().
       peek() mira el próximo char SIN avanzar el cursor (el lookahead
       real); putback() re-ingresa lo último que comiste (un char).
       El buffer de salida sigue siendo el de la L01: write sobre cout
       no aparece hasta el flush: el '\n' vs endl es la misma vara.
   3. BUENAS PRÁCTICAS:
       - Limpieza de entrada sanadora = clear() PRIMERO (si no, ignore
         no hace nada: el stream en fail no procesa) Y DESPUÉS
         ignore(max, '\n').
       - peek() antes de `>>` para decidir "¿el próximo dato es dígito
         o letra?": los parsers de consola sanos lo usan.
       - gcount() SOLO tiene sentido inmediatamente después de una
         lectura cruda: cualquier otra operación (o el propio `>>`)
         lo anula.
       - El parsing línea a línea (L05-L06) casi siempre es más
         predecible que token por token con `>>` directo sobre cin.
   ============================================================================= */
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

int main() {
    std::cout << "=== 1) Formateado: >> traduce tipos ===\n";
    std::istringstream entrada("3.14 hola");

    double d = 0;
    entrada >> d;               // "3.14" (tipo double) va al buffer y al doble
    std::cout << "int/double parseado: " << d << "; quedo en el stream: ";
    std::string restante;
    entrada >> restante;
    std::cout << restante << "\n";

    std::cout << "\n=== 2) El parseo PARCIAL de >> ===\n";
    std::istringstream parciales("12abc");
    int parte = 0;
    parciales >> parte;         // come "12"; "abc" queda para el proximo
    std::cout << "leido: " << parte << "; quedo: ";
    parciales >> restante;
    std::cout << restante << "\n";
    std::cout << ">> no es un parser estricto: si tu regla es 'entero TODO o nada',\n"
                 "necesitas getline + istringstream (L05-L06)\n";

    std::cout << "\n=== 3) Crudo: peek mira sin comer ===\n";
    std::istringstream comandos("42()");
    int numero = 0;
    comandos >> numero;
    std::cout << "numero: " << numero << "; proximo char: "
              << static_cast<char>(comandos.peek()) << "\n";
    std::cout << "peek NO avanzo: el '(' sigue para el proximo parser\n";

    std::cout << "\n=== 4) Crudo: get char a char (incluye los blancos) ===\n";
    std::istringstream chorro("ab cd");
    int leidos = 0;
    char c = 0;
    while (chorro.get(c)) {     // get(char): crudo: come los BLANCOS TAMBIEN
        std::cout << (c == ' ' ? '_' : c);
        ++leidos;
    }
    std::cout << " | total chars crudos: " << leidos << " (el blanco va incluido)\n";

    std::cout << "\n=== 5) ignore: la limpieza canonica (real esta vez) ===\n";
    std::istringstream falla("999 jaja");
    int n = 0;
    if (falla >> n) {
        std::cout << "int leido: " << n << "; el resto de la linea: '";
        std::string resto;
        std::getline(falla, resto);
        std::cout << resto << "'\n";
    }
    // el caso real: un stream en fail no sigue leyendo hasta clear+ignore:
    std::istringstream rota("hola 42");
    n = 0;
    if (!(rota >> n)) {
        std::cout << "expected fallo (no hay int al principio)\n";
        rota.clear();
        rota.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
        std::cout << "clear+ignore(' ') listo; un >> ahora SIRVE:\n";
        int v2 = 0;
        if (rota >> v2) std::cout << "leido tras sanar: " << v2 << "\n";
    }

    std::cout << "\n=== 6) gcount recuerda la ULTIMA lectura CRUDA ===\n";
    std::istringstream bloque("milho");
    char buffer[16] = {};
    bloque.get(buffer, 16);     // lee hasta '\n' o 15 chars
    std::cout << "get(buffer): '" << buffer << "' con gcount="
              << bloque.gcount() << "\n";
    std::cout << "el crudo te dice CUANTO tomo: el formateado no\n";
    return 0;
}
