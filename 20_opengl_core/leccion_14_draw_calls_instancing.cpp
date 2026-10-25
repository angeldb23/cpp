/* =============================================================================
   MÓDULO 20 - LECCIÓN 14: Draw calls — glDrawArrays, glDrawElements,
   instancing (glDrawArraysInstanced) y la mención del multi-draw indirect
   =============================================================================
   TEMA 9 de TEMAS.md: Draw calls: glDrawArrays, glDrawElements, instancing
   (glDrawArraysInstanced); multi-draw indirect (OpenGL 4.3: solo mención).

   CÓMO CORRERLA A MANO (desde la carpeta del módulo):
     g++ -std=c++20 -Wall -Wextra leccion_14_draw_calls_instancing.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec14 && ./lec14

   --- LOS TRES STILLOS DE "DIBUJA" ------------------------------------------
   glDrawArrays(mode, first, count): dibuja `count` vértices SEGUIDOS del
     VBO (empezando del vertice `first`). Es el simple: la data es un
     stream. Con 2D y sus quads se puede armar: tus datos pueden es un
     array de vértices planitocorridos (el batch del raylib de la L02.
   glDrawElements(mode, count, type, offset): dibuja con ÍNDICES del EBO
     (lección 06): compartir vértices o reusarlos con patrón: el ESTÁNDAR
     de mallas 3D (L18: cargador OBJ: todo EBO). NOTA: `type` debe
     matchear el tipo de índices: UNDERST_UINT para los UDens de C++).
   glDrawArraysInstanced(mode, first, count, instances): UN draw corriendo
     el vertex shader N VECES POR VÉRTICE (básicamente N "copias"): la
     única diferencia por instancia es el contenido del atributo
     "instanced" (glVertexAttribDivisor(loc, 1)):

   --- INSTANCING: POR QUÉ ES EL PATRÓN DEL 2D MODERNO --------------------------
   Un draw call tiene un OVERHEAD FIJO (validar estado, programar la
   pipeline): 1000 draws de 6 vértices SON MUCHOS MÁS lentos que 1 draw
   de 6000 (verificado siempre en el profiling del 15). El instancing hace
   y directamente: TODAS las copias en un solo draw:
     - VBO de GEOMETRÍA (vec4 pos/texUV por quad base, divisor 0): 6 vértices.
     - VBO de INSTANCIA (vec2 offset, vec3 color, divisor 1): 1 entrada por
       sprite: (offset, tint).
     - glDrawArraysInstanced(GL_TRIANGLES, 0, 6, N): N = sprites del frame.
   En el vertex shader:
     layout (location = 0) in vec2 aPos;     // geometría base (el quad)
     layout (location = 1) in vec2 iOffset;  // POR INSTANCIA (divisor 1)
     ... gl_Position = vec4(aPos + iOffset, 0.0, 1.0);
   Con divisor=1, ese atributo avanza UNA VEZ por instance, no por vertex:
   los 6 vértices del quad ven el MISMO iOffset.

   NOTA HISTÓRICA: el sprite batcher de raylib (16/17) dibuja UN quad POR
   draw dentro de un batch único; el instancing es la evolución natural:
   UN draw POR TEXTURA, no por sprite. El ejercicio 07 arma ese batcher
   (lógica pura). El cierre del módulo usa este batcher + el FBO L15.

   MENCIÓN 4.3: glMultiDrawArraysIndirect / glDrawElementsIndirect (forma
   arrays de draws desde UN buffer para hacer "todos los batches del
   mundo en 1 glCall"). A VERIFICAR: NO CORE 3.3, mentionado like TEMAS.md.
   --------------------------------------------------------------------------- */

#include "include/gl_util.hpp"
#include <iostream>
#include <vector>

/* --- SHADERS DEL INSTANCING (el corazoncito de la lección) -----------*/
static const char* VS_FUENTE = R"(
#version 330 core
layout (location = 0) in vec2 aPos;      // GEOMETRÍA del quad (divisor 0)
layout (location = 1) in vec2 iOffset;   // offset POR INSTANCIA (divisor 1)
layout (location = 2) in vec3 iColor;    // color POR INSTANCIA (divisor 1)
out vec3 vColor;
void main() {
    // escala del quad base: 8% de pantalla (en NDC), offset por instancia:
    gl_Position = vec4(aPos * 0.08f + iOffset, 0.0, 1.0);
    vColor = iColor;
}
)";

static const char* FS_FUENTE = R"(
#version 330 core
in vec3 vColor;
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

    glu::Contexto c = glu::abrir("Modulo 20 - L14 draw calls e instancing", 800, 600);
    if (c.contexto == nullptr) { SDL_Quit(); return 1; }

    GLuint prog = glu::enlazar(VS_FUENTE, FS_FUENTE);
    if (prog == 0) { glu::cerrar(c); SDL_Quit(); return 1; }

    // GEOMETRÍA BASE: el quad de 6 vértices (la unidad del sprite, L11)
    static const float quad6[6][2] = {
        {-0.5f, -0.5f}, {0.5f, -0.5f}, {0.5f,  0.5f},
        {-0.5f, -0.5f}, {0.5f,  0.5f}, {-0.5f, 0.5f},
    };

    // DATOS POR INSTANCIA: 40 sprites (offset.xy, color.rgb)
    const int N = 40;
    std::vector<float> instancias((size_t)N * 5);
    for (int i = 0; i < N; ++i) {
        float fx = (float)(i % 10), fy = (float)(i / 10);
        float* p = &instancias[(size_t)i * 5];
        p[0] = -0.9f + fx * 0.2f;                 // offset x
        p[1] = -0.7f + fy * 0.18f;                // offset y
        p[2] = 0.1f + (i % 7) / 7.0f;             // color r
        p[3] = 0.4f + (i % 5) / 5.0f;             // g
        p[4] = 0.2f + ((i * 3) % 9) / 9.0f;       // b
    }

    GLuint vao = 0, vboQuad = 0, vboInst = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // buffer de GEOMETRÍA: divisor 0 (avanza por vértice)
    glGenBuffers(1, &vboQuad);
    glBindBuffer(GL_ARRAY_BUFFER, vboQuad);
    glBufferData(GL_ARRAY_BUFFER, sizeof quad6, quad6, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (const void*)0);

    // buffer de INSTANCIAS: divisor 1 (avanza por instancia)
    glGenBuffers(1, &vboInst);
    glBindBuffer(GL_ARRAY_BUFFER, vboInst);
    glBufferData(GL_ARRAY_BUFFER, instancias.size() * sizeof(float),
                 instancias.data(), GL_DYNAMIC_DRAW);   // cambia per frame (L07)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (const void*)0);
    glVertexAttribDivisor(1, 1);        // <-- LA CLAVE del instancing
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                          (const void*)(2 * sizeof(float)));
    glVertexAttribDivisor(2, 1);
    glBindVertexArray(0);

    // En scales: comparar N draws de 6 vértices vs UN draw instanced es
    // exacto: el profiler del 15 (o el núcleo del ej 07) lo calcula.

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
        // UN draw call para los 40 sprites:
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, N);
        SDL_GL_SwapWindow(c.ventana);
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vboInst);
    glDeleteBuffers(1, &vboQuad);
    glDeleteProgram(prog);
    glu::cerrar(c);
    SDL_Quit();
    return 0;
}
