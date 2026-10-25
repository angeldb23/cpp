/* =============================================================================
   MÓDULO 18 - LECCIÓN 11: Múltiples ventanas, High-DPI y Wayland
   =============================================================================
   TEMA 8 de TEMAS.md: Múltiples ventanas, high-DPI y Wayland: scale factor,
   SDL_WINDOW_HIGH_PIXEL_DENSITY y DPI awareness (1 lección).

   CORRER A MANO (abre DOS ventanas por 3 segundos, ideal en Hyprland:)
     g++ -std=c++20 -Wall -Wextra leccion_11_multi_ventana_highdpi_wayland.cpp \
         $(pkg-config --cflags --libs sdl3) -o leccion11 && ./leccion11

   VERIFICADO EN CABECERAS (SDL_video.h, SDL3 3.4.18):
     - SDL_GetWindows(int *count) -> SDL_Window**: la lista de TODAS las
       ventanas vivas del proceso (destruí una y se descuenta sola).
     - Cada ventana tiene un SDL_WindowID propio: los eventos traen ese id
       (e.window.windowID / e.key.windowID), así la app SABE a qué ventana
       le correspondió cada evento.
     - SDL_GetWindowDisplayScale(window) -> float: la escala de LA VENTANA
       (no del monitor): 1.0 en displays "normales", 1.5/2.0 en HiDPI.
       SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED avisa cuando cambia.
     - SDL_GetWindowPixelDensity(window) -> float: ratio entre pixels del
       back buffer y el tamaño lógico: 1.0 = un pixel por punto; 2.0 = dos
       pixels por punto (HiDPI activo con SDL_WINDOW_HIGH_PIXEL_DENSITY).
     - SDL_GetWindowSizeInPixels(window, &w, &h): el tamaño del back buffer
       real (con density aplicada). SDL_GetWindowSize: el tamaño LÓGICO.
     - SDL_GetDisplayContentScale(SDL_DisplayID) -> float: scale del
       MONITOR entero (para UI de escritorio), distinto de la de la ventana.
     - SDL_GetWindowID / SDL_RaiseWindow / SDL_GetDisplayForWindow /
       SDL_GetDisplayBounds(displayID, &rect) resuelven layout multi-window.
   ============================================================================= */

#include <iostream>
#include <SDL3/SDL.h>

// -----------------------------------------------------------------------------
// 1. LAYOUT PARA HI-DPI: unidades LOGICAS (puntos) vs PIXELS del back buffer
//    La conversion es la del ej 08: pixel = round(punto * scale).
// -----------------------------------------------------------------------------
struct Layout {
    float scale = 1.0f;

    SDL_FRect enPuntos(float x, float y, float w, float h) const {
        return SDL_FRect{ x, y, w, h };           // dibujá SIEMPRE en puntos
    }
    int puntosAPixels(float v) const {
        return static_cast<int>(v * scale + 0.5f); // round-half-up (distinto del trunc!)
    }
};

static void mostrarLayout(float scale) {
    Layout ly{scale};
    SDL_FRect hud = ly.enPuntos(10, 10, 240, 40);   // HUD fijo en puntos
    std::cout << "scale         = " << ly.scale << "\n"
              << "HUD en puntos = " << hud.x << ", " << hud.y << ", "
              << hud.w << "x" << hud.h << "\n"
              << "HUD en pixels = " << ly.puntosAPixels(hud.x) << ", "
              << ly.puntosAPixels(hud.y) << ", "
              << ly.puntosAPixels(hud.w) << "x"
              << ly.puntosAPixels(hud.h) << "\n"
              << "(con SDL_WINDOW_HIGH_PIXEL_DENSITY, el back buffer puede ser\n"
              << " MAYOR que el tamaño lógico de la ventana: SDL_GetWindowSizeInPixels)\n";
}

// -----------------------------------------------------------------------------
// 2. DEMO: DOS ventanas, cada una con SU renderer propio y su scale
// -----------------------------------------------------------------------------
static int demoDosVentanas() {
    if (!SDL_Init(SDL_INIT_VIDEO)) { std::cout << "init " << SDL_GetError() << "\n"; return 1; }

    // SDL3 permite cualquier cantidad de ventanas de un proceso (SDL_GetWindows)
    SDL_Window* a = SDL_CreateWindow("Ventana A (main)", 480, 320,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    SDL_Window* b = SDL_CreateWindow("Ventana B (debug)", 320, 320, SDL_WINDOW_BORDERLESS);
    SDL_Renderer* ra = SDL_CreateRenderer(a, nullptr);
    SDL_Renderer* rb = SDL_CreateRenderer(b, nullptr);
    if (!ra || !rb) {
        std::cout << "render " << SDL_GetError() << " (queda una sola ventana entonces)\n";
    }

    // El ID de cada venta: lo que reportan los eventos (e.key.windowID etc.)
    std::cout << "id A = " << SDL_GetWindowID(a) << ", id B = " << SDL_GetWindowID(b) << "\n";
    std::cout << "scale A = " << SDL_GetWindowDisplayScale(a)
              << ", pixel density A = " << SDL_GetWindowPixelDensity(a) << "\n";
    mostrarLayout(SDL_GetWindowDisplayScale(a));

    // 3 segundos de "mini game loop" para DOS ventanas
    for (int i = 0; i < 150; ++i) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            // CON multi-window: el evento te dice a QUIÉN corresponde:
            if (e.type == SDL_EVENT_KEY_DOWN && e.key.windowID == SDL_GetWindowID(b)) {
                std::cout << "tecla en la ventana B (debug)\n";
            }
            if (e.type == SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED) {
                SDL_Window* wEvento = SDL_GetWindowFromID(e.window.windowID);
                std::cout << "scale cambió a " << SDL_GetWindowDisplayScale(wEvento)
                          << " en la ventana " << e.window.windowID << "\n";
            }
            if (e.type == SDL_EVENT_QUIT) { i = 1000; break; }
        }
        // render A y render B (cada renderer CLI es independiente)
        SDL_SetRenderDrawColor(ra, 40, 20, 60, 255);
        SDL_RenderClear(ra);
        SDL_RenderPresent(ra);
        SDL_SetRenderDrawColor(rb, 20, 40, 60, 255);
        SDL_RenderClear(rb);
        SDL_RenderPresent(rb);
        SDL_Delay(20);
    }

    // cierre: cada renderer ANTES de su ventana (lección 03)
    SDL_DestroyRenderer(ra); SDL_DestroyWindow(a);
    SDL_DestroyRenderer(rb); SDL_DestroyWindow(b);
    SDL_Quit();
    return 0;
}

/* -----------------------------------------------------------------------------
   3. WAYLAND (tu escritorio) — notas reales verificadas en la doc de SDL3
   - SDL3 es Wayland-first: corre en Wayland por defecto en Arch (en SDL2
     el default era X11 y se forzaba el otro con SDL_VIDEODRIVER=wayland).
   - Lo que CUIDÁS al desarrollar en Wayland:
        * Escala fraccional: el compositor escala el buffer; con
          SDL_WINDOW_HIGH_PIXEL_DENSITY pedís el back buffer en pixels
          físicos y el juego queda nítido (el idioma "puntos" de arriba).
        * "Borders": la decoración la dibuja el COMPOSITOR (o el app con
          xdg-decoration): SDL_WINDOW_BORDERLESS pide no tenerla, y en
          tiling (Hyprland) no ganas nada: el borde no existe.
        * Posiciones globales (x, y) de ventanas: en Wayland el compositor
          no las permite (por seguridad y diseño); SDL_SetWindowPosition
          actúa como "sugerencia" y puede ser ignorada.
     - Para forzar X11 en debugging puntual:
         SDL_VIDEODRIVER=x11 ./juego
   - Wayland + OpenGL: el contexto GL 3.3 core (módulo 20) funciona
     igual; solo cambia el surface del backend (EGL en Wayland).
----------------------------------------------------------------------------- */

/* =============================================================================
   QUÉ QUEDA TESTEABLE:
   - Layout::puntosAPixels y la regla "HUD en puntos": el corazón del ej 08
     (sin SDL: escala inyectada y redondeos controlados a mano).
   - Dos ventanas, IDs y dispatch por windowID: lógica pura, testeable SIN
     ventanas (el ej 02 replica el dispatch con eventos simulados).
   ============================================================================= */
int main() { return demoDosVentanas(); }
