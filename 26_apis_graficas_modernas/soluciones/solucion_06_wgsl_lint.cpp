/* =============================================================================
   MÓDULO 26 - SOLUCIÓN 06: WebGPU — lint de WGSL (bind groups, entry points)
   =============================================================================
   La solución de ejercicio_06_wgsl_lint.cpp. El lint es la validación
   BARATA que se hace ANTES de crear el WGPUShaderModule: si hay pares
   (group, binding) duplicados, el pipeline pasa a error state y el
   draw sale con un log — mejor detectarlo en el editor (como hace el
   lint del shader GL del módulo 20). El parser es de juguete (suficiente
   para este curso); el validador real es naga/wgpu-core en runtime.
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace {

struct BindingInfo {
    int         grupo = -1;
    int         identificador = -1;
    std::string tipo;
};

std::vector<BindingInfo> extraerBindings(const std::string& src) {
    std::vector<BindingInfo> out;
    std::size_t pos = 0;
    while (true) {
        const std::size_t g = src.find("@group(", pos);
        if (g == std::string::npos) break;
        const std::size_t b = src.find("@binding(", pos);
        if (b == std::string::npos) break;

        /* número después de cada "(": single-digit level del curso. */
        const auto dig = [&src](const std::size_t donde) -> int {
            const std::size_t d =
                src.find_first_of("0123456789", donde);
            return d == std::string::npos ? -1 : src[d] - '0';
        };
        const int grupo = dig(g);
        const int ident = dig(b);
        if (grupo < 0 || ident < 0) break;

        /* la declaración var vive cerca del cierre de binding(...). */
        const std::size_t fin_b = src.find(')', b);
        const std::size_t rel   = src.find("var", fin_b);
        std::string tipo = "otro";
        if (rel != std::string::npos) {
            const std::string tras = src.substr(rel, 24);
            if (tras.starts_with("var<uniform>"))              tipo = "uniform";
            else if (tras.find("texture_") != std::string::npos) tipo = "texture";
            else if (tras.find("sampler")  != std::string::npos) tipo = "sampler";
        }
        out.push_back({grupo, ident, tipo});
        pos = b + 1;
    }
    return out;
}

bool hayDuplicados(const std::vector<BindingInfo>& b) {
    for (std::size_t i = 0; i + 1 < b.size(); ++i) {
        for (std::size_t j = i + 1; j < b.size(); ++j) {
            if (b[i].grupo == b[j].grupo &&
                b[i].identificador == b[j].identificador) return true;
        }
    }
    return false;
}

struct LintResult { bool duplicados, tiene_vertex, tiene_fragment, tiene_pos; };

LintResult lint(const std::string& src) {
    return { hayDuplicados(extraerBindings(src)),
             src.find("@vertex")   != std::string::npos,
             src.find("@fragment") != std::string::npos,
             src.find("@builtin(position)") != std::string::npos };
}

} // namespace

/* ------------------------------ TESTS (no tocar)-------------------------- */
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
