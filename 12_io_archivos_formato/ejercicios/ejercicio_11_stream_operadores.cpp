/* =============================================================================
   MÓDULO 12 - EJERCICIO 11: operator<< y operator>> propios (L14 + L05/L06)
   =============================================================================
   DIFICULTAD: Media
   CONSIGNA (tema 8 de TEMAS.md: operator<< / operator>> propios):
     El catalogo de comandos de la consola de dev viaja por streams:
     le da a la clase el idioma de entrada/salida y TODO stream
     (ostringstream, archivo, stringstream de la línea del archivo)
     entienda lo mismo (L14: el operador toma ostream& GRAL, no solo
     un ofstream). struct Comando { nombre, magnitud (int), momento
     (double) }:
       1. std::ostream& operator<<(std::ostream&, const Comando&):
          EXACTO "nombre magnitud momento" con UN espacio como
          separador y momento con DOS decimales SIEMPRE (L03:
          std::format con "{:.2f}": el default del stream no pega
          dos decimales). Sin '\n' ni espacios al final.
       2. std::istream& operator>>(std::istream&, Comando&):
          leer nombre (una palabra SIN espacios), magnitud INT
          COMPLETO y momento DOUBLE COMPLETO (L06: token + from o
          stream-scan con eof-check: "3.5" para un int: fail). Ante
          cualquier cosa leible-mal: es.setstate(std::ios::failbit)
          y fuera (el while del archivo lo corta el stream).
       3. guardarComandos(ruta, comandos): un Comando por línea
          usando EL OPERADOR + '\n', con validación de cada paso
          (L12). false ante falla.
       4. cargarComandos(ruta, ok): una getline por línea (parser con
          istringstream: la línea podrida NO arruina el resto: L12:
          clear y seguir; ok=false solo si no abre).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_11_stream_operadores.cpp -o ej11 && ./ej11
   ============================================================================= */
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

struct Comando {
    std::string nombre;
    int magnitud = 0;
    double momento = 0.0;
};

// TODO 1: el operador de SALIDA (usuario L14): firma por const ref
//         y devuelve el stream (para poder componer):
std::ostream& operator<<([[maybe_unused]] std::ostream& os,
                         [[maybe_unused]] const Comando& c) {
    // TU CODIGO AQUI
    return os;    // placeholder
}

// TODO 2: el operador de ENTRADA: demanda EL CAMPO COMPLETO (L06):
std::istream& operator>>([[maybe_unused]] std::istream& is,
                         [[maybe_unused]] Comando& c) {
    // TU CODIGO AQUI
    return is;    // placeholder
}

// TODO 3: una línea por comando con el OPERADOR + '\n'; cada write
//         validado (L12):
bool guardarComandos([[maybe_unused]] const std::string& ruta,
                     [[maybe_unused]] const std::vector<Comando>& cs) {
    // TU CODIGO AQUI
    return false;    // placeholder
}

// TODO 4: getline + istringstream por línea: una podrida se SALTA:
std::vector<Comando> cargarComandos([[maybe_unused]] const std::string& ruta,
                                    [[maybe_unused]] bool* ok) {
    // TU CODIGO AQUI
    return {};    // placeholder
}

// ============= TESTS =============
bool iguales(const Comando& a, const Comando& b) {
    return a.nombre == b.nombre && a.magnitud == b.magnitud &&
           a.momento == b.momento;
}

TEST_CASE(SalidaExactaEnMemoria) {
    std::ostringstream ss;
    ss << Comando{"girar", 5, 0.92};
    ASSERT_EQ(ss.str(), "girar 5 0.92");
    ss.str("");
    ss << Comando{"saltar", -2, 1.0};
    ASSERT_EQ(ss.str(), "saltar -2 1.00");      // SIEMPRE dos decimales
    ss.str("");
    ss << Comando{"girar", 5, 0.92} << "|" << Comando{"pausa", 0, 0};
    ASSERT_EQ(ss.str(), "girar 5 0.92|pausa 0 0.00");   // compone SIN '\n'
}

TEST_CASE(EntradaFelizEHayStreamOk) {
    std::istringstream ss("girar 5 0.92");
    Comando c;
    ss >> c;
    ASSERT_TRUE(static_cast<bool>(ss));         // pérdida de focus: NO PATH
    ASSERT_TRUE(iguales(c, Comando{"girar", 5, 0.92}));
    std::istringstream dos("a 1 2.5 b 3 4.25");
    while (dos >> c) {}
    ASSERT_TRUE(c.magnitud == 3);               // el while con >> avanza
}

TEST_CASE(EntradaPodridaDejaElStreamEnFail) {
    Comando c;
    std::istringstream sinNada("   ");      // sin palabra: >> al estar vacío
    ASSERT_FALSE(static_cast<bool>(sinNada >> c));
    ASSERT_TRUE(sinNada.fail());            // la señal canon: ESTADO

    std::istringstream noint("saltar 2.5 0.9");
    ASSERT_FALSE(static_cast<bool>(noint >> c));   // magnitud no es int
    ASSERT_TRUE(noint.fail());

    std::istringstream nodouble("cat 4 x");
    ASSERT_FALSE(static_cast<bool>(nodouble >> c)); // momento no es double
    ASSERT_TRUE(nodouble.fail());

    std::istringstream concola("cat 4 0.5x");
    ASSERT_FALSE(static_cast<bool>(concola >> c));  // 0.5 con cola: literal
    ASSERT_TRUE(concola.fail());
}

TEST_CASE(RoundTripEnDisco) {
    const std::string ruta = "/tmp/opencode/curso12_ej11_comandos.txt";
    const std::vector<Comando> cs{{"girar", 5, 0.92},
                                  {"saltar", -2, 1.0},
                                  {"pausa", 0, 0.0}};
    ASSERT_TRUE(guardarComandos(ruta, cs));
    bool ok = false;
    const auto v = cargarComandos(ruta, &ok);
    ASSERT_TRUE(ok);
    ASSERT_EQ(v.size(), 3u);
    for (size_t i = 0; i < cs.size(); ++i) {
        ASSERT_TRUE(iguales(v[i], cs[i]));
    }
}

TEST_CASE(ElLoaderSaltaLaLineaPodrida) {
    const std::string ruta = "/tmp/opencode/curso12_ej11_raro.txt";
    ASSERT_TRUE(guardarComandos(ruta,
                                std::vector<Comando>{{"uno", 1, 0.5}}));
    {
        std::ofstream f(ruta, std::ios::app);       // basura de un mod:
        f << "roto x 0.5\n";
        f << "dos 2 0.75\n";
    }
    const auto v = cargarComandos(ruta, nullptr);
    ASSERT_EQ(v.size(), 2u);
    ASSERT_EQ(v[0].nombre, "uno");
    ASSERT_EQ(v[1].nombre, "dos");
    ASSERT_EQ(v[1].momento, 0.75);
}

int main() {
    return RUN_ALL_TESTS();
}
