/* =============================================================================
   MÓDULO 12 - EJERCICIO 04: slots de save en texto (L07 + error de L12)
   =============================================================================
   DIFICULTAD: Media
   CONSIGNA (tema 4 de TEMAS.md: archivos ifstream/ofstream, modos y texto):
     Los 3 slots de partida del curso van en UNA carpeta de disco real
     (dirTmp() de abajo: cada test usa su carpeta propia y la limpia):
       1. guardarSlot(dir, jugador, nombre, nivel): escribe el archivo
          "slot_<jugador>.txt" (ofstream en TRUNC: el save regenerado
          de siempre: L07) con EXACTAMENTE:
              jugador=<jugador>\n
              nombre=<nombre>\n
              nivel=<nivel>\n
          Devuelve false si la apertura falla SIEMPRE chequeando (!f).
       2. cargarSlot(dir, jugador, nombre, nivel): lee el archivo (getline
          en loop: L05) y completa los tres datos; false si no existe,
          si falta una línea o si el formato no es "clave=valor" (L12:
          cada lectura validada).
       3. copiarSlot(dir, de, a): copia TODO el save (con ifs/ofs en el
          mismo round trip de guardar/cargar) del slot "de" al "a".
          False si el origen no existe.
   NOTA: los archivos van a CADA test en su carpeta tmp propia: nada de
   basar el resultado en el CWD: el cwd de la corrida de verificar.sh no
   es una de las garantías del curso.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_04_slots_save.cpp -o ej04 && ./ej04
   ============================================================================= */
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

namespace fs = std::filesystem;

// (dada, no toques): carpeta tmp UNICA por corrida de tests:
std::string dirTmp(const char* etiqueta) {
    const fs::path dir =
        "/tmp/opencode/curso12_ej04_" + std::string(etiqueta);
    fs::remove_all(dir);             // limpieza de la corrida anterior
    fs::create_directories(dir); // realmente la crea
    return dir.string();
}

// TODO 1: guarda el save en texto (3 líneas exactas, arriba).
bool guardarSlot([[maybe_unused]] const std::string& dir,
                 [[maybe_unused]] const std::string& jugador,
                 [[maybe_unused]] const std::string& nombre,
                 [[maybe_unused]] int nivel) {
    // TU CODIGO AQUI (L07: ofstream + trunc + validar apertura)
    return false;    // placeholder
}

// TODO 2: carga el save; completo los TRES datos SOLO si TODO el formato
//         es válido (cualquier fallo: false y nada toca a medio llenar).
bool cargarSlot([[maybe_unused]] const std::string& dir,
                [[maybe_unused]] const std::string& jugador,
                [[maybe_unused]] std::string& nombre,
                [[maybe_unused]] int& nivel) {
    // TU CODIGO AQUI (getline en loop del L05 + validate por línea L12)
    return false;    // placeholder
}

// TODO 3: copia el slot: cargarSlot + guardarSlot (round trip con validación).
bool copiarSlot([[maybe_unused]] const std::string& dir,
                [[maybe_unused]] const std::string& de,
                [[maybe_unused]] const std::string& a) {
    // TU CODIGO AQUI
    return false;    // placeholder
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
    ASSERT_EQ(nombre, "Nombre2");       // trunc pisó lo viejo (L07)
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
    ASSERT_TRUE(nombre.empty());        // nada quedó parcialmente cargado
    ASSERT_EQ(nivel, 0);
}

TEST_CASE(CargarDeFormatoRotoFalla) {
    const std::string dir = dirTmp("roto");
    {
        std::ofstream f(dir + "/slot_mal.txt");
        f << "jugador=mal\nsolo-una-linea\n";   // faltan nombre y nivel
    }
    std::string nombre; int nivel = 0;
    ASSERT_FALSE(cargarSlot(dir, "mal", nombre, nivel));
    // Pero UNA línea con '=' y un int NO es un "jugador" válido:
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
    // el origen sigue intacto:
    ASSERT_TRUE(cargarSlot(dir, "origen", nombre, nivel));
    ASSERT_EQ(nivel, 3);
}

int main() {
    return RUN_ALL_TESTS();
}
