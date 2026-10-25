/* =============================================================================
   MÓDULO 20 - LECCIÓN 12: Estado global (1/2) — blend, depth test, cull face,
   scissor y viewport
   =============================================================================
   TEMA 8 de TEMAS.md (1 de 2): Estado global: blend, depth test, cull face,
   scissor, viewport. (El cachear estado va en la lección 13.)

   CÓMO CORRERLA A MANO (desde la carpeta del módulo):
     g++ -std=c++20 -Wall -Wextra leccion_12_estado_global.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec12 && ./lec12

   --- GL ES UNA MÁQUINA DE ESTADO GLOBAL (el diseño del API 1992) ------------
   TODAS las funciones gl* operan sobre "el contexto": flags y enlaces
   vigentes. glEnable(GL_BLEND) no altera "un sprite": altera TODO lo que
   se dibuje DESPUÉS. El legado glPushAttrib/glPopAttrib (guardar el
   estado en pila) fue REMOVIDO del core en 3.0: en core 3.3 guardás y
   restaurás el estado vos (lección 13: cache).

   LOS ESTADOS QUE USAREMOS SIEMPRE (core 3.3; verificados en glcorearb.h):

   BLEND (transparencia) : glEnable(GL_BLEND) +
     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
       Como se mezclan los colores nuevo vs el ya-dibujado:
         out = src.a * src.rgb + (1 - src.a) * dst.rgb
       Es LA fórmula de "alpha blend" de 2D (la misma por defecto de
       SDL/raylib del módulo 16/17). Variantes: premultiplied alpha usa
       (GL_ONE, GL_ONE_MINUS_SRC_ALPHA) — casi siempre con texturas
       pre-multiplicadas; lo alga en 21 con el postproceso.
   DEPTH TEST (3D): glEnable(GL_DEPTH_TEST) + glDepthFunc(GL_LESS default)
       Atención: con UN framebuffer con depth buffer (la L15 lo crea).
       El default framebuffer (la ventana) SÍ tiene depth (SDL_DOUBLEBUFFER
       + GL_DEPTH_SIZE pedida con SDL_GL_SetAttribute... core default 24).
   CULL FACE (no dibujar caras que miran al otro lado):
       glEnable(GL_CULL_FACE); glCullFace(GL_BACK); glFrontFace(GL_CCW)
       default: CCW = "cuando el vértice 1-2-3 se ve en sentido antihorario
       (la convención del 19), es la cara FRONT; el BACK se descarta
   SCISSOR: glEnable(GL_SCISSOR_TEST) + glScissor(x,y,w,h) (píxeles):
       barrier del "mini-receptáculo" (mini-pantalla): el clear y el draw
       solo afectan esa zona. base DEL LIB de la cámara de la lección 16.
   VIEWPORT: glViewport(x, y, w, h): dónde cae el clipspace en la ventana
       (el [-1,1] se estira/encoge a ese rect). NO recorta: transforma
       coords; activar el recorte de zona es el scissor.

   --- ORDEN TÍPICO POR FRAME --------------------------------------------------
   glClear(color|depth) -> estados (blend, depth, cull off/2D...) ->
   viewport/scissor (¡si la cámara lo hizo!) -> draws. Y cuidado con el
   vsync del backend de la ventana (SDL internal, la lección 01).
   --------------------------------------------------------------------------- */

#include "include/gl_util.hpp"
#include <iostream>

/* 2 quads: uno opaco (blend OFF) y otro 50% (blend ON). Depth está
   APAGADO a propósito (2D: el draw order lo manda el orden de llamadas). */

static const char* VS_FUENTE = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
out vec2 vUV;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    vUV = aPos + 0.5;
}
)";

static const char* FS_FUENTE = R"(
#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform vec3 uColor;
uniform float uAlpha;      // si el blend está ON, este alfa mezcla
void main() {
    FragColor = vec4(uColor * (1.0 - vUV.x*0.2), uAlpha);
}
)";

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL_Init fallo: " << SDL_GetError() << "\n";
        return 1;
    }

    glu::Contexto c = glu::abrir("Modulo 20 - L12 estado global", 800, 600);
    if (c.contexto == nullptr) { SDL_Quit(); return 1; }

    GLuint prog = glu::enlazar(VS_FUENTE, FS_FUENTE);
    if (prog == 0) { glu::cerrar(c); SDL_Quit(); return 1; }

    GLint uColor = glGetUniformLocation(prog, "uColor");
    GLint uAlpha = glGetUniformLocation(prog, "uAlpha");

    static const float quad[] = {
        -0.5f, -0.5f,   0.5f, -0.5f,   0.5f, 0.5f,
        -0.5f, -0.5f,   0.5f,  0.5f,  -0.5f, 0.5f,
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

    bool corriendo = true;
    SDL_Event ev;
    while (corriendo) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) corriendo = false;
        }
        glClearColor(0.06f, 0.07f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(prog);
        glBindVertexArray(vao);

        // 1) cuadrado opaco: blend APAGADO explícito (por si un frame anterior
        //    lo encendió: el estado NO se resetea solo):
        glDisable(GL_BLEND);
        glUniform3f(uColor, 1.0f, 0.6f, 0.2f);
        glUniform1f(uAlpha, 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 2) cuadrado 50%: blend ENCENDIDO. Vale para TODO lo que se dibuje
        //    después (lección 13: cachea en grupos para no llamar 2 veces).
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUniform3f(uColor, 0.2f, 0.8f, 1.0f);
        glUniform1f(uAlpha, 0.5f);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        SDL_GL_SwapWindow(c.ventana);
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(prog);
    glu::cerrar(c);
    SDL_Quit();
    return 0;
}
