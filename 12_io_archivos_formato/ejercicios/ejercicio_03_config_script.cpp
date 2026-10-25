/* =============================================================================
   MÓDULO 12 - EJERCICIO 03: parser de líneas de script con getline (L05-L06)
   =============================================================================
   DIFICULTAD: Media
   CONSIGNA (tema 3 de TEMAS.md: getline, parsing de líneas y validación):
     El nivel queda definido como un script de líneas "clave valor"
     (una por línea) que llega como std::istream& (test con istringstream).
       1. saltarLineasRuido(in, linea): por referencia lee del stream la
          PRÓXIMA línea "útil": no vacía y que NO empiece con '#'
          (comentarios). Devuelve false si el stream se agotó first
          (L05: while getline y el estado del stream decide).
       2. parsearLineaClaveValor(linea, clave, valor): "clave valor"
          separados por blancos. La clave es texto; el valor UN INT COMPLETO
          (L06: "100abc" NO es valor; peek tras >>).
       3. aplicarConfig(in): recorre TODAS las líneas útiles y devuelve
          un map<string,int> con la config parseada (L05 + 12 con los dos
          de arriba). Línea inválida: NO se rompe, simplemente se ignora
          (y NO deja el stream en fail).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_03_config_script.cpp -o ej03 && ./ej03
   ============================================================================= */
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

// TODO 1: lee la próxima línea ÚTIL (no vacía, sin '#' al inicio) en linea.
//         False si no hay más (L05: el bool del stream es el check canon).
bool saltarLineasRuido([[maybe_unused]] std::istream& in,
                       [[maybe_unused]] std::string& linea) {
    // TU CODIGO AQUI
    return false;    // placeholder
}

// TODO 2: "clave valor" -> clave, valor. False si falta el valor o si el
//         valor no es un int COMPLETO (peek, L06).
bool parsearLineaClaveValor([[maybe_unused]] const std::string& linea,
                            [[maybe_unused]] std::string& clave,
                            [[maybe_unused]] int& valor) {
    // TU CODIGO AQUI (istringstream sobre linea: tokens separados por
    // blancos, la L05: "42 17 foca" -> dos ints y un string)
    return false;    // placeholder
}

// TODO 3: integra 1 y 2 sobre TODO el stream; devuelve la config en el map.
std::map<std::string, int> aplicarConfig([[maybe_unused]] std::istream& in) {
    // TU CODIGO AQUI
    return {};    // placeholder
}

// ============= TESTS =============
TEST_CASE(SaltarRuidoBasico) {
    std::istringstream script("# comando de hatch\n\n// no-comment-style\n"
                              "salud 100\n");
    std::string linea;
    ASSERT_TRUE(saltarLineasRuido(script, linea));
    ASSERT_EQ(linea, "salud 100");
    ASSERT_FALSE(saltarLineasRuido(script, linea));   // ya no hay nada
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
    return RUN_ALL_TESTS();
}
