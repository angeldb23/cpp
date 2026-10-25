/* =============================================================================
   MÓDULO 12 - SOLUCIÓN 12: pipeline del autosave: parsear, reportar, respaldar
   =============================================================================
   INTEGRATORIO (ej. 12): L05/L06 (el script tolerante con int completo),
   L04 (un reporte con UNA reserva y format), L09-L11 (filesystem con
   error_code, copy_file overwrite) y L12 (apertura + operación + flush
   vistos). Las tres funciones EXISTEN juntas: el pipeline real del save.
   ============================================================================= */
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
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

// 1) el script "clave valor" con tolerancia por línea (L05/L06):
std::optional<Config> cargarConfig(const std::string& ruta) {
    std::ifstream f(ruta);
    if (!f) return std::nullopt;             // SOLO aquí va el nullopt
    Config c;
    std::string linea;
    while (std::getline(f, linea)) {
        size_t i = 0;
        while (i < linea.size() &&
               std::isspace(static_cast<unsigned char>(linea[i]))) ++i;
        if (i >= linea.size()) continue;     // vacía
        if (linea[i] == '#') continue;       // comentario
        std::istringstream ss(linea);
        std::string clave;
        int valor = 0;
        if (!(ss >> clave)) continue;        // sin clave
        if (!(ss >> valor)) continue;        // valor no enterable
        std::string sobra;
        if (ss >> sobra) continue;           // int CON COLA: línea podrida
        if (clave == "vida") c.vida = valor;
        else if (clave == "monedas") c.monedas = valor;
        else if (clave == "volumen") c.volumen = valor;
        // clave desconocida: línea ignorada SIN abortar
    }
    return c;
}

// (2a) línea: '\n' de +1 si el último byte NO es '\n' (y no vacío).
//      UNA lectura (L13): todo el archivo copiado y en memoria:
size_t contarLineas(const std::string& ruta) {
    std::ifstream f(ruta, std::ios::binary);
    if (!f) return 0;
    const std::string datos((std::istreambuf_iterator<char>(f)),
                            std::istreambuf_iterator<char>());
    size_t lineas = 0;
    for (const char c : datos) {
        if (c == '\n') ++lineas;
    }
    if (!datos.empty() && datos.back() != '\n') ++lineas;
    return lineas;
}

// 2) el reporte: header + fila por *.cfg DIRECTO, en orden lexicográfico:
std::string crearReporte(const std::string& dir) {
    std::string reporte;
    reporte.reserve(256);
    reporte += std::format("{:<16}{:>8}{:>7}\n", "ARCHIVO", "BYTES", "LINEAS");

    std::vector<fs::path> cfgs;
    std::error_code ec;
    for (fs::directory_iterator it(dir, ec), fin; it != fin; it.increment(ec)) {
        if (ec) break;
        if (it->is_regular_file(ec) && !ec &&
            it->path().extension() == ".cfg") {
            cfgs.push_back(it->path());
        }
    }
    std::sort(cfgs.begin(), cfgs.end());     // determinista (L10)
    for (const fs::path& p : cfgs) {
        std::error_code ec2;
        const auto bytes = fs::file_size(p, ec2);
        const size_t total = ec2 ? 0 : static_cast<size_t>(bytes);
        reporte += std::format("{:<16}{:>8}{:>7}\n",
                               p.filename().string(), total, contarLineas(p));
    }
    return reporte;
}

// 3) el backup: create_directories + copy_file(overwrite) con ec (L11);
//    política: SOLO archivos <= 4096 bytes y SOLO nivel 1:
int copiaSeguridad(const std::string& dir, const std::string& destino) {
    std::error_code ec;
    fs::create_directories(destino, ec);     // carnet vacío también se crea
    if (ec) return 0;
    int copiados = 0;
    for (fs::directory_iterator it(dir, ec), fin; it != fin; it.increment(ec)) {
        if (ec) break;
        if (!it->is_regular_file(ec) || ec) continue;
        if (it->path().extension() != ".cfg") continue;
        std::error_code ec2;
        if (fs::file_size(it->path(), ec2) > 4096 || ec2) continue;
        std::error_code ec3;
        if (fs::copy_file(it->path(), fs::path(destino) /
                                    it->path().filename(),
                          fs::copy_options::overwrite_existing, ec3)) {
            ++copiados;
        }
    }
    return copiados;
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
    ASSERT_TRUE(reporte.find(std::format("{:<16}{:>8}{:>7}\n",
                                         "ARCHIVO", "BYTES", "LINEAS")) !=
                std::string::npos);
    ASSERT_TRUE(reporte.find(std::format("{:<16}{:>8}{:>7}\n",
                                         "a.cfg", 4, 1)) != std::string::npos);
    ASSERT_TRUE(reporte.find(std::format("{:<16}{:>8}{:>7}\n",
                                         "b.cfg", 17, 2)) != std::string::npos);
    ASSERT_TRUE(reporte.find("a.cfg") < reporte.find("b.cfg"));
    escribir(dir + "/c.cfg", "una\ndos");                  // sin salto final
    ASSERT_TRUE(crearReporte(dir).find(
        std::format("{:<16}{:>8}{:>7}\n", "c.cfg", 7, 2)) !=
        std::string::npos);
}

TEST_CASE(ReporteDeCarpetaVaciaEsSoloElHeader) {
    const std::string dir = dirTmp("vacia");
    ASSERT_EQ(crearReporte(dir),
              std::format("{:<16}{:>8}{:>7}\n", "ARCHIVO", "BYTES", "LINEAS"));
}

TEST_CASE(BackupRespetaLaPoliticaDeTamanio) {
    const std::string dir = dirTmp("backup");
    const std::string destino = dir + "_bk";
    escribir(dir + "/chico.cfg", std::string(50, 'x'));
    escribir(dir + "/grande.cfg", std::string(5000, 'x'));  // > 4096: NO va
    fs::create_directories(dir + "/sub");
    escribir(dir + "/sub/anidados.cfg", "y");              // nivel 2: NO va
    ASSERT_EQ(copiaSeguridad(dir, destino), 1);
    ASSERT_TRUE(fs::exists(destino + "/chico.cfg"));
    ASSERT_FALSE(fs::exists(destino + "/grande.cfg"));
    ASSERT_FALSE(fs::exists(destino + "/sub"));
    // y copia el contenido 1:1:
    std::ifstream f2(destino + "/chico.cfg", std::ios::binary);
    const std::string copia((std::istreambuf_iterator<char>(f2)),
                            std::istreambuf_iterator<char>());
    ASSERT_EQ(copia, std::string(50, 'x'));
}

TEST_CASE(BackupCarpetaVaciaCopiaCero) {
    const std::string dir = dirTmp("backup_v");
    ASSERT_EQ(copiaSeguridad(dir, dir + "_bk"), 0);
    ASSERT_TRUE(fs::exists(dir + "_bk"));
}

int main() {
    std::cout << "Ejecutando pruebas para Solucion 12 (pipeline autosave)...\n";
    return RUN_ALL_TESTS();
}
