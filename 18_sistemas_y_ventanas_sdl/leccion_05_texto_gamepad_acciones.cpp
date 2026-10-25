/* =============================================================================
   MÓDULO 18 - LECCIÓN 05: Texto (IME), gamepad y mapeo a acciones
   =============================================================================
   TEMA 3 de TEMAS.md (2 de 2): Event loop: texto/IME, gamepad y mapping a
   acciones del módulo 16.

   CORRER A MANO (abre una ventana; probá escribir con la ventana enfocada)
     g++ -std=c++20 -Wall -Wextra leccion_05_texto_gamepad_acciones.cpp \
         $(pkg-config --cflags --libs sdl3) -o leccion05 && ./leccion05

   VERIFICADO EN CABECERAS (SDL3 3.4.18, SDL_keyboard.h / SDL_events.h /
   SDL_gamepad.h):
     - SDL_StartTextInput(SDL_Window*) / SDL_StopTextInput(SDL_Window*) ->
       bool. En SDL2 era global: SDL_StartTextInput() sin ventana.
     - SDL_SetTextInputArea(win, const SDL_Rect*, int cursor) -> bool es el
       reemplazo de SDL_SetTextInputRect(const SDL_Rect*) de SDL2: ahora es
       POR-VENTANA y pide posición del cursor (la usa el editor de método
       de entrada para mostrar el PopUp cerca de donde escribís).
     - El texto llega como SDL_EVENT_TEXT_INPUT con e.text.text (char[32],
       UTF-8) y el estado de composición llega en SDL_EVENT_TEXT_EDITING.
     - Gamepad: los eventos son SDL_EVENT_GAMEPAD_ADDED / REMOVED /
       BUTTON_DOWN / BUTTON_UP / AXIS_MOTION; el botón se describe como
       SDL_GamepadButton (abstracto: SDL_GAMEPAD_BUTTON_SOUTH, no "X de tu
       joystick": SDL nominó el layout del mando con un estándar abstracto
       (South/East/North/West): tu app no decide si es "A" o "X").
       axis va de -32768..32767 (Sint16) en e.gpad.axis_value.

   LA IDEA DEL ACTION MAPPING (nosotros ya la armamos en el 16):
     Eventos crudos -> [mapeo] -> Acciones lógicas (Jump, Shoot, MoveX...)
     - raylib: IsKeyDown -> máscara de acciones (polling,les estado)
     - SDL:    eventos DOWN/UP -> el mapeo necesita EDGE DETECTION:
               DOWN sin repeat = press; UP = release; junto con el estado
               previo de la tecla construí pulso/hold del módulo 16.
     La traducción a un struct InputState por frame es lo que hace que la
     SIMULACIÓN no dependa ni del input raw ni del timing del SO (tests del
     ej 02, del 03 en adelante, etc.).
   ============================================================================= */

#include <iostream>
#include <SDL3/SDL.h>   // iostream + SDL; std::format se vio en el módulo 12

#include <SDL3/SDL.h>

// -----------------------------------------------------------------------------
// 1. TEXT INPUT: cómo habilitarlo y recibir UNA línea de texto
// -----------------------------------------------------------------------------
struct EditorTexto {
    bool activo = false;
    std::string contenido;

    // Traducción evento -> estado. Es función PURA: testeable sin SDL.
    void recibir(const SDL_Event& e) {
        switch (e.type) {
            case SDL_EVENT_TEXT_INPUT:
                contenido += e.text.text;   // UTF-8 crudo (aunque sea multi-byte)
                break;
            case SDL_EVENT_KEY_DOWN:
                if (e.key.key == SDLK_BACKSPACE && !contenido.empty()) {
                    // pop de un codepoint UTF-8: si termina en cont, saco
                    // bytes de continuación (128..193)
                    while (!contenido.empty()) {
                        unsigned char c = contenido.back();
                        contenido.pop_back();
                        if (c < 128 || c > 191) break; // byte inicial (o ASCII)
                    }
                }
                break;
            default: break;
        }
    }
};

// -----------------------------------------------------------------------------
// 2. GAMEPAD -> ACCIÓN (basis del ej 02; lo extendemos ahí con teclado)
// -----------------------------------------------------------------------------
struct Acciones { bool saltar = false; bool disparar = false; float mover_x = 0.0f; };

// Convierte UN evento SDL de gamepad. Devuelve true si tocó algo.
static bool gamepadAEventoAccion(const SDL_Event& e, Acciones& a) {
    switch (e.type) {
        case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
            if (e.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH)  a.saltar   = true;  // "A" (Xbox) o X (Play)... abstracto!
            if (e.gbutton.button == SDL_GAMEPAD_BUTTON_EAST)   a.disparar = true;
            return true;
        case SDL_EVENT_GAMEPAD_BUTTON_UP:
            if (e.gbutton.button == SDL_GAMEPAD_BUTTON_SOUTH)  a.saltar   = false;
            if (e.gbutton.button == SDL_GAMEPAD_BUTTON_EAST)   a.disparar = false;
            return true;
        case SDL_EVENT_GAMEPAD_AXIS_MOTION:
            if (e.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTX)
                a.mover_x = float(e.gaxis.value) / 32767.0f; // -1..1
            return true;
        default: return false;
    }
}

// -----------------------------------------------------------------------------
// 3. DEMO: ventana con caja de texto + log de eventos (correrla a mano)
// -----------------------------------------------------------------------------
static int demoTextoGamepad() {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) { // el gamepad es SIEMPRE
        std::cout << "SDL_Init: " << SDL_GetError() << "\n"; return 1;
    }
    SDL_Window* win = SDL_CreateWindow("Leccion 05", 640, 480, SDL_WINDOW_RESIZABLE);
    if (!win) { SDL_Quit(); return 1; }
    SDL_StartTextInput(win); // desde AHORA los keydown van a traer texto

    std::cout << "Escribí con la ventana enfocada (el texto llega por eventos):\n";
    SDL_Event e;
    EditorTexto ed;
    Acciones a;
    bool corriendo = true;
    while (corriendo) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) corriendo = false;
            else if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_ESCAPE) corriendo = false;
            else ed.recibir(e);
            gamepadAEventoAccion(e, a);
            if (e.type == SDL_EVENT_TEXT_INPUT)
                std::cout << "texto: '" << e.text.text << "' | buffer: '" << ed.contenido << "'\n";
        }
        SDL_Delay(16);
    }
    SDL_StopTextInput(win);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}

/* =============================================================================
   NOTA DE TEST (¡importante para este módulo!)
   - EditorTexto::recibir y gamepadAEventoAccion son PERO PURO FUNCTIONS,
     juicios con SDL_Event (que es POD). Se los puede testear SIN VENTANA:
     construir SDL_Event a mano ({}, type=SDL_EVENT_TEXT_INPUT, strcpy...).
   - El EJ 02 no usa SDL_Event directamente: replica su forma en un struct
     propio (down/repeat/mod/key) para que los tests compilen sin librería.
     El patrón es: la LÓGICA del mapeo vive en tipos tuyos; SDL queda en
     el BORDE de la app (adapters). Es exactamente la architectura que
     veremos otra vez con OpenGL en el módulo 20.
   ============================================================================= */
int main() { return demoTextoGamepad(); }
