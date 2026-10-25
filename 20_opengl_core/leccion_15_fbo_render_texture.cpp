/* =============================================================================
   MÓDULO 20 - LECCIÓN 15: Framebuffers (1/2) — FBO, attachments y render to
   texture
   =============================================================================
   TEMA 10 de TEMAS.md (1 de 2): Framebuffers: FBO, color/depth attachments,
   render to texture. (Blit y MSAA van en la lección 16.)

   CÓMO CORRERLA A MANO (desde la carpeta del módulo):
     g++ -std=c++20 -Wall -Wextra leccion_15_fbo_render_texture.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec15 && ./lec15

   --- QUÉ ES UN FBO -----------------------------------------------------------
   Un FBO (Framebuffer Object) es una "pantalla" propia del GPU: un conjunto
   de anexos (attachments) donde puede caer la salida de un draw:
     - COLOR attachments: una o más texturas donde aterriza el vec4 del FS.
     - DEPTH (y STENCHI no depurado: el z-buffer), necesario si hay depth test.
   El más conocido: el DEFAULT framebuffer (el de la ventana) ES un FBO;
   acá creamos un FBO "de diseño" con UNA textura como color.

   PASOS (todas core 3.3; verificadas en glcorearb.h local):
     glGenFramebuffers(1, &fbo)             <- el FBO (el marco)
     glBindFramebuffer(GL_FRAMEBUFFER, fbo) <- lo ACTIVA: los draws CAEN aquí
     glGenTextures -> glBindTexture(GL_TEXTURE_2D, tex)
     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA,
                  GL_UNSIGNED_BYTE, nullptr)        <- color attachment sin dato inicial
     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            GL_TEXTURE_2D, tex, 0)  <- colgá la textura
     (depth: glRenderbufferStorage + glFramebufferRenderbuffer con
      GL_DEPTH_COMPONENT: hacer DEPTH ATTACHMENT: lección 16 con MSAA)
     glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE
     (si NO es completa, todos los draws fallan SIN avisar)
     glBindFramebuffer(GL_FRAMEBUFFER, 0)   <- volver al default (la ventana)

   FLUJO DE UN POST-PROCESO SIMPLE (la L16 lo expande con blit/MSAA):
     1. bind FBO: dibuja normal: TODO el color cae al attachment.
     2. bind default (0): dibuja UN quad fullscreen usando la textura como
        input: AHÍ se el efecto (vignette, blur, tone map del 21...).
   El IMPORTANTISIMO: el resultado REST: los sprites del cuadro se dibujaron
   UNA VEZ (a un buffer estático); el "screen-space pass" SOLO lee: la
   ley del SSR ("reproducible": el mismo imagen -> siempre el mismo post).

   UNA COMPLETA A VERIFICAR antes de creentarla en tu GPU:
     - el FBO tiene SU resolución propia (no la de la ventana): al dibujar
       dentro hay que re-programar glViewport con el tamaño del FBO (y
       al volver, restaurar el de la ventana: acá en el bucle es parte del
       paso 1/2: la escena interna a su resolución y el quad final en
       pantalla usa la ventana entera).

   --- vIGNETTE minimalista (el efecto del "cierre" del módulo) ---------------
   vignette: oscurecer los BORDOS del post-quad: distancia del UV (0.5,0.5):
     vec2 d = uv - 0.5;
     float osc = smoothstep(0.4, 0.85, length(d));   // 0 centro, 1 bordes
     FragColor = vec4(tex.rgb * (1.0 - 0.55 * osc), 1.0);
   --------------------------------------------------------------------------- */

#include "include/gl_util.hpp"
#include <iostream>
#include <cmath>

/* Dos programas: la ESCENA (lo que se renderiza al FBO) y el POST-PROCESO
   (quad fullscreen que LEE esa textura y aplica el vignette).             */
static const char* VS_SIGUE = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
uniform float uTiempo;
void main() {
    float a = uTiempo * 0.5;
    // el triángulo gira en torno al centro (módulo 19: matriz sino hacer chai)
    vec2 g = vec2(cos(a), sin(a));
    vec2 r = vec2(-g.y, g.x);
    vec2 p = aPos * (0.5f) * g + aPos * (0.5f) * r * 0.9f + vec2(0.0f, 0.1f);
    gl_Position = vec4(p, 0.0, 1.0);
}
)";

static const char* FS_SIGUE = R"(
#version 330 core
out vec4 FragColor;
uniform vec3 uColor;
void main() { FragColor = vec4(uColor, 1.0); }
)";

static const char* VS_POST = R"(
#version 330 core
layout (location = 0) in vec2 aPos;    // quad fullscreen [-1..1]
out vec2 vUV;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    vUV = aPos * 0.5 + 0.5;            // NDC -> UV
}
)";

static const char* FS_POST = R"(
#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D uEscena;
void main() {
    vec4 tex = texture(uEscena, vUV);
    vec2 d = vUV - vec2(0.5);
    float osc = smoothstep(0.40, 0.85, length(d));
    FragColor = vec4(tex.rgb * (1.0 - 0.55 * osc), 1.0);
}
)";

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL_Init fallo: " << SDL_GetError() << "\n";
        return 1;
    }

    glu::Contexto c = glu::abrir("Modulo 20 - L15 FBO render to texture", 800, 600);
    if (c.contexto == nullptr) { SDL_Quit(); return 1; }

    GLuint progScene = glu::enlazar(VS_SIGUE, FS_SIGUE);
    GLuint progPost  = glu::enlazar(VS_POST,  FS_POST);
    if (progScene == 0 || progPost == 0) { glu::cerrar(c); SDL_Quit(); return 1; }

    const int FBO_W = c.ancho, FBO_H = c.alto;

    // --- crear el FBO + la textura color -------------------------------------
    GLuint fbo = 0, texColor = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glGenTextures(1, &texColor);
    glBindTexture(GL_TEXTURE_2D, texColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, FBO_W, FBO_H, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    // sampler: LINEAR, sin mipmaps (el post-quad es 1:1 de tamaño)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, texColor, 0);
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "FBO incompleto (0x" << std::hex << status << ")\n";
        glu::cerrar(c);
        SDL_Quit();
        return 1;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);   // volver al default (ventana)

    // quad fullscreen para el pass del FBO.
    static const float quad6[6][2] = {
        {-1.0f, -1.0f}, {1.0f, -1.0f}, {1.0f,  1.0f},
        {-1.0f, -1.0f}, {1.0f,  1.0f}, {-1.0f, 1.0f},
    };
    GLuint vao = 0, vbo = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof quad6, quad6, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (const void*)0);
    glBindVertexArray(0);

    GLint uTiempo = glGetUniformLocation(progScene, "uTiempo");
    GLint uColor  = glGetUniformLocation(progScene, "uColor");
    GLint uEscena = glGetUniformLocation(progPost, "uEscena");

    float t = 0.0f;
    bool corriendo = true;
    SDL_Event ev;
    Uint64 anterior = SDL_GetPerformanceCounter();

    while (corriendo) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) corriendo = false;
        }
        Uint64 ahora = SDL_GetPerformanceCounter();
        t += (float)((double)(ahora - anterior) / SDL_GetPerformanceFrequency());
        anterior = ahora;

        /* PASO 1: dibujar la escena DENTRO del FBO (caen al attachment). */
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, FBO_W, FBO_H);
        glClearColor(0.10f, 0.10f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(progScene);
        glUniform1f(uTiempo, t);
        glUniform3f(uColor, 1.0f, 0.6f, 0.2f);
        glBindVertexArray(vao);
        // 1 triángulo fijo para ver el pass (simple): NO es el chequering
        glDrawArrays(GL_TRIANGLES, 0, 3);   // usa quad[0..2] como triángulo

        /* PASO 2: volver AL DEFAULT y dibujar el quad with post-processing. */
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, c.ancho, c.alto);
        glClearColor(0.06f, 0.07f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(progPost);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texColor);   // la "escena" del paso 1
        glUniform1i(uEscena, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        SDL_GL_SwapWindow(c.ventana);
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteTextures(1, &texColor);
    glDeleteFramebuffers(1, &fbo);
    glDeleteProgram(progScene);
    glDeleteProgram(progPost);
    glu::cerrar(c);
    SDL_Quit();
    return 0;
}
