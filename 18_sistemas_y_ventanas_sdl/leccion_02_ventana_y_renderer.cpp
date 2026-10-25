/* =============================================================================
   MÓDULO 18 - LECCIÓN 02: Ventana y renderer — creación, flags y destrucción
   ordenada
   =============================================================================
   TEMA 2 de TEMAS.md (1 de 2): Ventana y renderer: creación, flags (Vulkan,
   high-DPI, borderless), vsync y destrucción ordenada.

   CÓMO TRAER SDL CON FETCHCONTENT (tags reales, verificados en GitHub el
   22/10/2026; NUNCA master):
     include(FetchContent)
     FetchContent_Declare(
         SDL3
         GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
         GIT_TAG        release-3.4.18        # <- tag oficial con fecha de release
     )
     FetchContent_MakeAvailable(SDL3)
     target_link_libraries(mi_juego PRIVATE SDL3::SDL3)  # target de SDL3
   (Si instalás SDL por pacman: sudo pacman -S sdl3, y en este compilador
    directo: g++ ... $(pkg-config --cflags --libs sdl3). Ambos caminos son
    válidos; con FetchContent la build es autosuficiente.
    Para SDL2 legacy (el compare de la lección 01): tag release-2.32.10).

   CORRER A MANO (abre una ventana):
     g++ -std=c++20 -Wall -Wextra leccion_02_ventana_y_renderer.cpp \
         $(pkg-config --cflags --libs sdl3) -o leccion02 && ./leccion02

   En el módulo 16 (raylib) la ventana era una llamada (`InitWindow`). SDL te
   descompone el proceso en pasos EXPLÍCITOS y por eso es el momento ideal para
   entender las CAPAS:
     1. SDL_Init / SDL_InitSubSystem  -> subsistema de video y/o eventos
     2. SDL_CreateWindow              -> la ventana (X11/Wayland abstraída)
     3. SDL_CreateRenderer            -> el CONTEXTO 2D sobre esa ventana
     4. render, render, render
     5. SDL_DestroyRenderer y SDL_DestroyWindow -> cierre en orden inverso

   PECULIARIDADES VERIFICADAS EN LAS CABECERAS (SDL3 3.4.18):
   - SDL_Init devuelve bool (true OK, false error): en SDL2 era 0 OK.
   - SDL_CreateWindow es (title, w, h, flags) y devuelve SDL_Window*.
   - SDL_CreateRenderer(window, name) donde 'name' es el driver de render
     deseado (nullptr = auto) y devuelve SDL_Renderer* o NULL si falla.
   - En SDL3, destruir la ventana libera el renderer asociado si todavía
     estaba vivo (el "ownership" que antes era tuyo). Aun así: destruí el
     renderer EXPLÍCITAMENTE primero (invariante de RAII y costumbre del
     mundo real; write the code that says what you mean).

   FLAGS QUE USAREMOS MÁS (todos en SDL_video.h):
     SDL_WINDOW_RESIZABLE          0x20        -> el usuario lo agranda
     SDL_WINDOW_BORDERLESS         0x10        -> sin decoración (hyprland!)
     SDL_WINDOW_FULLSCREEN         0x1         -> fullscreen real
     SDL_WINDOW_HIGH_PIXEL_DENSITY 0x2000      -> rendir en pixels físicos
                                                  (antes ALLOW_HIGHDPI en 2.x)
     SDL_WINDOW_OPENGL 0x2 / SDL_WINDOW_VULKAN 0x10000000 -> contexto GL3.3+
                                                  o surface Vulkan (mód 26)
   ============================================================================= */

#include <iostream>
#include <SDL3/SDL.h>

// -----------------------------------------------------------------------------
// 1. EL ORDEN EXACTO (bibliografía de cada llamada, con "qué pasa por debajo")
// -----------------------------------------------------------------------------
int demoVentana() {
    std::cout << "=== LECCIÓN 02: ventana y renderer ===\n";

    // 1) Subsistema: VIDEO trae eventos (gratis). AUDIO/texto se encadenan
    //    en las lecciones 05 y 08.
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL_Init falla: " << SDL_GetError() << "\n";
        return 1;
    }

    // 2) Ventana. Job del compositor: crearla en Wayland o X11 y devolverte
    //    un ID opaco. Los flags son una capa ON/OFF en un Uint64 SDL3.
    SDL_Window* ventana = SDL_CreateWindow(
        "Modulo 18 - Leccion 02",
        800, 600,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (ventana == nullptr) {
        std::cout << "SDL_CreateWindow falla: " << SDL_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    // 3) Renderer. nullptr -> SDL elige. En tu Intel UHD 600 el driver va a
    //    ser OpenGL (Mesa) para 3.3 core, y el renderer 2D acelera.
    SDL_Renderer* renderer = SDL_CreateRenderer(ventana, nullptr);
    if (renderer == nullptr) {
        std::cout << "SDL_CreateRenderer falla: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(ventana);
        SDL_Quit();
        return 1;
    }

    // Info honesta de qué driver arrancó (mesa: opengl u degl en sandboxes)
    std::cout << "Driver de render: " << SDL_GetRendererName(renderer) << "\n";

    // 4) Un frame mínimo: limpiar a azul y presentar.
    SDL_SetRenderDrawColor(renderer, 40, 60, 120, 255);
    SDL_RenderClear(renderer);   // nota: el clear TAMBIÉN es un draw
    SDL_RenderPresent(renderer); // flip del back buffer

    // 5) Irse: sin loop, 3 segundos y cerrá (cuando corra, verás base azul).
    SDL_Delay(3000);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(ventana);
    SDL_Quit();
    return 0;
}

/* ¿QUÉ PASA POR DEBAJO DEL RENDERER? (nivel del minuto)
   El SDL_Renderer es una API 2D que por detrás elige un BACKEND:
   GL 3.3+ (tu Celeron N4020 con Mesa), Vulkan, D3D en Windows, o metal.
   Cada llamada a SDL_RenderX se traduce a comandos de ese backend (por eso
   es común que sea "2D acelerado"). La lección 07 (batching manual) muestra
   por qué interesa agrupar dibujos desde arriba para no dañar la fila de
   draw calls del backend. */

/* -----------------------------------------------------------------------------
   2. ORDEN DE DESTRUCCIÓN (los errores clásicos del mundo SDL)
   La regla es INVERSA a la creación:
     creado:   Init -> Window -> Renderer
     destruí:  Renderer -> Window -> SDL_Quit
   - Si destruyés la ventana ANTES que el renderer: SDL3 lo tolera (le quita
     la referencia al renderer), pero NO hagas código que dependa de eso.
   - SDL_DestroyRenderer(renderer) es el call que flushesa/cierra contextos
     GL del backend; ir reverso a la creación evita leaks con doble-destroy.
   RAII aquí: el patrón "guard en scope" del Módulo 07:
      struct Scoped {
          SDL_Window* w; SDL_Renderer* r;
          ~Scoped() { SDL_DestroyRenderer(r); SDL_DestroyWindow(w); }
      };
   Los miembros se destruyen EN REVERSA al orden de declaración: si declarás
   'w' primero y 'r' después, ~Scoped destruye 'r' y luego 'w'.
   La lección 03 arma el wrapper completo con movimiento (Módulo 08) y tests.
--------------------------------------------------------------------------- */

/* =============================================================================
   CÓMO PROBARLO A MANO
     make-like compila como todos los del curso y va demoVentana();
     (descomenta el return y compila). Recordá: los tests del ej 02 NUNCA
     abren una ventana — están hechos para compilar SOLAMENTE los calls
     a SDL y el rastro lógico es del wrapper RAII puro (modulo 07/08).
   ============================================================================= */
int main() { return demoVentana(); }
