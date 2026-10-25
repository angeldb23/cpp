/* =============================================================================
   MÓDULO 12 - LECCIÓN 15: serialización a mano II: JSON mínimo y parser con estado
   =============================================================================
   1. EXPLICACIÓN CONCEPTUAL:
       El CSV de L14 sirve para filas planas. El día que tu estado es
       ANIDADO (inventario con ítems, cada ítem con atributos), el
       formato estructurado mínimo es JSON:
         { "juego": "curso12", "nivel": 3,
           "items": [ { "nombre": "pocion", "tipo": "cura" } ] }
       Sus piezas:
         - objeto  { ... }  -> map<string, valor>  (el estado del nivel)
         - lista   [ ... ]  -> vector<valor>       (ítems, waypoints)
         - string, entero, real, booleano, null (los tipos de dato)
         - texto con escapes: \" \\ \n \t \uXXXX. El CSV doblaba las
           comillas; acá se anteponga un backslash: OTRO convenio,
           mismo concepto: el ESCAPADO.

       ¿Por qué a mano si existen librerías? Dos razones de curso:
         a) entender QUÉ ES un formato: algo que sabe escapar y anidar.
            Una librería JSON es el constructor + parser de estas piezas
            ya armadas: si sabés construirlas, la API de la librería
            deja de ser magia.
         b) el parser es la mejor escuela de "código con estado": un
            cursor, un carácter por vez, una decisión por tipo: es la
            mecánica del LEXER real.

       El patrón del parser manual, que es el mismo de todos:
         1) saltar espacios,
         2) mirar el próximo carácter (¿{, [, ", d, t, n?),
         3) despejar al parse del tipo que toca (string, int, lista),
         4) consumir el cursor y devolver el valor.
       Con nesting (item dentro de lista dentro de objeto) aparece la
       RECURSIÓN natural del módulo 04: la función se llama a sí misma
       para el valor interno (del "stack" de estados: la pila de stack
       real del programa, exactamente como en el recursion frame del
       módulo 02). El parser abajo ES DIDÁCTICO: string + int con
       escape, y el objeto "save" con sus tres campos: NO es un parser
       JSON completo (boolean/null/lista completa): eso lo compra uno
       (nlohmann/json): aquí lo que se aprende es la MECHANICA y la
       CUOTA de "estado o error".

       LA TRAMPA final: un '\n' real dentro de un string DEBE ir como
       \n (dos caracteres): si escribís el salto literal, el archivo
       queda con UN JSON INVÁLIDO (cualquier parser externo lo rechazá):
       es el MISMO error que la L14 escapando en el CSV (el escape es
       de la SERIALIZACIÓN, no del formato (format y std::format no
       saben escapar: esa lógica es TU función: escaparString)).

   2. ¿QUÉ PASA POR DEBAJO?
       El writer (aJson) arma el texto con INDENTACIÓN (pretty print),
       la cual crece el archivo para humanos; la versión minífica (sin
       indentación) es el autosave por frame (mismos datos, dos estilos).
       La lección de "modelos de estado": el parser abajo tiene un
       CURSOR (cursor_) que va avanzando: es el mismo modelo del estado
       oculto del stream (L01: un cursor interno invisible): por eso
       parseString se puede llamar dos veces seguidas y avanza solo: el
       ejemplo de la sección 5 te lo muestra en vivo.
       El "por qué anidando": TODO parser real (de JSON, de GLSL, de
       YAML, de LOS archivos de motor REALES) usa este skeleton:
       saltar espacios + decidir por el primer char + despechar: no hay
       YAML ż no hay GLSL: LA RESSETEA es: la maquina ESTADO (un bit
       mínimo más del "estado del lexer").
   3. BUENAS PRÁCTICAS:
       - Escapar SIEMPRE. Y versionar el save JSON como en el binario
         L08: la version evita el "era otro esquema": un campo nuevo en
         el future no rompe la iteración de los mods viejos.
       - PARSE devolviendo optional: el parser que no informa el fallo
         NO SE USA (la especialidad de la L06/L12).
       - Pretty print solo para el save visual; minify para el autosave.
       - PRODUCCIÓN: librería JSON (nlohmann, rapidjson) por rendimiento
         y utf-8 correcto: aquí se aprende la base; ese es el alcance.
   ============================================================================= */
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace minijson {
    // WRITER: estado -> texto JSON, con escape de comillas, barra y control
    std::string escaparString(const std::string& s) {
        std::string out;
        out += '"';
        for (char c : s) {
            switch (c) {
                case '"':  out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n";  break;
                case '\t': out += "\\t";  break;
                case '\r': out += "\\r";  break;
                default:   out += c; break;   // (utf-8 pasa crudo: curso)
            }
        }
        out += '"';
        return out;
    }

    struct ItemSave {
        std::string nombre;
        int cantidad = 0;
    };

    struct Save {
        int nivel = 1;
        int monedas = 0;
        std::vector<ItemSave> items;
    };

    // writer en memoria: el estado completo con indentación (L03/L04):
    std::string aJson(const Save& save) {
        std::string out = "{\n";
        out += "  \"nivel\": " + std::to_string(save.nivel) + ",\n";
        out += "  \"monedas\": " + std::to_string(save.monedas) + ",\n";
        out += "  \"items\": [\n";
        for (size_t i = 0; i < save.items.size(); ++i) {
            const ItemSave& item = save.items[i];
            const char* coma = (i + 1 < save.items.size()) ? "," : "";
            out += std::format("    {{ {}, {} }}{}\n",
                               "\"nombre\": " + escaparString(item.nombre),
                               "\"cantidad\": " + std::to_string(item.cantidad),
                               coma);
        }
        out += "  ]\n}\n";
        return out;
    }

    bool guardar(const std::string& ruta, const Save& save) {
        std::ofstream f(ruta, std::ios::trunc);
        if (!f) return false;
        f << aJson(save);
        return static_cast<bool>(f);    // L12: validar el write, no el plan
    }

    // PARSER: scanner con ESTADO (cursor) y casos mono-caracter:
    class Parser {
    public:
        explicit Parser(std::string texto) : s(std::move(texto)) {}

        // el punto: parseString + parseInt + come son las PIEZAS; el
        // parse del save completo arma con ellas (didáctico: "nivel"):
        std::optional<int> parseCampo(const std::string& clave) {
            if (come('{')) {                      // esperamos el objeto entero
                auto llave = parseString();       // la clave del primer campo
                if (!llave || *llave != clave || !come(':')) return std::nullopt;
                return parseInt();
            }
            return std::nullopt;
        }

        // parse string con escape: consume "abc\"def" y devuelve el VALOR
        std::optional<std::string> parseString() {
            saltaEspacios();
            if (!come('"')) return std::nullopt;
            std::string out;
            while (cursor_ < s.size()) {
                const char c = s[cursor_];
                if (c == '"') { ++cursor_; return out; }   // comilla de cierre
                if (c == '\\') {                           // ESCAPE: 2 chars
                    if (!parseEscape(&out)) return std::nullopt;
                    continue;
                }
                out += c;
                ++cursor_;
            }
            return std::nullopt;     // nunca llega la comilla de cierre
        }

        std::optional<int> parseInt() {
            saltaEspacios();
            const size_t inicial = cursor_;
            std::istringstream flu(s.substr(cursor_));
            int valor = 0;
            if (!(flu >> valor)) return std::nullopt;
            const size_t avanzado = static_cast<size_t>(flu.tellg());
            if (avanzado == 0) return std::nullopt;
            cursor_ += avanzado;      // el cursor avanza SOLO si falló nada
            (void)inicial;
            return valor;
        }

        void saltaEspacios() {
            while (cursor_ < s.size() &&
                   std::isspace(static_cast<unsigned char>(s[cursor_]))) {
                ++cursor_;
            }
        }

        bool come(char c) {   // come UN char si coincide (token del lexer)
            saltaEspacios();
            if (cursor_ < s.size() && s[cursor_] == c) { ++cursor_; return true; }
            return false;
        }

        size_t cursor_ = 0;   // (de este lado: el ESTADO OCULTO del parser)
    private:
        std::string s;

        bool parseEscape(std::string* out) {   // asume cursor_ en '\\'
            ++cursor_;
            if (cursor_ >= s.size()) return false;
            const char esc = s[cursor_++];
            switch (esc) {
                case '"':  *out += '"';  break;
                case '\\': *out += '\\'; break;
                case 'n':  *out += '\n'; break;
                case 't':  *out += '\t'; break;
                case 'r':  *out += '\r'; break;
                default: return false;     // \uXXXX: queda para la librería
            }
            return true;
        }
    };
} // namespace minijson

int main() {
    std::cout << "=== 1) el escapado ===\n";
    const std::string cruda = "dijo \"listo\" y\\sigue\nen otra linea";
    std::cout << "cruda:  " << cruda << "\n";
    std::cout << "JSON:   " << minijson::escaparString(cruda) << "\n";

    std::cout << "\n=== 2) writer: estado -> JSON con indentación ===\n";
    minijson::Save save;
    save.nivel = 3;
    save.monedas = 250;
    save.items = {{"pocion", 2}, {"mapa", 1}};
    std::cout << minijson::aJson(save);

    std::cout << "=== 3) guardar en archivo ===\n";
    const std::string ruta = "/tmp/opencode/curso12_leccion15.json";
    std::cout << "guardar: " << (minijson::guardar(ruta, save) ? "ok" : "fallo")
              << "\n";

    std::cout << "\n=== 4) parser: string con escapes ===\n";
    minijson::Parser p2("\"ab\\\"cd\\\\niever\\nlives\"");
    auto texto = p2.parseString();
    std::cout << "parseado: " << (texto ? *texto : "<falla>") << "\n";
    std::cout << "(\\\" \\\\ y \\n se volvieron comilla, barra y salto de "
                 "linea REALES)\n";

    std::cout << "\n=== 5) parser: el int y el cursor ===\n";
    minijson::Parser p3("42 100");
    auto a = p3.parseInt();
    std::cout << "int #1: " << (a ? std::to_string(*a) : "?")
              << " (cursor en " << p3.cursor_ << ")\n";
    auto b = p3.parseInt();
    std::cout << "int #2: " << (b ? std::to_string(*b) : "?")
              << " (cursor en " << p3.cursor_ << ")\n";
    std::cout << "el cursor avanza solo: es el modelo de estado oculto "
                 "del stream de la L01\n";
    return 0;
}
