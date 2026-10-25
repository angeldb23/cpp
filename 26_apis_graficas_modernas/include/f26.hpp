#pragma once
/* =============================================================================
   f26.hpp — cabecera común del MÓDULO 26 (APIs Gráficas Modernas)
   =============================================================================
   Igual que mate3d.hpp (M19), gl_util.hpp (M20) o fisica2d.hpp (M23): tipos y
   helpers compartidos entre lecciones. NADA de esta cabecera abre ventanas, no
   linkea librerías gráficas y no resuelve ningún TODO del curso: solo describe
   TIPOS DE MODELO que las lecciones usan para razonar sobre GPUs sin necesitar
   un driver.

   Convención del módulo: las lecciones compilan headless (g++ suelto o CMake,
   igual que en módulos 20-25) y el código con API REAL (Vulkan / SDL_GPU /
   WebGPU) vive en bloques `#if __has_include(...)` que SOLO se activan en los
   targets de demo de `26_apis_graficas_modernas/CMakeLists.txt`. Los
   ejercicios/soluciones NUNCA incluyen esta cabecera: son lógica pura con
   test_framework.hpp.
   ============================================================================= */
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

namespace f26 {

/* ---- Capacidades de una GPU "modelo" (lo que consulta cada API) ---------- */
struct GpuCap {
    std::string nombre;
    int         api_major = 0;   /* versión máxima de la API del driver */
    int         api_minor = 0;
    bool        device_local_heap = true;  /* VRAM íntegra */
    bool        host_visible_heap = true;  /* mappable (BAR / host) */
    std::uint64_t ram_mb    = 0;
    std::uint64_t vram_mb   = 0;   /* 0 en iGPU: comparte RAM */
    int         samples_max = 1;   /* MSAA máximo (1, 2, 4, 8...) */
    bool        soporta_compute = true;
};

/* ---- Overhead: costos de pasar datos al GPU (L01) ------------------------- */
/* Modelo mental: un driver moderno gasta CPU por draw call en validar estado
   traslada, mientras Vulkan/SDL_GPU mueven ese costo a la CREACIÓN del
   pipeline (una vez). Números de juguete que CALIBRAMOS con nuestra máquina:
   sirven para comparar órdenes de magnitud, no benchmarks reales. */
struct CostosDraw {
    double overhead_gl_por_call_us   = 1.5; /* driver valida estado por call */
    double overhead_vk_por_call_us   = 0.3; /* solo emite el cmd packet */
    double costo_crear_pipeline_us   = 1200.0; /* una vez, por PSO */
    double costo_crear_pass_gl_us    = 60.0;   /* GL re-valida por frame */
    double costo_crear_pass_vk_us    = 40.0;   /* una vez + renderPass cached */

    /* Con N draws de la misma pipeline, ¿cuánto ahorró la API explícita? */
    static double tiempoGL(const CostosDraw& c, int draws, int frames) {
        return double(draws) * frames * c.overhead_gl_por_call_us
             + double(frames) * c.costo_crear_pass_gl_us;
    }
    static double tiempoVK(const CostosDraw& c, int draws, int frames,
                           int pipelines /*creadas una vez*/) {
        return double(draws) * frames * c.overhead_vk_por_call_us
             + double(frames) * c.costo_crear_pass_vk_us
             + pipelines * c.costo_crear_pipeline_us;
    }
};

/* ---- "Pipeline State" compartido entre APIs (L04 SDL_GPU, L10 decisión) --- */
/* Cada API moderna cachea el ESTADO COMPLETO en un objeto: aquí el modelo
   mínimo que una lección puede comparar. */
enum class Topologia { Triangulos, Lineas, Puntos };

struct PipelineSpec {
    Topologia topo = Topologia::Triangulos;
    int  samples            = 1;    /* MSAA */
    bool depth_test         = true;
    bool blend_on           = false;
    std::string shaderFmt = "";      /* "SPIR-V", "DXIL", "MSL", "WGSL" */
};

/* ---- Memoria (L08, VMA): lo que expone un device físico ------------------- */
struct MemoryProp {
    std::uint32_t bits_host_visible  = 0x1;
    std::uint32_t bits_device_local  = 0x2; /* VRAM */
    std::uint32_t bits_host_cached   = 0x4; /* GPU no rompe coaster CPU */
    std::uint64_t tamano_mb = 0;
};

/* ---- Doubles: redondeo simple (estabilidad de valores en tests) ---------- */
inline double r2(double v) { return std::round(v * 100.0) / 100.0; }

} // namespace f26
