/* =============================================================================
   MÓDULO 12 - EJERCICIO 01: telemetry del HUD por stream de estados (L01-L02)
   =============================================================================
   DIFICULTAD: Básico
   CONSIGNA (tema 1 de TEMAS.md: iostreams y estados del stream):
     Un motor de juego manda telemetry "por un stream" para poder probarlo
     con istringstream (L05: el patrón del módulo). Escribí las funciones
     contra std::ostream& y std::istream&, SIN cin ni cout adentro:
       1. logEvento(out, nombre, valor): escribe una línea
          "[evento] nombre=valor\r\n"? NO: '\n' simple (endl es un flush
          caro: L01). Debe devolver true SIEMPRE que el stream quedó
          bien después de escribir (L12: validar la operación).
       2. leerEtiquetaValor(in, etiqueta, valor): parsea UNA línea con el
          formato "vida=100" (L05: getline + istringstream, con '=' como
          delimitador). Devuelve false si la línea está vacía, si no
          tiene '=', o si lo que queda después de '=' no es un int
          COMPLETO (L06: parse con peek: "12abc" NO es int).
       3. saludDelStream(s): devuelve un string "ok" / "fail" / "bad"
          según el estado (L01): fail y bad con clear NO (solo leéctelo).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_01_telemetria.cpp -o ej01 && ./ej01
   ============================================================================= */
#include <iostream>
#include <sstream>
#include <string>
#include "../../include/test_framework.hpp"

// TODO 1: escribe "nombre=valor\n" en out con el prefijo [telemetry]; devuelve
//         true si el stream quedó bien (sin failbit/badbit) tras escribir.
bool logEvento([[maybe_unused]] std::ostream& out,
               [[maybe_unused]] const std::string& nombre,
               [[maybe_unused]] int valor) {
    // TU CODIGO AQUI
    return false;    // placeholder
}

// TODO 2: de la línea "etiqueta=valor" llena ambos parámetros. False si:
//         - no hay '=' ,
//         - la etiqueta queda vacía,
//         - lo que queda tras '=' no es UN int completo ("12x" u "12 30" NO).
//         OJO: los espacios ALREDEDOR del valor SÍ se toleran ("vida= 42").
bool leerEtiquetaValor([[maybe_unused]] std::istream& in,
                       [[maybe_unused]] std::string& etiqueta,
                       [[maybe_unused]] int& valor) {
    // TU CODIGO AQUI (getline + istringstream: la receta de la L05)
    return false;    // placeholder
}

// TODO 3: "ok" si !fail() && !bad(); "fail" si fail() sin bad();
//         "bad" si bad(). (L01: los tres estados; eof no es error).
//         Firma con std::ios&: sirve PARA TODO stream (in y out).
std::string saludDelStream([[maybe_unused]] const std::ios& s) {
    // TU CODIGO AQUI
    return "";    // placeholder
}

// ============= TESTS =============
TEST_CASE(LogEscribeYValida) {
    std::ostringstream out;
    ASSERT_TRUE(logEvento(out, "vida", 100));
    ASSERT_TRUE(logEvento(out, "espera", 5000));
    const std::string esperado = "[telemetry] vida=100\n[telemetry] espera=5000\n";
    ASSERT_EQ(out.str(), esperado);
    // y el stream sigue sano despues de escribir:
    ASSERT_EQ(saludDelStream(out), "ok");

    std::ostringstream rotoDeFabrica;   // L01: un stream de salida no falla
    (void)rotoDeFabrica;                // aca: nada mas que validar el log
}

TEST_CASE(LogEnStreamRoto_devuelveFalse) {
    std::ostringstream out;
    out.setstate(std::ios::badbit);     // simulamos el stream roto (L12)
    ASSERT_FALSE(logEvento(out, "vida", 100));
}

TEST_CASE(EtiquetaValorBasico) {
    std::istringstream in("vida=100\n");
    std::string etiqueta;
    int valor = 0;
    ASSERT_TRUE(leerEtiquetaValor(in, etiqueta, valor));
    ASSERT_EQ(etiqueta, "vida");
    ASSERT_EQ(valor, 100);
}

TEST_CASE(EtiquetaValorConEspacios) {
    std::istringstream in("velocidad = 42\nsegunda= 7\n tercera = 9 \n");
    std::string etiqueta;
    int valor = 0;
    ASSERT_TRUE(leerEtiquetaValor(in, etiqueta, valor));
    ASSERT_EQ(etiqueta, "velocidad");
    ASSERT_EQ(valor, 42);
    ASSERT_TRUE(leerEtiquetaValor(in, etiqueta, valor));
    ASSERT_EQ(etiqueta, "segunda");
    ASSERT_EQ(valor, 7);
    ASSERT_TRUE(leerEtiquetaValor(in, etiqueta, valor));
    ASSERT_EQ(etiqueta, "tercera");
    ASSERT_EQ(valor, 9);
}

TEST_CASE(EtiquetaValorRechazaMalos) {
    int valor = 0;
    std::string etiqueta;
    {
        std::istringstream in("sin igual aqui\n");      // no hay '='
        ASSERT_FALSE(leerEtiquetaValor(in, etiqueta, valor));
    }
    {
        std::istringstream in("=50\n");                 // etiqueta vacia
        ASSERT_FALSE(leerEtiquetaValor(in, etiqueta, valor));
    }
    {
        std::istringstream in("vida=hola\n");           // valor no int
        ASSERT_FALSE(leerEtiquetaValor(in, etiqueta, valor));
        ASSERT_EQ(saludDelStream(in), "fail");          // queda en failbit
    }
    {
        std::istringstream in("vida=12abc\n");          // parse parcial: NO
        ASSERT_FALSE(leerEtiquetaValor(in, etiqueta, valor));
    }
}

TEST_CASE(SaludDelStream) {
    std::istringstream sano("algo");
    ASSERT_EQ(saludDelStream(sano), "ok");
    std::istringstream enFail("hola");
    int x = 0; (void)(enFail >> x);                     // falla: failbit
    ASSERT_EQ(saludDelStream(enFail), "fail");
}

int main() {
    return RUN_ALL_TESTS();
}
