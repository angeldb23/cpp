/* =============================================================================
   MÓDULO 12 - EJERCICIO 07: diagnóstico de E/S: qué falló y dónde (L12)
   =============================================================================
   DIFICULTAD: Media
   CONSIGNA (tema 6 de TEMAS.md: errores de E/S: fail/bad/eof y validación):
     El autosave del curso escribe y lee con VALIDACIÓN MANUAL de cada
     paso (L12: apertura, operación, y el cierre SIEMPRE mirado). Los
     streams NO lanzan: la señal es el ESTADO del stream. Sin
     excepciones en este ejercicio:
       1. guardarDuro(ruta, contenido, razon): el save completo del
          tema 6: valida APERTURA (`if (!f)`), ESCRITURA (el write
          crudo), FLUSH y CLOSE explícito (el destructor hace flush
          pero NO te deja ver el resultado: L12). En cada falla
          devuelve false y deja en *razon (si no es nullptr) la
          palabra correspondiente: "apertura", "escritura", "flush" o
          "cierre". Todo bien: true y *razon="".
       2. diagnostico(s): "bad" si badbit (irrecuperable), "fail" si
          solo failbit (recuperable con clear) y "ok" en todo lo
          demás. OJO (L12): eofbit SOLO NO es error: un getline que
          leyó chars y cruzó el final deja eofbit SIN failbit: eso es
          "ok". El orden del chequeo importa: bad se mira PRIMERO.
       3. sumarEnteros(in, suma): acumula ints con `>>` hasta que el
          stream corta el loop (fail). Devuelve true si el corte es
          TOLERABLE: !(in.bad()) (el corte canon eof+fail NO es bad:
          L12) y false solo ante badbit. La suma de lo leído detalló
          queda en *suma.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_07_diagnostico_io.cpp -o ej07 && ./ej07
   ============================================================================= */
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "../../include/test_framework.hpp"

namespace fs = std::filesystem;

// (dada, no toques): ruta de lata única por corrida (a la manera del ej06):
std::string dirTmp(const char* etiqueta) {
    const fs::path dir = "/tmp/opencode/curso12_ej07_" + std::string(etiqueta);
    fs::remove_all(dir);
    fs::create_directories(dir);
    return dir.string();
}

// TODO 1: el save con los CUATRO chequeos de la L12 (apertura, write,
//         flush, close) y la razón en *razon (nullptr tolerado):
bool guardarDuro([[maybe_unused]] const std::string& ruta,
                 [[maybe_unused]] const std::string& contenido,
                 [[maybe_unused]] std::string* razon) {
    // TU CODIGO AQUI
    return false;    // placeholder
}

// TODO 2: "bad" / "fail" / "ok" según rdstate (bad PRIMERO; eof solo no molesta):
std::string diagnostico([[maybe_unused]] std::ios& s) {
    // TU CODIGO AQUI
    return "???";    // placeholder
}

// TODO 3: suma con >> hasta el corte canon; true si el estado final
//         no tiene badbit:
bool sumarEnteros([[maybe_unused]] std::istream& in,
                  [[maybe_unused]] int* suma) {
    // TU CODIGO AQUI
    return false;    // placeholder
}

// ============= TESTS =============
TEST_CASE(DiagnosticoDelStreamIntocado) {
    std::istringstream limpio("hola que tal");
    ASSERT_EQ(diagnostico(limpio), "ok");       // sin operaciones: "ok"
}

TEST_CASE(DiagnosticoTrasParseoMalo) {
    std::istringstream s("abc");
    int n = 0;
    s >> n;                                     // pidió un int: falló
    ASSERT_TRUE(s.fail());
    ASSERT_EQ(diagnostico(s), "fail");
}

TEST_CASE(EofSoloEsOkNoEsError) {
    std::istringstream s("12");
    std::string linea;
    std::getline(s, linea);        // leyó "12" Y cruzó el final: eofbit
    ASSERT_TRUE(s.eof());
    ASSERT_FALSE(s.fail());        // extractó chars: NO hay failbit
    ASSERT_EQ(diagnostico(s), "ok");            // eof solo: no es error
}

TEST_CASE(RecuperaConClearEIgnore) {
    std::istringstream s("basura\n7 8 9");
    int n = 0;
    s >> n;                                     // falla por la palabra
    ASSERT_EQ(diagnostico(s), "fail");
    s.clear();                                  // fail es RECUPERABLE
    s.ignore(64, '\n');                         // fuera la línea podrida
    ASSERT_EQ(diagnostico(s), "ok");
    int suma = 0;
    ASSERT_TRUE(sumarEnteros(s, &suma));        // el tail sirve de nuevo
    ASSERT_EQ(suma, 24);
}

TEST_CASE(GuardarDuroCaminoFeliz) {
    const std::string dir = dirTmp("feliz");
    const std::string ruta = dir + "/save.txt";
    std::string razon = "prellena";
    ASSERT_TRUE(guardarDuro(ruta, "vida=100\n", &razon));
    ASSERT_EQ(razon, "");                       // vacía cuando todo bien
    std::ifstream f(ruta);
    std::string linea;
    std::getline(f, linea);
    ASSERT_EQ(linea, "vida=100");
}

TEST_CASE(GuardarDuroAperturaFallada) {
    const std::string ruta = "/tmp/opencode/curso12_ej07_no_hay_carpeta/x.txt";
    std::string razon;
    ASSERT_FALSE(guardarDuro(ruta, "x", &razon));
    ASSERT_EQ(razon, "apertura");               // el chequeo UNO es ese
    ASSERT_FALSE(fs::exists(ruta));             // no hay archivo fantasma
}

TEST_CASE(GuardarDuroToleraRazonNula) {
    const std::string dir = dirTmp("null");
    ASSERT_TRUE(guardarDuro(dir + "/a.txt", "contenido", nullptr));
    ASSERT_FALSE(guardarDuro("/no/existe/curso12_ej07/y.txt", "x", nullptr));
}

TEST_CASE(SumarHastaElFinalYEstadoCanon) {
    std::istringstream s("3\n4\n5");            // sin '\n' final
    int suma = 0;
    ASSERT_TRUE(sumarEnteros(s, &suma));        // eof+fail: NO es bad
    ASSERT_EQ(suma, 12);
    ASSERT_EQ(diagnostico(s), "fail");          // cortó por fail (con eof)
    ASSERT_FALSE(s.bad());                      // ...y por eso es "true"
}

TEST_CASE(SumarParaEnLaPalabraPeroNoEsBad) {
    std::istringstream s("3 cuatro 5");
    int suma = 0;
    ASSERT_TRUE(sumarEnteros(s, &suma));        // sin bad: corte tolerable
    ASSERT_EQ(suma, 3);                         // el "5" quedó afuera
}

int main() {
    return RUN_ALL_TESTS();
}
