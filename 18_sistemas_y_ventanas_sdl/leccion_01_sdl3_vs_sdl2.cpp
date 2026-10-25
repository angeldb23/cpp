/* =============================================================================
   MÓDULO 18 - LECCIÓN 01: SDL3 vs SDL2 — qué cambió de verdad
   =============================================================================
   TEMA 1 de TEMAS.md: SDL3 vs SDL2: cambios reales (1 lección).

   Esta lección NO abre ventanas: compila, linkea y muestra constantes.
   Cómo compilar acá:
     g++ -std=c++20 -Wall -Wextra leccion_01_sdl3_vs_sdl2.cpp \
         $(pkg-config --cflags --libs sdl3) -o leccion01 && ./leccion01
   (cada lección explica cómo correr su demo a mano; los tests del módulo
    nunca abren ventanas)

   ─────────────────────────────────────────────────────────────────────────────
   Todo lo que esta lección afirma está verificado contra las CABECERAS
   INSTALADAS del sistema (SDL3 3.4.18 y SDL2 2.32.74) y contra los releases
   oficiales de GitHub (libsdl-org/SDL): tags release-3.4.18 / release-2.32.10.

   La técnica para corroborarlo vos mismo (¡hacelo!):
     grep -n "SDL_RenderTexture(" /usr/include/SDL3/SDL_render.h

   1) RETORNO DE ERRORES: int (-1 = falla) -> bool (false = falla).
      SDL2: extern DECLSPEC int SDLCALL SDL_RenderCopy(SDL_Renderer*, ...)
      SDL3: extern SDL_DECLSPEC bool SDLCALL SDL_RenderTexture(SDL_Renderer*, ...)
      Igual para SDL_Init, SDL_CreateWindowAndRenderer, SDL_SetRenderVSync, etc.

   2) RENDER 2D: SDL_RenderCopy / SDL_RenderCopyEx -> SDL_RenderTexture /
      SDL_RenderTextureRotated, y aparece SDL_RenderTextureTiled (tiled, que
      SDL2 no tenía). Las funciones nuevas trabajan con SDL_FRect (float):
      el rasterizador 2D de SDL3 ahora recorta y redondea la geometría en
      float con sub-pixel accuracy, sin los artefactos del renderer legacy.

   3) FLAGS DE VENTANA PASAN A 64-BIT: SDL2 usaba Uint32 y el flag
      SDL_WINDOW_ALLOW_HIGHDPI (0x00002000). SDL3 usa Uint64
      (SDL_UINT64_C) y el flag se llama SDL_WINDOW_HIGH_PIXEL_DENSITY
      (mismo valor 0x2000; nombre mejor: describe densidad de píxeles,
      que es lo que en realidad te interesa medir).
        SDL2: SDL_WINDOW_ALLOW_HIGHDPI = 0x00002000        (SDL_video.h, Uint32)
        SDL3: SDL_WINDOW_HIGH_PIXEL_DENSITY = SDL_UINT64_C(0x...2000)

   4) AUDIO: el modelo anclado al device con callback cambia a STREAMS.
      SDL2: SDL_OpenAudioDevice(dev, 0, &spec, &have, 0) y tu callback
            llenaba el buffer a tiempo (te empujan un buffer y la latencia
            la cargás vos). Si el formato pedía otra cosa, convertías vos.
      SDL3: SDL_OpenAudioDeviceStream(device, &spec, callback, userdata)
            te devuelve un SDL_AudioStream*: vos metés datos cuando querés
            con SDL_PutAudioStreamData(stream, buf, len), y SDL encadena
            automáticamente la conversión de formato/rate/canales.
            El callback (SDL_AudioStreamCallback) corre EN el hilo del
            device (¡cuidado con lo que tocás ahí!) y su firma es
            (void* userdata, SDL_AudioStream* stream, int additional_amount,
             int total_amount)
            donde additional_amount es lo que falta para completar el chunk
            de este frame y total_amount el total que pide por frame.
      La conversión de formato y el resampleo viven SIEMPRE en el stream:
      SDL_SetAudioStreamFormat(stream, &src, &dst) se puede llamar en
      cualquier momento y el stream se adapta.

   5) EVENTOS: la cola de eventos cambia en tres frentes:
      - timestamps en 64 bits, en NANOSEGUNDOS, por evento.
      - los tipos quedan como enum SDL_EVENT_* (en SDL2 eran SDL_QUIT,
        SDL_KEYDOWN, etc.): renombrados y organizados por rangos numericos
        (0x100 quit, 0x300 teclado, 0x4xx ventana, 0x6xx gamepad...).
      - NUEVOS eventos nombrados: SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED
        (la escala del display cambió: Wayland/HiDPI/balanceo de layout).
      - SDL_QueueUserEvent() fue REMOVIDO: se usa SDL_PushEvent().
   6) INIT DEVUELVE bool: (SDL_Init/SDL_InitSubSystem/SDL_CreateWindow...)
      y SDL_GetError() sigue siendo tu amigo cuando algo no apareció.
   7) MAIN CALLBACKS: SDL_AppInit/SDL_AppEvent/SDL_AppIterate/SDL_AppQuit
      es la nueva forma de escribir el programa para plataformas donde
      no controlás el main loop (iOS, Android, Web). Los usamos al cerrar
      el módulo, en el port de Asteroids.
   8) WAYLAND-PRIMERO: SDL3 soporta Wayland de forma nativa por defecto
      (en Arch Linux + KDE/GNOME/Hyprland) y X11 es una opción secundaria;
      detallamos esto en la última lección.
   ============================================================================= */

#include <iostream>
#include <SDL3/SDL.h>

// -----------------------------------------------------------------------------
// 1. CHEQUEOS EN TIEMPO DE COMPILACION: lo que las cabeceras de SDL3 Prometen
//    (esto es gratis pero digerible para el estudiante)
// -----------------------------------------------------------------------------
// El flag de densidad de pixeles existe y vale 0x2000
#if defined(SDL_WINDOW_HIGH_PIXEL_DENSITY)
static_assert(SDL_WINDOW_HIGH_PIXEL_DENSITY == SDL_UINT64_C(0x2000),
              "SDL_WINDOW_HIGH_PIXEL_DENSITY debe valer 0x2000");
#else
#error "Tu SDL3 no define SDL_WINDOW_HIGH_PIXEL_DENSITY: version muy vieja"
#endif

// El union de eventos es etiquetado por 'type'; SDL_EVENT_QUIT es el 0x100.
static_assert(SDL_EVENT_QUIT == 0x100, "SDL_EVENT_QUIT es 0x100 en SDL3");
static_assert(SDL_EVENT_KEY_DOWN == 0x300, "SDL_EVENT_KEY_DOWN es 0x300 en SDL3");

// -----------------------------------------------------------------------------
// 2. TABLA de migracion (la misma estructura que el ej 9 usa de referencia)
// -----------------------------------------------------------------------------
struct Migracion {
    const char* sdl2;
    const char* sdl3;
    const char* nota;
};

static const Migracion TABLA[] = {
    {"SDL_Init(u32) -> int",
     "SDL_Init(flags) -> bool",
     "los numeros negativos son int en SDL2; en SDL3, bool con false == error"},
    {"SDL_RenderCopy / SDL_RenderCopyEx (rects int, retorno int)",
     "SDL_RenderTexture / SDL_RenderTextureRotated (SDL_FRect, retorno bool)",
     "el rasterizador nuevo dibuja en float con recorte subpixel"},
    {"SDL_WINDOW_ALLOW_HIGHDPI",
     "SDL_WINDOW_HIGH_PIXEL_DENSITY",
     "mismos bits 0x2000, nombre preciso: densidad de pixeles"},
    {"SDL_OpenAudioDevice(+callback push)",
     "SDL_OpenAudioDeviceStream(+stream pull)",
     "vos metes bytes con PutAudioStreamData y SDL tira automaticamente"},
    {"SDL_QueueUserEvent(...)",
     "SDL_PushEvent(&event)",
     "removido en SDL3, misma generalizacion del push"},
    {"SDL_SetTextInputRect(rect)",
     "SDL_SetTextInputArea(window, rect, cursor)",
     "por-ventana y con cursor; antes no tenias control de ventana"},
    {"SDL_CreateCond / SDL_CondWait (int, en SDL_thread.h)",
     "SDL_CreateCondition / SDL_WaitConditionTimeout (bool, SDL_mutex.h)",
     "renombres SDL3: condiciones mudaron al header de mutexes"},
    {"char* SDL_GetBasePath(void)",
     "const char* SDL_GetBasePath(void) (cacheado)",
     "en SDL3 el string es propiedad de SDL: NO hay que SDL_free"},
};

static void imprimirTabla() {
    std::cout << "=== SDL2 -> SDL3 (cambios comprobados en cabeceras) ===\n";
    for (const Migracion& m : TABLA) {
        std::cout << "+ " << m.sdl2 << "  ->  " << m.sdl3 << "\n"
                  << "     (nota) " << m.nota << "\n";
    }
}

// -----------------------------------------------------------------------------
// 3. DEMO MAIN (sin ventanas): imprime versiones y constantes reales
// -----------------------------------------------------------------------------
int main() {
    std::cout << "=== LECCIÓN 01: SDL3 vs SDL2 ===\n";
    // OTRA DIFERENCIA REAL: en SDL2 el versionado era un struct SDL_version
    // + SDL_VERSION(&v). En SDL3 TODO es un int codificado (3004018 = 3.4.18)
    // y existe SDL_VERSION (macro, compile-time) vs SDL_GetVersion() (runtime).
    const int compileTime = SDL_VERSION;            // versión de los headers
    const int linked       = SDL_GetVersion();       // versión de la lib linkeada
    std::cout << "Version de los headers SDL:   "
              << SDL_VERSIONNUM_MAJOR(compileTime) << "."
              << SDL_VERSIONNUM_MINOR(compileTime) << "."
              << SDL_VERSIONNUM_MICRO(compileTime) << "\n";
    std::cout << "Version de la lib linkeada:   "
              << SDL_VERSIONNUM_MAJOR(linked) << "."
              << SDL_VERSIONNUM_MINOR(linked) << "."
              << SDL_VERSIONNUM_MICRO(linked) << "\n\n";

    std::cout << "Constantes verificadas en tiempo de compilacion:\n"
              << "  SDL_EVENT_QUIT             = 0x" << std::hex << SDL_EVENT_QUIT << std::dec << "\n"
              << "  SDL_EVENT_KEY_DOWN         = 0x" << std::hex << SDL_EVENT_KEY_DOWN << std::dec << "\n"
              << "  SDL_WINDOW_HIGH_PIXEL_DENSITY = 0x" << std::hex
              << SDL_WINDOW_HIGH_PIXEL_DENSITY << std::dec << "\n\n";

    imprimirTabla();
    return 0;
}

/* =============================================================================
   QUÉ MIRAR CUANDO LA CORRAS A MANO
   - Solo imprime: no hay ventanas, no SDL_Init, todo por constants.
   - La versión de headers y la de runtime PUEDEN ser distintas (es el clásico
     caso de tener una lib actualizada y compilar contra una baja distinta);
     SDL te da funciones para ambas, y en 3.x tenes SDL_GetVersion().
   - La próxima lección entra de verdad: ventana + renderer + orden correcto
     de destrucción (el tema "destrucción ordenada" del módulo es crítico).
   ============================================================================= */
