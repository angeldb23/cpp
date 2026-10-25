/* =============================================================================
   MÓDULO 25 - EJERCICIO 09: CRC32 (tabla 0xEDB88320 + streaming por chunks)
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 09 (integridad del pack: el checksum de ZIP/PNG/GZIP).

   CONSIGNA
   --------
   Completá el CRC32 reflejanizado (polinomio 0xEDB88320):

   1. `tabla()`: generá las 256 entradas partiendo de Crc32::crearTabla():
        para cada i en [0,256): c = i; 8 veces:
            c = (c & 1) ? (0xEDB88320 ^ (c >> 1)) : (c >> 1);
        tabla[i] = c.
   2. `update(data, n)`: el update por byte (streaming):
        crc = (crc >> 8) ^ tabla[(crc ^ byte) & 0xFF]
      (crc ARRANCA en 0xFFFFFFFF: ya está inicializado en el miembro.)

   `final()` y `deUnaVez()` YA están: final() hace el XOR de salida con
   0xFFFFFFFF; deUnaVez() es un update único + final.

   El test es el vector universal del RFC: CRC32("123456789") = 0xCBF43926.
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
        // TODO
        (void)data; (void)n;
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
        // TODO
        return {};
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
