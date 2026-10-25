/* =============================================================================
   MÓDULO 12 - LECCIÓN 14: serialización a mano I: CSV, escape y operator<<
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       Serializar es guardar TU ESTADO como secuencia de caracteres que
       después PODÉS volver a leer (deserializar). Antes de tocar
       librerías, la base del problema es la misma en todos los formatos:
         1) elegir UNA representación de cada tipo (número, texto, lista);
         2) elegir delimitadores (campo, registro, lista);
         3) ESCAPAR todo dato que contenga un delimitador. Esta parte es
            la que todos olvidan: el día que un nombre del jugador lleva
            coma, el parser muere si no hay escape.

       CSV (comma-separated values) es el formato mínimo:
           nombre,cantidad
       El estándar de hecho es el RFC 4180 y sus tres reglas:
         - un campo puede ir entre comillas: "text, con coma"
         - una comilla interna se escapa DOBLÁNDOLA: "dijo ""hola"""
         - línea nueva: '\n' separa registros.
       ¿Por qué SIEMPRE escapar? Por los datos de afuera: nombres del
       jugador, nombres de mods, comentarios. El escapado es una función
       se escribe UNA vez (acá) y se usa SIEMPRE: el día que aparece el
       dato con coma, el formato no cambia: la función ya estaba lista.

       Y acá se cruzan los módulos anteriores:
         - operator<< propio de la clase (módulo 05): una función pura
           "estado -> la línea del CSV". Los sends del formateo están
           claros: SIEMPRE el mismo orden, SIEMPRE la misma sintaxis.
         - getline(linea, campo, ',') para el parseo (L05).
         - istringstream para la línea (L05).
         - validación de cada lectura con el bool del stream (L12).
       La lectura y la escritura son DOS funciones SEPARADAS: nunca una
       que haga las dos cosas en un solo paso.

       El uso del operator<< del tipo: darle a la clase un lenguaje
       de salida:
         std::ostream& operator<<(std::ostream& out, const Item& it)
       se puede componer: `f << item << '\n'` y también `cout << item`:
       el mismo DSL sirve para archivo y para consola (los operadores
       toman std::ostream&: sirven para CUALQUIER ostream, no solo
       archivos!).

   2. ¿QUÉ PASA POR DEBAJO?
       El escape DOBLA cada comilla al escribir y el parser las vuelve
       a juntar al leer: `dijo ""hola""` queda en disco tal literal y,
       al leer, el escáner con estado "dentro de comillas" entiende
       que esas dos comillas juntas son UNA comilla real, no el cierre
       del campo. Escapar y des-escapar son LAS DOS CARAS de la misma
       regla.
            getline(linea, campo, ',') simplifica el SPLIT: pero tiene una
       trampa conocida: NO entiende de comillas: un campo con
       "" doblada lo parte en pedazos. El parse serio escanea con
       estado "estoy dentro de comillas" (la técnica del ejercicio
       8): el estado del escaneo es la única manera de distinguir
       "comilla escapada" de "comilla de cierre".
       Los "campos multi-línea" del RFC (un '\n' dentro entre comillas)
       NO los vemos acá: el CSV es de filas planas; para estado
       anidado con saltos, JSON (L15).

   3. BUENAS PRÁCTICAS:
       - SIEMPRE escribir el HEADER (nombres de columnas) en la primera
         línea: el CSV lo va a abrir "alguien" (tu vos del año 2027 o el
         Excel de alguien más: te lo van a agradecer).
       - Escapado en AMBOS lados (al escribir y al leer).
       - NO meter '\n' dentro de los campos: el CSV es de filas planas;
         para texto libre con saltos: JSON (L15).
       - Los operator<< servibles para reporte/log/consola; el CSV es el
         ARCHIVO de datos intercambiable con hasta una planilla.
       - NO usar std::format para EL campo con COMA: format no escapa;
         el escape es TU función aparte (acá abajo).
   ============================================================================= */
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace csv {
    // escape: TODO campo va ENTRE COMILLAS, las internas se doblan
    std::string escapar(const std::string& campo) {
        std::string fuera = "\"";
        for (char c : campo) {
            if (c == '"') fuera += "\"\"";
            else fuera += c;
        }
        fuera += "\"";
        return fuera;
    }

    struct Item {
        std::string nombre;
        int cantidad = 0;
    };

    // operator<< del item: una línea CSV ENTERA SIN '\n' final:
    // (el '\n' lo agrega quien arma el archivo: el operador formatea UN item)
    std::ostream& operator<<(std::ostream& out, const Item& item) {
        return out << escapar(item.nombre) << ',' << item.cantidad;
    }

    // LA TRAMPA del getline(linea, campo, ','): NO entiende de comillas:
    // un nombre con "" doblandola partiria el campo en pedazos. El parse
    // serio se hace con un ESCANEO con estado "estoy dentro de comillas":
    // el mismo algoritmo de los lexers (y el mismo fondo del parser del
    // JSON de la L15: estado + un char por vez).
    std::optional<Item> parsearLinea(const std::string& linea) {
        size_t i = 0;
        // saltar blancos frente a la comilla inicial:
        while (i < linea.size() &&
               std::isspace(static_cast<unsigned char>(linea[i]))) ++i;
        if (i >= linea.size() || linea[i] != '"') return std::nullopt;
        ++i;    // entramos al campo citado
        std::string nombre;
        bool comillasCerradas = false;
        while (i < linea.size()) {
            const char c = linea[i];
            if (c == '"') {
                if (i + 1 < linea.size() && linea[i + 1] == '"') {
                    nombre += '"';      // comilla DOBLADA: escapa UNA
                    i += 2;
                } else {
                    comillasCerradas = true;
                    ++i;                // la comilla de cierre se come
                    break;
                }
            } else {
                nombre += c;
                ++i;
            }
        }
        if (!comillasCerradas) return std::nullopt;     // campo sin cierre
        if (i >= linea.size() || linea[i] != ',') return std::nullopt;
        ++i;
        // el 2do campo es un int (y SOLO un int: parse estricto L06):
        std::istringstream resto(linea.substr(i));
        int cantidad = 0;
        if (!(resto >> cantidad)) return std::nullopt;
        char sobra = 0;
        if (resto.get(sobra) &&
            !std::isspace(static_cast<unsigned char>(sobra)))
            return std::nullopt;
        return Item{nombre, cantidad};
    }

    // el PIPE completo: guardar y cargar con validación de cada paso:
    bool guardar(const std::string& ruta, const std::vector<Item>& items) {
        std::ofstream f(ruta, std::ios::trunc);
        if (!f) return false;
        f << "nombre,cantidad\n";                   // SIEMPRE el header
        for (const Item& item : items) {
            f << item << '\n';
            if (!f) return false;                    // validar CADA write (L12)
        }
        return true;
    }

    std::vector<Item> cargar(const std::string& ruta, bool* ok) {
        std::vector<Item> items;
        std::ifstream f(ruta);
        if (ok) *ok = static_cast<bool>(f);
        if (!f) return items;
        std::string linea, header;
        std::getline(f, header);                    // tirar el header
        while (std::getline(f, linea)) {
            if (linea.empty()) continue;
            if (auto item = csv::parsearLinea(linea)) {   // llamado maya
                items.push_back(*item);
            }
        }
        return items;
    }
} // namespace csv

int main() {
    std::cout << "=== 1) escapar (y volver atras con el parser) ===\n";
    std::string cruda = "palas \"mediana\", secas";
    std::string escapada = csv::escapar(cruda);
    std::cout << "cruda:    " << cruda << "\n";
    std::cout << "escapada: " << escapada << "\n";
    if (auto item = csv::parsearLinea(escapada + ",1")) {
        std::cout << "parseada bie la vuelta reinco: '" << item->nombre << "'\n";
    }

    std::cout << "\n=== 2) el eslabon los campos ===\n";
    std::vector<csv::Item> data{
        {"espada", 1}, {"polvo de vida", 12}, {"mapas \"raros\", v2", 3}};
    const std::string ruta = "/tmp/opencode/curso12_leccion14.csv";
    std::cout << "guardar: " << (csv::guardar(ruta, data) ? "ok" : "fallo") << "\n";

    std::cout << "=== 3) operador a conseguir el stream BUENO ===\n";
    for (const csv::Item& item : data) {
        std::cout << "  hacia cout: " << item << "\n";
    }

    std::cout << "\n=== 4) volver a leer el archivo desde el disco ===\n";
    bool ok = false;
    const std::vector<csv::Item> v = csv::cargar(ruta, &ok);
    std::cout << "cargar=" << (ok ? "true" : "false") << ", " << v.size()
              << " items:\n";
    for (const csv::Item& item : v) {
        std::cout << "  nombre real: '" << item.nombre << "' x"
                  << item.cantidad << "\n";
    }
    std::cout << "el nombre 'mapas \"raros\", v2' sobrevivio la enganada: "
                 "la COMA y las comillas no rompieron la fila\n";
    return 0;
}
