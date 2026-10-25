/* =============================================================================
   MÓDULO 12 - SOLUCIÓN 03: parser de líneas de script con getline (ej. 03)
   =============================================================================
   L05/L06 son las lecciones de respaldo: el bool del getline es el.
   check canon del "leer hasta el final"; los RUIDOS (vacías, '#', '//')
   se saltan; el int COMPLETO es valido con la cola-prohibida (L06: si
   sobra una palabra tras el valor, la línea se cae); el stream NO sale
   en fail: las inválidas se recorren y el parser sigue.
   ============================================================================= */
#include <cctype>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

bool esBlanco(unsigned char c) { return std::isspace(c) != 0; }

// 1) el getline-loop CON la decisión de "línea útil":
bool saltarLineasRuido(std::istream& in, std::string& linea) {
    std::string l;
    while (std::getline(in, l)) {
        size_t i = 0;
        while (i < l.size() && esBlanco(l[i])) ++i;     // blancos al frente
        if (i >= l.size()) continue;                    // vacía (o solo ws)
        if (l.compare(i, 1, "#") == 0) continue;        // comentario '#'
        if (l.compare(i, 2, "//") == 0) continue;       // comentario '//'
        linea = l;                                      // útil: salida
        return true;
    }
    return false;                                       // el stream se agotó
}

// 2) tokens SOLO por blancos (L05) + el int COMPLETO (L06):
bool parsearLineaClaveValor(const std::string& linea,
                            std::string& clave, int& valor) {
    std::istringstream ss(linea);
    std::string c;
    if (!(ss >> c)) return false;                       // no hay clave
    int v = 0;
    if (!(ss >> v)) return false;                       // no hay int tras
    std::string sobra;
    if (ss >> sobra) return false;                      // "100abc":[sobra]
    clave = c;
    valor = v;
    return true;
}

// 3) integreación de 1+2: por línea útil: válida la mete, inválida IGNORA:
std::map<std::string, int> aplicarConfig(std::istream& in) {
    std::map<std::string, int> config;
    std::string linea;
    while (saltarLineasRuido(in, linea)) {
        std::string clave;
        int valor = 0;
        if (parsearLineaClaveValor(linea, clave, valor)) {
            config[clave] = valor;
        }    // inválida: se salta; el stream sigue listo para la próxima
    }
    return config;
}

// ============= TESTS =============
TEST_CASE(SaltarRuidoBasico) {
    std::istringstream script("# comando de hatch\n\n// no-comment-style\n"
                              "salud 100\n");
    std::string linea;
    ASSERT_TRUE(saltarLineasRuido(script, linea));
    ASSERT_EQ(linea, "salud 100");
    ASSERT_FALSE(saltarLineasRuido(script, linea));
}

TEST_CASE(SaltarRuidoSoloComentarios) {
    std::istringstream script("# solo comentarios\n#y mas\n");
    std::string linea;
    ASSERT_FALSE(saltarLineasRuido(script, linea));
}

TEST_CASE(ParsearLineaOk) {
    std::string clave; int valor = 0;
    ASSERT_TRUE(parsearLineaClaveValor("salud 100", clave, valor));
    ASSERT_EQ(clave, "salud");
    ASSERT_EQ(valor, 100);
    ASSERT_TRUE(parsearLineaClaveValor("  nivel   3  ", clave, valor));
    ASSERT_EQ(clave, "nivel");
    ASSERT_EQ(valor, 3);
}

TEST_CASE(ParsearLineaMalas) {
    std::string clave; int valor = 0;
    ASSERT_FALSE(parsearLineaClaveValor("sin-valor", clave, valor));
    ASSERT_FALSE(parsearLineaClaveValor("clave hola", clave, valor));
    ASSERT_FALSE(parsearLineaClaveValor("clave 100abc", clave, valor));
}

TEST_CASE(AplicarConfigCompleto) {
    std::istringstream script(
        "# config de partida\n"
        "salud 100\n\n"
        "velocidad 42 \n"
        "# pierna con espacio al final:\n"
        "monedas 17\n"
        "mala-linea-sin-valor\n"
        "otra-mala abc\n");
    const auto config = aplicarConfig(script);
    ASSERT_EQ(config.size(), 3u);
    ASSERT_EQ(config.at("salud"), 100);
    ASSERT_EQ(config.at("velocidad"), 42);
    ASSERT_EQ(config.at("monedas"), 17);
}

int main() {
    std::cout << "Ejecutando pruebas para Solucion 03 (parser de script)...\n";
    return RUN_ALL_TESTS();
}
