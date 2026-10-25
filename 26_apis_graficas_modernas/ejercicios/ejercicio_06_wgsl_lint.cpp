/* =============================================================================
   MÓDULO 26 - EJERCICIO 06: WebGPU — lint de WGSL (bind groups, entry points)
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 06 (WebGPU: C API, WGSL, bind groups y límites).

   CONSIGNA
   --------
   Implementá el lint del WGSL que antes suben a wgpu/Dawn:

   1. `extraerBindings(src)`: encuentra los pares `@group(n) @binding(m)`
      y califica la declaración `var` siguiente:
      "uniform"  -> var<uniform>...
      "texture"  -> la línea contiene texture_...
      "sampler"  -> la línea contiene sampler
      "otro"     -> cualquier otra cosa.
   2. `hayDuplicados(bindings)`: dos pares con el MISMO (group, binding)
      es error de declaraciones duplicadas.
   3. `lint(src)`: arma el reporte: duplicados, tiene @vertex, tiene
      @fragment y tiene @builtin(position) como salida del vertex.
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <string>
#include <vector>

namespace {

struct BindingInfo {
    int         grupo = -1;
    int         identificador = -1;
    std::string tipo;
};

std::vector<BindingInfo> extraerBindings(const std::string& src) {
    // TODO
    return {};   // stub vacío (el test protege con size)
}

bool hayDuplicados(const std::vector<BindingInfo>& b) {
    // TODO
    return false;
}

struct LintResult { bool duplicados, tiene_vertex, tiene_fragment, tiene_pos; };

LintResult lint(const std::string& src) {
    // TODO
    return {false, false, false, false};
}

} // namespace

/* ------------------------------ TESTS (no tocar)-------------------------- */
/* El quad de la lección: un solo UBO y dos entry points. */
static const char* kQuad =
    "struct Uniforms {\n"
    "    mvp: mat4x4f,\n"
    "    color: vec4f,\n"
    "}\n"
    "@group(0) @binding(0) var<uniform> uniforms: Uniforms;\n"
    "@vertex fn vs(@builtin(vertex_index) i: u32) -> @builtin(position) vec4f {\n"
    "    return vec4f(0.0, 0.0, 0.0, 1.0);\n"
    "}\n"
    "@fragment fn fs() -> @location(0) vec4f {\n"
    "    return uniforms.color;\n"
    "}\n";

TEST_CASE(BindingsDelQuad) {
    const auto b = extraerBindings(kQuad);
    ASSERT_EQ(b.size(), 1u);
    ASSERT_EQ(b[0].grupo, 0);
    ASSERT_EQ(b[0].identificador, 0);
    ASSERT_EQ(b[0].tipo, "uniform");
}

TEST_CASE(BindingsTexturaSampler) {
    static const char* kTex =
        "@group(0) @binding(0) var tex: texture_2d<f32>;\n"
        "@group(0) @binding(1) var samp: sampler;\n"
        "@group(1) @binding(0) var<storage, read> datos: array<f32>;\n";
    const auto b = extraerBindings(kTex);
    ASSERT_EQ(b.size(), 3u);
    ASSERT_EQ(b[0].tipo, "texture");
    ASSERT_EQ(b[1].tipo, "sampler");
    ASSERT_EQ(b[1].identificador, 1);
    /* storage ES otro tipo de binding (no uniform): queda "otro" */
    ASSERT_EQ(b[2].tipo, "otro");
    ASSERT_EQ(b[2].grupo, 1);
    ASSERT_FALSE(hayDuplicados(b));
}

TEST_CASE(DuplicadosDetectados) {
    static const char* kDup =
        "@group(0) @binding(0) var tex: texture_2d<f32>;\n"
        "@group(0) @binding(0) var samp: sampler;\n";
    const auto b = extraerBindings(kDup);
    ASSERT_EQ(b.size(), 2u);
    ASSERT_TRUE(hayDuplicados(b));
}

TEST_CASE(LintDelQuad) {
    const auto r = lint(kQuad);
    ASSERT_FALSE(r.duplicados);
    ASSERT_TRUE(r.tiene_vertex);
    ASSERT_TRUE(r.tiene_fragment);
    ASSERT_TRUE(r.tiene_pos);
}

TEST_CASE(LintDeUnWgslTruncado) {
    /* Falta el fragment y el @builtin(position): dos fallas de lint */
    static const char* kTru =
        "@group(0) @binding(0) var<uniform> u: Uniforms;\n"
        "@vertex fn vs(@builtin(vertex_index) i: u32) -> vec4f {\n"
        "    return vec4f(0.0, 0.0, 0.0, 1.0);\n"
        "}\n";
    const auto r = lint(kTru);
    ASSERT_TRUE(r.tiene_vertex);
    ASSERT_FALSE(r.tiene_fragment);
    ASSERT_FALSE(r.tiene_pos);
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
