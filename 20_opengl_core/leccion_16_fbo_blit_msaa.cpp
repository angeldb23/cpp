/* =============================================================================
   MÓDULO 20 - LECCIÓN 16: Framebuffers (2/2) — blitFramebuffer, MSAA con
   renderbuffer multisample y la validación de samples (núcleo del ej 08)
   =============================================================================
   TEMA 10 de TEMAS.md (2 de 2): blit y multisampling (MSAA).

   CÓMO CORRERLA A MANO (desde la carpeta del módulo):
     g++ -std=c++20 -Wall -Wextra leccion_16_fbo_blit_msaa.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec16 && ./lec16

   --- EL PROBLEMA DEL ALIASING EN BORDES (el "staircase") ----------------------
   La rasterización del 21 decide por pixel: los bordes del triángulo
   escalan en "escalera". El ANTIALIASING clásico de la GL es el MSAA:
   Multi SAMPLE Anti-Aliasing: cada pixel del framebuffer guarda VARIOS
   sub-samples (4x: 4 puntos dentro del pixel); un triángulo que "corta" el
   pixel en la mitad cubre 2/4 de los samples y el color final es el promedio
   (el FS corre 1 vez por pixel, NO por sample: es barato: la cobertura es
   la que promedia).

   --- HARDWARE DEL MSAA EN CORE 3.3 -------------------------------------------
   MSAA en el DEFAULT framebuffer: SDL_GL_SetAttribute(MULTISAMPLEBUFFERS,1)
   + MULTISAMPLESAMPLES,4: lo pide EL CONTEXTO (gl_util.hpp::abrir msaa=true).
   MSAA en un FBO PROPIO: UNA preposición importante: las TEXTURAS están
   "un-sample-per-pixel"; el MSAA necesita RENDERBUFFER (memoria del GPU
   especial):
     glGenRenderbuffers(1, &rbo);
     glBindRenderbuffer(GL_RENDERBUFFER, rbo);
     glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4,
                                      GL_RGBA8, w, h);     // 4x = samples
     glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_RENDERBUFFER, rbo);
     (y otro rbo GL_DEPTH_COMPONENT24 con 4x si hay depth test).
   UN renderbuffer multisample NO SE PUEDE MUESTREAR como textura (el FS
   post-quad NO puede leerlo): se RESUELVE (downsample al 1x) con:
     glBlitFramebuffer(sx0, sy0, sx1, sy1, dx0, dy0, dx1, dy1,
                       GL_COLOR_BUFFER_BIT, GL_NEAREST);
       read fbo (multisample) -> draw fbo (1x, la textura del post)
   ¡SECUENCIA DEL FRAME CON MSAA:
     1. bind read FBO (multisample) -> dibuja.
     2. glBindFramebuffer(GL_READ_FRAMEBUFFER, fboMs);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboPost);
        glBlitFramebuffer(...) -> "resolución" del MSAA al 1x.
     3. bind default + glUseProgram(post): quad que LEE la textura 1x.
   A VERIFICAR (perfil de tu Mesa UHD 600): glGetIntegerv(GL_SAMPLES) or 4x
   (support en Mesa es sutil; glRenderbufferStorageMultisample with 4
   samples likely OK; si devuelve INCOMPLETE, bajá a 2x: N, OBJ de TODO
   el soporte esto sí: los TESTS corren en la lección, no el profiling).

   --- CANTIDAD DE SAMPLES: LA VALIDACIÓN SIN GPU (núcleo ej 08) --------------
   La petición de samples viene del game.h en la clase "lo que pido";
   el HARDWARE tiene un máximo (glGetInteger) Y acepta 1/2/4/8 pocasimport:
   la función EN C++ (sin GL) es la que esta lección muestra:
     sampleValidos(pedido, max): clamp potencias de 2 dentro del rango
         (0 pedido, max aplicado y vecinos inmediatos o un solo APARición.
   --------------------------------------------------------------------------- */

#include "include/gl_util.hpp"
#include <iostream>
#include <vector>

/* LA FUNCIÓN (la lógica EXACTA del ejercicio 08): ajusta la potencia de 2
   pedida al máximo soportado (samples válidos: 2/4/8; MSAA off = 0).      */
static int sampleValidos(int pedido, int maxHardware) {
    if (pedido <= 1) return 0;                     // MSAA apagado
    int s = 1;
    while (s * 2 <= pedido) s *= 2;                // potencia de 2 mas cercano
    if (s > maxHardware) {
        s = 1;
        while (s * 2 <= maxHardware) s *= 2;       // máximo POTENCIA-2 del GPU
    }
    return s;
}

int main() {
    /* 1) El núcleo (lógica pura, se imprime en salida: el ej 08 lo testea) */
    struct Caso { int pedido, max, esperado; };
    const Caso casos[] = {
        {0, 4, 0}, {1, 4, 0}, {2, 4, 2}, {3, 4, 2}, {4, 4, 4},
        {8, 4, 4}, {16, 4, 4}, {4, 8, 4}, {0, 0, 0},
    };
    bool todoOk = true;
    for (const Caso& c : casos) {
        int got = sampleValidos(c.pedido, c.max);
        bool ok = (got == c.esperado);
        todoOk = todoOk && ok;
        std::cout << "pedido=" << c.pedido << " max=" << c.max
                  << " -> " << got << (ok ? " ok" : " BUG") << "\n";
    }
    std::cout << (todoOk ? "sampleValidos OK en casos de prueba\n"
                         : "sampleValidos con BUGS (corregir antes de usar)\n");

    /* 2) La parte GL: FBO multisample + blit + post-quad (el flujo real). */
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL_Init fallo: " << SDL_GetError() << "\n";
        return 1;
    }
    glu::Contexto c = glu::abrir("Modulo 20 - L16 FBO blit y MSAA", 800, 600,
                                 /*msaa en default:*/ false);
    if (c.contexto == nullptr) { SDL_Quit(); return 1; }

    const int W = c.ancho, H = c.alto;
    const int SAMPLES = sampleValidos(4, 4);   // A VERIFICAR tu Mesa (ver arriba)

    // FBO ms (renderbuffer multisample) + FBO post (textura 1x) -----
    GLuint fboMs = 0, rboColor = 0, rboDepth = 0;
    glGenFramebuffers(1, &fboMs);
    glBindFramebuffer(GL_FRAMEBUFFER, fboMs);
    glGenRenderbuffers(1, &rboColor);
    glBindRenderbuffer(GL_RENDERBUFFER, rboColor);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, SAMPLES,
                                     GL_RGBA8, W, H);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                              GL_RENDERBUFFER, rboColor);
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, SAMPLES,
                                     GL_DEPTH_COMPONENT24, W, H);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, rboDepth);

    GLuint fboPost = 0, texPost = 0;
    glGenFramebuffers(1, &fboPost);
    glBindFramebuffer(GL_FRAMEBUFFER, fboPost);
    glGenTextures(1, &texPost);
    glBindTexture(GL_TEXTURE_2D, texPost);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, W, H, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, texPost, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "FBO ms/post INCOMPLETO (bajar SAMPLES a 2 o 0)\n";
        glu::cerrar(c);
        SDL_Quit();
        return 1;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // quad fullscreen simple + un triángulo de la "escena" (sin shaders de la
    // escena: en este bucle realizamos el flujo completo MSAA -> blit.
    static const float quad6[6][2] = {
        {-1.0f, -1.0f}, {1.0f, -1.0f}, {1.0f,  1.0f},
        {-1.0f, -1.0f}, {1.0f,  1.0f}, {-1.0f, 1.0f},
    };
    static const float tri[3][2] = { {-0.6f,-0.4f}, {0.6f,-0.4f}, {0.0f, 0.5f} };

    GLuint vaoQuad = 0, vboQuad = 0;
    glGenVertexArrays(1, &vaoQuad);
    glBindVertexArray(vaoQuad);
    glGenBuffers(1, &vboQuad);
    glBindBuffer(GL_ARRAY_BUFFER, vboQuad);
    glBufferData(GL_ARRAY_BUFFER, sizeof quad6, quad6, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (const void*)0);

    GLuint vaoTri = 0, vboTri = 0;
    glGenVertexArrays(1, &vaoTri);
    glBindVertexArray(vaoTri);
    glGenBuffers(1, &vboTri);
    glBindBuffer(GL_ARRAY_BUFFER, vboTri);
    glBufferData(GL_ARRAY_BUFFER, sizeof tri, tri, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (const void*)0);
    glBindVertexArray(0);

    GLuint progTri = glu::enlazar(R"(
#version 330 core
layout (location = 0) in vec2 aPos;
void main() { gl_Position = vec4(aPos, 0.0, 1.0); }
)", R"(
#version 330 core
out vec4 FragColor;
void main() { FragColor = vec4(1.0, 0.6, 0.2, 1.0); }
)");
    if (progTri == 0) { glu::cerrar(c); SDL_Quit(); return 1; }

    bool corriendo = true;
    SDL_Event ev;
    while (corriendo) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) corriendo = false;
        }
        /* 1) escena al FBO multisample. */
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboMs);
        glViewport(0, 0, W, H);
        glClearColor(0.10f, 0.10f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(progTri);
        glBindVertexArray(vaoTri);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        /* 2) RESOLVER: blit del multisample al 1x (la "instantánea"). */
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fboMs);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboPost);
        glBlitFramebuffer(0, 0, W, H, 0, 0, W, H,
                          GL_COLOR_BUFFER_BIT, GL_NEAREST);

        /* 3) default + post-quad que LEE la textura resuelta. */
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, W, H);
        glClearColor(0.06f, 0.07f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(0);       // (el curso no tiene post shader acá: clear)
        glBindVertexArray(vaoQuad);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        SDL_GL_SwapWindow(c.ventana);
    }

    glDeleteVertexArrays(1, &vaoQuad);
    glDeleteBuffers(1, &vboQuad);
    glDeleteVertexArrays(1, &vaoTri);
    glDeleteBuffers(1, &vboTri);
    glDeleteRenderbuffers(1, &rboDepth);
    glDeleteRenderbuffers(1, &rboColor);
    glDeleteFramebuffers(1, &fboMs);
    glDeleteTextures(1, &texPost);
    glDeleteFramebuffers(1, &fboPost);
    glDeleteProgram(progTri);
    glu::cerrar(c);
    SDL_Quit();
    return 0;
}
