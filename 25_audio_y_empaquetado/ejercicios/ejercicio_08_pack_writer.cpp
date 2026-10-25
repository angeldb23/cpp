/* =============================================================================
   MÓDULO 25 - EJERCICIO 08: escritor del pack (TOC + alineación a 16)
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 08 (formato pack "VPAK", TOC antes de los datos).

   CONSIGNA
   --------
   Completá `PackWriter` para que produzca el binario de la lección:

     magic     "VPAK" (little endian = 0x4B415056)   4 bytes
     version   1                                     4 bytes
     count     cantidad de lumps                     4 bytes
     TOC (count entradas):
       nameLen u32, name (nameLen bytes), offset u64, size u64
     DATA: los lumps concatenados, CADA UNO alineado a 16 bytes
           (padding con ceros entre lumps y después del header).

   Pasos sugeridos (los "3 momentos" de la lección):
     1. Serializar el TOC con offsets de relleno (0) para saber su tamaño.
     2. Cursor = alinear(12 + tocSize): ahí arranca el primer lump; cada
        lump se alinea de nuevo y ahí va su offset real.
     3. Serializar todo: header + TOC con offsets reales + datos con padding.

   Los tres test miran bytes crudos del buffer: no hace falta lecto-ayuda.
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace {

inline constexpr std::uint32_t kMagicPack = 0x4B415056; /* "VPAK" LE */
inline constexpr std::uint32_t kVersionPack = 1;
inline constexpr std::size_t kAlineacion = 16;

class PackWriter {
public:
    void addLump(std::string nombre, std::vector<std::uint8_t> datos) {
        lumps_.push_back(Lump{ std::move(nombre), std::move(datos), 0 });
    }

    std::vector<std::uint8_t> build() {
        // TODO
        return {};
    }

private:
    struct Lump {
        std::string nombre;
        std::vector<std::uint8_t> datos;
        std::uint64_t offset = 0;
    };
    std::vector<Lump> lumps_;
};

} // namespace

/* ------------------------------ TESTS (no tocar)-------------------------- */
TEST_CASE(HeaderYCount) {
    PackWriter w;
    w.addLump("sfx/paso.bin", { 1, 2, 3, 4, 5 });
    w.addLump("sfx/tiro.bin", { 9, 9, 9, 9, 9, 9 });
    const std::vector<std::uint8_t> file = w.build();
    ASSERT_TRUE(file.size() >= 12);
    /* magic LE: los 4 bytes son 'V','P','A','K' */
    ASSERT_EQ(file[0], static_cast<std::uint8_t>(0x56)); /* 'V' */
    ASSERT_EQ(file[1], static_cast<std::uint8_t>(0x50)); /* 'P' */
    ASSERT_EQ(file[2], static_cast<std::uint8_t>(0x41)); /* 'A' */
    ASSERT_EQ(file[3], static_cast<std::uint8_t>(0x4B)); /* 'K' */
    /* version = 1 (LE) */
    ASSERT_EQ(file[4] | (file[5] << 8) | (file[6] << 16) | (file[7] << 24),
              kVersionPack);
    /* count = 2 (LE) */
    ASSERT_EQ(file[8] | (file[9] << 8) | (file[10] << 16) | (file[11] << 24),
              static_cast<std::uint32_t>(2));
}

TEST_CASE(TocOffsetsAlineadosYDatos) {
    PackWriter w;
    w.addLump("a.bin", { 1, 2, 3, 4, 5 });
    w.addLump("bbbb.bin", { 10, 20, 30 });
    const std::vector<std::uint8_t> file = w.build();
    ASSERT_TRUE(file.size() > 12);
    std::size_t p = 12; /* después del header */
    auto u32 = [&](const std::uint8_t* q) -> std::uint32_t {
        return static_cast<std::uint32_t>(q[0]) | (static_cast<std::uint32_t>(q[1]) << 8)
             | (static_cast<std::uint32_t>(q[2]) << 16)
             | (static_cast<std::uint32_t>(q[3]) << 24);
    };
    auto u64 = [&](const std::uint8_t* q) -> std::uint64_t {
        return u32(q) + (static_cast<std::uint64_t>(u32(q + 4)) << 32);
    };
    /* entrada 1: a.bin */
    const std::uint32_t len1 = u32(file.data() + p); p += 4;
    ASSERT_EQ(len1, static_cast<std::uint32_t>(5));
    const std::string n1(reinterpret_cast<const char*>(file.data() + p), len1);
    p += len1;
    ASSERT_TRUE(n1 == "a.bin");
    const std::uint64_t off1 = u64(file.data() + p); p += 8;
    const std::uint64_t sz1 = u64(file.data() + p); p += 8;
    ASSERT_EQ(sz1, static_cast<std::uint64_t>(5));
    ASSERT_EQ(off1 % kAlineacion, static_cast<std::uint64_t>(0));
    ASSERT_GT(file.size(), static_cast<std::size_t>(off1 + 5));
    ASSERT_EQ(file[static_cast<std::size_t>(off1)], 1);
    ASSERT_EQ(file[static_cast<std::size_t>(off1 + 4)], 5);
    /* entrada 2: bbbb.bin */
    const std::uint32_t len2 = u32(file.data() + p); p += 4;
    ASSERT_EQ(len2, static_cast<std::uint32_t>(8));
    const std::string n2(reinterpret_cast<const char*>(file.data() + p), len2);
    p += len2;
    ASSERT_TRUE(n2 == "bbbb.bin");
    const std::uint64_t off2 = u64(file.data() + p); p += 8;
    const std::uint64_t sz2 = u64(file.data() + p); p += 8;
    ASSERT_EQ(sz2, static_cast<std::uint64_t>(3));
    ASSERT_EQ(off2 % kAlineacion, static_cast<std::uint64_t>(0));
    ASSERT_GT(off2, off1); /* van creciendo */
    ASSERT_EQ(file[static_cast<std::size_t>(off2)], 10);
    ASSERT_EQ(file[static_cast<std::size_t>(off2 + 2)], 30);
    /* todo entra en el archivo: el último dato termina dentro del buffer */
    ASSERT_GE(file.size(), static_cast<std::size_t>(off2 + sz2));
}

TEST_CASE(AlineacionEntreLumps) {
    /* un lump de 17 bytes FUERZA padding: el próximo offset no puede ser
       header+17, hay que saltar hasta el próximo múltiplo de 16. */
    PackWriter w;
    w.addLump("x.bin", std::vector<std::uint8_t>(17, 0xAB));
    w.addLump("y.bin", std::vector<std::uint8_t>(4, 0xCD));
    const std::vector<std::uint8_t> file = w.build();
    ASSERT_TRUE(file.size() > 12);
    std::size_t p = 12;
    auto u32 = [&](const std::uint8_t* q) -> std::uint32_t {
        return static_cast<std::uint32_t>(q[0]) | (static_cast<std::uint32_t>(q[1]) << 8)
             | (static_cast<std::uint32_t>(q[2]) << 16)
             | (static_cast<std::uint32_t>(q[3]) << 24);
    };
    auto u64 = [&](const std::uint8_t* q) -> std::uint64_t {
        return u32(q) + (static_cast<std::uint64_t>(u32(q + 4)) << 32);
    };
    const std::uint32_t len1 = u32(file.data() + p); p += 4 + len1;
    const std::uint64_t off1 = u64(file.data() + p); p += 16;
    const std::uint32_t len2 = u32(file.data() + p); p += 4 + len2;
    const std::uint64_t off2 = u64(file.data() + p); p += 16;
    ASSERT_EQ(off1 % kAlineacion, static_cast<std::uint64_t>(0));
    ASSERT_EQ(off2 % kAlineacion, static_cast<std::uint64_t>(0));
    /* 17 bytes no dividen 16: debe haber AL MENOS 1 byte de padding antes
       del segundo lump (off2 - (off1 + 17) >= 1). */
    ASSERT_GE(off2 - off1, static_cast<std::uint64_t>(17) + 1);
    ASSERT_TRUE(file.size() >= static_cast<std::size_t>(off2 + 4));
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
