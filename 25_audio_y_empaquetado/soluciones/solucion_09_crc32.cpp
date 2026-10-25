/* =============================================================================
   MÓDULO 25 - SOLUCIÓN 09: CRC32 (tabla 0xEDB88320 + streaming por chunks)
   =============================================================================
   La tabla sale de la división polinómica "invertida" (bitwise) que construye
   el update reflejado: cada entrada es lo que le pasa al registro de CRC al
   meter el byte i sin datos previos. Con la tabla, cada byte del update es
   un solo XOR + lookup: esa es la versión que usan ZIP/PNG/GZIP en sus
   entradas/checksums.

   Nota del streaming: crc_ NO se reinicia entre updates (arranca 0xFFFFFFFF
   en el miembro y se arrastra): por eso dos chunks concatenados dan lo mismo
   que un one-shot. El XOR final con 0xFFFFFFFF solo se hace en final().
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <array>
#include <cstdint>

namespace {

class Crc32 {
public:
    static const std::array<std::uint32_t, 256>& tabla() {
        static const std::array<std::uint32_t, 256> t = crearTabla();
        return t;
    }

    void update(const std::uint8_t* data, std::size_t n) {
        for (std::size_t i = 0; i < n; ++i) {
            crc_ = (crc_ >> 8) ^ tabla()[(crc_ ^ data[i]) & 0xFFu];
        }
    }

    std::uint32_t final() const { return crc_ ^ 0xFFFFFFFFu; }

    /* One-shot: todo en una pasita (el caso típico del writer). */
    static std::uint32_t deUnaVez(const std::uint8_t* data, std::size_t n) {
        Crc32 c;
        c.update(data, n);
        return c.final();
    }

private:
    static std::array<std::uint32_t, 256> crearTabla() {
        std::array<std::uint32_t, 256> t{};
        for (std::uint32_t i = 0; i < 256; ++i) {
            std::uint32_t c = i;
            for (int bit = 0; bit < 8; ++bit) {
                c = (c & 1u) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
            }
            t[i] = c;
        }
        return t;
    }

    std::uint32_t crc_ = 0xFFFFFFFFu;
};

} // namespace

/* ------------------------------ TESTS (no tocar)-------------------------- */
TEST_CASE(VectorRFC) {
    const std::uint8_t rfc[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9' };
    /* El vector de testeo del RFC 3721 (el "check value" del zlib/gzip). */
    ASSERT_EQ(Crc32::deUnaVez(rfc, sizeof(rfc)), 0xCBF43926u);
}

TEST_CASE(StreamingPorChunks) {
    const std::uint8_t datos[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    const std::uint32_t oneShot = Crc32::deUnaVez(datos, sizeof(datos));
    /* Chunking: 2 updates (3 + 5 bytes) tienen que dar LO MISMO que uno. */
    Crc32 c;
    c.update(datos, 3);
    c.update(datos + 3, 5);
    ASSERT_EQ(c.final(), oneShot);
    /* Y el valor absoluto está congelado (calculado con la misma fórmula). */
    ASSERT_EQ(Crc32::deUnaVez(datos, sizeof(datos)), 0x3FCA88C5u);
}

TEST_CASE(VacioYCorrupcion) {
    /* CRC del mensaje vacío = 0 (0xFFFFFFFF XOR 0xFFFFFFFF). */
    ASSERT_EQ(Crc32::deUnaVez(nullptr, 0), 0x00000000u);
    /* Un byte corrupto cambia el CRC: esa es la detección del pack. */
    const std::uint8_t lump[] = { 10, 20, 30, 40 };
    const std::uint8_t bad[] = { 10, 20, 31, 40 };
    ASSERT_NE(Crc32::deUnaVez(lump, sizeof(lump)), Crc32::deUnaVez(bad, sizeof(bad)));
    ASSERT_EQ(Crc32::deUnaVez(lump, sizeof(lump)), 0xB422A3F1u);
    ASSERT_EQ(Crc32::deUnaVez(bad, sizeof(bad)), 0xAD3992B0u);
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
