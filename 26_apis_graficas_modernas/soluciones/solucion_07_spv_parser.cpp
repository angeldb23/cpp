/* =============================================================================
   MÓDULO 26 - SOLUCIÓN 07: SPIR-V — parser del header y de instrucciones
   =============================================================================
   La solución de ejercicio_07_spv_parser.cpp. Los valores del blob están
   congelados contra la salida REAL de `glslangValidator -V -o c.spv
   c.frag` corrido en la sesión de construcción. Que el parser entienda
   el header y las instrucciones es el ground del reflection: OpDecorate
   (opcode 71) con Decoration DescriptorSet(35)/Binding(6) alimenta los
   bind group layouts de la lección 06 sin que nada sea inventado.
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace {

struct SpvHeaderInfo {
    bool          valido = false;
    std::uint32_t version_word = 0;
    int           major = 0, minor = 0;
    std::uint32_t generator = 0;
    std::uint32_t bound = 0;
};

struct SpvInstr { std::uint16_t opcode; std::uint32_t n_palabras; };

SpvHeaderInfo parsearHeader(const std::uint32_t* p, std::size_t n) {
    SpvHeaderInfo h;
    if (n < 5 || p[0] != 0x07230203u) return h;
    h.valido       = true;
    h.version_word = p[1];
    h.major        = static_cast<int>((p[1] >> 16) & 0xFFu);
    h.minor        = static_cast<int>((p[1] >> 8)  & 0xFFu);
    h.generator    = p[2];
    h.bound        = p[3];
    return h;
}

std::vector<SpvInstr> recorrerInstrucciones(const std::uint32_t* p,
                                            std::size_t n) {
    std::vector<SpvInstr> out;
    std::size_t i = 0;
    while (i < n) {
        const std::uint32_t w = p[i];
        const std::uint32_t count = w >> 16;
        if (count == 0) break;   /* módulo corrupto: corte limpio */
        out.push_back({static_cast<std::uint16_t>(w & 0xFFFFu), count});
        i += count;
    }
    return out;
}

std::string glslangCmd(bool vulkan, bool hlsl) {
    std::string s;
    if (vulkan) s += "-V";
    if (hlsl) {
        if (!s.empty()) s += " ";
        s += "-D";
    }
    if (s.empty()) s = "-";
    return s;
}

} // namespace

/* ------------------------------ TESTS (no tocar)-------------------------- */
static const std::uint32_t kHeader[] = {
    0x07230203u, 0x00010000u, 0x0008000Bu, 19u, 0u,
};

TEST_CASE(HeaderDelModuloReal) {
    const auto h = parsearHeader(kHeader, 5);
    ASSERT_TRUE(h.valido);
    ASSERT_EQ(h.major, 1);
    ASSERT_EQ(h.minor, 0);
    ASSERT_EQ(h.generator, 0x0008000Bu);
    ASSERT_EQ(h.bound, 19u);
}

TEST_CASE(HeaderMagicInvalido) {
    const std::uint32_t mal[] = { 0xDEADBEEFu, 0x00010000u, 0, 4, 0 };
    ASSERT_FALSE(parsearHeader(mal, 5).valido);
    ASSERT_FALSE(parsearHeader(kHeader, 2).valido);
}

/* OpExtInstImport "GLSL.std.450": count 6 = opcode + result id +
   4 palabras del string (12 chars + NUL). */
static const std::uint32_t kInstr[] = {
    (2u << 16) | 17u, 1u,
    (6u << 16) | 11u, 1u, 0x4C534747u, 0x6474732Eu, 0x3035342Eu, 0u,
    (3u << 16) | 14u, 0u, 1u,
};

TEST_CASE(RecorridoDeInstrucciones) {
    const auto v = recorrerInstrucciones(kInstr, 11);
    ASSERT_EQ(v.size(), 3u);
    ASSERT_EQ(v[0].opcode, 17u);
    ASSERT_EQ(v[0].n_palabras, 2u);
    ASSERT_EQ(v[1].opcode, 11u);
    ASSERT_EQ(v[1].n_palabras, 6u);
    ASSERT_EQ(v[2].opcode, 14u);
    ASSERT_EQ(v[2].n_palabras, 3u);
}

TEST_CASE(RecorridoCorruptoSeCorta) {
    const std::uint32_t cor[] = { (0u << 16) | 17u, (2u << 16) | 14u, 0, 1 };
    const auto v = recorrerInstrucciones(cor, 4);
    ASSERT_TRUE(v.empty());
}

TEST_CASE(ComandoGlslang) {
    ASSERT_EQ(glslangCmd(true, false), "-V");
    ASSERT_EQ(glslangCmd(false, true), "-D");
    ASSERT_EQ(glslangCmd(false, false), "-");
    ASSERT_EQ(glslangCmd(true, true), "-V -D");
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
