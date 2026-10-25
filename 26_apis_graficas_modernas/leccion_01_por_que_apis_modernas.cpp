/* =============================================================================
   MÓDULO 26 - LECCIÓN 01: por qué Vulkan/WebGPU (y dónde queda OpenGL)
   =============================================================================
   TEMA 1: "Por qué Vulkan/WebGPU: explicit control, lower overhead,
   portabilidad y comparación mental con OpenGL" — 1 lección.

   LA TRADUCCIÓN DE UNA DÉCADA
   ---------------------------
   2004-2014: el driver OpenGL era la "centralita" del GPU. Un juego llama
   glBindTexture/glDrawElements y el driver, EN ESE INSTANTE, decide:
   - validar que ese estado sea legal (¿esa textura con ese sampler?),
   - traducir tu llamada a los comandos nativos del GPU de cada vendor
     (Intel/AMD/NVIDIA compilan={< tus llamadas} a ISA distintas),
   - re-programar el pipeline por los cambios de estado ("¿cambió el
     blend? tengo que tocar registros").
   Con cientos de draw calls chicos por frame ESO consume CPU ANTES de que
   el GPU tenga trabajo real. Y lo peor: cada driver lo hace distinto, con
   magia que no podés controlar y cuellos de botella que surgen de sorpresa.

   Las APIs modernas invierten el reparto: el COSTO se paga una vez, en la
   CREACIÓN de objetos ("pipeline state object", "render pass", "descriptor
   set"), y el draw call queda casi gratis: emitir comandos ya validados.
   El estado que en OpenGL estaba repartido en cientos de variables del
   driver pasa a ser structs TUYOS, y la sincronización (barreras,
   semáforos, fences) deja de ser heurística del driver y pasa a ser
   código explícito que vos escribís.

   TRES APIs, TRES PÚBLICOS
   ------------------------
   * Vulkan (Khronos, 2016): explícita total. Vale la pena si tu motor
     necesita control fino de memoria, command buffers desde varios
     threads o un margen de CPU mínimo medido al microsegundo. Es una API
     C con más de 1.000 funciones y TODO depende de vos.
   * SDL_GPU (SDL3, 2024-): la capa "misma idea, backends Vulkan/D3D12/
     Metal". Es el nivel justo para el motor del dueño: ~90% del estilo de
     Vulkan sin escribir 2.000 líneas para un triángulo.
   * WebGPU (W3C, estandarizándose): la API del navegador, con seguridad
     de memoria en el sandbox y WGSL como lenguaje de shaders. Su C API
     (wgpu-native de Mozilla, Dawn de Google) también sirve en desktop.

   OPENGL NO MUERE
   ---------------
   Para el dueño: GL 3.3 core en su UHD 600 es perfecto y ya lo trabajamos
   en los módulos 20/21. La pregunta correcta no es "¿qué API es mejor?"
   sino "¿mi escena paga el costo fijo de la API explícita?". Simulamos
   abajo un modelo de costos con números DIDÁCTICOS (no benchmarks): si
   tenés 40 draw calls, GL casi no se nota; si tenés 2.000, la API
   explícita ES diferencia real. El cierre formal de esta elección está en
   la lección 10 y en cierre/DECISION_API.md.

   COMPILACIÓN DE ESTA LECCIÓN (like M20-25):
     g++ -std=c++20 -Wall -Wextra leccion_01*.cpp -> ./a.out
     (nada depende de driver: es un simulador educativo)
   ============================================================================= */
#include "include/f26.hpp"

#include <cstdio>
#include <string>
#include <vector>

namespace {

/* Una escena prototipo: draws por frame, PSOs del juego y cuántos frames
   de medición simula cada fila de la tabla. */
struct Escena {
    const char* nombre;
    int draws_frame;
    int pipelines;
    int frames;
};

} // namespace

int main() {
    /* Costos congelados para el simulador: ver f26.hpp::CostosDraw. Son
       órdenes de magnitud plausibles: usarlos COMPARATIVAMENTE, no como
       terminó de perf del driver real. */
    const f26::CostosDraw c{};

    const std::vector<Escena> escenas = {
        {"rpg 2D draws chicos",    40,   6, 300}, /* 60 Hz * 5 s */
        {"mid: platformer sprites",300,  10, 300},
        {"bullet hell 2k draws",  2000,   4, 300},
    };

    std::printf("%-26s %9s %9s %12s\n", "escena", "GL us/f", "VK us/f",
                "exceso GL");
    for (const auto& e : escenas) {
        const double gl_us_frame =
            f26::CostosDraw::tiempoGL(c, e.draws_frame, e.frames)
            / double(e.frames);
        const double vk_us_frame =
            f26::CostosDraw::tiempoVK(c, e.draws_frame, e.frames, e.pipelines)
            / double(e.frames);
        std::printf("%-26s %9.1f %9.1f %12.1f\n", e.nombre, gl_us_frame,
                    vk_us_frame, gl_us_frame - vk_us_frame);
    }

    /* Conclusión operativa (la que la LECCIÓN 10 va a formalizar):
       1) exceso GL pequeño  -> quedate en GL 3.3 (tu motor del M22 hoy).
       2) exceso GL grande   -> considerá SDL_GPU (o Vulkan si querés el
          control total y estás dispuesto a pagar la complejidad.
       3) nunca abandones GL porque "es moderno": el costo fijo de la API
          explícita (crear pipelines, subir meshes, sincronizar a mano)
          existe también y se paga en TU tiempo de desarrollo. */
    std::printf("\nRegla de bolsillo: la API explicita gana cuando el costo\n"
                "VARIABLE (por draw) pesa mas que el costo FIJO (por pipeline).\n"
                "El precio de la API explicita es TU tiempo, no solo CPU.\n");
    return 0;
}
