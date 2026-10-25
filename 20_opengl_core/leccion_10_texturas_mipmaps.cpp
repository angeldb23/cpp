/* =============================================================================
   MÓDULO 20 - LECCIÓN 10: Texturas 2D (1/2) — glGenTextures, wrap, filter y
   mipmaps con glGenerateMipmap
   =============================================================================
   TEMA 7 de TEMAS.md (1 de 2): Texturas 2D: glGenTextures, wrap/filter/
   mipmap, glGenerateMipmap. (sRGB y atlas van en la lección 11.)

   CÓMO CORRERLA A MANO (desde la carpeta del módulo):
     g++ -std=c++20 -Wall -Wextra leccion_10_texturas_mipmaps.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec10 && ./lec10

   --- UNA TEXTURA ES MEMORIA + SAMPLER STATE --------------------------------
   Dos mitades dentro del MISMO objeto (GL_TEXTURE_2D):
     1) DATOS: bytes subidos con glTexImage2D (lección 09: desde stb).
     2) SAMPLER: CÓMO SE LEE cuando el fragment shader pide texture(...):
        - WRAP: qué pasa cuando la UV sale de [0,1] (GL_REPEAT repite el
          patrón; GL_CLAMP_TO_EDGE estira el borde; GL_MIRRORED...).
        - FILTER: qué pasa cuando 1 texel NO mapea 1 pixel (MAG_FILTER
          cámara CERCA -> un texel cubre varios pixels; MIN_FILTER
          LEJOS -> varios texels caen en un pixel).
   ¡ATENCIÓN! Una textura SIN sampler configurado es "incompleta": el
   default del MIN_FILTER pide niveles mip que no existen sin
   glGenerateMipmap y el GL dibuja NEGRO (bug clásico del primer quad
   texturizado). Regla del curso: SIEMPRE los 4 glTexParameteri
   (wrap S/T + filter MIN/MAG), aunque den por obvios.

   --- MIPMAPS: la cadena de imágenes pre-calculadas --------------------------
   "Mip" = versiones precalculadas más chicas de la MISMA textura:
     256x256, 128x128, 64x64, ..., 1x1 (cadena completa).
   ¿POR QUÉ? Cerca de lejos, un pixel de pantalla cubre MUCHOS texels:
   muestrear directo da ALIASING (centelleo, patrones de Moiré). Con
   mipmaps el sampler elige el nivel cuya resolución "casual" con el
   espacio de proyección (el 1x1 es la media de TODA la imagen: lejo ->
   ver la media está bien).
   glGenerateMipmap: el driver CALCULA la cadena (en VRAM) por vos;
   después cada draw elige nivel por LOD. GL_LINEAR_MIPMAP_LINEAR
   (trilinear) interpola entre los 2 niveles vecinos: suave, estándar.

   Funciones core 3.3 (verificadas en glcorearb.h):
     glGenTextures / glBindTexture(GL_TEXTURE_2D, id) / glDeleteTextures
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S/T, GL_REPEAT|...)
     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN/MAG_FILTER, ...)
     glGenerateMipmap(GL_TEXTURE_2D)
     glActiveTexture(GL_TEXTURE0+...) + glBindTexture  (sampler "texture")
   --------------------------------------------------------------------------- */

#include "include/gl_util.hpp"
#include <iostream>
#include <vector>

/* Shaders con la PRIMERA textura del curso: sampler2D + UV.          ***/
static const char* VS_FUENTE = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;
out vec2 vUV;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    vUV = aUV;
}
)";

static const char* FS_FUENTE = R"(
#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D uTex;
void main() {
    FragColor = texture(uTex, vUV);   // sampleo en el punto UV del fragmento
}
)";

/* Una textura CHECKERBOARD 8x8 generada EN C++ (sin archivo): útil para ver
   el aliasing/mipmap sin assets. La textura RAW "checker" RGB UNSIGNED_BYTE. */
static std::vector<unsigned char> generarChecker(int lado) {
    std::vector<unsigned char> px((size_t)lado * lado * 3);
    for (int y = 0; y < lado; ++y) {
        for (int x = 0; x < lado; ++x) {
            bool blanco = ((x / 2) + (y / 2)) % 2 == 0;
            unsigned char v = blanco ? 255 : 0;
            size_t i = ((size_t)y * lado + (size_t)x) * 3;
            px[i] = v; px[i+1] = v; px[i+2] = v;
        }
    }
    return px;
}

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL_Init fallo: " << SDL_GetError() << "\n";
        return 1;
    }

    glu::Contexto c = glu::abrir("Modulo 20 - L10 texturas y mipmaps", 800, 600);
    if (c.contexto == nullptr) { SDL_Quit(); return 1; }

    GLuint prog = glu::enlazar(VS_FUENTE, FS_FUENTE);
    if (prog == 0) { glu::cerrar(c); SDL_Quit(); return 1; }

    // ---- textura -----------------------------------------------------------
    const int LADO = 8;
    std::vector<unsigned char> px = generarChecker(LADO);

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0);          // unit 0: sampler del FS
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, LADO, LADO, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, px.data());

    // SAMPLER (¡la mitad que se olvida y aleja un quad en negro!):
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);   // trilinear: con MIPMAPS!
    glGenerateMipmap(GL_TEXTURE_2D);            // la cadena completa 8->1

    // ---- quad con UV --------------------------------------------------------
    static const float ver[] = {
        // pos(vec2)               uv(vec2)
        -0.85f, -0.85f,   0.0f, 0.0f,
         0.85f, -0.85f,   4.0f, 0.0f,   // UV fuera de [0,1]: REPEAT x4
         0.85f,  0.85f,   4.0f, 4.0f,
        -0.85f,  0.85f,   0.0f, 4.0f,
    };
    static const unsigned int idx[] = { 0, 1, 2,  0, 2, 3 };

    GLuint vao = 0, vbo = 0, ebo = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof ver, ver, GL_STATIC_DRAW);
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof idx, idx, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          (const void*)(2 * sizeof(float)));
    glBindVertexArray(0);

    GLint uTex = glGetUniformLocation(prog, "uTex");

    bool corriendo = true;
    SDL_Event ev;
    while (corriendo) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) corriendo = false;
        }
        glClearColor(0.06f, 0.07f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(prog);
        glUniform1i(uTex, 0);              // uTex = GL_TEXTURE0 (ya enlazada)
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void*)0);
        SDL_GL_SwapWindow(c.ventana);
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
    glDeleteTextures(1, &tex);
    glDeleteProgram(prog);
    glu::cerrar(c);
    SDL_Quit();
    return 0;
}
