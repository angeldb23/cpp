/* =============================================================================
   MÓDULO 12 - SOLUCIÓN 10: JSON mínimo: writer y parser con estado (ej. 10)
   =============================================================================
   L15: el WRITER escapa (comillas, backslash, '\n', '\t'): el mismo
   ESCAPADO del CSV con otro convenio (backslash, no doblar). El
   PARSER es la máquina con cursor: skipEspacios + decidir por el
   primer char + consumo: claves EN ORDEN (otra: nullopt), int por
   from_chars SIN cola (L06) y nada restante tras '}' final. Todo
   fallo: nullopt SIN estado a medias (el Perfil se llena al final).
   ============================================================================= */
#include <charconv>
#include <cctype>
#include <iostream>
#include <optional>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

struct ItemSave {
    std::string nombre;
    int cantidad = 0;
};

struct Perfil {
    std::string jugador;
    int nivel = 1;
    int monedas = 0;
    std::vector<ItemSave> items;
};

// 1) el escape JSON: backslash ANTEPUESTO (el CSV doblaba: el espejo):
std::string escaparJson(const std::string& s) {
    std::string fuera = "\"";
    for (const char c : s) {
        switch (c) {
            case '"':  fuera += "\\\""; break;
            case '\\': fuera += "\\\\"; break;
            case '\n': fuera += "\\n";  break;
            case '\t': fuera += "\\t";  break;
            default:   fuera += c; break;
        }
    }
    fuera += '"';
    return fuera;
}

// 2) el writer minificado, SIEMPRE con el mismo orden de claves:
std::string serializar(const Perfil& p) {
    std::string out = "{\"jugador\":" + escaparJson(p.jugador);
    out += ",\"nivel\":" + std::to_string(p.nivel);
    out += ",\"monedas\":" + std::to_string(p.monedas);
    out += ",\"items\":[";
    for (size_t i = 0; i < p.items.size(); ++i) {
        if (i > 0) out += ',';
        out += "{\"nombre\":" + escaparJson(p.items[i].nombre);
        out += ",\"cantidad\":" + std::to_string(p.items[i].cantidad) + "}";
    }
    out += "]}";
    return out;
}

// ----- el parser con cursor (L15: un char por vez, estado explícito) -----
class CursorJson {
public:
    explicit CursorJson(const std::string& s) : s_(s) {}

    void saltar() {
        while (i_ < s_.size() &&
               std::isspace(static_cast<unsigned char>(s_[i_]))) ++i_;
    }
    bool hay() const { return i_ < s_.size(); }
    bool come(char c) {
        saltar();
        if (!hay() || s_[i_] != c) return false;
        ++i_;
        return true;
    }
    char actual() const { return s_[i_]; }

    bool leerInt(int& valor) {                 // SOLO [+-]?[0-9]+ (L06)
        saltar();
        const size_t ini = i_;
        if (hay() && (s_[i_] == '-' || s_[i_] == '+')) ++i_;
        while (hay() &&
               std::isdigit(static_cast<unsigned char>(s_[i_]))) ++i_;
        if (i_ == ini) return false;           // no hay dígitos
        int v = 0;
        const auto r = std::from_chars(s_.data() + ini, s_.data() + i_, v);
        if (r.ec != std::errc()) return false;
        valor = v;
        return true;
    }

    bool leerString(std::string& out) {        // '"' + body con escapes
        saltar();
        if (!hay() || s_[i_] != '"') return false;
        ++i_;
        bool cerro = false;
        while (hay()) {
            const char c = s_[i_];
            ++i_;
            if (c == '"') { cerro = true; break; }
            if (c == '\\' && hay()) {
                const char e = s_[i_];
                ++i_;
                switch (e) {
                    case '"':  out += '"'; break;
                    case '\\': out += '\\'; break;
                    case 'n':  out += '\n'; break;
                    case 't':  out += '\t'; break;
                    default:   return false;   // escape desconocido: NO pasa
                }
                continue;
            }
            out += c;
        }
        return cerro;                          // sin cierre: string no
    }

private:
    const std::string& s_;
    size_t i_ = 0;
};

// un ítem: {"nombre":<str>,"cantidad":<int>} (claves en orden):
bool parsearItem(CursorJson& cur, ItemSave& item) {
    std::string clave;
    if (!cur.come('{')) return false;
    if (!cur.leerString(clave) || clave != "nombre") return false;
    if (!cur.come(':')) return false;
    if (!cur.leerString(item.nombre)) return false;
    if (!cur.come(',')) return false;
    clave.clear();
    if (!cur.leerString(clave) || clave != "cantidad") return false;
    if (!cur.come(':')) return false;
    if (!cur.leerInt(item.cantidad)) return false;
    return cur.come('}');
}

// la lista de ítems: [] o [{...},{...}] (espacios tolerados entre tokens):
std::optional<std::vector<ItemSave>> parsearItems(CursorJson& cur) {
    if (!cur.come('[')) return std::nullopt;
    std::vector<ItemSave> items;
    cur.saltar();
    if (cur.hay() && cur.actual() == ']') {    // lista VACÍA: 0 ítems
        return cur.come(']') ? std::optional{items} : std::nullopt;
    }
    while (cur.hay()) {
        ItemSave item;
        if (!parsearItem(cur, item)) return std::nullopt;
        items.push_back(item);
        cur.saltar();
        if (cur.hay() && cur.actual() == ',') {
            cur.come(',');                     // hay otro ítem después
            continue;
        }
        break;
    }
    return cur.come(']') ? std::optional{items} : std::nullopt;
}

// 3) el parser completo: cuatro claves EN ORDEN y nada restante:
std::optional<Perfil> parsear(const std::string& texto) {
    Perfil perfil;
    CursorJson cur(texto);
    if (!cur.come('{')) return std::nullopt;

    std::string clave;
    if (!cur.leerString(clave) || clave != "jugador") return std::nullopt;
    if (!cur.come(':')) return std::nullopt;
    if (!cur.leerString(perfil.jugador)) return std::nullopt;
    if (!cur.come(',')) return std::nullopt;

    clave.clear();
    if (!cur.leerString(clave) || clave != "nivel") return std::nullopt;
    if (!cur.come(':')) return std::nullopt;
    if (!cur.leerInt(perfil.nivel)) return std::nullopt;
    if (!cur.come(',')) return std::nullopt;

    clave.clear();
    if (!cur.leerString(clave) || clave != "monedas") return std::nullopt;
    if (!cur.come(':')) return std::nullopt;
    if (!cur.leerInt(perfil.monedas)) return std::nullopt;
    if (!cur.come(',')) return std::nullopt;

    clave.clear();
    if (!cur.leerString(clave) || clave != "items") return std::nullopt;
    if (!cur.come(':')) return std::nullopt;
    auto items = parsearItems(cur);
    if (!items) return std::nullopt;
    perfil.items = *items;

    if (!cur.come('}')) return std::nullopt;
    cur.saltar();
    if (cur.hay()) return std::nullopt;        // texto restante: NO
    return perfil;
}

// ============= TESTS =============
TEST_CASE(EscaparJsonExacto) {
    ASSERT_EQ(escaparJson("a"), "\"a\"");
    ASSERT_EQ(escaparJson("a\"b"), "\"a\\\"b\"");
    ASSERT_EQ(escaparJson("a\\b"), "\"a\\\\b\"");
    ASSERT_EQ(escaparJson("linea uno\nlista dos"), "\"linea uno\\nlista dos\"");
    ASSERT_EQ(escaparJson("tab\taqui"), "\"tab\\taqui\"");
}

TEST_CASE(SerializarFormatoExacto) {
    const Perfil p{"ana", 3, 120, {{"pocion", 2}, {"mapa", 1}}};
    ASSERT_EQ(serializar(p),
              "{\"jugador\":\"ana\",\"nivel\":3,\"monedas\":120,"
              "\"items\":[{\"nombre\":\"pocion\",\"cantidad\":2},"
              "{\"nombre\":\"mapa\",\"cantidad\":1}]}");
    const Perfil v{};
    ASSERT_EQ(serializar(v),
              "{\"jugador\":\"\",\"nivel\":1,\"monedas\":0,\"items\":[]}");
}

TEST_CASE(VueltaCompletaConDatosFeos) {
    Perfil p{"dijo \"hola\" \\ fin\t;;", 8, -3,
             {{"con\nsalto", 7}, {"con\"comilla", 4}}};
    const std::string texto = serializar(p);
    const auto q = parsear(texto);
    ASSERT_TRUE(static_cast<bool>(q));
    ASSERT_EQ(q->jugador, p.jugador);
    ASSERT_EQ(q->nivel, 8);
    ASSERT_EQ(q->monedas, -3);
    ASSERT_EQ(q->items.size(), 2u);
    ASSERT_EQ(q->items[0].nombre, "con\nsalto");
    ASSERT_EQ(q->items[0].cantidad, 7);
    ASSERT_EQ(q->items[1].nombre, "con\"comilla");
    ASSERT_EQ(q->items[1].cantidad, 4);
}

TEST_CASE(ParserToleraEspaciosEntreTokens) {
    const auto p = parsear(" {  \"jugador\" : \"ana\" , \"nivel\" : 4 , "
                           "\"monedas\" : 20 , \"items\" : [ { \"nombre\" : "
                           "\"mapa\" , \"cantidad\" : 1 } ]  } ");
    ASSERT_TRUE(static_cast<bool>(p));
    ASSERT_EQ(p->jugador, "ana");
    ASSERT_EQ(p->nivel, 4);
    ASSERT_EQ(p->monedas, 20);
    ASSERT_EQ(p->items.size(), 1u);
    ASSERT_EQ(p->items[0].nombre, "mapa");
}

TEST_CASE(MalformadosDevuelvenNulopt) {
    ASSERT_FALSE(parsear("").has_value());
    ASSERT_FALSE(parsear("[1,2]").has_value());
    ASSERT_FALSE(parsear("{\"jugador\":\"ana\"}").has_value());
    ASSERT_FALSE(parsear("{\"otro\":1,\"nivel\":1,\"monedas\":0,\"items\":[]}")
                     .has_value());
    ASSERT_FALSE(parsear("{\"jugador\":\"a\",\"nivel\":1,\"monedas\":3,}")
                     .has_value());
    ASSERT_FALSE(parsear(
        "{\"jugador\":\"a\",\"nivel\":1,\"monedas\":0,\"items\":[]} crash")
        .has_value());
    ASSERT_FALSE(parsear(
        "{\"jugador\":\"a\",\"nivel\":\"2\",\"monedas\":0,\"items\":[]}")
        .has_value());
}

TEST_CASE(ElParserNoDejaEstadoAMediaEstructura) {
    for (const char* podrida : {
             "{\"jugador\":\"a\",\"nivel\":1",
             "{\"jugador\":\"a\",\"nivel\":1,\"monedas\":2,\"items\":[{\"nombre\"",
         }) {
        ASSERT_FALSE(parsear(podrida).has_value());
    }
}

int main() {
    std::cout << "Ejecutando pruebas para Solucion 10 (JSON minimo)...\n";
    return RUN_ALL_TESTS();
}
