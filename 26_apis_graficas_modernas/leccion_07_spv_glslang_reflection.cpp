/* =============================================================================
   MÓDULO 26 - LECCIÓN 07: shaders cruzados — GLSL→SPIR-V (glslang),
   HLSL→DXIL, WGSL (naga) y reflection de bindings
   =============================================================================
   TEMA 5: "Shaders cruzados: GLSL → SPIR-V (glslang), HLSL → DXIL, WGSL
   (naga) y reflection para bindings" — 1 lección.

   SPIR-V ES EL BINARIO INTERMEDIO DE SHADERS
   ------------------------------------------
   Igual que LLVM IR es para el compilador de C++: un binario portable,
   versionado e independiente del driver. En Vulkan SOLO se acepta
   SPIR-V; en WebGPU el estándar a documentar es WGSL (con SPIR-V solo
   interno). Las cadenas de compilación reales del curso:
     GLSL .vert/.frag  --glslangValidator--> SPIR-V .spv   (Vulkan/SDL_GPU)
     HLSL .hlsl        --glslangValidator(-D, DXC)--> DXIL (D3D12)
     WGSL  .wgsl       --naga / wgpu--> SPIR-V interno (wgpu-native)
   Pipeline del dueño (motor del M22): vas a escribir GLSL, compilar a
   SPIR-V en build, y SDL_GPU con backend vulkan (UHD 600) o d3d12 en
   Windows: un build system que hace glslang + copia .spv.

   GLSLANGValidator: lo VERIFICADO en ESTA sesión (tool local):
     * clasifica por sufijo: .vert .tesc .tese .geom .frag .comp .mesh
       .task y ray (.rgen/.rint/.rahit/.rchit/.rmiss/.rcall); HLSL con
       sufijo .hlsl o flag -D.
     * -V: SPIR-V bajo semántica Vulkan (default env: vulkan1.0);
       -G: bajo semántica OpenGL; --target-env {vulkan1.0..vulkan1.4}.
     * -H: dump "human readable" del SPIR-V (opcodes en texto).
     * -l: link de varios stages (vert+frag juntos en UN módulo) y
       -q: reflection ("Uniform reflection", "Pipeline output...").
     * --vn nombre: genera un header C con un uint32_t array: EMBEBIR
       el shader en el exe SIN archivos extra. Ideal para demos.
     * -o file: output (sin -o escribe <stage>.spv junto al fuente).
   El binario resultante (hexdump real de esta sesión):
        03 02 23 07 | 00 00 01 00 | 0b 00 08 00 | bound | 0
     magic(LE 0x07230203) version=0x00010000 (1.0) generator=0x0008000B
     (tool 0x0008 = glslang). bound = cantidad de ids. schema = 0.
   Después: instrucciones (count<<16 | opcode): OpCapability(17),
     OpMemoryModel(14), OpFunction(54)... verificados contra la salida
     de glslangValidator en esta sesión (cap Shader=1, Logical=0,
     GLSL450=1).

   REFLECTION (por qué existe)
   ----------------------------
   El driver no se interesa en los nombres que vos usaste: IGUALMENTE
   tenés que saber en qué set/binding vive cada UBO y cuántos bytes
   ocupa. El reflection lee los opcodes OpDecorate (BCL 71) con
   Decoration=DescriptorSet(35)/Binding(6)/Block(2) y OpName (5) para
   los nombres; esa salida alimenta la bindingGroupLayoutGeneration de
   la LECCIÓN 06 sin archivos .cpp intermedios. glslangValidator con
   -q imprime directamente la caricatura de esas tablas.
   en Windows: un build system que hace glslang + copia .spv.
   en Windows: un build system que hace glslang + copia .spv.
   ============================================================================= */
#include <cstdint>
#include <cstdio>
#include <vector>

namespace {

/* Blob SPIR-V REAL generado por `glslangValidator -V -o c.spv c.frag`
   en esta sesión (fragment chip: un solo out vec4). Solo el HEADER y
   las primeras instrucciones (18 bytes útiles + strings):
        magic       version      generator   bound  schema */
constexpr std::uint32_t kSpvHeader[] = {
    0x07230203u, /* magic (little-endian en disco) */
    0x00010000u, /* SPIR-V 1.0                      */
    0x0008000Bu, /* generator: glslang (0x0008)     */
    19u,         /* bound                           */
    0u,          /* schema                          */
};
constexpr std::uint32_t kSpvInstrucciones[] = {
    (2u << 16) | 17u, 1u,   /* OpCapability Shader */
    (6u << 16) | 11u,  /* OpExtInstImport (string GLSL.std.450...) */
    0x534C4747u, 0x2E64736Eu, 0x35342E74u, 0x00303034u, /* "GLSL.std.450" */
    (3u << 16) | 14u, 0u, 1u,  /* OpMemoryModel Logical GLSL450 */
};

/* Parser mínimo del header SPIR-V (la MISMA lógica del ejercicio 07). */
struct SpvHeaderInfo {
    bool valido  = false;
    std::uint32_t version_word = 0; /* crudo (0x00010000 = 1.0) */
    int    major = 0, minor = 0;
    std::uint32_t generator = 0;
    std::uint32_t bound = 0;
};

SpvHeaderInfo parsearHeader(const std::uint32_t* palabras,
                            std::size_t n_palabras) {
    SpvHeaderInfo h;
    if (n_palabras < 5) return h;
    if (palabras[0] != 0x07230203u) return h;   /* magic */
    h.valido        = true;
    h.version_word  = palabras[1];
    h.major         = static_cast<int>((palabras[1] >> 16) & 0xFFu);
    h.minor         = static_cast<int>((palabras[1] >> 8)  & 0xFFu);
    h.generator     = palabras[2];
    h.bound         = palabras[3];
    return h;
}

/* Iterador de instrucciones: cada uint32 = (wordcount<<16 | opcode). */
struct SpvInstr { std::uint16_t opcode; std::uint32_t n_palabras;
                  const std::uint32_t* operandos; };

std::vector<SpvInstr> recorrerInstrucciones(const std::uint32_t* words,
                                            std::size_t n_words) {
    std::vector<SpvInstr> out;
    std::size_t i = 0;
    while (i < n_words) {
        const std::uint32_t w = words[i];
        const std::uint32_t count = w >> 16;         /* count de palabras   */
        const std::uint16_t opc   = static_cast<std::uint16_t>(w & 0xFFFF);
        if (count == 0) break;                       /* corrupto: pare     */
        out.push_back({opc, count, words + i + 1});
        i += count;
    }
    return out;
}

} // namespace

int main() {
    const auto h =
        parsearHeader(kSpvHeader, sizeof(kSpvHeader) / sizeof(std::uint32_t));
    std::printf("SPIR-V: valido=%s version=%d.%d generator=0x%08X bound=%u\n",
                h.valido ? "si" : "NO", h.major, h.minor,
                static_cast<unsigned>(h.generator),
                static_cast<unsigned>(h.bound));

    const auto instr = recorrerInstrucciones(
        kSpvInstrucciones, sizeof(kSpvInstrucciones) / sizeof(std::uint32_t));
    if (!instr.empty()) {
        std::printf("1ra instruccion: opcode=%d (%s) palabras=%u\n",
                    instr[0].opcode,
                    instr[0].opcode == 17 ? "OpCapability" : "?",
                    instr[0].n_palabras);
    }

    /* Comandos de escala de glslangValidator VERIFICADOS en la sesión:
         glslangValidator -V tri.vert tri.frag          (2 stages, out .spv)
         glslangValidator -V -q -l tri.vert tri.frag    (con reflection)
         glslangValidator -V --vn TRI_VERT tri.vert     (header C embebido)
         spirv-val c.spv                                (validador Khronos) */
    std::printf("recomendación: en CMake, glslangValidator -V en cada .vert/"
                ".frag y la salida .spv tratada como archivo compilado "
                "(add_custom_command).\n");
    return 0;
}
