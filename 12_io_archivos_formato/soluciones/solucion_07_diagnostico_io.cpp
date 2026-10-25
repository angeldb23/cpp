/* =============================================================================
   MÓDULO 12 - SOLUCIÓN 07: diagnóstico de E/S: qué falló y dónde (ej. 07)
   =============================================================================
   L12: el save "duro" valida SUS CUATRO instantes (apertura, write,
   flush, close) y devuelve false en el PRIMER chequeo fallido con la
   razón a mano. El diagnóstico es un ORDEN: bad primero (es
   irrecuperable), fail segundo (recuperable), y todo lo demás
   (incluido el eofbit solo: NO es error) es "ok". El corte del
   `while (in >> n)` es anon: fail + eof juntos NO es bad.
   ============================================================================= */
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "../../include/test_framework.hpp"

// 1) el save duro: cuatro chequeos, la razón aduce cada falla:
bool guardarDuro(const std::string& ruta,
                 const std::string& contenido,
                 std::string* razon) {
    const auto falla = [&](const char* r) {
        if (razon) *razon = r;
        return false;
    };
    std::ofstream f(ruta, std::ios::trunc);
    if (!f) return falla("apertura");                    // 1) apertura
    f.write(contenido.data(),
            static_cast<std::streamsize>(contenido.size()));
    if (!f) return falla("escritura");                   // 2) operación
    f.flush();
    if (!f) return falla("flush");                       // 2 bis: el flush REAL
    f.close();
    if (f.fail()) return falla("cierre");                // 3) close no miente
    if (razon) *razon = "";
    return true;
}

// 2) bad ANTES, fail después, todo lo demás (incluido eof solo): "ok":
std::string diagnostico(std::ios& s) {
    if (s.rdstate() & std::ios::badbit) return "bad";
    if (s.rdstate() & std::ios::failbit) return "fail";
    return "ok";
}

// 3) el corte canon del while: fail + eof; tolerable SI no quedó bad:
bool sumarEnteros(std::istream& in, int* suma) {
    int n = 0;
    while (in >> n) *suma += n;
    return !in.bad();
}

// ============= TESTS =============
TEST_CASE(DiagnosticoDelStreamIntocado) {
    std::istringstream limpio("hola que tal");
    ASSERT_EQ(diagnostico(limpio), "ok");
}

TEST_CASE(DiagnosticoTrasParseoMalo) {
    std::istringstream s("abc");
    int n = 0;
    s >> n;
    ASSERT_TRUE(s.fail());
    ASSERT_EQ(diagnostico(s), "fail");
}

TEST_CASE(EofSoloEsOkNoEsError) {
    std::istringstream s("12");
    std::string linea;
    std::getline(s, linea);
    ASSERT_TRUE(s.eof());
    ASSERT_FALSE(s.fail());
    ASSERT_EQ(diagnostico(s), "ok");
}

TEST_CASE(RecuperaConClearEIgnore) {
    std::istringstream s("basura\n7 8 9");
    int n = 0;
    s >> n;
    ASSERT_EQ(diagnostico(s), "fail");
    s.clear();
    s.ignore(64, '\n');
    ASSERT_EQ(diagnostico(s), "ok");
    int suma = 0;
    ASSERT_TRUE(sumarEnteros(s, &suma));
    ASSERT_EQ(suma, 24);
}

TEST_CASE(GuardarDuroCaminoFeliz) {
    const std::string dir = "/tmp/opencode/curso12_ej07_feliz";
    std::filesystem::create_directories(dir);
    const std::string ruta = dir + "/save.txt";
    std::string razon = "prellena";
    ASSERT_TRUE(guardarDuro(ruta, "vida=100\n", &razon));
    ASSERT_EQ(razon, "");
    std::ifstream f(ruta);
    std::string linea;
    std::getline(f, linea);
    ASSERT_EQ(linea, "vida=100");
}

TEST_CASE(GuardarDuroAperturaFallada) {
    const std::string ruta = "/tmp/opencode/curso12_ej07_no_hay_carpeta/x.txt";
    std::string razon;
    ASSERT_FALSE(guardarDuro(ruta, "x", &razon));
    ASSERT_EQ(razon, "apertura");
    ASSERT_FALSE(std::filesystem::exists(ruta));
}

TEST_CASE(GuardarDuroToleraRazonNula) {
    const std::string dir = "/tmp/opencode/curso12_ej07_null";
    std::filesystem::create_directories(dir);
    ASSERT_TRUE(guardarDuro(dir + "/a.txt", "contenido", nullptr));
    ASSERT_FALSE(guardarDuro("/no/existe/curso12_ej07/y.txt", "x", nullptr));
}

TEST_CASE(SumarHastaElFinalYEstadoCanon) {
    std::istringstream s("3\n4\n5");
    int suma = 0;
    ASSERT_TRUE(sumarEnteros(s, &suma));
    ASSERT_EQ(suma, 12);
    ASSERT_EQ(diagnostico(s), "fail");
    ASSERT_FALSE(s.bad());
}

TEST_CASE(SumarParaEnLaPalabraPeroNoEsBad) {
    std::istringstream s("3 cuatro 5");
    int suma = 0;
    ASSERT_TRUE(sumarEnteros(s, &suma));
    ASSERT_EQ(suma, 3);
}

int main() {
    std::cout << "Ejecutando pruebas para Solucion 07 (diagnostico de E/S)...\n";
    return RUN_ALL_TESTS();
}
