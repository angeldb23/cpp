/* =============================================================================
   MÓDULO 20 - LECCIÓN 06: Buffers (1/2) — VBO, EBO y qué guarda un VAO
   =============================================================================
   TEMA 4 de TEMAS.md (1 de 2): Buffers: VBO, EBO, VAO, glBufferData vs
   glBufferSubData y los hints STATIC/DYNAMIC/STREAM. (La elección de hint y
   la actualización parcial van en la lección 07.)

   CÓMO CORRERLA A MANO (desde la carpeta del módulo):
     g++ -std=c++20 -Wall -Wextra leccion_06_vbo_ebo_vao.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec06 && ./lec06

   --- LOS TRES OBJETOS DE ESTA LECCIÓN ---------------------------------------
   VBO (Vertex Buffer Object): memoria de la GPU con los datos por vértice
     en bruto (bytes a los que no les importa su semántica). El que da
     sentido a esos bytes es el glVertexAttribPointer del VAO.

   EBO (Element/Index Buffer): lista de ÍNDICES (tipo uint) al VBO. Sirve
     para compartir vértices: un cuadrado son 4 vértices + 6 índices (2
     vértices repetidos) en vez de 6 vértices. Con mallas reales (L18) el
     ahorro es brutal: un cubo pasa de 72 vértices a 24 posiciones (y en
     personajes, el reuso es muchísimo mayor).

   VAO (Vertex Array Object): NO es memoria: es UNA CONFIGURACIÓN GRABADA —
     qué location interpreta qué buffer, con qué tamaño/tipo/stride/offset
     y si está habilitado. Todo lo que hacés mientras un VAO está enlazado
     queda grabado en él:
       - glEnableVertexAttribArray(n)            (flag por location)
       - glVertexAttribPointer(n, ...)           (formato + buffer activo)
     Trampa clásica: el buffer que "queda grabado" para cada location es el
     que estaba ENLAZADO en GL_ARRAY_BUFFER al momento de llamar
     glVertexAttribPointer. Excepción: el EBO (GL_ELEMENT_ARRAY_BUFFER) sí
     que queda grabado en el VAO solo por enlazarlo.
   A VERIFICAR mención: glCreateBuffers/glCreateVertexArrays son 4.5 (DSA):
   se crean directo en memoria sin enlazar primero. En 3.3 core NO existen.

   --- EL FLUJO TÍPICO (setup una vez; render solo enlaza y dibuja) ------------
     setup:   bind VAO -> bind VBO -> glBufferData -> bind EBO -> bufferData
              -> glEnableVertexAttribArray + glVertexAttribPointer x cada uno
     render:  bind VAO -> glDrawElements(GL_TRIANGLES, cant, GL_UNSIGNED_INT, 0)
                                       (el tipo SIEMPRE matchea `quint16` del
                                        estándar UNSIGNED_SHORT o UNSIGNED_INT)
   --------------------------------------------------------------------------- */

#include "include/gl_util.hpp"
#include <iostream>
#include <vector>

/* Shaders: EXACTAMENTE los de la lección 02 (punto de partida) más un uniform
   para notar el tint global del ejemplo (2/ abajo: el cuadrado "viso").  --- */
static const char* VS_FUENTE = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
uniform vec2 uOffset;
void main() {
    gl_Position = vec4(aPos + uOffset, 0.0, 1.0);
}
)";

static const char* FS_FUENTE = R"(
#version 330 core
uniform vec3 uColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(uColor, 1.0);
}
)";

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL_Init fallo: " << SDL_GetError() << "\n";
        return 1;
    }

    glu::Contexto c = glu::abrir("Modulo 20 - L06 VBO/EBO/VAO", 800, 600);
    if (c.contexto == nullptr) { SDL_Quit(); return 1; }

    GLuint prog = glu::enlazar(VS_FUENTE, FS_FUENTE);
    if (prog == 0) { glu::cerrar(c); SDL_Quit(); return 1; }

    /* CUADRADO con 4 vértices + 6 índices (la lección del EBO):
       v0 abajo-izq, v1 abajo-der, v2 arriba-der, v3 arriba-izq.
       2 triángulos: (0,1,2) y (0,2,3): el índice 0 y el 2 SE REPITEN:
       los vértices compartidos no se dibujan dos veces.               */
    static const float ver[] = {
        -0.75f, -0.75f,   // v0
         0.75f, -0.75f,   // v1
         0.75f,  0.75f,   // v2
        -0.75f,  0.75f,   // v3
    };
    static const unsigned int idx[] = { 0, 1, 2,   0, 2, 3 };

    GLuint vao = 0, vbo = 0, ebo = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);                 // desde acá: TODO queda en el VAO

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof ver, ver, GL_STATIC_DRAW);

    // El EBO va bajo OTRO target: GL_ELEMENT_ARRAY_BUFFER. Y cuidado:
    // enlazar un EBO con un VAO activo lo GRABA en el VAO (es la excepción
    // del principio "los buffers no se graban en el VAO": el ELEMENT sí).
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof idx, idx, GL_STATIC_DRAW);

    // Formato: pos es vec2 con stride 8 (2 floats), offset 0 (lección 08).
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (const void*)0);

    glBindVertexArray(0);           // desactivar el VAO (higiene del setup)

    GLint uOffset = glGetUniformLocation(prog, "uOffset");
    GLint uColor  = glGetUniformLocation(prog, "uColor");

    bool corriendo = true;
    SDL_Event ev;
    while (corriendo) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) corriendo = false;
        }

        glClearColor(0.06f, 0.07f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(prog);
        glBindVertexArray(vao);     // el render SOLO necesita el VAO + prog

        glUniform2f(uOffset, -0.0f, 0.0f);
        glUniform3f(uColor, 1.0f, 0.6f, 0.2f);
        // Draw INDEXADO: 6 índices del EBO enlazado en el VAO. El último
        // parámetro es el OFFSET dentro del EBO, no puntero de memoria.
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (const void*)0);

        SDL_GL_SwapWindow(c.ventana);
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &ebo);       // espejo del orden exacto: EBO antes
    glDeleteBuffers(1, &vbo);       // que VBO "no importa"; no deja huella
    glDeleteProgram(prog);
    glu::cerrar(c);
    SDL_Quit();
    return 0;
}
