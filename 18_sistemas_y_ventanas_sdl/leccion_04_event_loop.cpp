/* =============================================================================
   MÓDULO 18 - LECCIÓN 04: El event loop de SDL3
   =============================================================================
   TEMA 3 de TEMAS.md (1 de 2): Event loop: SDL_PollEvent, tipos de evento.

   CORRER A MANO (abre una ventana)
     g++ -std=c++20 -Wall -Wextra leccion_04_event_loop.cpp \
         $(pkg-config --cflags --libs sdl3) -o leccion04 && ./leccion04

   En raylib (módulo 16) el input era POLLING puro: preguntábamos
   IsKeyDown cada frame. SDL te da las DOS formas:
     - POLLING: SDL_GetKeyboardState(...) -> const bool* por scancodes
     - EVENTOS: SDL_PollEvent que DRENA una cola (queue) mantenida por SDL.

   LA COLA DE EVENTOS (qué pasa por debajo):
   - El SO (X11/Wayland) entrega eventos; SDL los traduce y los empuja en
     una cola interna (thread-safe).
   - SDL_PollEvent(&e) saca UN evento por llamada y devuelve false cuando
     la cola quedó vacía (nunca bloquea).
   - SDL_WaitEvent(&e) SÍ bloquea: útil para apps de escritorio que no
     quieren quemar CPU (tu Celeron agradece). Los juegos casi siempre usan
     Poll + timestep fijo del módulo 16.

   EL SDL_Event (struct gigante / union etiquetada, SDL_events.h):
     Cada evento tiene 'type' (SDL_EVENT_*) y los datos viven en un miembro
     con el nombre que corresponde: e.key, e.text, e.window, e.gamepad...
     Confirmado en las cabeceras:
       SDL_EVENT_QUIT          0x100  (cerrá la ventana)
       SDL_EVENT_KEY_DOWN/UP   0x300/ (y este rango de teclado)
         SDL_KeyboardEvent { type, timestamp, windowID, scancode, key,
                             down (bool), repeat (bool), ... }
       SDL_EVENT_WINDOW_*      (moved/resized/display_scale_changed...)
       SDL_EVENT_TEXT_INPUT    (lección 05, con texto UTF-8)

   DETALLE CLAVE QUE NO HAY QUE PERDER:
     event.key.repeat == true significa "el SO me re-notifica mientras lo
     dejo apretado" (key repeat del sistema). Para acciones de juego
     (saltar, disparar) tenés QUE FILTRARLO o el salto salta cada repeat.
     Lo testeamos de verdad en el ej 02.
   ============================================================================= */

#include <iostream>
#include <SDL3/SDL.h>

// -----------------------------------------------------------------------------
// 1. UN LOOP POLL MÍNIMO (la base de TODO juego SDL)
// -----------------------------------------------------------------------------
static int demoEventLoop() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL_Init: " << SDL_GetError() << "\n";
        return 1;
    }

    SDL_Window* win = SDL_CreateWindow("Leccion 04 - eventos", 640, 480, SDL_WINDOW_RESIZABLE);
    if (win == nullptr) { SDL_Quit(); return 1; }

    SDL_Renderer* ren = SDL_CreateRenderer(win, nullptr);
    if (ren == nullptr) { SDL_DestroyWindow(win); SDL_Quit(); return 1; }

    bool corriendo = true;
    int frames = 0;
    while (corriendo) {
        // ---- EVENTOS: drenar TODO lo que está en la cola (TODOS los frames)
        SDL_Event e;
        while (SDL_PollEvent(&e)) {  // false cuando la cola queda vacía
            switch (e.type) {
                case SDL_EVENT_QUIT:
                    corriendo = false;
                    break;
                case SDL_EVENT_KEY_DOWN:
                    std::cout << "keydown: " << SDL_GetKeyName(e.key.key)
                              << (e.key.repeat ? " (repeat! lo filtro)" : "") << "\n";
                    if (e.key.key == SDLK_ESCAPE) corriendo = false;
                    break;
                case SDL_EVENT_KEY_UP:
                    std::cout << "keyup: " << SDL_GetKeyName(e.key.key) << "\n";
                    break;
                case SDL_EVENT_WINDOW_RESIZED:
                    std::cout << "resized a " << e.window.data1 << "x"
                              << e.window.data2 << "\n";
                    break;
                default:
                    break; // hay decenas: no las manejamos acá
            }
        }

        // ---- INPUT POR POLLING (la alternativa, misma iteración)
        int numkeys = 0;
        const bool* keys = SDL_GetKeyboardState(&numkeys);
        if (keys[SDL_SCANCODE_LEFT]) {
            // move_left = true (acá sólo imprimimos cada ~240 frames)
        }

        // ---- RENDER mínimos
        SDL_SetRenderDrawColor(ren, 30, 30, 40, 255);
        SDL_RenderClear(ren);
        SDL_RenderPresent(ren);
        ++frames;
        if (frames % 240 == 0)
            std::cout << "frame " << frames << " (apretá ESC o cerrá)\n";
        SDL_Delay(8); // sin vsync ni timer: finto 120 fps, para mirar nomás
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}

/* -----------------------------------------------------------------------------
   2. ESTRUCTURA RECOMENDADA PARA UN JUEGO (síntesis de los módulos 16+18)
       while (corriendo) {
           drenarEventosEnAcciones();   // arriba: traducción -> InputState
           acumularTiempo();            // chrono del módulo 16
           while (acumulador >= paso) { simular(paso); acumulador -= paso; }
           render(alpha);               // interpolación del módulo 16
       }
   - El input se lee UNA vez por frame (drenar la cola entera), no dentro
     del loop de simulación: así la simulación queda determinista.
   - "Traducir eventos a acciones" es la parte testable (ej 02) y es la
     misma IDEA de action-mapping del módulo 16, pero invertida: raylib
     da estados (apretado sí/no), SDL te da hechos (down/up con repeat).
----------------------------------------------------------------------------- */

/* =============================================================================
   COMO PROBARLA A MANO: abre una ventana, dale focus, apretá teclas (y
   sostené una para ver el repeat), agrandá la ventana, cerrá con ESC.
   Mirá el stdout: cada evento que el switch no nos muestro, se ignora.
   Para VER la tasa de eventos sostenidos, cambiá SDL_Delay(8) por 0.
   ============================================================================= */
int main() { return demoEventLoop(); }
