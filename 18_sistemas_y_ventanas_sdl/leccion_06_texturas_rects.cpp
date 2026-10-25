/* =============================================================================
   MÓDULO 18 - LECCIÓN 06: Texturas, rects y color mod
   =============================================================================
   TEMA 4 de TEMAS.md (1 de 2): Renderer 2D: texturas, rects, color mod,
   blend mode y draw order.

   CORRER A MANO (abre una ventana 2 segundos)
     g++ -std=c++20 -Wall -Wextra leccion_06_texturas_rects.cpp \
         $(pkg-config --cflags --libs sdl3) -o leccion06 && ./leccion06

   VERIFICADO EN CABECERAS (SDL3 3.4.18, SDL_render.h):
     - SDL_CreateTexture(renderer, format, access, w, h) -> SDL_Texture*
     - SDL_CreateTextureFromSurface(renderer, SDL_Surface*) -> SDL_Texture*
       (en SDL3 la superficie se puede liberar enseguida: la textura hace
       su propia copia en GPU).
     - SDL_RenderTexture(window->renderer, texture, srcrect* SDL_FRect*,
       dstrect* SDL_FRect*) -> bool: es el reemplazo de SDL_RenderCopy.
       srcrect == nullptr = textura entera; dstrect == nullptr = a toda
       la ventana (y NO estira desde la esquina: mapea el rect entero).
     - color mod: SDL_SetTextureColorMod(texture, r, g, b) (Uint8) +
       SDL_SetTextureAlphaMod(texture, a). El "tint" del módulo 17 era
       exactamente esto (el flash rojo de daño lo hacía con color mod).
     - draw order: SDL NO tiene z-buffer 2D: lo que dibujás al final,
       queda ARRIBA. El orden de llamadas ES el orden de dibujo.

   LOS RECTS: SDL_Rect { int x,y,w,h } vs SDL_FRect { float ... }:
     - int: el renderer legacy de SDL2. Sirve para rects de UI/plantillas.
     - float: la API fina moderna (RenderTexture usa FRect). La razón:
       el rasterizador recorta en subpixel y el scaling/rotation se acercan
       MUCHO mejor. Nosotros preferimos float en el juego (los objetos se
       mueven en floats), y solo convertimos a int cuando un quad es puro.
       (Ojo: w y h pueden ser negativos — SDL render los sabe tratar).
   ============================================================================= */

#include <iostream>
#include <SDL3/SDL.h>

// -----------------------------------------------------------------------------
// 1. CARGA MINIMA DE UN BMP (sin SDL_image: stb_image llega en el módulo 20;
//    SDL_image es una extensión oficial separada; el core de SDL trae BMP)
// -----------------------------------------------------------------------------
static SDL_Texture* texturaDesdeBMP(SDL_Renderer* ren, const char* ruta) {
    SDL_Surface* s = SDL_LoadBMP(ruta);  // SDL_LoadBMP: solo BMP, sin extension
    if (s == nullptr) return nullptr;
    SDL_Texture* t = SDL_CreateTextureFromSurface(ren, s);
    SDL_DestroySurface(s);  // SDL3: sí, DestroySurface (era SDL_FreeSurface)
    return t;               // la textura NO depende del surface
}

// -----------------------------------------------------------------------------
// 2. DEMO: un sprite (o un cuadrado blanco sin archivo), dos tint y draw order
// -----------------------------------------------------------------------------
static int demoTexturasRects() {
    if (!SDL_Init(SDL_INIT_VIDEO)) { std::cout << "init " << SDL_GetError() << "\n"; return 1; }
    SDL_Window*   win = SDL_CreateWindow("Leccion 06", 640, 480, SDL_WINDOW_RESIZABLE);
    SDL_Renderer* ren = SDL_CreateRenderer(win, nullptr);
    if (!ren) { SDL_DestroyWindow(win); SDL_Quit(); return 1; }

    // Sprite "procedural": textura blanca 32x32 con ATLAS de 2x2 tiles pintados
    SDL_Texture* tex = SDL_CreateTexture(
        ren, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 32, 32);
    std::cout << (tex ? "textura creada\n" : "fallo textura\n");

    // BONUS: si existe un BMP real en assets/, lo cargamos por la API de
    // archivos (la lección 09 explica las rutas relativas con SDL_GetBasePath)
    SDL_Texture* archTex = texturaDesdeBMP(ren, "assets/sprite.bmp");
    if (archTex != nullptr) std::cout << "BMP cargado OK\n";
    if (tex) {
        // Contenido indefinido de fábrica: la pintamos blanco UNA vez usándola
        // como render target (el equivalente 2D del FBO del módulo 20)
        SDL_SetRenderTarget(ren, tex);
        SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
        SDL_RenderClear(ren);
        SDL_SetRenderTarget(ren, nullptr); // de vuelta a la ventana
        SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND); // blend con alpha
        SDL_SetTextureAlphaMod(tex, 200);                  // transparencia global
    }

    // Posiciones en float (el juego mueve en floats; convert SÓLO al raster)
    SDL_FRect jugador  { 200.0f, 200.0f, 64.0f, 64.0f };
    SDL_FRect sombra   { 202.0f, 240.0f, 64.0f, 20.0f };
    SDL_FRect parte_txt{ 0, 0, 16, 16 };  // sub-rect del "atlas" (srcrect)
    bool verde = false;

    for (int frame = 0; frame < 120; ++frame) {
        if (frame == 60) verde = true;         // "flash" a los 60 frames

        SDL_SetRenderDrawColor(ren, 25, 25, 35, 255);
        SDL_RenderClear(ren);

        if (verde) SDL_SetTextureColorMod(tex, 0, 255, 80);   // tint verde
        else       SDL_SetTextureColorMod(tex, 255, 255, 255);// neutral

        // ORDEN = CAPAS: sombra primero (queda abajo), luego el sprite
        SDL_RenderTexture(ren, tex, &parte_txt, &sombra);
        SDL_RenderTexture(ren, tex, nullptr,    &jugador);

        SDL_RenderPresent(ren);
        SDL_Delay(30);
    }

    if (archTex != nullptr) SDL_DestroyTexture(archTex); // si no había BMP, queda nulo y no pasa nada
    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}

/* -----------------------------------------------------------------------------
   3. QUE PASA POR DEBAJO (mapa mental del renderer 2D)
   - Cada SDL_RenderTexture arma un quad (4 vértices) con las coords de los
     rects, un color final (texture mod + draw color), y lo mete a una lista.
   - El backend se toma la lista y la convierte a draw calls EN EL ORDEN
     DE LAS LLAMADAS (no hay z-buffer 2D). El batching manual del ej 04
     reordena esa lista de items ANTES de que se la entregues.
   - El color mod multiplica componente a componente (es el "multiply"
     del módulo 17): RGB*mod/255; mezclarlo con blend mode da los glow.
   - El Alpha mod y el blend mode se acumulan en la textura, NUNCA en
     el 'destino': pintar "en transparente" no aísla al dibujo siguiente.
----------------------------------------------------------------------------- */

/* =============================================================================
   COMO PROBARLA A MANO: la ventana 120 frames (~3.6 s), flash verde y
   redibujo. Si no tenés un sprite, la textura creada en TARGET 32x32 se
   ve un cuadrado blanco (alpha 200): con el srcrect/parte (16x16) la sombra
   muestra UN sub-rect del mismo "atlas" — el concepto de atlas + sub-rect
   del módulo 17, pero a mano en SDL.
   ============================================================================= */
int main() { return demoTexturasRects(); }
