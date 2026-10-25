/* =============================================================================
   MÓDULO 12 - SOLUCIÓN 05: save binario con header y round trip (ej. 05)
   =============================================================================
   L08 es la lección de respaldo: MAGIC + VERSION + COUNT arriba y data
   de ancho FIJO (int32 int32 uint8). La carga valida CADA premisa
   (L12): apertura, magic, version, "el archivo mide EXACTO lo que
   cuenta el header" (el corte a mitad y el count absurdo quedan
   ífuera por la ARITMÉTICA del tamaño) y no toca la lista a medias.
   ============================================================================= */
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

#include <cstring>      // memcmp

namespace fs = std::filesystem;

struct Waypoint {
    std::int32_t x = 0;
    std::int32_t y = 0;
    std::uint8_t tipo = 0;
};

// (dada, copiada del ejercicio): carpeta única por corrida:
std::string dirTmp(const char* etiqueta) {
    const fs::path dir = "/tmp/opencode/curso12_ej05_" + std::string(etiqueta);
    fs::remove_all(dir);
    fs::create_directories(dir);
    return dir.string();
}

constexpr char MAGIA[] = "WPAY";
constexpr std::uint32_t VERSION = 1;
constexpr std::uint32_t COUNT_MAX = 1'000'000;   // techo del count absurdo

bool escribirUint32(std::ofstream& f, std::uint32_t v) {
    f.write(reinterpret_cast<const char*>(&v), sizeof(v));
    return static_cast<bool>(f);                 // L12: validar en cada paso
}

bool escribirInt32(std::ofstream& f, std::int32_t v) {
    f.write(reinterpret_cast<const char*>(&v), sizeof(v));
    return static_cast<bool>(f);
}

// 1) el write completo con su canon (L08): validar apertura, cada
//    write del header y cada write de data:
bool guardarWaypoints(const std::string& dir,
                      const std::vector<Waypoint>& lista) {
    std::ofstream f(dir + "/waypoints.bin",
                    std::ios::binary | std::ios::trunc);
    if (!f) return false;                        // 1) apertura
    f.write(MAGIA, 4);                                    // 2) header
    if (!f) return false;
    if (!escribirUint32(f, VERSION)) return false;
    const std::uint32_t count = static_cast<std::uint32_t>(lista.size());
    if (!escribirUint32(f, count)) return false;
    for (const Waypoint& wp : lista) {                    // 3) data
        if (!escribirInt32(f, wp.x)) return false;
        if (!escribirInt32(f, wp.y)) return false;
        f.put(static_cast<char>(wp.tipo));
        if (!f) return false;
    }
    f.flush();
    return !f.fail();                       // 4) cierre vistos (L12)
}

// 3) los bytes exactos (el checkpoint + size del FS):
size_t tamanoEsperado(size_t n) {
    return 4 + sizeof(std::uint32_t) + sizeof(std::uint32_t) +
           n * (sizeof(std::int32_t) + sizeof(std::int32_t) +
                sizeof(std::uint8_t));     // header + 9 bytes por waypoint
}

// 2) TODOS los compromisos del read validados:
bool cargarWaypoints(const std::string& dir, std::vector<Waypoint>& lista) {
    const std::string ruta = dir + "/waypoints.bin";
    std::ifstream f(ruta, std::ios::binary);
    if (!f) return false;                              // no existe

    char magia[4] = {};
    f.read(magia, 4);                                  // magic
    if (!f || std::memcmp(magia, MAGIA, 4) != 0) return false;

    std::uint32_t version = 0, count = 0;              // version y count
    f.read(reinterpret_cast<char*>(&version), sizeof(version));
    f.read(reinterpret_cast<char*>(&count), sizeof(count));
    if (!f) return false;
    if (version != VERSION) return false;

    std::error_code ec;                                // el check del tamaño
    const std::uintmax_t real = fs::file_size(ruta, ec);
    if (ec) return false;
    if (static_cast<uintmax_t>(tamanoEsperado(count)) != real) return false;
    if (count > COUNT_MAX) return false;    // y el count absurdo no pasa

    std::vector<Waypoint> tmp;                // TODO o NADA (L12)
    tmp.reserve(count);
    for (std::uint32_t i = 0; i < count; ++i) {
        Waypoint wp;
        f.read(reinterpret_cast<char*>(&wp.x), sizeof(wp.x));
        f.read(reinterpret_cast<char*>(&wp.y), sizeof(wp.y));
        f.read(reinterpret_cast<char*>(&wp.tipo), sizeof(wp.tipo));
        if (!f) return false;
        tmp.push_back(wp);
    }
    lista = tmp;                              // solo al final: completo
    return true;
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
        const std::uint32_t versionVieja = 0;
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
    std::cout << "Ejecutando pruebas para Solucion 05 (waypoints en binario)...\n";
    return RUN_ALL_TESTS();
}
