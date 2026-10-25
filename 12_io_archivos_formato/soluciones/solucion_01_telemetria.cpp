/* =============================================================================
   MÓDULO 12 - SOLUCIÓN 01: telemetría del HUD por stream de estados (ej. 01)
   =============================================================================
   L01 es la lección de respaldo: los streams NO lanzan, la señal es el
   ESTADO: el `bool` del stream tras una operación es la validación
   canon, y el estado del stream (fail/bad, con eof SIN error) decide
   qué reporta el programa. getline + istringstream (L05) es la receta
   del parseo, y el int COMPLETO es exigencia del L06 (peek de la cola).
   ============================================================================= */
#include <iostream>
#include <sstream>
#include <string>
#include "../../include/test_framework.hpp"

// 1) escribir el evento; la validación es el bool del stream (L01/L12):
bool logEvento(std::ostream& out, const std::string& nombre, int valor) {
    out << "[telemetry] " << nombre << '=' << valor << '\n';
    return !out.fail();    // write ok y stream sano: mismo chequeo de una vez
}

// 2) getline + istringstream (L05); espacios alrededor del valor SÍ se
//    toleran; la cola NO-numérica ('12abc', '12 30') mata la línea (L06):
bool leerEtiquetaValor(std::istream& in, std::string& etiqueta, int& valor) {
    std::string linea;
    if (!std::getline(in, linea)) return false;           // se agotó
    const size_t pos = linea.find('=');
    if (pos == std::string::npos) return false;           // sin '='
    if (pos == 0) return false;                           // etiqueta vacía

    std::string izquierda = linea.substr(0, pos);
    size_t ini = 0, fin = izquierda.size();
    const auto esBlanco = [](unsigned char c) { return std::isspace(c); };
    while (ini < fin && esBlanco(izquierda[ini])) ++ini;
    while (fin > ini && esBlanco(izquierda[fin - 1])) --fin;
    if (ini == fin) return false;                         // solo blancos

    std::istringstream resto(linea.substr(pos + 1));
    int n = 0;
    if (!(resto >> n)) {                                  // no hay int
        in.setstate(std::ios::failbit);   // la señal canon a quién parseó
        return false;
    }
    std::string sobra;
    if (resto >> sobra) {                                 // cola basura
        in.setstate(std::ios::failbit);
        return false;
    }
    etiqueta = izquierda.substr(ini, fin - ini);
    valor = n;
    return true;
}

// 3) el orden del chequeo: bad ANTES que fail (si hay bad, esto es
//    definitivo); el eof solo NO es error (L12):
std::string saludDelStream(const std::ios& s) {
    if (s.bad()) return "bad";
    if (s.fail()) return "fail";
    return "ok";
}

// ============= TESTS =============
TEST_CASE(LogEscribeYValida) {
    std::ostringstream out;
    ASSERT_TRUE(logEvento(out, "vida", 100));
    ASSERT_TRUE(logEvento(out, "espera", 5000));
    const std::string esperado = "[telemetry] vida=100\n[telemetry] espera=5000\n";
    ASSERT_EQ(out.str(), esperado);
    ASSERT_EQ(saludDelStream(out), "ok");

    std::ostringstream rotoDeFabrica;
    (void)rotoDeFabrica;
}

TEST_CASE(LogEnStreamRoto_devuelveFalse) {
    std::ostringstream out;
    out.setstate(std::ios::badbit);
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
        std::istringstream in("sin igual aqui\n");
        ASSERT_FALSE(leerEtiquetaValor(in, etiqueta, valor));
    }
    {
        std::istringstream in("=50\n");
        ASSERT_FALSE(leerEtiquetaValor(in, etiqueta, valor));
    }
    {
        std::istringstream in("vida=hola\n");
        ASSERT_FALSE(leerEtiquetaValor(in, etiqueta, valor));
        ASSERT_EQ(saludDelStream(in), "fail");
    }
    {
        std::istringstream in("vida=12abc\n");
        ASSERT_FALSE(leerEtiquetaValor(in, etiqueta, valor));
    }
}

TEST_CASE(SaludDelStream) {
    std::istringstream sano("algo");
    ASSERT_EQ(saludDelStream(sano), "ok");
    std::istringstream enFail("hola");
    int x = 0; (void)(enFail >> x);
    ASSERT_EQ(saludDelStream(enFail), "fail");
}

int main() {
    std::cout << "Ejecutando pruebas para Solucion 01 (telemetria por stream)...\n";
    return RUN_ALL_TESTS();
}
