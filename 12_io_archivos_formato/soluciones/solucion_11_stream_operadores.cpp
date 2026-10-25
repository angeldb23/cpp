/* =============================================================================
   MÓDULO 12 - SOLUCIÓN 11: operator<< y operator>> propios (ej. 11)
   =============================================================================
   L14 (con L05/L06): el operador de salida toma CUALQUIER ostream
   (consola, ostringstream, archivo): compone SIN magia pega. El de
   entrada exige tokens COMPLETOS (leído por token + verificación de
   cola; L06) y ante input podrido deja la señal canon: el stream en
   failbit: el `while (is >> c)` la usa corta solo (L12).
   ============================================================================= */
#include <fstream>
#include <format>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

struct Comando {
    std::string nombre;
    int magnitud = 0;
    double momento = 0.0;
};

// 1) el operador de SALIDA: los campos con UN espacio y el double
//    SIEMPRE a 2 decimales (std::format: el default del stream no pega):
std::ostream& operator<<(std::ostream& os, const Comando& c) {
    return os << c.nombre << ' ' << c.magnitud << ' '
              << std::format("{:.2f}", c.momento);
}

// helper: UN token leído como NÚMERO COMPLETO (L06: la cola mata):
template<typename T>
bool tokenNumerico(std::istream& is, T& valor) {
    std::string tok;
    if (!(is >> tok)) return false;
    std::istringstream p(tok);
    T v{};
    if (!(p >> v)) return false;
    char sobra = 0;
    if (p.get(sobra)) return false;        // algo quedó en el token
    valor = v;
    return true;
}

// 2) el operador de ENTRADA: palabra + int completo + double completo;
//    ante lo podrido: setstate(failbit) (la señal canon del stream):
std::istream& operator>>(std::istream& is, Comando& c) {
    std::string palabra;
    if (!(is >> palabra)) {                // nada (o solo blancos)
        is.setstate(std::ios::failbit);
        return is;
    }
    int mag = 0;
    double mom = 0.0;
    if (!tokenNumerico(is, mag) || !tokenNumerico(is, mom)) {
        is.setstate(std::ios::failbit);
        return is;
    }
    c.nombre = palabra;                    // todo o nada: al final
    c.magnitud = mag;
    c.momento = mom;
    return is;
}

// 3) UNA línea por comando; cada write visto (L12):
bool guardarComandos(const std::string& ruta,
                     const std::vector<Comando>& cs) {
    std::ofstream f(ruta, std::ios::trunc);
    if (!f) return false;
    for (const Comando& c : cs) {
        f << c << '\n';                    // EL OPERADOR en acción
        if (!f) return false;
    }
    f.flush();
    return !f.fail();
}

// 4) getline + istringstream POR LÍNEA: la podrida se salta (L12):
std::vector<Comando> cargarComandos(const std::string& ruta, bool* ok) {
    std::vector<Comando> cs;
    const auto falla = [&] {
        if (ok) *ok = false;
        return std::vector<Comando>{};
    };
    std::ifstream f(ruta);
    if (!f) return falla();
    if (ok) *ok = true;
    std::string linea;
    while (std::getline(f, linea)) {
        if (linea.empty()) continue;
        std::istringstream ss(linea);
        Comando c;
        if (ss >> c) cs.push_back(c);      // inválida: clear subclass deja
    }
    return cs;
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
    ASSERT_EQ(ss.str(), "saltar -2 1.00");
    ss.str("");
    ss << Comando{"girar", 5, 0.92} << "|" << Comando{"pausa", 0, 0};
    ASSERT_EQ(ss.str(), "girar 5 0.92|pausa 0 0.00");
}

TEST_CASE(EntradaFeliz) {
    std::istringstream ss("girar 5 0.92");
    Comando c;
    ss >> c;
    ASSERT_TRUE(static_cast<bool>(ss));
    ASSERT_TRUE(iguales(c, Comando{"girar", 5, 0.92}));
    std::istringstream dos("a 1 2.5 b 3 4.25");
    while (dos >> c) {}
    ASSERT_TRUE(c.magnitud == 3);
}

TEST_CASE(EntradaPodridaDejaElStreamEnFail) {
    Comando c;
    std::istringstream sinNada("   ");
    ASSERT_FALSE(static_cast<bool>(sinNada >> c));
    ASSERT_TRUE(sinNada.fail());

    std::istringstream noint("saltar 2.5 0.9");
    ASSERT_FALSE(static_cast<bool>(noint >> c));
    ASSERT_TRUE(noint.fail());

    std::istringstream nodouble("cat 4 x");
    ASSERT_FALSE(static_cast<bool>(nodouble >> c));
    ASSERT_TRUE(nodouble.fail());

    std::istringstream concola("cat 4 0.5x");
    ASSERT_FALSE(static_cast<bool>(concola >> c));
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
        std::ofstream f(ruta, std::ios::app);
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
    std::cout << "Ejecutando pruebas para Solucion 11 (operadores de stream)...\n";
    return RUN_ALL_TESTS();
}
