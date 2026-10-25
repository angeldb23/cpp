/* =============================================================================
   MÓDULO 20 - LECCIÓN 11: Texturas 2D (2/2) — sRGB, gamma y ATLAS de sprites
   =============================================================================
   TEMA 7 de TEMAS.md (2 de 2): sRGB y atlas.

   CÓMO CORRERLA A MANO (desde la carpeta del módulo):
     g++ -std=c++20 -Wall -Wextra leccion_11_srgb_atlas.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec11 && ./lec11

   --- SRGB: qué es y cuándo usarlo (y por qué 21 lo retoma) -------------------
   La imagen PNG guardada por un editor está en sRGB: los bytes están
   codificados con la curva gamma del monitor, NO en física lineal. La GPU
   mezcla (L12) e ilumina (21) con números lineal; si le entregás sRGB sin
   marcar, la mezcla sale más oscura/de contraste raro ("50% gris + 50%
   gris" da ~73% y no la mitad percibida).

   El GL lo etiqueta con FORMATOS internos (core 3.0+):
     GL_SRGB8_ALPHA8 / GL_SRGB8: "los bytes subidos están en sRGB: al
       lecturas del shader se devuelven LINEALIZADOS". (El escribir de
       vuelta al framebuffer con corrección inversa es GL_ARB_framebuffer_
       sRGB / mesa: A VERIFICAR en tu build Mesa; alternativa segura y
       didáctica: el pow() manual en el fragment shader, gamma 2.2.)
     GL_RGBA8 (default de la lección 09): "lineal": el byte se usa como
       viene (físico); el shader opera igual siempre.
   REGLA DEL CURSO: imagen de disco con arte (sprites, texturas de luz de
   luna) -> internal sRGB + BLEND; buffers intermedios (FBO de la L15) ->RGBA8:
   los datos intermedios ya están lineales (pega con lo del 21).

   LA REGLA ENGANCHADA DEL CURSO: conversión out -> uniform "sRGB value in,
   lineal en el shader, E conversion needed": Las imágenes del 09 con
   glTexImage2D internal GL_SRGB8_ALPHA8 en el 21 (texturas iluminadas), o
   conversión manual en shader: pow(color.rgb, vec3(1.0/2.2)).

   --- ATLAS: UNA TEXTURA TODOS LOS SPRITES -----------------------------------
   Cada sprite en su propia textura = UNA TEXTURA DISTINTA en ----? No...
   Cambiar de textura enlazada en cada draw call es de lo MAS CARO del
   estado (los caches de fetch se vacían a cada swap): "texture thrash";
   en 2D con miles de sprites por frame es el asesino silencioso del perf.
   EL ATLAS: una única textura grande donde TODO el set de sprites vive
   en sub-rectángulos; el fragment shader recibe SOLO UN texto; los QUADS
   del nivel cambian las UV para apuntar al sprite correcto.

   UV de un sprite: el espacio UV del quad NO es [0,1]: es la VENTANA del
   sprite dentro del atlas:
     u0 = x / texW ; v0 = y / texH ; u1 = (x+w)/texW ; v1 = (y+h)/texH
   ¡OJO! las UV apuntan al CENTRO del texel en el borde del sprite:
     u0 = (x + 0.5) / texW etc. (half-texel). Con LINEAR en MIN/MAG, un
   sprite pegado al vecino "sangra" (color bleeding): half-texel inset o
   padding de 1-2 px entre sprites son las defensas estándar.
     . El PACKER (quién coloca cada sprite dónde sin solaparse) ES el
   ejercicio 04 (shelf packing): el simple estándar; bin-packing fino
   queda para el módulo 22 si hiciera falta (ECS/asset registry).

   Emisión 2D-up: la textura atlas con flip del 09 (stbi flip) y las UV
  ROS: la coordenada v del atlas: cohesivo TODO por convención, decide
   media vez (recomendado: FLIP OFF + escribir UV conforme al espacio del
   atlas tal como se ve en el archivo).
   --------------------------------------------------------------------------- */

#include "include/gl_util.hpp"
#include <iostream>
#include <vector>

/* Shaders del ATLAS: 1 textura enlazada, UV por sprite (uniform). El color
   "tint" es el sprite del módulo 17 (raylib usa ítem de la misma op).     */
static const char* VS_FUENTE = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
out vec2 vUV;
uniform vec4 uUVRect;    // u0, v0, u1, v1: EL VENTANA del sprite en el atlas
void main() {
    // El quad base es [-0.5,0.5]x[-0.5,0.5]; al estirarlo con Pos
    // "pos*escala" queda el mar de los sprites (el UV mapea a las esquinas):
    gl_Position = vec4(aPos * vec2(0.4, 0.4), 0.0, 1.0);
    vUV = mix(uUVRect.xy, uUVRect.zw, aPos + 0.5);   // corner: rect in-out
}
)";

static const char* FS_FUENTE = R"(
#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D uAtlas;
uniform vec3 uTint;
void main() {
    vec4 tex = texture(uAtlas, vUV);
    FragColor = vec4(tex.rgb * uTint, tex.a);
}
)";

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL_Init fallo: " << SDL_GetError() << "\n";
        return 1;
    }

    glu::Contexto c = glu::abrir("Modulo 20 - L11 sRGB y atlas", 800, 600);
    if (c.contexto == nullptr) { SDL_Quit(); return 1; }

    GLuint prog = glu::enlazar(VS_FUENTE, FS_FUENTE);
    if (prog == 0) { glu::cerrar(c); SDL_Quit(); return 1; }

    /* ATLAS 4 x 1 tiles de 16x16 = textura 64x16, generada en C++:
       tile 0 = cuadrado rojo, tile 1 = verde, tile 2 = azul, tile 3 = blanco */
    const int TEXW = 64, TEXH = 16, TILE = 16;
    std::vector<unsigned char> px((size_t)TEXW * TEXH * 4);
    const unsigned char colores[4][4] = {
        {255,   0,   0, 255}, {  0,255,   0, 255},
        {  0,   0,255, 255}, {255,255,255, 255},
    };
    for (int y = 0; y < TEXH; ++y)
        for (int x = 0; x < TEXW; ++x) {
            int tile = x / TILE;
            size_t i = ((size_t)y * TEXW + (size_t)x) * 4;
            px[i]   = colores[tile][0];
            px[i+1] = colores[tile][1];
            px[i+2] = colores[tile][2];
            px[i+3] = colores[tile][3];
        }

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    // NOTA SRGB: si el pipeline decide tratar la atlas como imagen sRGB,
    // el internalformat cambia y NADA más: los UV son los mismos.
    // A VERIFICAR en tu Mesa (21 cierra): GL_FRAMEBUFFER_SRGB o pow manual.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TEXW, TEXH, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, px.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // NEAREST con atlas evita bleeding; LINEAR solo + half-texel inset si
    // rotás/escala el sprite (21 lo muestra con el tint del modulo 17).

    // quad del batch: un VBO, UV "abierta" por uniform (uUVRect).
    static const float quad[6][2] = {
        {-0.5f, -0.5f}, { 0.5f, -0.5f}, { 0.5f,  0.5f},
        {-0.5f, -0.5f}, { 0.5f,  0.5f}, {-0.5f,  0.5f},
    };
    GLuint vao = 0, vbo = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof quad, quad, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (const void*)0);
    glBindVertexArray(0);

    GLint uUVRect = glGetUniformLocation(prog, "uUVRect");
    GLint uAtlas  = glGetUniformLocation(prog, "uAtlas");
    GLint uTint   = glGetUniformLocation(prog, "uTint");

    // DIBUJAR los 4 tiles (un draw c/u; el batcher real de instancing es L14;
    // glBlendFunc defered de la lección 12:ossal tras BMI)
    bool corriendo = true;
    SDL_Event ev;
    while (corriendo) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) corriendo = false;
        }
        glClearColor(0.06f, 0.07f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(prog);
        glUniform1i(uAtlas, 0);
        glBindVertexArray(vao);
        for (int tile = 0; tile < 4; ++tile) {
            float u0 = (tile * TILE + 0.5f) / (float)TEXW;   // half-texel inset
            float v0 = (0          + 0.5f) / (float)TEXH;
            float u1 = ((tile + 1) * TILE - 0.5f) / (float)TEXW;
            float v1 = (TEXH        - 0.5f) / (float)TEXH;
            glUniform4f(uUVRect, u0, v0, u1, v1);
            glUniform3f(uTint, 1, 1, 1);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        SDL_GL_SwapWindow(c.ventana);
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteTextures(1, &tex);
    glDeleteProgram(prog);
    glu::cerrar(c);
    SDL_Quit();
    return 0;
}
