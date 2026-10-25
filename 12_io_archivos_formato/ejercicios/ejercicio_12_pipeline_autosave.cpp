/* =============================================================================
   MÓDULO 12 - EJERCICIO 12: pipeline del autosave: parsear, reportar, respaldar
   =============================================================================
   DIFICULTAD: Alta
   INTEGRATORIO (todo el módulo: L03/L04 + L05/L06 + L09-L12 + L14):
     El cierre del módulo es el problema REAL de la carpeta del save:
     una sóla utilidad que lee la config, arma el reporte de la
     carpeta y hace el backup. Las tres funciones juntas usan TODO el
     kit del módulo:
       1. cargarConfig(ruta): optional<Config>: el script "clave
          valor" de una por línea (L05): líneas vacías y '#' al frente
          NO se procesan; clave de la lista de abajo (la DESCONOCIDA
          se salta la línea entera SIN abortar) y valor INT COMPLETO
          (L06; línea con valor podrido: también se salta, el stream
          sirve para la siguiente línea). Claves: "vida",
          "monedas", "volumen". nullopt SOLO si el archivo no abre.
          Sin claves: el Config con sus defaults.
       2. crearReporte(dir): UN string con UNA reserva (L04):
          header std::format("{:<16}{:>8}{:>7}\n", "ARCHIVO",
          "BYTES", "LINEAS") + una fila POR CADA *.cfg DIRECTO de dir
          (L10; nombre lexicográfico) en el MISMO formato con
          el nombre (SIN path), el file_size (L09/L11, con ec)
          y cuántas LÍNEAS tiene (conteo rbuf=UNA lectura L13: '\n' de +
          1 si el último byte no es '\n' Y no vacío).
       3. copiaSeguridad(dir, destino): create_directories destino
          (L11) y copia (copy_file overwrite: L11) CADA *.cfg directo
          de dir con los TWO límites (política del autosave): el
          archivo va SI el file_size <= 4096; devuelve cuántos
          copió. Errores por error_code (L11): NO lanza.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_12_pipeline_autosave.cpp -o ej12 && ./ej12
   ============================================================================= */
#include <filesystem>
#include <format>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

namespace fs = std::filesystem;

struct Config {
    int vida = 100;
    int monedas = 0;
    int volumen = 50;
};

// (dada, no toques): tmp única por corrida:
std::string dirTmp(const char* etiqueta) {
    const fs::path dir = "/tmp/opencode/curso12_ej12_" + std::string(etiqueta);
    fs::remove_all(dir);
    fs::create_directories(dir);
    return dir.string();
}

// TODO 1: el parser del script con tolerancia de líneas (L05/L06):
std::optional<Config> cargarConfig([[maybe_unused]] const std::string& ruta) {
    // TU CODIGO AQUI
    return std::nullopt;    // placeholder
}

// TODO 2: el reporte L04+L10+L13: header + fila por *.cfg directo:
std::string crearReporte([[maybe_unused]] const std::string& dir) {
    // TU CODIGO AQUI
    return "";    // placeholder
}

// TODO 3: el backup con su política de tamaño (L11 con exp):
int copiaSeguridad([[maybe_unused]] const std::string& dir,
                   [[maybe_unused]] const std::string& destino) {
    // TU CODIGO AQUI
    return -1;    // placeholder
}

// ============= TESTS =============
bool escribir(const std::string& ruta, const std::string& contenido) {
    std::ofstream f(ruta, std::ios::binary);
    f << contenido;
    return true;
}

std::string configAString(const Config& c) {
    return std::to_string(c.vida) + "/" + std::to_string(c.monedas) + "/" +
           std::to_string(c.volumen);
}

// (dada): contiene es C++23; en C++20 el chequeo es find() != npos:
bool intiene(const std::string& todo, const std::string& pieza) {
    return todo.find(pieza) != std::string::npos;
}

TEST_CASE(ConfigConComentariosYMalos) {
    const std::string ruta = "/tmp/opencode/curso12_ej12_cfg.cfg";
    escribir(ruta,
             "# comentario entero\n"
             "\n"
             "vida 75\n"
             "monedas 200\n"
             "brillo alto\n"        // clave desconocida: SE SALTA
             "volumen 3x\n"         // valor podrido: SE SALTA
             "volumen 30\n");       // y el bien de arriba manda
    const auto c = cargarConfig(ruta);
    ASSERT_TRUE(static_cast<bool>(c));
    ASSERT_EQ(configAString(*c), "75/200/30");
}

TEST_CASE(ConfigSinClavesDaLosDefaults) {
    const std::string ruta = "/tmp/opencode/curso12_ej12_defaults.cfg";
    escribir(ruta, "# solo comentarios\n\n");
    const auto c = cargarConfig(ruta);
    ASSERT_TRUE(static_cast<bool>(c));
    ASSERT_EQ(configAString(*c), "100/0/50");
}

TEST_CASE(ConfigLaAperturaFallidaEsNulopt) {
    ASSERT_FALSE(cargarConfig("/curso12_ej12/no_existe.cfg").has_value());
}

TEST_CASE(ReporteFilasExactas) {
    const std::string dir = dirTmp("reporte");
    escribir(dir + "/b.cfg", "vida 1\nmonedas 2\n");       // 17 bytes / 2
    escribir(dir + "/a.cfg", "una\n");                     // 4 bytes / 1
    const std::string reporte = crearReporte(dir);
    // header + dos filas ordenadas lexicográficamente (L10):
    ASSERT_TRUE(intiene(reporte, std::format("{:<16}{:>8}{:>7}\n",
                                             "ARCHIVO", "BYTES", "LINEAS")));
    ASSERT_TRUE(intiene(reporte, std::format("{:<16}{:>8}{:>7}\n",
                                             "a.cfg", 4, 1)));
    ASSERT_TRUE(intiene(reporte, std::format("{:<16}{:>8}{:>7}\n",
                                             "b.cfg", 17, 2)));
    // y las filas en ese orden: la 'a' antes que la 'b':
    ASSERT_TRUE(reporte.find("a.cfg") < reporte.find("b.cfg"));
    // sin '\n' final lo que... la última línea SIN '\n' UNA sola cuenta:
    escribir(dir + "/c.cfg", "una\ndos");                  // sin salto final
    ASSERT_TRUE(intiene(crearReporte(dir), 
        std::format("{:<16}{:>8}{:>7}\n", "c.cfg", 7, 2)));
}

TEST_CASE(ReporteDeCarpetaVaciaEsSoloElHeader) {
    const std::string dir = dirTmp("vacía");
    ASSERT_EQ(crearReporte(dir),
              std::format("{:<16}{:>8}{:>7}\n", "ARCHIVO", "BYTES", "LINEAS"));
}

TEST_CASE(BackupRespetaLaPoliticaDeTamanio) {
    const std::string dir = dirTmp("backup");
    const std::string destino = dir + "_bk";
    escribir(dir + "/chico.cfg", std::string(50, 'x'));
    escribir(dir + "/grande.cfg", std::string(5000, 'x'));  // > 4096: NO va
    fs::create_directories(dir + "/sub");
    escribir(dir + "/sub/anidados.cfg", "y");              // Nivel 2: NO va
    ASSERT_EQ(copiaSeguridad(dir, destino), 1);
    ASSERT_TRUE(fs::exists(destino + "/chico.cfg"));
    ASSERT_FALSE(fs::exists(destino + "/grande.cfg"));
    ASSERT_FALSE(fs::exists(destino + "/sub"));            // nivel 2: NO va
    // y copia el contenido 1:1:
    std::ifstream f2(destino + "/chico.cfg", std::ios::binary);
    const std::string copia((std::istreambuf_iterator<char>(f2)),
                            std::istreambuf_iterator<char>());
    ASSERT_EQ(copia, std::string(50, 'x'));
}

TEST_CASE(BackupCarpetaVaciaCopiaCero) {
    const std::string dir = dirTmp("backup_v");
    ASSERT_EQ(copiaSeguridad(dir, dir + "_bk"), 0);
    ASSERT_TRUE(fs::exists(dir + "_bk"));          // create_directories igual
}

int main() {
    return RUN_ALL_TESTS();
}
