/* =============================================================================
   MÓDULO 26 - LECCIÓN 06: WebGPU — C API, adapter/device, WGSL, bind groups y
   límites del navegador
   =============================================================================
   TEMA 4: "WebGPU: WGPU C API, adapter/device, shader module (WGSL), bind
   groups y límites de navegador" — 1 lección.

   WEBGPU = EL MODELO DE VULKAN CON CONTRATO DE SEGURIDAD
   -------------------------------------------------------
   * El orden mental ya aprendido sigue igual:
        instance -> adapter -> device -> (pipeline, buffers, texturas)
        -> command encoder -> submit -> surface (swapchain).
   * La diferencia: en WebGPU TODO se declara por adelantado (bind group
     layouts, límites del device) y se valida al crear cada pipeline.
     En el navegador esa validación es técnica de seguridad (el sandbox
     no permite tocar memoria ajena); nativo te viene de regalo como
     "spec con chequeo en runtime".
   * Implementaciones: wgpu (Rust, gfx-rs) y Dawn (Google, la de Chrome).
     La C API estándar es webgpu.h (webgpu-headers, W3C/Khronos).

   HECHOS VERIFICADOS HOY CONTRA EL REPO gfx-rs/wgpu-native (README +
   ffi/wgpu.h leídos en la sesión):
   * "The bindings are based on the WebGPU-native header found at
     ffi/webgpu-headers/webgpu.h and wgpu-native specific items in
     ffi/wgpu.h": o sea, webgpu.h es la API estándar C; wgpu.h la EXTIENDE.
   * Backends de instancia WGPUInstanceBackend: Vulkan (1<<0), GL (1<<1),
     Metal (1<<2), DX12 (1<<3), BrowserWebGPU (1<<5). O sea: desde
     nativo podés apuntar al backend del navegador (comprimido con WASM)
     o al GL (secundario) además de los "primary".
   * Swapchain/surface: WGPUSurfaceConfigurationExtras con
     desiredMaximumFrameLatency (1=minimizar latencia, 2=balance
     default, 3+=throughput): son EXACTAMENTE los in-flight frames de la
     LECCIÓN 03 persistidos en la spec web.
   * Estado nuevo de superficie en wgpu-native: Occluded (ventana tapada
     en Metal/macOS): salteás el frame y rejentrás cuando vuelva.
   * WGPUNativeFeature_Immediates = push constants portables: en WGSL
     se declaran con var<immediate>.
   * WGPUShaderSourceGLSL: wgpu puede compilar GLSL A mano al vuelo
     (con naga por debajo); en la práctica del curso usamos WGSL nativo.

   WGSL: EL GLSL DE LOS NAVEGADORES
   ---------------------------------
   Contraste rápido con el GLSL que ya conocés (M20/M21):
       GLSL:  layout(std140) uniform Luces { vec4 pos; };
       WGSL:  struct Luces { pos: vec4f, }
              @group(0) @binding(0) var<uniform> luces: Luces;
   Los atributos van con @: @group, @binding, @location, @vertex,
   @fragment, @compute, @builtin(position). Los tipos: vec2f/vec4f
   (no vec4), mat4x4f, u32/i32/f32. Los structs terminan por campo con
   coma; las funciones "fn nombre()"; devolución "-> @location(0) vec4f".

   BIND GROUPS (el "descriptor set" de la web)
   -------------------------------------------
       @group(0) @binding(0) var<uniform> mvp: mat4x4f;   // UBO
       @group(0) @binding(1) var          tex: texture_2d<f32>;  // textura
       @group(0) @binding(2) var          samp: sampler;         // sampler
   El host Declara bindGroupLayout con la misma pareja (binding, tipo),
   y en el pass setBindGroup(0, bindGroup). Si el par no matchea el
   layout, NO hay crash: el pipeline pasa a "error state" y el driver
   loggea la validación fallar (esa es la manera web de decir "estado
   inválido").

   LÍMITES DEL NAVEGADOR
   ----------------------
   El device se crea pidiendo límites (maxTextureDimension2D,
   maxBindGroups, maxColorAttachments, maxBufferSize, ...). Cada cifra
   del spec cambia entre versiones: NO inventamos valores en el curso:
   A VERIFICAR en https://www.w3.org/TR/webgpu/#limits (sección
   "Default limits") antes de citarlos al dueño.

   EN ESTA LECCIÓN (demo): lint mínimo de WGSL en C++ puro: extraer
   pares (group, binding) y verificar no-duplicados + entry points
   presentes. La MISMA lógica es el NÚCLEO del ejercicio 06.
   ============================================================================= */
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {

/* WGSL mínimo del quad: la misma pieza que el dueño pondría en su HTML
   o en su app wgpu-native nativa. */
const char* kWGSLQuad = R"WGSL(
struct Uniforms {
    mvp: mat4x4f,
    color: vec4f,
}
@group(0) @binding(0) var<uniform> uniforms: Uniforms;

@vertex
fn vs(@builtin(vertex_index) i: u32) -> @builtin(position) vec4f {
    var pos = array<vec2f, 6>(vec2f(-1.0, -1.0), vec2f( 1.0, -1.0),
                             vec2f(-1.0,  1.0), vec2f(-1.0,  1.0),
                             vec2f( 1.0, -1.0), vec2f( 1.0,  1.0));
    return vec4f(pos[i], 0.0, 1.0);
}

@fragment
fn fs() -> @location(0) vec4f {
    return uniforms.color;
}
)WGSL";

struct BindingInfo {
    int         grupo = -1;
    int         identificador = -1;
    std::string tipo;   /* "uniform", "texture", "sampler" o "otro" */
};

/* Escanea pares @group(n) @binding(m) y califica el var siguiente por
   su forma: var<uniform>, texture_..., sampler. */
std::vector<BindingInfo> extraerBindings(const std::string& src) {
    std::vector<BindingInfo> out;
    std::size_t pos = 0;
    while (true) {
        const std::size_t g = src.find("@group(", pos);
        if (g == std::string::npos) break;
        const std::size_t grupo_num = src.find_first_of("0123456789", g);
        const std::size_t b = src.find("@binding(", pos);
        if (b == std::string::npos || b < g) { pos = g + 1; continue; }
        const std::size_t bin_num = src.find_first_of("0123456789", b);
        if (grupo_num == std::string::npos || bin_num == std::string::npos) break;

        const std::size_t fin_g = src.find(')', g);
        const std::size_t rel = src.find("var", fin_g);
        std::string tipo = "otro";
        if (rel == std::string::npos) break;
        const std::string siguiente = src.substr(rel, rel + 24 <= src.size() ? 24 : src.size() - rel);
        if (siguiente.starts_with("var<uniform>")) tipo = "uniform";
        else if (siguiente.find("texture_") != std::string::npos) tipo = "texture";
        else if (siguiente.find("sampler") != std::string::npos) tipo = "sampler";

        out.push_back({src[grupo_num] - '0', src[bin_num] - '0', tipo});
        pos = b + 1;
    }
    return out;
}

} // namespace

int main() {
    const std::string src(kWGSLQuad);
    const auto bindings = extraerBindings(src);

    std::printf("bindings encontrados: %zu\n", bindings.size());
    for (const auto& b : bindings) {
        std::printf("  @group(%d) @binding(%d) tipo=%s\n", b.grupo,
                    b.identificador, b.tipo.c_str());
    }

    /* Checks de lint (los mismos que pide el ejercicio 06): */
    bool dup = false;
    for (std::size_t i = 0; i + 1 < bindings.size(); ++i) {
        for (std::size_t j = i + 1; j < bindings.size(); ++j) {
            if (bindings[i].grupo == bindings[j].grupo &&
                bindings[i].identificador == bindings[j].identificador) dup = true;
        }
    }
    std::printf("duplicados (group,binding): %s\n", dup ? "SI" : "no");
    std::printf("@vertex: %s | @fragment: %s | @builtin(position): %s\n",
                src.find("@vertex") == std::string::npos ? "NO" : "si",
                src.find("@fragment") == std::string::npos ? "NO" : "si",
                src.find("@builtin(position)") == std::string::npos ? "NO" : "si");
    return 0;
}
