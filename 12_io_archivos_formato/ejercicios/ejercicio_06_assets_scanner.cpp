/* =============================================================================
   MÓDULO 12 - EJERCICIO 06: escaneo de assets con filesystem (L09-L10-L11)
   =============================================================================
   DIFICULTAD: Media-Alta
   CONSIGNA (tema 5 de TEMAS.md: filesystem: path, iteradores y mutación):
     El gestor de assets del curso tiene que procesar carpetas con el
     API de las L09-L11 y reportar así que hizo (cosas reales que
     tocan disco, en una carpeta tmp propia):
       1. coleccionarPNGs(dir): recorre dir de forma RECURSIVA (L10) y
          devuelve los archivos ".png" (SOLO archivos; extension EXACTA
          vs minúscula: los ".PNG" también cuentan) ORDENADOS por
          path lexicográfico (L10: sort determinista).
       2. organizar(dir): DE shul de dir (NO es "assets"): crea la
          subcarpeta "salida" (create_directories: L11) y mueve (rename,
          L11) ahí TODO*.png que esté DIRECTO (nivel 1) de dir. Devuelve
          cuántos movió. Los .png de subcarpetas NO se tocan.
       3. limpiarVacias(dir): borra (remove: L11) las subcarpetas de dir
          (nivel 1 SOLO) que estén VACÍAS; devuelve cuántas borró.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_06_assets_scanner.cpp -o ej06 && ./ej06
   ============================================================================= */
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

namespace fs = std::filesystem;

// (dada, no toques): tmp UNICA por corrida:
std::string dirTmp(const char* etiqueta) {
    const fs::path dir = "/tmp/opencode/curso12_ej06_" + std::string(etiqueta);
    fs::remove_all(dir);
    fs::create_directories(dir);
    return dir.string();
}

// TODO 1: los .png recursivos con ORDEN determinista (L10 + sort).
std::vector<std::string> coleccionarPNGs([[maybe_unused]] const std::string& dir) {
    // TU CODIGO AQUI
    return {};    // placeholder
}

// TODO 2: crea dir/"salida" y mueve los *.png DIRECTOS de dir; devuelve
//         cuántos movió (L11: create_directories + rename con ec).
int organizar([[maybe_unused]] const std::string& dir) {
    // TU CODIGO AQUI
    return -1;    // placeholder
}

// TODO 3: borra las subcarpetas VACÍAS de nivel 1; cuántas.
int limpiarVacias([[maybe_unused]] const std::string& dir) {
    // TU CODIGO AQUI
    return -1;    // placeholder
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
    // y en orden lexicográfico (determinista):
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
    ASSERT_EQ(pngs.size(), 2u);     // .png y .PNG cuentan; .txt no
}

TEST_CASE(OrganizarMueveNivelUno) {
    const std::string dir = dirTmp("organizar");
    fs::create_directories(dir + "/sub");
    for (const char* rel : {"a.png", "texto.txt", "sub/media.png"}) {
        std::ofstream{dir + "/" + rel} << "x";
    }
    const int movidos = organizar(dir);
    ASSERT_EQ(movidos, 1);
    // a.png ya no está en dir, está en salida:
    ASSERT_FALSE(fs::exists(dir + "/a.png"));
    ASSERT_TRUE(fs::exists(dir + "/salida/a.png"));
    // y el .txt y el de sub NO se movieron:
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
    ASSERT_EQ(organizar(dir), 0);   // ya no hay pngs en nivel 1
    ASSERT_TRUE(fs::exists(dir + "/salida/b.png"));
}

TEST_CASE(LimpiarVacias) {
    const std::string dir = dirTmp("limpiar");
    fs::create_directories(dir + "/vacia1");
    fs::create_directories(dir + "/vacia2");
    fs::create_directories(dir + "/llena");
    std::ofstream{dir + "/llena/dato.txt"} << "x";
    std::ofstream{dir + "/suelto.txt"} << "x";
    ASSERT_EQ(limpiarVacias(dir), 2);        // solo las dos vacías
    ASSERT_FALSE(fs::exists(dir + "/vacia1"));
    ASSERT_TRUE(fs::exists(dir + "/llena"));
    ASSERT_TRUE(fs::exists(dir + "/suelto.txt"));   // archivos no se tocan
}

int main() {
    return RUN_ALL_TESTS();
}
