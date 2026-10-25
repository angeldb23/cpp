/* =============================================================================
   MÓDULO 12 - EJERCICIO 09: CSV de scores con ESCAPE (L14)
   =============================================================================
   DIFICULTAD: Media-Alta
   CONSIGNA (tema 8 de TEMAS.md: serialización simple: CSV a mano):
     La tabla de scores viaja como CSV (el formato intercambiable con
     planillas) y el dato de afuera te arruina la fila: el nombre con
     "," o con '"'. Por eso el CSV del curso es el RFC 4180 chico
     (L14): SIEMPRE comillas y comilla interna DOBLADA:
       1. escaparCampo(campo): el campo ENTRANTE entre comillas, y
          cada '"' interna DOBLADA ("dijo ""hola""": L14). Todo campo
          va citado, aunque no tenga nada raro: uniforme = sin casos.
       2. lineaCsv(nombre, puntos): UNA fila SIN '\n' final: el campo
          escapado + ',' + el int (el '\n' lo agrega quien arma el
          archivo, como en el ej02).
       3. parsearRegistro(linea): optional<Registro>: el ESCÁNER con
          estado "dentro de comillas" de la L14: comilla inicial, body
          con "" por comilla escapada, coma y el int COMPLETO al final
          (L06). nullopt ante: sin comilla inicial, sin cierre, sin
          coma, puntos con basura ("7x"), o línea vacía.
       4. guardarCsv(ruta, registros): header "nombre,puntos\n" y una
          fila por registro + '\n', validando CADA operación (L12:
          apertura + cada write). false ante cualquier falla.
       5. cargarCsv(ruta, ok): el loader TOLERANTE (L14): tira el
          header, y la línea que no parsea se SALTA (los mods pueden
          escribir filas raras, pero el archivo abre igual). Ante un
          solo dato el CSV devuelve... la LISTA. ok=false si no abre.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_09_csv_con_escape.cpp -o ej09 && ./ej09
   ============================================================================= */
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

struct Registro {
    std::string nombre;
    int puntos = 0;
};

// TODO 1: TODO entre comillas, '"'-interna DOBLADA (L14):
std::string escaparCampo([[maybe_unused]] const std::string& campo) {
    // TU CODIGO AQUI
    return "";    // placeholder
}

// TODO 2: fila SIN '\n': campo escapado + ',' + int:
std::string lineaCsv([[maybe_unused]] const std::string& nombre,
                     [[maybe_unused]] int puntos) {
    // TU CODIGO AQUI
    return "";    // placeholder
}

// TODO 3: el escáner con estado de la L14 (comillas + estricto L06):
std::optional<Registro> parsearRegistro([[maybe_unused]] const std::string& linea) {
    // TU CODIGO AQUI
    return std::nullopt;    // placeholder
}

// TODO 4: header + filas; validar CADA paso (L12):
bool guardarCsv([[maybe_unused]] const std::string& ruta,
                [[maybe_unused]] const std::vector<Registro>& registros) {
    // TU CODIGO AQUI
    return false;    // placeholder
}

// TODO 5: loader: tira header, salteaIENo-parsea, resto en lista:
std::vector<Registro> cargarCsv([[maybe_unused]] const std::string& ruta,
                                [[maybe_unused]] bool* ok) {
    // TU CODIGO AQUI
    return {};    // placeholder
}

// ============= TESTS =============
TEST_CASE(EscaparDoblaLasComillasInternas) {
    ASSERT_EQ(escaparCampo("sencillo"), "\"sencillo\"");
    ASSERT_EQ(escaparCampo(""), "\"\"");        // vacío también va citado
    ASSERT_EQ(escaparCampo("a\"b"), "\"a\"\"b\"");
    ASSERT_EQ(escaparCampo("a, b"), "\"a, b\"");        // la coma no escapa
    ASSERT_EQ(escaparCampo("dijo \"hola\""), "\"dijo \"\"hola\"\"\"");
}

TEST_CASE(LineaCsvExacta) {
    ASSERT_EQ(lineaCsv("ana", 3), "\"ana\",3");
    ASSERT_EQ(lineaCsv("co\", lvl", 120), "\"co\"\", lvl\",120");
}

TEST_CASE(ParsearElCaminoFeliz) {
    const auto r = parsearRegistro("\"mapas \"\"raros\"\", v2\",7");
    ASSERT_TRUE(static_cast<bool>(r));
    ASSERT_EQ(r->nombre, "mapas \"raros\", v2");
    ASSERT_EQ(r->puntos, 7);

    const auto s = parsearRegistro("\"bocón\",-4");   // con signo
    ASSERT_TRUE(static_cast<bool>(s));
    ASSERT_EQ(s->nombre, "bocón");
    ASSERT_EQ(s->puntos, -4);
}

TEST_CASE(ParsearRechazaLoPodrido) {
    ASSERT_FALSE(parsearRegistro("").has_value());              // vacía
    ASSERT_FALSE(parsearRegistro("sin comillas,3").has_value()); // campo crudo
    ASSERT_FALSE(parsearRegistro("\"sin cierre,3").has_value()); // comilla abierta
    ASSERT_FALSE(parsearRegistro("\"ana\"").has_value());       // sin coma
    ASSERT_FALSE(parsearRegistro("\"ana\",\"3x\"").has_value()); // no es int
    ASSERT_FALSE(parsearRegistro("\"ana\",3 onzas").has_value());// int con cola
}

TEST_CASE(RoundTripEnDisco) {
    const std::string ruta = "/tmp/opencode/curso12_ej09_save.csv";
    const std::vector<Registro> data{
        {"ana", 120}, {"bo, la", 88}, {"dijo \"hola\"", -2}};
    ASSERT_TRUE(guardarCsv(ruta, data));
    bool ok = false;
    const auto v = cargarCsv(ruta, &ok);
    ASSERT_TRUE(ok);
    ASSERT_EQ(v.size(), 3u);
    ASSERT_EQ(v[0].nombre, "ana");
    ASSERT_EQ(v[1].nombre, "bo, la");           // la COMA sobrevivió
    ASSERT_EQ(v[2].nombre, "dijo \"hola\"");    // la comilla sobrevivió
    ASSERT_EQ(v[2].puntos, -2);
    // y el header está en la primera línea, siempre:
    std::ifstream f(ruta);
    std::string header;
    std::getline(f, header);
    ASSERT_EQ(header, "nombre,puntos");
}

TEST_CASE(ElLoaderEsToleranteConLineasProblemáticas) {
    const std::string ruta = "/tmp/opencode/curso12_ej09_raro.csv";
    ASSERT_TRUE(guardarCsv(ruta, std::vector<Registro>{{"ana", 5}, {"dos", 6}}));
    {
        std::ofstream f(ruta, std::ios::app);   // un mod Injecting basura:
        f << "fila sin comillas\n";
    }
    ASSERT_EQ(cargarCsv(ruta, nullptr).size(), 2u);   // salta la podrida
}

TEST_CASE(GuardarYcargarAnteAperturaFallida) {
    ASSERT_FALSE(guardarCsv("/curso12_ej09/no_existe/x.csv", {}));
    bool ok = true;
    ASSERT_TRUE(cargarCsv("/curso12_ej09/no_existe/x.csv", &ok).empty());
    ASSERT_FALSE(ok);
}

int main() {
    return RUN_ALL_TESTS();
}
