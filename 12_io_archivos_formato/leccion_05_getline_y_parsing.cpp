/* =============================================================================
   MÓDULO 12 - LECCIÓN 05: getline, parsing de líneas y el cóctel >> + getline
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       std::getline(stream, linea) lee desde el stream HASTA '\n' (o EOF)
       y te entrega toda la línea como std::string SIN el '\n'. Es la
       operación base de todo parser de línea: tomá la línea, después
       lo tratás con el parse que quieras.

       La trampa más común (de novicio a senior: todo un semestre):
       mezclar `>>` con getline SIN limpiar:
         cin >> numero;          // come "42", deja el '\n' EN el stream
         std::getline(cin, s);   // ve el '\n' como línea completa: ""
       Regla de cocina: si tu interfaz lee números y después líneas,
       después de cada `>>` comé el '\n' pendiente:
         cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
       (y clear() primero si el `>>` falló: L02 lo introduce, L06 lo
       arma como receta completa.)

       getline funciona con CUALQUIER istream: cin en vivo, ifstream de
       archivo (L07) o istringstream: un string que actúa de archivo.
       La clave testable del módulo: TODO el pipeline se escribe contra
       std::istream&, y los tests corren sobre istringstream en vez de
       archivos de verdad: el tipo es el mismo, sin disco de por medio
       (los filepaths se testean en L07+).

       Parsing de una línea dentro de un string: 3 estilos clásicos:
         a) istringstream en la línea + >> para tokens simples
            ("42 17 foca" -> dos ints y un string).
         b) getline(linea, campo, ',') para CAMPOS delimitados (CSV, L14).
         c) buscar con find + substr (módulo 03) delimitadores raros o
            reglas no triviales ("clave=valor" con '=' en los valores).
       La distinción importa: >> es para tokens SEPARADOS DE BLANCOS;
       el problema real casi siempre es otro separador.

       getline devuelve el stream: `while (std::getline(f, s))` es el
       idioma de leer TODO el archivo. Detalle finito: si la última
       línea no tiene '\n', getline la entrega igual; si ya llegó al
       final y no hay NADA, devuelve eofbit+failbit y string vacío:
       línea inexistente NO es lo mismo que línea vacía.
   2. ¿QUÉ PASA POR DEBAJO?
       getline es no-formateado: lee char a char (o bloque a bloque del
       buffer) hasta toparse con '\n' o quedarte sin entrada; el string
       destino crece a medida (resize a demanda hasta capacidad).
       Con istringstream el "archivo" es un string: mismo API, mismo
       cursor interno. Y las dos situaciones de la última getline
       generan ESTADOS DISTINTOS: (línea entregada sin '\n' final:
       solo eofbit) vs (no había línea: eofbit + failbit y string
       vacío). Por eso el check canon es `while (getline(...))` y no
       `s.empty()` solo.
   3. BUENAS PRÁCTICAS:
       - Estructura canon de un parser de líneas:
         while (getline(linea)) { if (linea.empty() || linea[0]=='#')
         continue; parse(linea); }: salta vacías y comentarios gratis.
       - NO parses números con std::stoi sobre pedazos sueltos: stoi
         LANZA; istringstream con `>>` no lanza: la señal es el estado
         del stream (L06).
       - istringstream y su hermano ostringstream: reutilízales en los
         tests; el disco de verdad solo en los tests de file paths
         (L07 en adelante).
       - El texto de Windows llega con '\r' antes del '\n': la línea
         termina en "...\r": escapá el '\r' (o hacer trim de blancos
         al final) ANTES de parsear (el ejercicio del CSV lo pisa).
   ============================================================================= */
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// (instructivo) el pipeline canonico de parsing, escrito contra istream&
std::vector<int> parsearNumerosPorLinea(std::istream& in) {
    std::vector<int> numeros;
    std::string linea;
    while (std::getline(in, linea)) {
        if (linea.empty() || linea[0] == '#') continue;  // skip linea vacia y comentario
        std::istringstream fila(linea);
        int n = 0;
        if (fila >> n) numeros.push_back(n);
    }
    return numeros;
}

int main() {
    std::cout << "=== 1) getline toma la linea ENTERA (a diferencia del >>) ===\n";
    std::string linea;
    std::istringstream enquivo("una frase con espacios\nsegunda linea");
    std::getline(enquivo, linea);
    std::cout << "getline #1: '" << linea << "' (chars: " << linea.size() << ")\n";
    std::getline(enquivo, linea);
    std::cout << "getline #2: '" << linea << "'\n";

    std::cout << "\n=== 2) LA trampa: >> deja el '\\n' en el stream ===\n";
    std::istringstream trapas("42\nsoy la linea que sigue");
    int numero = 0;
    trapas >> numero;
    std::getline(trapas, linea);
    std::cout << "tras >>, getline #1 dio: '" << linea << "'  <- VACIA: era el \\n\n";
    std::getline(trapas, linea);
    std::cout << "getline #2 dio:           '" << linea << "'  <- la data\n";
    trapas.clear();
    // Cookbook: si en vez de getline usabas la linea y encontraste el bug:
    // el ignore las SALVA: cin.ignore(...) despues del >>.
    std::istringstream limpio("42\nsoy la linea que sigue");
    limpio >> numero;
    limpio.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // el FIX
    std::getline(limpio, linea);
    std::cout << "con ignore:               '" << linea << "'  <- directo\n";

    std::cout << "\n=== 3) el parser corre PROBADO en istringstream ===\n";
    std::istringstream script("# comentario y vacios\n\n7\n42\n3.5x skipped\n");
    const std::vector<int> nums = parsearNumerosPorLinea(script);
    std::cout << "numeros parseados: ";
    for (int n : nums) std::cout << n << ' ';
    std::cout << "(la '3.5x' fallo y se ignoro: parse robusto)\n";

    std::cout << "\n=== 4) tokens separados por blancos: getline != >> ===\n";
    std::istringstream tokens("vida 100 mana 40");
    std::string etiqueta;
    int valor = 0;
    tokens >> etiqueta >> valor;   // "vida", 100
    std::cout << "par: '" << etiqueta << "'=" << valor << '\n';
    tokens >> etiqueta >> valor;
    std::cout << "par: '" << etiqueta << "'=" << valor << '\n';
    std::cout << "(>> salta blancos: ideal 'etiqueta valor'; con DELIMITADOR "
                 "distinto hay getline(linea, campo, ',') o find: L14)\n";

    std::cout << "\n=== 5) CRLF de Windows: el '\\r' vestigial ===\n";
    std::istringstream windows("una\r\ndos\r\n");
    std::getline(windows, linea);      // trae "una\r" (el '\n' se necesita)
    std::cout << "1er getline de texto CRLF: '" << linea << "', size="
              << linea.size() << " (hay un \\r colgado)\n";
    if (!linea.empty() && linea.back() == '\r') {
        linea.pop_back();
    }
    std::cout << "tras trim: '" << linea << "'\n";
    return 0;
}
