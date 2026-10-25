/* =============================================================================
   MÓDULO 20 - LECCIÓN 03: El pipeline programable (2/2) — varyings, uniforms
   y atributos dinámicos
   =============================================================================
   TEMA 2 de TEMAS.md (2 de 2): Pipeline programable.

   CÓMO CORRERLA A MANO (desde la carpeta del módulo):
     g++ -std=c++20 -Wall -Wextra leccion_03_varyings_uniforms.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec03 && ./lec03

   --- LOS TRES CANALES DE DATOS DEL PIPELINE ---------------------------------
   1. ATRIBUTOS (in en el vertex shader): datos POR VÉRTICE; provienen del
      VBO (lección 02). Un mismo VBO puede tener MUCHOS atributos
      entrelazados (interleaved): pos/color/uv mezclados vértice a vértice
      (lección 08).

   2. VARYINGS (out en VS + in en FS, con EL MISMO NOMBRE): datos que el GPU
      INTERPOLA entre los vértices del triángulo hacia cada fragmento.
      Ejemplo clásico: color por vértice -> gradiente suave gratis. La
      interpolación es en hardware (correcta en perspectiva cuando hay
      proyección con w): nadie la calcula en software.

   3. UNIFORMS: valores CONSTANTES del draw call; los setea C++ con
      glUniform*. "uniform" = un solo valor para todo el draw (contrario a
      lo por-vértice de los atributos). Se resuelven con
      glGetUniformLocation (que devuelve -1 si el nombre no existe o el
      compilador eliminó el uniform por no usarse: usar ese -1 en glUniform
      es un no-op silencioso, ni crashea ni avisa) y CONVIENE cachear el
      GLint al linkar: la consulta siempre va al driver y el costo por
      draw es chico pero se suma.                                              */

#include "include/gl_util.hpp"
#include <iostream>
#include <cmath>

/* Shaders con: 2 atributos (pos vec2, color vec3) + 2 uniforms
   (uOffset vec2, uIntensidad float). El gradiente sale del VARYING vColor. */
static const char* VS_FUENTE = R"(
#version 330 core
layout (location = 0) in vec2 aPos;      // [x,y] en NDC
layout (location = 1) in vec3 aColor;    // color POR VERTICE

uniform vec2  uOffset;                   // constante del draw entero
uniform float uIntensidad;               // brillo global

out vec3 vColor;                         // varying: out en VS
void main() {
    gl_Position = vec4(aPos + uOffset, 0.0, 1.0);
    vColor      = aColor * uIntensidad;  // el GPU lo interpola
}
)";

static const char* FS_FUENTE = R"(
#version 330 core
in  vec3 vColor;                         // in en FS, mismo nombre, mismo tipo
out vec4 FragColor;
void main() {
    FragColor = vec4(vColor, 1.0);
}
)";

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL_Init fallo: " << SDL_GetError() << "\n";
        return 1;
    }

    glu::Contexto c = glu::abrir("Modulo 20 - L03 varyings y uniforms", 800, 600);
    if (c.contexto == nullptr) { SDL_Quit(); return 1; }

    GLuint prog = glu::enlazar(VS_FUENTE, FS_FUENTE);
    if (prog == 0) { glu::cerrar(c); SDL_Quit(); return 1; }

    // Uniforms resueltos UNA VEZ, cacheados en variables C++.
    GLint uOffset     = glGetUniformLocation(prog, "uOffset");
    GLint uIntensidad = glGetUniformLocation(prog, "uIntensidad");
    if (uOffset < 0)
        std::cout << "uOffset no vive (el log lo tiro: -1 -> glUniform es no-op)\n";

    /* DOS TRIÁNGULOS en UN VBO entrelazado: cada vértice es pos(vec2) +
       color(vec3) = 5 floats. STRIDE = 5*4 = 20 bytes; el color empieza
       en el byte 8 (después de los 2 floats de pos).                   */
    static const float ver[] = {
        // triángulo izquierdo (gradiente rojo->azul->amarillo)
        -0.9f, -0.5f,   1.0f, 0.0f, 0.0f,   // V0: pos(2) + color(3)
        -0.1f, -0.5f,   0.0f, 0.0f, 1.0f,   // V1
        -0.5f,  0.5f,   1.0f, 1.0f, 0.0f,   // V2
        // triángulo derecho (gris/verde)
         0.1f, -0.5f,   0.0f, 1.0f, 0.0f,
         0.9f, -0.5f,   0.2f, 0.2f, 0.2f,
         0.5f,  0.5f,   0.0f, 0.9f, 0.0f,
    };

    GLuint vao = 0, vbo = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ver), ver, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);   // aPos: 2 floats, desde el byte 0
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (const void*)0);
    glEnableVertexAttribArray(1);   // aColor: 3 floats, desde el byte 8
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (const void*)(2 * sizeof(float)));

    /* Bucle: dos draws del MISMO programa, cambiando UNIFORMS entre ambos.
       El canal de datos que cambia por/frame es C++ -> uniforms; el máquina
       pipeline queda constante por el programa y el VAO.               */
    float t = 0.0f;
    bool corriendo = true;
    SDL_Event ev;
    Uint64 anterior = SDL_GetPerformanceCounter();

    while (corriendo) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) corriendo = false;
        }

        Uint64 ahora = SDL_GetPerformanceCounter();
        float dt = (float)((double)(ahora - anterior) / SDL_GetPerformanceFrequency());
        anterior = ahora;
        t += dt;

        glClearColor(0.06f, 0.07f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(prog);
        glBindVertexArray(vao);

        float baila = 0.3f * std::sin(t);
        glUniform2f(uOffset, baila, 0.0f);                 // uniform del draw 1
        glUniform1f(uIntensidad, 0.7f + 0.3f * std::sin(t));
        glDrawArrays(GL_TRIANGLES, 0, 3);                  // V0..V2

        glUniform2f(uOffset, -baila, 0.0f);                // uniform contrario
        glUniform1f(uIntensidad, 0.4f);
        glDrawArrays(GL_TRIANGLES, 3, 3);                  // V3..V5

        SDL_GL_SwapWindow(c.ventana);
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(prog);
    glu::cerrar(c);
    SDL_Quit();
    return 0;
}
