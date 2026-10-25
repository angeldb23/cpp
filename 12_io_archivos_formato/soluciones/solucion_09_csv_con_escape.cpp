/* =============================================================================
   MÓDULO 12 - SOLUCIÓN 09: CSV de scores con ESCAPE (ej. 09)
   =============================================================================
   L14: escribir y leer son FUNCIONES SEPARADAS; escapar y des-escapar
   son las DOS CARAS de la misma regla (doblada al escribir, distrita al
   leer) y el parseo serio es el ESCÁNER con estado que sabe distinguir
   la comilla escapada de la comilla de cierre. El loader es tolerante:
   la línea que no parsea se salta (el mod podria haberla escrito).
   ============================================================================= */
#include <cctype>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

struct Registro {
    std::string nombre;
    int puntos = 0;
};

// 1) TODO entra entre comillas; la '"'-interna se DOBLA (L14):
std::string escaparCampo(const std::string& campo) {
    std::string fuera = "\"";
    for (const char c : campo) {
        if (c == '"') fuera += "\"\"";
        else fuera += c;
    }
    fuera += "\"";
    return fuera;
}

// 2) UNA fila SIN '\n' (el archivo pone el '\n'):
std::string lineaCsv(const std::string& nombre, int puntos) {
    return escaparCampo(nombre) + ',' + std::to_string(puntos);
}

// 3) el escáner con estado: dentro de comillas o afuera; UNA por vez:
std::optional<Registro> parsearRegistro(const std::string& linea) {
    size_t i = 0;
    while (i < linea.size() &&
           std::isspace(static_cast<unsigned char>(linea[i]))) ++i;
    if (i >= linea.size() || linea[i] != '"') return std::nullopt;
    ++i;                                        // profondamos a la cita
    bool cierro = false;
    std::string nombre;
    while (i < linea.size()) {
        const char c = linea[i];
        if (c == '"') {
            if (i + 1 < linea.size() && linea[i + 1] == '"') {
                nombre += '"';                  // la DOBLADA escapea una
                i += 2;
            } else {
                cierro = true;                  // la de cierre se come
                ++i;
                break;
            }
        } else {
            nombre += c;
            ++i;
        }
    }
    if (!cierro) return std::nullopt;           // campo sin cierre
    if (i >= linea.size() || linea[i] != ',') return std::nullopt;
    ++i;                                        // ya con la coma
    // el 2do campo: UN int COMPLETO (L06: la cola no debe existir):
    std::istringstream resto(linea.substr(i));
    int puntos = 0;
    if (!(resto >> puntos)) return std::nullopt;
    std::string sobra;
    if (resto >> sobra) return std::nullopt;    // "3 onzas": afuera
    return Registro{nombre, puntos};
}

// 4) el header SIEMPRE y cada write validado (L12):
bool guardarCsv(const std::string& ruta, const std::vector<Registro>& registros) {
    std::ofstream f(ruta, std::ios::trunc);
    if (!f) return false;
    f << "nombre,puntos\n";
    if (!f) return false;
    for (const Registro& r : registros) {
        f << lineaCsv(r.nombre, r.puntos) << '\n';
        if (!f) return false;
    }
    f.flush();
    return !f.fail();
}

// 5) el loader tolerante: header afuera, podrida saltada:
std::vector<Registro> cargarCsv(const std::string& ruta, bool* ok) {
    std::vector<Registro> items;
    const auto falla = [&] {
        if (ok) *ok = false;
        return std::vector<Registro>{};
    };
    std::ifstream f(ruta);
    if (!f) return falla();
    if (ok) *ok = true;
    std::string header, linea;
    std::getline(f, header);                    // tirar el header
    while (std::getline(f, linea)) {
        if (linea.empty()) continue;
        if (auto r = parsearRegistro(linea)) items.push_back(*r);
    }
    return items;
}

// ============= TESTS =============
TEST_CASE(EscaparDoblaLasComillasInternas) {
    ASSERT_EQ(escaparCampo("sencillo"), "\"sencillo\"");
    ASSERT_EQ(escaparCampo(""), "\"\"");
    ASSERT_EQ(escaparCampo("a\"b"), "\"a\"\"b\"");
    ASSERT_EQ(escaparCampo("a, b"), "\"a, b\"");
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

    const auto s = parsearRegistro("\"bocón\",-4");
    ASSERT_TRUE(static_cast<bool>(s));
    ASSERT_EQ(s->nombre, "bocón");
    ASSERT_EQ(s->puntos, -4);
}

TEST_CASE(ParsearRechazaLoPodrido) {
    ASSERT_FALSE(parsearRegistro("").has_value());
    ASSERT_FALSE(parsearRegistro("sin comillas,3").has_value());
    ASSERT_FALSE(parsearRegistro("\"sin cierre,3").has_value());
    ASSERT_FALSE(parsearRegistro("\"ana\"").has_value());
    ASSERT_FALSE(parsearRegistro("\"ana\",\"3x\"").has_value());
    ASSERT_FALSE(parsearRegistro("\"ana\",3 onzas").has_value());
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
    ASSERT_EQ(v[1].nombre, "bo, la");
    ASSERT_EQ(v[2].nombre, "dijo \"hola\"");
    ASSERT_EQ(v[2].puntos, -2);
    std::ifstream f(ruta);
    std::string header;
    std::getline(f, header);
    ASSERT_EQ(header, "nombre,puntos");
}

TEST_CASE(ElLoaderEsToleranteConLineasProblemáticas) {
    const std::string ruta = "/tmp/opencode/curso12_ej09_raro.csv";
    ASSERT_TRUE(guardarCsv(ruta, std::vector<Registro>{{"ana", 5}, {"dos", 6}}));
    {
        std::ofstream f(ruta, std::ios::app);
        f << "fila sin comillas\n";
    }
    ASSERT_EQ(cargarCsv(ruta, nullptr).size(), 2u);
}

TEST_CASE(GuardarYcargarAnteAperturaFallida) {
    ASSERT_FALSE(guardarCsv("/curso12_ej09/no_existe/x.csv", {}));
    bool ok = true;
    ASSERT_TRUE(cargarCsv("/curso12_ej09/no_existe/x.csv", &ok).empty());
    ASSERT_FALSE(ok);
}

int main() {
    std::cout << "Ejecutando pruebas para Solucion 09 (CSV con escape)...\n";
    return RUN_ALL_TESTS();
}
