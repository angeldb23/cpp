/* =============================================================================
   MÓDULO 25 - SOLUCIÓN 08: escritor del pack (TOC + alineación a 16)
   =============================================================================
   Los "3 momentos" del cooker de la lección 08: primero se serializa el TOC
   con offsets falso-0 para MEDIR su tamaño (el tamaño del TOC depende de los
   largos de los nombres, y los offsets dependen del tamaño del TOC: crece
   mutuo). Después se recorre de nuevo asignando offsets alineados a 16, y
   recién ahí se arma el buffer final con header + TOC real + datos.

   El layout es el mismo que leen los packs "custom" de motores chicos (sin
   compresión y con compresión por chunk si se agrega: la TOC no cambia).
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

void pushU32(std::vector<std::uint8_t>& v, std::uint32_t x) {
    v.push_back(static_cast<std::uint8_t>(x));
    v.push_back(static_cast<std::uint8_t>(x >> 8));
    v.push_back(static_cast<std::uint8_t>(x >> 16));
    v.push_back(static_cast<std::uint8_t>(x >> 24));
}
void pushU64(std::vector<std::uint8_t>& v, std::uint64_t x) {
    pushU32(v, static_cast<std::uint32_t>(x));
    pushU32(v, static_cast<std::uint32_t>(x >> 32));
}
void pushStr(std::vector<std::uint8_t>& v, const std::string& s) {
    pushU32(v, static_cast<std::uint32_t>(s.size()));
    for (char c : s) { v.push_back(static_cast<std::uint8_t>(c)); }
}
std::size_t alinear(std::size_t cursor) {
    const std::size_t resto = cursor % kAlineacion;
    return resto == 0 ? cursor : cursor + (kAlineacion - resto);
}

class PackWriter {
public:
    void addLump(std::string nombre, std::vector<std::uint8_t> datos) {
        lumps_.push_back(Lump{ std::move(nombre), std::move(datos), 0 });
    }

    std::vector<std::uint8_t> build() {
        /* MOMENTO 1: medir el TOC (offsets/size provisorios). */
        std::vector<std::uint8_t> toc;
        for (const Lump& l : lumps_) {
            pushStr(toc, l.nombre);
            pushU64(toc, 0); /* offset */
            pushU64(toc, 0); /* size */
        }
        /* MOMENTO 2: offsets. header=12, primer lump alineado, y cada
           lump siguiente re-alineado (padding entre lumps). */
        std::size_t cursor = alinear(12 + toc.size());
        for (Lump& l : lumps_) {
            cursor = alinear(cursor);
            l.offset = cursor;
            cursor += l.datos.size();
        }
        /* MOMENTO 3: buffer final. */
        std::vector<std::uint8_t> out;
        out.reserve(cursor);
        pushU32(out, kMagicPack);
        pushU32(out, kVersionPack);
        pushU32(out, static_cast<std::uint32_t>(lumps_.size()));
        for (const Lump& l : lumps_) {
            pushStr(out, l.nombre);
            pushU64(out, l.offset);
            pushU64(out, l.datos.size());
        }
        out.resize(alinear(out.size()), 0); /* padding hasta el primer dato */
        for (const Lump& l : lumps_) {
            out.resize(alinear(out.size()), 0); /* re-alinear entre lumps */
            out.insert(out.end(), l.datos.begin(), l.datos.end());
        }
        return out;
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
