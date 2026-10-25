/* =============================================================================
   MÓDULO 12 - EJERCICIO 10: JSON mínimo: writer y parser con estado (L15)
   =============================================================================
   DIFICULTAD: Alta
   CONSIGNA (tema 8 de TEMAS.md: serialización simple: JSON a mano):
     El save del perfil va en JSON (el anidando ítems no entra en un
     CSV: acá sí vale la estructura). El formato ES TUYO: minificado,
     con las claves SIEMPRE en el mismo orden (comparar con la L15):
         {"jugador":<str>,"nivel":<int>,"monedas":<int>,
          "items":[{"nombre":<str>,"cantidad":<int>},...]}
       1. escaparJson(s): como la L15: '"', '\\', '\n' y '\t'
          anteponiendo backslash: el mismo ESCAPADO del CSV con otro
          convenio (L15).
       2. serializar(perfil): TODO el perfil minificado de la línea de
          arriba, con std::format para los números (L03). items vacío:
          "items":[] (sin espacios).
       3. parsear(texto): optional<Perfil>: el parser CON CURSOR de la
          L15: skipEspacios + expect('{') + clave por clave EN ORDEN
          (jugador, nivel, monedas, items: clave inesperada o
          desordenada: nullopt) + string con escape + int ESTRICTO
          (L06: de [0-9-]+ via from_chars, cola = callo) + lista de
          ítems recursiva y tolera espacios ENTRE tokens. Después de
          '}' final NO puede quedar nada (salvo espacios). Cualquier
          cosa rara: nullopt, y el estado del Save no queda a medias.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_10_json_minimo.cpp -o ej10 && ./ej10
   ============================================================================= */
#include <format>
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

// TODO 1: escapa: '"' -> \", '\\' -> \\, '\n' -> \n, '\t' -> \t (L15):
std::string escaparJson([[maybe_unused]] const std::string& s) {
    // TU CODIGO AQUI
    return "";    // placeholder
}

// TODO 2: el dicto minificado de arriba, SIN espacios de más (L03):
std::string serializar([[maybe_unused]] const Perfil& p) {
    // TU CODIGO AQUI
    return "";    // placeholder
}

// TODO 3: el parser con cursor: claves EN ORDEN, int estricto (L06),
//         espacios tolerados, nada restante al final (L15):
std::optional<Perfil> parsear([[maybe_unused]] const std::string& texto) {
    // TU CODIGO AQUI (helpers adentro del namespace: skipEspacios,
    // leerEntreComillas, leerIntEstricto... la L15 muestra la base)
    return std::nullopt;    // placeholder
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
    const Perfil p{
        "ana", 3, 120, {{"pocion", 2}, {"mapa", 1}}};
    ASSERT_EQ(serializar(p),
              "{\"jugador\":\"ana\",\"nivel\":3,\"monedas\":120,"
              "\"items\":[{\"nombre\":\"pocion\",\"cantidad\":2},"
              "{\"nombre\":\"mapa\",\"cantidad\":1}]}");
    const Perfil v{};    // defaults, sin items:
    ASSERT_EQ(serializar(v),
              "{\"jugador\":\"\",\"nivel\":1,\"monedas\":0,\"items\":[]}");
}

TEST_CASE(VelvetaViajeConDatosFeos) {
    Perfil p{
        "dijo \"hola\" \\ fin\t;;", 8, -3,
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
    ASSERT_FALSE(parsear("[1,2]").has_value());        // no es un objeto
    ASSERT_FALSE(parsear("{\"jugador\":\"ana\"}").has_value()); // sin el resto
    ASSERT_FALSE(parsear("{\"otro\":1,\"nivel\":1,\"monedas\":0,\"items\":[]}")
                     .has_value());                    // primer clave INCORRECTA
    ASSERT_FALSE(parsear("{\"jugador\":\"a\",\"nivel\":1,\"monedas\":3,}")
                     .has_value());                    // coma final extra
    ASSERT_FALSE(parsear(
        "{\"jugador\":\"a\",\"nivel\":1,\"monedas\":0,\"items\":[]} crash")
        .has_value());                                 // texto restante
    ASSERT_FALSE(parsear(
        "{\"jugador\":\"a\",\"nivel\":\"2\",\"monedas\":0,\"items\":[]}")
        .has_value());                                 // nivel NO quiere string
}

TEST_CASE(ElParserNoDejaEstadoAMediaEstructura) {
    // la ausencia de '}' final no devuelven un Perfil desconcertante:
    for (const char* podrida : {
             "{\"jugador\":\"a\",\"nivel\":1",           // cortada a mitad
             "{\"jugador\":\"a\",\"nivel\":1,\"monedas\":2,\"items\":[{\"nombre\"",
         }) {
        ASSERT_FALSE(parsear(podrida).has_value());
    }
}

int main() {
    return RUN_ALL_TESTS();
}
