/* =============================================================================
   MÓDULO 18 - LECCIÓN 07: Rotación/flip, blend modes y batching manual
   =============================================================================
   TEMA 4 de TEMAS.md (2 de 2): Renderer 2D: blend mode, rotation/flip y
   batching manual.

   CORRER A MANO (abre una ventana)
     g++ -std=c++20 -Wall -Wextra leccion_07_batching_manual.cpp \
         $(pkg-config --cflags --libs sdl3) -o leccion07 && ./leccion07

   VERIFICADO EN CABECERAS (SDL_render.h, SDL3 3.4.18):
     - SDL_RenderTextureRotated(renderer, texture, const SDL_FRect* srcrect,
       const SDL_FRect* dstrect, const double angle,
       const SDL_FPoint* center, const SDL_FlipMode flip) — reemplaza al
       SDL_RenderCopyEx de SDL2: ángulo en GRADOS (convención: horario en
       pantalla, y=abajo), center es el origen del giro (NULL = centro) y
       flip vale SDL_FLIP_NONE / SDL_FLIP_HORIZONTAL / SDL_FLIP_VERTICAL.
     - SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND) y
       SDL_SetTextureBlendMode(tex, ...) son modos: NONE, BLEND (alpha),
       ADD, MUL, MOD (tint multiplicativo), MUL (multiplicativa)...
   - Con SDL 2 el renderer interno hacía batching de quads con un máximo
     de SDL2 (batching automático con un límite de 8192 quads por batch).
     En SDL3 el batching interno existe igual, PERO es un detalle de
     implementación sin garantía de reordenado (el env var SDL_RENDER_
     BATCHING apaga el batching automático de SDL2; en SDL3 revisar en
     la doc de tu versión — A VERIFICAR). Lo importante pedagógico: el
     batching EXTERNO (nuestro, del ej 04) añade control real del orden:
   ============================================================================= */

#include <iostream>
#include <SDL3/SDL.h>

// -----------------------------------------------------------------------------
// 1. LAS CUATRO PIEZAS DE UN "DRAW ITEM" (lo que mandás a la GPU)
//    Es la MISMA anatomía del command-buffer del módulo 16 y los sprites del
//    módulo 17, ahora en tipos SDL: layer (orden), texture (subida), rect y
//    transform (rotación/flip/tint). El Batcher del ej 04 juntará estos
//    structs por (layer, texture) para minimizar draw calls.
// -----------------------------------------------------------------------------
struct DrawItem {
    int layer = 0;              // fondo < player < UI
    SDL_Texture* tex = nullptr; // agrupación por textura
    SDL_FRect src { 0, 0, 32, 32 }; // sub-rect (atlas)
    SDL_FRect dst { 0, 0, 32, 32 }; // destino en pantalla
    double angle = 0.0;         // grados horarios (pantalla y-abajo)
    SDL_FlipMode flip = SDL_FLIP_NONE;
    Uint8 r = 255, g = 255, b = 255, a = 255; // tint vía color+alpha mod
};

// Dibuja un item con TODA su transform: la sobrecarga "fina"
static void dibujarItem(SDL_Renderer* ren, const DrawItem& it) {
    SDL_SetTextureColorMod(it.tex, it.r, it.g, it.b);
    SDL_SetTextureAlphaMod(it.tex, it.a);
    if (it.angle == 0.0 && it.flip == SDL_FLIP_NONE) {
        SDL_RenderTexture(ren, it.tex, &it.src, &it.dst);
    } else {
        SDL_FPoint centro{ it.dst.x + it.dst.w * 0.5f, it.dst.y + it.dst.h * 0.5f };
        SDL_RenderTextureRotated(ren, it.tex, &it.src, &it.dst,
                                 it.angle, &centro, it.flip);
    }
}

/* -----------------------------------------------------------------------------
   2. BATCHING: por qué importa en una máquina modesta
     Un draw call = cambiar de textura + subir geometría + ejecutar shader.
     En tu Celeron con GL 3.3 (Mesa), el costo dominante es el cambio de
     bind de textura: cada textura distinta paga un bind extra. Consejos
     de este tramo:
     - Agrupar por textura ANTES de dibujar (el ej 04: sort + merge).
     - Atlas: varias texturas en UNA (lección del módulo 17, acá se mapea
       con srcrect y no existe "cargar sub-imagen").
     - Orden estable: dibujar (layer asc, texture asc) mantiene el orden
       de capas Y reduce cambios de estado.
   Lo hace la LÓGICA (testable), la llamada queda igual.
----------------------------------------------------------------------------- */

// -----------------------------------------------------------------------------
// 3. DEMO: 3 sprites rotados con flip + blend ADD de brillo
// -----------------------------------------------------------------------------
static int demoRotacionFlip() {
    if (!SDL_Init(SDL_INIT_VIDEO)) { std::cout << "init " << SDL_GetError() << "\n"; return 1; }
    SDL_Window* win = SDL_CreateWindow("Leccion 07", 640, 480, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* ren = SDL_CreateRenderer(win, nullptr);
    if (!ren) { SDL_DestroyWindow(win); SDL_Quit(); return 1; }

    SDL_Texture* tex = SDL_CreateTexture(
        ren, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 32, 32);
    SDL_SetRenderTarget(ren, tex);
    SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
    SDL_RenderClear(ren);
    SDL_SetRenderTarget(ren, nullptr);

    DrawItem fondo { 0, tex, {}, { 0, 0, 320, 480 }, 0.0};
    DrawItem notas { 1, tex, {}, { 300, 100, 64, 64 }, 45.0, SDL_FLIP_HORIZONTAL};
    DrawItem brillo{ 2, tex, {}, { 120, 240, 96, 96 }, 15.0, SDL_FLIP_NONE, 255, 200, 120, 180};

    for (int frame = 0; frame < 120; ++frame) {
        // la rotación AVANZA: el ángulo es un dato, no una llamada mágica
        notas.angle  = double(frame) * 1.5;
        brillo.angle = 45.0 - double(frame) * 0.5;
        if (frame % 40 == 0) {
            SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_ADD);
            std::cout << "blend ADD activado (el brillo mejora)\n";
        }
        SDL_SetRenderDrawColor(ren, 20, 20, 30, 255);
        SDL_RenderClear(ren);
        // en orden: fondo, notas, brillo (sin batcher: son 3)
        dibujarItem(ren, fondo);
        dibujarItem(ren, notas);
        dibujarItem(ren, brillo);
        SDL_RenderPresent(ren);
        SDL_Delay(30);
    }

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}

/* =============================================================================
   QUÉ QUEDA TESTEABLE DE ESTA LECCIÓN (sin ventanas):
   - El Batcher (ej 04): lista de DrawItem -> sort estable por
     (layer, texture, orden de llegada) y, para cada subsecuencia de la
     MISMA textura, UN draw call. Se comprueba con contadores, no con ojos.
   - La anatomía del DrawItem de arriba es 100% C++ sin SDL en los tests
     (los tests replican el struct con un id de textura en vez de puntero).
   ============================================================================= */
int main() { return demoRotacionFlip(); }
