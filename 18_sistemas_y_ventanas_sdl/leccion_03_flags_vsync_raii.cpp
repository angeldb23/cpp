/* =============================================================================
   MÓDULO 18 - LECCIÓN 03: Flags, vsync y wrappers RAII de ventana/renderer
   =============================================================================
   TEMA 2 de TEMAS.md (2 de 2): Ventana y renderer: creación, flags (Vulkan,
   high-DPI, borderless), vsync y destrucción ordenada.

   CORRER A MANO (abre una ventana)
     g++ -std=c++20 -Wall -Wextra leccion_03_flags_vsync_raii.cpp \
         $(pkg-config --cflags --libs sdl3) -o leccion03 && ./leccion03

   CONTENIDO:
     1) Mini-tabla de flags: cuál elegir y cómo combinarlos
     2) Vertical Sync: SDL_SetRenderVSync + cuándo apagarlo (profiling)
     3) Wrapper RAII VentanaRenderer con movimiento (Módulos 07+08)

   VERIFICADO (cabeceras SDL3 3.4.18):
     - SDL_SetRenderVSync(SDL_Renderer*, int vsync) -> bool. vsync:
       0 = apagado, 1 = sincronizado al vblank. En SDL2 el equivalente era
       SDL_RenderSetVSync(renderer, vsync) -> int (desde SDL 2.0.17).
     - En SDL3 no existe flag "fullscreen desktop" aparte (SDL2 sí lo tenía:
       SDL_WINDOW_FULLSCREEN_DESKTOP). En SDL3, SDL_WINDOW_FULLSCREEN ya
       es "fullscreen a la resolución del escritorio", y el modo exacto se
       fija con SDL_SetWindowFullscreenMode(window, nullptr) para escritorio
       (o un SDL_DisplayMode* para modo fijo) — verificado en SDL_video.h.
     - SDL_WINDOW_BORDERLESS: sin decoración; ideal en Wayland tiling
       (Hyprland dibuja sus propios bordes si quiere).
   ============================================================================= */

#include <iostream>
#include <utility>  // std::exchange
#include <SDL3/SDL.h>

// -----------------------------------------------------------------------------
// 1. FLAGS: MINI TABLA DE REFERENCIA (valores en SDL_video.h)
// -----------------------------------------------------------------------------
static void imprimirFlags() {
    std::cout << "=== LECCIÓN 03: flags y vsync ===\n";
    std::cout << "  SDL_WINDOW_FULLSCREEN           0x1 (fullscreen real)\n"
              << "  SDL_WINDOW_OPENGL               0x2 (contexto GL; módulos 20+)\n"
              << "  SDL_WINDOW_BORDERLESS           0x10 (sin bordes; tiling Wayland)\n"
              << "  SDL_WINDOW_RESIZABLE            0x20 (obligatoria en apps de escritorio)\n"
              << "  SDL_WINDOW_HIGH_PIXEL_DENSITY   0x2000 (back buffer en pixels físicos)\n"
              << "  SDL_WINDOW_VULKAN               0x10000000 (surface Vulkan; módulo 26)\n"
              << "\n  Se combinan con | y se chequean con & (Uint64 en SDL3, Uint32 en SDL2).\n";
}

/* -----------------------------------------------------------------------------
   2. VSYNC: ¿POR QUÉ IMPORTA EN TU CELERON?
   - VSync ON: SDL_RenderPresent bloquea hasta el vblank del monitor. Se va
     el tearing, PERO si un frame tarda más que el presupuesto, esperás TODO
     un vblank extra (visible como "hitch"). 
   - VSync OFF: present no bloquea; FPS máximos y tearing en movimiento
     rápido. Para PROFILING sirve: se mide el trabajo real por frame.
   - El cap lo pone el renderer (no la ventana): SDL_SetRenderVSync(ren, 1)
     se llama una vez, al crear el renderer.
   - Estrategia en esta máquina modesta: desarrollo con vsync ON + timestep
     fijo del módulo 16; para medir performance, vsync OFF y comparar.
----------------------------------------------------------------------------- */

// -----------------------------------------------------------------------------
// 3. WRAPPER RAII: ventana + renderer como UNA unidad "pantalla"
//    (move-only, como los wrappers de raylib del módulo 17, lección 01)
// -----------------------------------------------------------------------------
class VentanaRenderer {
public:
    VentanaRenderer() = default;

    static VentanaRenderer abrir(const char* titulo, int w, int h, SDL_WindowFlags flags) {
        SDL_Window* win = SDL_CreateWindow(titulo, w, h, flags);
        if (win == nullptr) return VentanaRenderer();                 // inválido
        SDL_Renderer* ren = SDL_CreateRenderer(win, nullptr);
        if (ren == nullptr) { SDL_DestroyWindow(win); return VentanaRenderer(); }
        return VentanaRenderer(win, ren);
    }

    // Destructor: renderer ANTES que window (orden inverso a la creación)
    ~VentanaRenderer() { cerrar(); }

    VentanaRenderer(const VentanaRenderer&) = delete;
    VentanaRenderer& operator=(const VentanaRenderer&) = delete;

    // Movimiento: transfiere el par y deja el origen vacío (Módulo 08)
    VentanaRenderer(VentanaRenderer&& o) noexcept
        : win_(std::exchange(o.win_, nullptr)),
          ren_(std::exchange(o.ren_, nullptr)) {}
    VentanaRenderer& operator=(VentanaRenderer&& o) noexcept {
        if (this != &o) {
            cerrar();
            win_ = std::exchange(o.win_, nullptr);
            ren_ = std::exchange(o.ren_, nullptr);
        }
        return *this;
    }

    bool valida() const { return win_ != nullptr && ren_ != nullptr; }
    SDL_Window*   ventana()  const { return win_; }
    SDL_Renderer* renderer() const { return ren_; }

    void cerrar() {  // manual e idempotente (como reset() del módulo 07)
        SDL_DestroyRenderer(ren_); ren_ = nullptr;
        SDL_DestroyWindow(win_);   win_ = nullptr;
    }

private:
    VentanaRenderer(SDL_Window* win, SDL_Renderer* ren) : win_(win), ren_(ren) {}
    SDL_Window*   win_ = nullptr;
    SDL_Renderer* ren_ = nullptr;
};

// Demo: abre/cierra la ventana con RAII (corrala a mano; los ej no la usan)
static int demoRAII() {
    imprimirFlags();
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL_Init falla: " << SDL_GetError() << "\n";
        return 1;
    }
    {
        VentanaRenderer vr = VentanaRenderer::abrir(
            "Leccion 03 - RAII", 640, 480,
            SDL_WINDOW_RESIZABLE | SDL_WINDOW_BORDERLESS);
        if (!vr.valida()) { SDL_Quit(); return 1; }

        SDL_SetRenderVSync(vr.renderer(), 1); // vsync ON, una sola vez
        SDL_SetRenderDrawColor(vr.renderer(), 20, 40, 30, 255);
        SDL_RenderClear(vr.renderer());
        SDL_RenderPresent(vr.renderer());
        SDL_Delay(2000); // mirá la ventana 2 segundos y se cierra sola
    } // ~VentanaRenderer: renderer y window sin leaks
    SDL_Quit();
    return 0;
}

/* =============================================================================
   COMO CORRERLA A MANO: abre una ventana 2 segundos (sin bordes, con vsync)
   y se cierra sola. Podés añadirle un print de SDL_GetRendererName() del
   módulo 16 si querés ver qué backend eligió SDL en tu máquina.
   El ejercicio 01 del módulo replica ESTA CASA pero con handles contables
   (no abre ventanas: testea el ORDEN de destrucción con contadores).
   ============================================================================= */
int main() { return demoRAII(); }
