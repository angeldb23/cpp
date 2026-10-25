/* =============================================================================
   MÓDULO 12 - SOLUCIÓN 06: escaneo de assets con filesystem (ej. 06)
   =============================================================================
   L09-L11: el recursive_directory_iterator NUNCA se ordena (orden del
   FS): el sort determinista es TU trabajo (L10). La mutación directa,
   create_directories + rename y remove, SIEMPRE con error_code cuando
   el fallo es informativo   y la lista-pregunta "¿está vacía?" es
   fs::is_empty, no un conteo a mano.
   ============================================================================= */
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

namespace fs = std::filesystem;

// (dada, copiada del ejercicio): tmp única por corrida:
std::string dirTmp(const char* etiqueta) {
    const fs::path dir = "/tmp/opencode/curso12_ej06_" + std::string(etiqueta);
    fs::remove_all(dir);
    fs::create_directories(dir);
    return dir.string();
}

bool esPNG(const fs::path& p) {
    const std::string ext = p.extension().string();
    return ext == ".png" || ext == ".PNG";    // EXACTA: sin mayúscula-obligada
}

// 1) gobernada recursiva (L10) + sort determinista (el orden del FS
//    no es prometido: el sort hace el atributo):
std::vector<std::string> coleccionarPNGs(const std::string& dir) {
    std::vector<std::string> out;
    std::error_code ec;
    fs::recursive_directory_iterator it(dir, ec);
    fs::recursive_directory_iterator fin;
    if (ec) return out;                        // dir ni existe: lista vacía
    for (; it != fin; it.increment(ec)) {
        if (ec) return out;
        if (it->is_regular_file(ec) && !ec && esPNG(it->path())) {
            out.push_back(it->path().string());
        }
    }
    std::sort(out.begin(), out.end());         // determinista SIEMPRE
    return out;
}

// 2) la mudanza directo (L11): "salida" creada y rename de nivel 1:
int organizar(const std::string& dir) {
    const fs::path salida = fs::path(dir) / "salida";
    fs::create_directories(salida);            // si ya está: no molesta
    int movidos = 0;
    std::error_code ec;
    for (fs::directory_iterator it(dir, ec), fin; it != fin; it.increment(ec)) {
        if (ec) break;
        if (!it->is_regular_file(ec) || ec || !esPNG(it->path())) continue;
        std::error_code ec2;
        fs::rename(it->path(), salida / it->path().filename(), ec2);
        if (!ec2) ++movidos;
    }
    return movidos;
}

// 3) las subcarpetas VACÍAS de nivel 1 (is_empty: L11) se van:
int limpiarVacias(const std::string& dir) {
    int borradas = 0;
    std::error_code ec;
    for (fs::directory_iterator it(dir, ec), fin; it != fin; it.increment(ec)) {
        if (ec) break;
        if (!it->is_directory(ec) || ec) continue;
        if (!fs::is_empty(it->path(), ec) || ec) continue;
        std::error_code ec2;
        if (fs::remove(it->path(), ec2) && !ec2) ++borradas;
    }
    return borradas;
}

// ============= TESTS =============
template<typename T>
bool contiene(const std::vector<T>& v, const T& x) {
    for (const T& e : v) if (e == x) return true;
    return false;
}

TEST_CASE(ColeccionarPNGsRecursivo) {
    const std::string dir = dirTmp("colectar");
    fs::create_directories(dir + "/sub/deep");
    for (const char* rel : {"a.png", "b.txt", "sub/c.png", "sub/deep/d.png"}) {
        std::ofstream{dir + "/" + rel} << "x";
    }
    const auto pngs = coleccionarPNGs(dir);
    ASSERT_EQ(pngs.size(), 3u);
    ASSERT_TRUE(contiene(pngs, dir + "/a.png"));
    ASSERT_TRUE(contiene(pngs, dir + "/sub/c.png"));
    ASSERT_TRUE(contiene(pngs, dir + "/sub/deep/d.png"));
    ASSERT_FALSE(contiene(pngs, dir + "/b.txt"));
    for (size_t i = 1; i < pngs.size(); ++i) {
        ASSERT_TRUE(pngs[i-1] < pngs[i]);
    }
}

TEST_CASE(ExtensionMinusculaYMayuscula) {
    const std::string dir = dirTmp("case");
    for (const char* rel : {"p.png", "q.PNG", "r.txt"}) {
        std::ofstream{dir + "/" + rel} << "x";
    }
    const auto pngs = coleccionarPNGs(dir);
    ASSERT_EQ(pngs.size(), 2u);
}

TEST_CASE(OrganizarMueveNivelUno) {
    const std::string dir = dirTmp("organizar");
    fs::create_directories(dir + "/sub");
    for (const char* rel : {"a.png", "texto.txt", "sub/media.png"}) {
        std::ofstream{dir + "/" + rel} << "x";
    }
    const int movidos = organizar(dir);
    ASSERT_EQ(movidos, 1);
    ASSERT_FALSE(fs::exists(dir + "/a.png"));
    ASSERT_TRUE(fs::exists(dir + "/salida/a.png"));
    ASSERT_TRUE(fs::exists(dir + "/texto.txt"));
    ASSERT_TRUE(fs::exists(dir + "/sub/media.png"));
}

TEST_CASE(OrganizarMueveUnSoloPiso) {
    const std::string dir = dirTmp("movido");
    fs::create_directories(dir + "/sub");
    for (const char* rel : {"b.png", "c.png"}) {
        std::ofstream{dir + "/" + rel} << "x";
    }
    ASSERT_EQ(organizar(dir), 2);
    ASSERT_EQ(organizar(dir), 0);
    ASSERT_TRUE(fs::exists(dir + "/salida/b.png"));
}

TEST_CASE(LimpiarVacias) {
    const std::string dir = dirTmp("limpiar");
    fs::create_directories(dir + "/vacia1");
    fs::create_directories(dir + "/vacia2");
    fs::create_directories(dir + "/llena");
    std::ofstream{dir + "/llena/dato.txt"} << "x";
    std::ofstream{dir + "/suelto.txt"} << "x";
    ASSERT_EQ(limpiarVacias(dir), 2);
    ASSERT_FALSE(fs::exists(dir + "/vacia1"));
    ASSERT_TRUE(fs::exists(dir + "/llena"));
    ASSERT_TRUE(fs::exists(dir + "/suelto.txt"));
}

int main() {
    std::cout << "Ejecutando pruebas para Solucion 06 (assets scanner)...\n";
    return RUN_ALL_TESTS();
}
