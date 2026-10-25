/* =============================================================================
   MÓDULO 12 - SOLUCIÓN 04: slots de save en texto (ej. 04)
   =============================================================================
   L07 + L12: el ofstream en TRUNC pisa el viejo (el save regenerado),
   la CARGA llena TODO o NADA (temporales para no dejar el save a
   medias) y cada línea exige su "clave=valor" con int COMPLETO (L06).
   El copiarSlot es el round trip de las dos: cargar + guardar.
   ============================================================================= */
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "../../include/test_framework.hpp"

namespace fs = std::filesystem;

// (dada, copiada del ejercicio): carpeta tmp única por corrida:
std::string dirTmp(const char* etiqueta) {
    const fs::path dir =
        "/tmp/opencode/curso12_ej04_" + std::string(etiqueta);
    fs::remove_all(dir);
    fs::create_directories(dir);
    return dir.string();
}

// 1) el save en TRUNC: 3 líneas exactas + validar TODO (apertura + write):
bool guardarSlot(const std::string& dir, const std::string& jugador,
                 const std::string& nombre, int nivel) {
    std::ofstream f(dir + "/slot_" + jugador + ".txt", std::ios::trunc);
    if (!f) return false;                    // L12: apertura
    f << "jugador=" << jugador << '\n'
      << "nombre=" << nombre << '\n'
      << "nivel=" << nivel << '\n';
    f.flush();
    return !f.fail();                        // L12: operación + flush vistos
}

// 2) TODO o NADA: los tres datos van en temporales y SOLO al final
//    se asignan (el fallo NO deja el save a medias):
bool cargarSlot(const std::string& dir, const std::string& jugador,
                std::string& nombre, int& nivel) {
    std::ifstream f(dir + "/slot_" + jugador + ".txt");
    if (!f) return false;                               // no existe
    std::string linea;
    if (!std::getline(f, linea)) return false;
    if (linea.compare(0, 8, "jugador=") != 0) return false;
    if (!std::getline(f, linea)) return false;
    if (linea.compare(0, 7, "nombre=") != 0) return false;
    const std::string nombreTmp = linea.substr(7);
    if (!std::getline(f, linea)) return false;
    if (linea.compare(0, 6, "nivel=") != 0) return false;
    const std::string restoNivel = linea.substr(6);
    std::istringstream p(restoNivel);                   // int COMPLETO (L06)
    int nivelTmp = 0;
    if (!(p >> nivelTmp)) return false;
    std::string sobra;
    if (p >> sobra) return false;                       // "abc"/"7x" NO
    nombre = nombreTmp;
    nivel = nivelTmp;
    return true;
}

// 3) el round trip con las dos de arriba (y sus validaciones):
bool copiarSlot(const std::string& dir, const std::string& de,
                const std::string& a) {
    std::string nombre;
    int nivel = 0;
    if (!cargarSlot(dir, de, nombre, nivel)) return false;
    return guardarSlot(dir, a, nombre, nivel);
}

// ============= TESTS =============
TEST_CASE(GuardarYCargarSlot) {
    const std::string dir = dirTmp("guardar");
    ASSERT_TRUE(guardarSlot(dir, "angel", "Caballero", 7));
    std::string nombre; int nivel = 0;
    ASSERT_TRUE(cargarSlot(dir, "angel", nombre, nivel));
    ASSERT_EQ(nombre, "Caballero");
    ASSERT_EQ(nivel, 7);
}

TEST_CASE(GuardarPisaElViejo) {
    const std::string dir = dirTmp("pisa");
    ASSERT_TRUE(guardarSlot(dir, "angel", "Nombre1", 1));
    ASSERT_TRUE(guardarSlot(dir, "angel", "Nombre2", 2));
    std::string nombre; int nivel = 0;
    ASSERT_TRUE(cargarSlot(dir, "angel", nombre, nivel));
    ASSERT_EQ(nombre, "Nombre2");
    ASSERT_EQ(nivel, 2);
    std::ifstream f(dir + "/slot_angel.txt");
    std::string todas;
    std::string linea;
    while (std::getline(f, linea)) todas += linea + "\n";
    ASSERT_EQ(todas, "jugador=angel\nnombre=Nombre2\nnivel=2\n");
}

TEST_CASE(CargarDeInexistenteFalla) {
    const std::string dir = dirTmp("inexistente");
    std::string nombre; int nivel = 0;
    ASSERT_FALSE(cargarSlot(dir, "fantasma", nombre, nivel));
    ASSERT_TRUE(nombre.empty());
    ASSERT_EQ(nivel, 0);
}

TEST_CASE(CargarDeFormatoRotoFalla) {
    const std::string dir = dirTmp("roto");
    {
        std::ofstream f(dir + "/slot_mal.txt");
        f << "jugador=mal\nsolo-una-linea\n";
    }
    std::string nombre; int nivel = 0;
    ASSERT_FALSE(cargarSlot(dir, "mal", nombre, nivel));
    {
        std::ofstream f2(dir + "/slot_pirata.txt");
        f2 << "jugador=pirata\nnombre=hola\nnivel=abc\n";
    }
    std::string nombre2; int nivel2 = 0;
    ASSERT_FALSE(cargarSlot(dir, "pirata", nombre2, nivel2));
}

TEST_CASE(CopiarSlot) {
    const std::string dir = dirTmp("copiar");
    ASSERT_TRUE(guardarSlot(dir, "origen", "Refleja", 3));
    ASSERT_TRUE(copiarSlot(dir, "origen", "destino"));
    std::string nombre; int nivel = 0;
    ASSERT_TRUE(cargarSlot(dir, "destino", nombre, nivel));
    ASSERT_EQ(nombre, "Refleja");
    ASSERT_EQ(nivel, 3);
    ASSERT_TRUE(cargarSlot(dir, "origen", nombre, nivel));
    ASSERT_EQ(nivel, 3);
}

int main() {
    std::cout << "Ejecutando pruebas para Solucion 04 (slots de save: texto)...\n";
    return RUN_ALL_TESTS();
}
