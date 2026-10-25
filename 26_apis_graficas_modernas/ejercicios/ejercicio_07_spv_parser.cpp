/* =============================================================================
   MÓDULO 26 - EJERCICIO 07: SPIR-V — parser del header y de instrucciones
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 07 (shaders cruzados: glslang, DXIL, naga y reflection).

   CONSIGNA
   --------
   1. `parsearHeader(palabras, n)`: el header SPIR-V son 5 uint32:
        [0] magic 0x07230203 (little-endian en disco),
        [1] versión (major<<16 | minor<<8 | patch; ex.: 0x00010000 = 1.0),
        [2] generator (tool<<16 | versión de la tool; ex.: 0x0008000B
            = glslang 11),
        [3] bound (cantidad de ids generados),
        [4] schema (siempre 0).
      Si el magic no matchea o hay menos de 5 palabras: valido=false.
   2. `recorrerInstrucciones(palabras, n)`: cada uint32 es
      (count << 16 | opcode); operandos = las siguientes count-1 palabras.
      Si count == 0, el módulo está corrupto (corte).
   3. `glslangCmd(vulkan, hlsl)`: devuelve la línea de comando de
      glslangValidator: "-V" para Vulkan, "-D" para HLSL input ("-" si
      ninguno). Es la misma convención de la lección.
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
    // TODO
    (void)p; (void)n;
    return {};
}

std::vector<SpvInstr> recorrerInstrucciones(const std::uint32_t* p,
                                            std::size_t n) {
    // TODO
    return {};
}

std::string glslangCmd(bool vulkan, bool hlsl) {
    // TODO
    return "";
}

} // namespace

/* ------------------------------ TESTS (no tocar)-------------------------- */
/* Header REAL del fragment compilado con glslangValidator -V en esta
   sesión (c.frag -> c.spv, hexdump verificado). */
static const std::uint32_t kHeader[] = {
    0x07230203u, 0x00010000u, 0x0008000Bu, 19u, 0u,
};

TEST_CASE(HeaderDelModuloReal) {
    const auto h = parsearHeader(kHeader, 5);
    ASSERT_TRUE(h.valido);
    ASSERT_EQ(h.major, 1);
    ASSERT_EQ(h.minor, 0);
    ASSERT_EQ(h.generator, 0x0008000Bu);  /* glslang (tool 8), ver 11 */
    ASSERT_EQ(h.bound, 19u);
}

TEST_CASE(HeaderMagicInvalido) {
    const std::uint32_t mal[] = { 0xDEADBEEFu, 0x00010000u, 0, 4, 0 };
    ASSERT_FALSE(parsearHeader(mal, 5).valido);
    ASSERT_FALSE(parsearHeader(kHeader, 2).valido);  /* largo insuficiente */
}

/* Instrucciones reales del mismo blob: OpCapability Shader(1),
   OpExtInstImport GLSL.std.450, OpMemoryModel Logical(0) GLSL450(1). */
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
    ASSERT_EQ(v[0].opcode, 17u);   /* OpCapability */
    ASSERT_EQ(v[0].n_palabras, 2u);
    ASSERT_EQ(v[1].opcode, 11u);   /* OpExtInstImport (6 palabras) */
    ASSERT_EQ(v[1].n_palabras, 6u);
    ASSERT_EQ(v[2].opcode, 14u);   /* OpMemoryModel */
    ASSERT_EQ(v[2].n_palabras, 3u);
}

TEST_CASE(RecorridoCorruptoSeCorta) {
    const std::uint32_t cor[] = { (0u << 16) | 17u, (2u << 16) | 14u, 0, 1 };
    const auto v = recorrerInstrucciones(cor, 4);
    ASSERT_TRUE(v.empty());        /* count==0 => invalido, no crashea */
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
