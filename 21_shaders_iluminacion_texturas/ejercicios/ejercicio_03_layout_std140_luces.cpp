/* =============================================================================
   EJERCICIO 03 (L04): layout std140 bloque luces (272 bytes)
   Verificar offsets/stride/total igual que L04.
   ============================================================================ */

#include "../../include/test_framework.hpp"
#include <cstddef>

namespace {

/* --- EJERCICIO: completar el struct para que coincida con L04 ---
   struct LuzStd140 {       // 32 bytes
       float color[3];      // @0
       float intensidad;    // @12
       float pos[3];        // @16 (align 16)
       float alcance;       // @28
   };
   struct BloqueLuces {
       int  cuenta;         // @0
       char pad[12];        // @4..15
       LuzStd140 luces[8];  // @16, stride 32
   };
   TOTAL = 16 + 8*32 = 272 bytes
*/

// STUB: tamaños incorrectos
struct LuzStd140 {
    float color[3];
    float intensidad;
    float pos[3];
    float alcance;
};

struct BloqueLuces {
    int cuenta;
    char pad[4];   // MAL: debería ser 12
    LuzStd140 luces[8];
};

} // namespace

TEST_CASE(test_luz_std140_tamano) {
    ASSERT_EQ(sizeof(LuzStd140), 32);
}

TEST_CASE(test_luz_std140_offset_color) {
    LuzStd140 l{};
    ASSERT_EQ(reinterpret_cast<size_t>(&l.color) - reinterpret_cast<size_t>(&l), 0);
}

TEST_CASE(test_luz_std140_offset_intensidad) {
    LuzStd140 l{};
    ASSERT_EQ(reinterpret_cast<size_t>(&l.intensidad) - reinterpret_cast<size_t>(&l), 12);
}

TEST_CASE(test_luz_std140_offset_pos) {
    LuzStd140 l{};
    ASSERT_EQ(reinterpret_cast<size_t>(&l.pos) - reinterpret_cast<size_t>(&l), 16);
}

TEST_CASE(test_luz_std140_offset_alcance) {
    LuzStd140 l{};
    ASSERT_EQ(reinterpret_cast<size_t>(&l.alcance) - reinterpret_cast<size_t>(&l), 28);
}

TEST_CASE(test_bloque_cuenta_offset) {
    BloqueLuces b{};
    ASSERT_EQ(reinterpret_cast<size_t>(&b.cuenta) - reinterpret_cast<size_t>(&b), 0);
}

TEST_CASE(test_bloque_luces_base_offset) {
    BloqueLuces b{};
    ASSERT_EQ(reinterpret_cast<size_t>(&b.luces[0]) - reinterpret_cast<size_t>(&b), 16);
}

TEST_CASE(test_bloque_total_272) {
    ASSERT_EQ(sizeof(BloqueLuces), 272);
}

TEST_CASE(test_luces_stride_32) {
    BloqueLuces b{};
    size_t stride = reinterpret_cast<size_t>(&b.luces[1]) - reinterpret_cast<size_t>(&b.luces[0]);
    ASSERT_EQ(stride, 32);
}

int main() {
    return RUN_ALL_TESTS();
}