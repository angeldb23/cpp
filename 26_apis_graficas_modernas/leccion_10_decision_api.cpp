/* =============================================================================
   MÓDULO 26 - LECCIÓN 10: decisión de API — cuándo GL 3.3, cuándo SDL_GPU,
   cuándo Vulkan nativo, y migración incremental
   =============================================================================
   TEMA 8: "Decisión de API: cuándo OpenGL 3.3 basta, cuándo SDL_GPU, cuándo
   Vulkan nativo y migración incremental" — 1 lección.

   CRITERIOS
   ----------
   * Costo fijo de desarrollo: Vulkan nativo > SDL_GPU > GL 3.3. Para
     el material didáctico del triángulo: ~900 líneas en Vulkan puro,
     ~250 en SDL_GPU, ~150 en GL 3.3.
   * Techo de funcionalidad: GL 3.3 llega hasta shading forward y
     sombras (módulo 21); SDL_GPU cubre shaders standard + compute
     passes; Vulkan nativo expone TODO lo que la placa soporte, al
     costo de escribirlo vos.
   * Plataforma: GL 3.3 corre en la UHD 600 sin dudas; SDL_GPU va a
     Vulkan (Linux) / D3D12 (Windows) / Metal (macOS); Vulkan nativo
     está verificado en esta máquina (vulkaninfo: ANV 1.4, ver
     DECISIONES.md del módulo 26).
   * Debugging: el driver GL valida poco y tarde; Vulkan trae
     validation layers que aclaran el error exacto. En SDL_GPU el
     debugmode boolean de SDL_CreateGPUDevice activa lo mismo por
     backend.
   * Escala de escena: el simulador de la L01 mostró que con <500
     draws/frame la diferencia GL vs explícita es ruido, y se hace
     real con miles.

   MIGRACIÓN INCREMENTAL (el plan recomendado, SEM del M22)
   ---------------------------------------------------------
   1. Mantener el renderer actual (GL 3.3) corriendo.
   2. Extraer el "estado de pipeline" como datos: una struct PipelineSpec
      (como la de f26.hpp en la LECCIÓN 04) que hoy la capa GL traduce a
      llamadas gl*. El día que agregues SDL_GPU, la misma spec alimenta
      SDL_GPUGraphicsPipelineCreateInfo.
   3. Extraer el frame en "passes declarativos" (LECCIÓN 09): renderer
      igual a "fixture" que ejecuta un registro, no una secuencia
      hardcodeada de binds.
   4. Sumar backend SDL_GPU como alternativa compilable (interface
      abstracta igual a la del módulo 22: resource manager + renderer).
      Compilás GL y SDL_GPU en paralelo, elegís por flag.
   5. Vulkan nativo solo si un proyecto específico justifica pelearse
      con sus ~1.000 funciones: en la práctica del curso llega con
      SDL_GPU.

   EN ESTA LECCIÓN: un decidir() con las reglas de arriba en forma de
   código: cero magia, es la tabla del cierre/DECISION_API.md.
   ============================================================================= */
#include <cstdint>
#include <cstdio>
#include <vector>

namespace {

/* Perfil del proyecto: las entradas de la decisión. */
struct PerfilProyecto {
    int  draws_frame;      /* draw calls típicos por frame    */
    int  pipelines;        /* PSOs distintos (creados 1 vez)  */
    bool necesita_compute; /* partículas GPU, culling, etc.   */
    bool corto_plazo;      /* prototipo < 3 meses             */
    bool portar_windows;   /* binario Windows también         */
};

enum class Api { GL33, SDL_GPU, VulkanNativo };

const char* nombre_api(Api a) {
    switch (a) {
        case Api::GL33:         return "GL 3.3 core";
        case Api::SDL_GPU:      return "SDL_GPU";
        case Api::VulkanNativo: return "Vulkan nativo";
    }
    return "?";
}

/* La decisión, en forma de reglas (sin pesos, sin magia):
     - prototipo corto y escena chica sin compute -> GL 3.3;
     - escena gorda, compute o multi-OS -> SDL_GPU;
     - control fino de memoria/threading con tiempo de equipo
       disponible -> Vulkan nativo. */
Api decidir(const PerfilProyecto& p) {
    if (p.corto_plazo && p.draws_frame < 500 && !p.necesita_compute)
        return Api::GL33;
    if (p.draws_frame >= 500 || p.necesita_compute || p.portar_windows)
        return Api::SDL_GPU;
    return Api::VulkanNativo;
}

} // namespace

int main() {
    const std::vector<PerfilProyecto> perfiles = {
        /* juego chico de primer año (prototipo rápido): */
        {40,   6, false, true,  false},
        /* bullet hell con partículas en GPU: */
        {2000, 4, true,  false, false},
        /* motor del M22 con idea de porte a Windows: */
        {300, 10, false, false, true },
        /* escena chica pero con control fino de memoria: */
        {200,  8, false, false, false},
    };

    for (std::size_t i = 0; i < perfiles.size(); ++i) {
        std::printf("perfil %zu -> %s\n", i + 1,
                    nombre_api(decidir(perfiles[i])));
    }

    /* El plan del dueño (cierre/DECISION_API.md): GL 3.3 hoy, extraer
       PipelineSpec + frame graph YA, backend SDL_GPU cuando el M22
       quiera porte. Vulkan nativo: solo si llega un caso concreto. */
    std::printf("\nplan: GL 3.3 hoy, spec de pipeline + frame graph ya,"
                " SDL_GPU después (ver cierre/DECISION_API.md).\n");
    return 0;
}
