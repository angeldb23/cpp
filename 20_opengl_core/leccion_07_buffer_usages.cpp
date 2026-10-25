/* =============================================================================
   MÓDULO 20 - LECCIÓN 07: Buffers (2/2) — glBufferData vs glBufferSubData y
   los hints STATIC/DYNAMIC/STREAM_DRAW (decisión por patrón de uso)
   =============================================================================
   TEMA 4 de TEMAS.md (2 de 2).

   CÓMO CORRERLA A MANO (desde la carpeta del módulo):
     g++ -std=c++20 -Wall -Wextra leccion_07_buffer_usages.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec07 && ./lec07

   --- EL HINT ------------------------------------------------------------------
   glBufferData(target, size, data, HINT): el HINT le dice al driver CÓMO
   pensás usar el buffer, para que él decida DÓNDE guardarlo (VRAM, buffers
   de subida, RAM). Los tres del core (verificados en glcorearb.h):
     GL_STATIC_DRAW  : lo subo UNA vez y lo dibujo MUCHAS (malla del nivel).
     GL_DYNAMIC_DRAW : lo modifico con frecuencia, en PARTES (partículas,
                       quads de UI).
     GL_STREAM_DRAW  : lo reescribo COMPLETO casi todos los frames (batch de
                       instancing de la L14, texto del HUD).
   El sufijo _DRAW importa: existen variantes _READ/_COPY para buffers de
   escritura del GPU (no los usamos en el curso).
   Es UN ENCUESTRO, no una ley: es driver-público (Intel/Mesa con UHD 600 usa
   la hint para decidir entre VRAM y RAM del sistema; NVIDIA respeta más la
   frecuencia que la base pequeña). La regla del ejercicio 03 (cadencia de
   updates) cubre los 3 casos prácticos del curso.

   --- glBufferData vs glBufferSubData ----------------------------------------
   (RE)ASIGNA el espacio completo. Puede mover el buffer a otra
     caché e invalida el contenido previo. Úsalo cuando cambia el TAMAÑO o
     cuando reescribís TODO (también re-crea el espacio con data=nullptr,
     el patrón "orphaning").
   glBufferSubData(target, offset, size, data): ACTUALIZA la ventana
     [offset, offset+size) del buffer ASIGNADO, sin tocar la memoria.
     Regla dura (GL_INVALID_VALUE si no): offset + size <= tamaño asignado.
   RACE CLÁSICA: si el GPU aún está leyendo la región que vos reescribís,
   el frame sale con datos mezclados. Patrones de esquivarlo: (a) doblar el
   buffer en C++ (ping-pong de vector y re-subida completa); (b) orfanar el
   buffer viejo con glBufferData(nullptr) y crear uno nuevo en la misma
   gl (conocido con nombre comercial "orphaning").
   MENCION 4.4 (no core 3.3): glBufferStorage + glMapBufferRange con
   GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT: memoria persistente del
   lado C++ para no re-subir nunca más. glMapBufferRange (sí core, 3.0)
   sirve sin persistencia para actualizar con INVALIDATE. El par
   out/in vBrillo del programa es la mecánica 1:1 de la lección 03.       */

#include "include/gl_util.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>

/* ---------------------------------------------------------------------------
   NÚCLEO TESTEABLE (es la materia del ejercicio 03): la política de uso,
   como funciones puras con tabla implícita. Ejemplos:
     malla del nivel        (1 subida, mil draws)           -> STATIC + data
     partículas por subpartes (updates parciales)           -> DYNAMIC + subData
     quads del HUD (reescribir TODO cada frame)             -> STREAM  + data
 ------------------------------------------------------------------------- */
struct Patron {
    int  usosDeDibujo = 0;          // draws como "vida" del buffer
    int  reescriturasCompletas = 0; // cuántas veces se reescribe entero
    bool subidasParciales = false;  // si se toca por sub-rangos
};

static const char* usagePara(const Patron& p) {
    if (p.reescriturasCompletas >= p.usosDeDibujo) return "GL_STREAM_DRAW";
    if (p.subidasParciales)                        return "GL_DYNAMIC_DRAW";
    return "GL_STATIC_DRAW";
}

static const char* operacionPara(const Patron& p) {
    return p.subidasParciales ? "glBufferSubData" : "glBufferData";
}


int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL_Init fallo: " << SDL_GetError() << "\n";
        return 1;
    }

    glu::Contexto c = glu::abrir("Modulo 20 - L07 buffer usages", 800, 600);
    if (c.contexto == nullptr) { SDL_Quit(); return 1; }

    Patron malla {}
        ; malla.usosDeDibujo = 1000; malla.reescriturasCompletas = 0;
    Patron particulas{ 1000, 0, true };
    Patron hud      { 1000, 1000, false };

    std::cout << "malla:      " << usagePara(malla) << " + " << operacionPara(malla)
              << "     (esperado: STATIC + bufferData)\n";
    std::cout << "particulas: " << usagePara(particulas) << " + "
              << operacionPara(particulas) << " (esperado: DYNAMIC+ bufferSubData)\n";
    std::cout << "hud:        " << usagePara(hud) << " + " << operacionPara(hud)
              << "     (esperado: STREAM + bufferData)\n";

    static const char* VS_FUENTE = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in float aBrillo;
out float vBrillo;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    vBrillo = aBrillo;
}
)";

    static const char* FS_FUENTE = R"(
#version 330 core
in float vBrillo;                      // si no matchea el out del VS -> link error
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 1.0, 1.0, vBrillo);
}
)";

    GLuint prog = glu::enlazar(VS_FUENTE, FS_FUENTE);
    if (prog == 0) { glu::cerrar(c); SDL_Quit(); return 1; }

    /* DEMO STREAMING: 4000 partículas x (pos vec2 + brillo float) = 12
       bytes cada una -> 48.000 bytes reescritos COMPLETOS por frame,
       estilo STREAM_DRAW + glBufferSubData por la totalidad.          */
    const int CANT = 4000;
    std::vector<float> datos((size_t)CANT * 3);

    GLuint vao = 0, vbo = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, datos.size() * sizeof(float), nullptr,
                 GL_STREAM_DRAW);        // primera asignación (data=nullptr)
    glEnableVertexAttribArray(0);        // aPos
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void*)0);
    glEnableVertexAttribArray(1);        // aBrillo: byte 8 con stride 12
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (const void*)(2 * sizeof(float)));
    glBindVertexArray(0);

    bool corriendo = true;
    SDL_Event ev;
    int frame = 0;
    while (corriendo) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) corriendo = false;
        }
        // 1) Simular física (matemática del módulo 19, sin estado por ε).
        for (int i = 0; i < CANT; ++i) {
            float* p = &datos[(size_t)i * 3];
            p[0] = 0.8f * std::cos(0.07f * (float)(i + frame));
            p[1] = 0.8f * std::sin(0.05f * (float)(i - frame));
            p[2] = 0.5f + 0.5f * std::sin(0.03f * (float)i);
        }
        // 2) Subida del frame COMPLETA por subData (el patrón streaming del
        //    apartado de arriba: no orphaning porque reescribimos TODO).
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * datos.size(), datos.data());

        glClearColor(0.06f, 0.07f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(prog);
        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, CANT);
        SDL_GL_SwapWindow(c.ventana);
        ++frame;
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(prog);
    glu::cerrar(c);
    SDL_Quit();
    return 0;
}
