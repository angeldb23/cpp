/* =============================================================================
   MÓDULO 12 - EJERCICIO 05: save binario con header y round trip (L08)
   =============================================================================
   DIFICULTAD: Media
   CONSIGNA (tema 4 de TEMAS.md: binario con modos, texto vs binario):
     El save real del ítem (posiciones de waypoints) va en BINARIO y con
     el canon de la L08: header MAGIC+VERSION y data de ancho fijo:
       1. guardarWaypoints(dir, lista): escribe "waypoints.bin" en dir:
          MAGIC "WPAY" (4 bytes) + VERSION (uint32=1) + COUNT (uint32)
          + para cada waypoint: X (int32) + Y (int32) + TIPO (uint8).
          False si la apertura o CUALQUIER write fallan (L12).
       2. cargarWaypoints(dir, lista): devuelve false si no existe, si el
          MAGIC no es WPAY, si la VERSION no es 1, si el archivo está
          corto medio-data o si COUNT es absurdamente grande; en los
          casos de datos correctos: los llena TODOS (y no toca la lista
          en el fallo: L12).
       3. tamanoEsperado(n): el número de BYTES exactos que debe ocupar
          el archivo para n waypoints (header + data): el checkpoint del
          problema (size() del FS luego DEBE dar este número: L10).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_05_waypoints_bin.cpp -o ej05 && ./ej05
   ============================================================================= */
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

namespace fs = std::filesystem;

struct Waypoint {
    std::int32_t x = 0;
    std::int32_t y = 0;
    std::uint8_t tipo = 0;    // 0 normal, 1 checkpoint, 2 meta
};

// (dada, no toques): carpeta con nombre UNICO por corrida:
std::string dirTmp(const char* etiqueta) {
    const fs::path dir = "/tmp/opencode/curso12_ej05_" + std::string(etiqueta);
    fs::remove_all(dir);
    fs::create_directories(dir);
    return dir.string();
}

constexpr char MAGIA[] = "WPAY";
constexpr std::uint32_t VERSION = 1;

// TODO 1: la escritura completa con el canon de la L08 (validando TODO).
bool guardarWaypoints([[maybe_unused]] const std::string& dir,
                      [[maybe_unused]] const std::vector<Waypoint>& lista) {
    // TU CODIGO AQUI (ofstream binary|trunc; write del header+count+data)
    return false;    // placeholder
}

// TODO 2: la carga con TODAS las validaciones de la consigna.
bool cargarWaypoints([[maybe_unused]] const std::string& dir,
                     [[maybe_unused]] std::vector<Waypoint>& lista) {
    // TU CODIGO AQUI (ifstream binary: leery validar magic, version,
    // count; luego cargar cada waypoint; cualquier fallo: false)
    return false;    // placeholder
}

// TODO 3: bytes exactos esperados para n waypoints.
size_t tamanoEsperado([[maybe_unused]] size_t n) {
    // TU CODIGO AQUI
    return 0;    // placeholder
}

// ============= TESTS =============
TEST_CASE(RoundTripCompleto) {
    const std::string dir = dirTmp("roundtrip");
    const std::vector<Waypoint> lista{
        {10, 20, 0}, {-100, 250, 1}, {0, 0, 2}, {3000000, -3000000, 1}};
    ASSERT_TRUE(guardarWaypoints(dir, lista));
    std::vector<Waypoint> salida;
    ASSERT_TRUE(cargarWaypoints(dir, salida));
    ASSERT_EQ(salida.size(), lista.size());
    for (size_t i = 0; i < lista.size(); ++i) {
        ASSERT_EQ(salida[i].x, lista[i].x);
        ASSERT_EQ(salida[i].y, lista[i].y);
        ASSERT_EQ(salida[i].tipo, lista[i].tipo);
    }
}

TEST_CASE(TamanoEnDiscoEsElEsperado) {
    const std::string dir = dirTmp("tamano");
    const std::vector<Waypoint> lista{{1,2,0},{3,4,1},{5,6,2}};
    ASSERT_TRUE(guardarWaypoints(dir, lista));
    const size_t esperado = tamanoEsperado(3);
    ASSERT_TRUE(esperado > 0);
    const size_t real = static_cast<size_t>(
        fs::file_size(dir + "/waypoints.bin"));
    ASSERT_EQ(real, esperado);
}

TEST_CASE(MagicEquivoconRechaza) {
    const std::string dir = dirTmp("falso");
    {
        std::ofstream f(dir + "/waypoints.bin", std::ios::binary);
        const char basura[12] = {'X','Y','Z','z', 1,0,0,0, 1,2,3,4};
        f.write(basura, sizeof(basura));
    }
    std::vector<Waypoint> salida;
    ASSERT_FALSE(cargarWaypoints(dir, salida));
}

TEST_CASE(VersionViejaRechaza) {
    const std::string dir = dirTmp("version");
    {
        std::ofstream f(dir + "/waypoints.bin", std::ios::binary);
        const std::uint32_t versionVieja = 0;    // no es la nuestra
        const std::uint32_t count = 1;
        f.write(MAGIA, 4);
        f.write(reinterpret_cast<const char*>(&versionVieja),
                sizeof(versionVieja));
        f.write(reinterpret_cast<const char*>(&count), sizeof(count));
    }
    std::vector<Waypoint> salida;
    ASSERT_FALSE(cargarWaypoints(dir, salida));
}

TEST_CASE(CorteAMitadRechaza) {
    const std::string dir = dirTmp("corto");
    const std::vector<Waypoint> lista{{1,2,0},{3,4,1},{5,6,2},{7,8,0}};
    ASSERT_TRUE(guardarWaypoints(dir, lista));
    // el archivo queda con SOLO el header y el count: media data menos:
    std::string bytes;
    {
        std::ifstream f(dir + "/waypoints.bin", std::ios::binary);
        bytes = std::string(
            (std::istreambuf_iterator<char>(f)),
             std::istreambuf_iterator<char>());
    }
    ASSERT_EQ(bytes.size(), tamanoEsperado(4));
    {
        std::ofstream f(dir + "/waypoints.bin",
                        std::ios::binary | std::ios::trunc);
        f.write(bytes.data(), 16);              // header + count, SIN data
    }
    std::vector<Waypoint> salida;
    ASSERT_FALSE(cargarWaypoints(dir, salida));
}

TEST_CASE(NoExisteRechaza) {
    const std::string dir = dirTmp("no-existe");
    std::vector<Waypoint> salida;
    ASSERT_FALSE(cargarWaypoints(dir, salida));
}

int main() {
    return RUN_ALL_TESTS();
}
