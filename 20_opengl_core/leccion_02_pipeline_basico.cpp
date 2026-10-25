/* =============================================================================
   MÓDULO 20 - LECCIÓN 02: El pipeline programable (1/2) — vertex/fragment,
   atributos y gl_Position
   =============================================================================
   TEMA 2 de TEMAS.md (1 de 2 lecciones): Pipeline programable.

   CÓMO CORRERLA A MANO (desde la carpeta del módulo):
     g++ -std=c++20 -Wall -Wextra leccion_02_pipeline_basico.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec02 && ./lec02

   --- EL PIPELINE POR DENTRO: mapa que usan TODAS las lecciones ---------------
   Cuando el GPU dibuja UN TRIÁNGULO lo pasa por etapas enchufadas como una
   batería. Las dos programables que usamos siempre:

     (1) VERTEX SHADER: corre UNA VEZ POR VÉRTICE. Su salida obligatoria es
         gl_Position (vec4) en COORDENADAS DE CLIP. No dibuja nada: solo
         DECIDE DÓNDE está el vértice.

     [ rasterización (hardware, no programable): de los 3 gl_Position salen
       los FRAGMENTOS (píxeles candidatos del triángulo) y los "varyings"
       declarados via out/in se INTERPOLAN hacia cada fragmento. ]

     (2) FRAGMENT SHADER: corre UNA VEZ POR FRAGMENTO sobreviviente. Su
         salida es un vec4 RGBA que cae al framebuffer.

   ¿POR QUÉ "clipspace"? Tras el vertex shader el GPU divide por w (la
   perspectiva) y normaliza: x/w, y/w deben quedar en [-1,1]; lo que queda
   fuera de los 4 planos de clip se descarta. En 2D w=1 y el clipspace es
   el cuadrado [-1,1]x[-1,1], con y hacia ARRIBA. El paso de clipspace a
   píxeles de ventana lo hace el GL con el viewport (lección 12).

   --- QUÉ ES UN "ATRIBUTO" ----------------------------------------------------
   Atributo = dato POR VÉRTICE (posición, color, UV). En el shader se declara
   `layout (location = 0) in vec3 aPos;`. Ese 0..15 es el ÍNDICE que después
   usás en glVertexAttribPointer (lección 08): shader y C++ se sincronizan
   POR ese número; el nombre es documentación para humanos.

   --- FUNCIONES GL QUE APARECEN (todas core <= 3.3; verificadas en
   glcorearb.h local) -----------------------------------------------------------
     glGenVertexArrays      -> N nombres de VAO
     glBindVertexArray      -> activa uno (router de atributos)
     glGenBuffers/glBindBuffer -> nombres y target (GL_ARRAY_BUFFER)
     glBufferData           -> copia del C++ a memoria del GPU
     glEnableVertexAttribArray(0)
     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, offset)
     glUseProgram           -> qué programa corre el pipeline
     glDrawArrays(GL_TRIANGLES, 0, 3)
     glDeleteVertexArrays / glDeleteBuffers / glDeleteProgram
   --------------------------------------------------------------------------- */

#include "include/gl_util.hpp"
#include <iostream>

/* --- SHADERS (GLSL 330 core, como raw strings de C++) ------------------------
   gl_Position = vec4(aPos, 1.0): w=1 fija cuadrado clipspace 2D. El fragment
   shader pinta TODO el triángulo naranja. El fill-rate importa: en 1080p,
   si el triángulo tapara la pantalla, el fragment shader correría ~2M
   veces por frame.                                                         */
static const char* VS_FUENTE = R"(
#version 330 core
layout (location = 0) in vec3 aPos;   // dato por vértice, entra por location 0
void main() {
    gl_Position = vec4(aPos, 1.0);    // salida oficial de la etapa vértice
}
)";

static const char* FS_FUENTE = R"(
#version 330 core
out vec4 FragColor;                   // TODO fragment shader necesita un out
void main() {
    FragColor = vec4(1.0, 0.6, 0.2, 1.0);
}
)";

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL_Init fallo: " << SDL_GetError() << "\n";
        return 1;
    }

    glu::Contexto c = glu::abrir("Modulo 20 - L02 pipeline basico", 800, 600);
    if (c.contexto == nullptr) { SDL_Quit(); return 1; }

    // 1) Compilar y linkear (la mecánica completa es la lección 04).
    GLuint prog = glu::enlazar(VS_FUENTE, FS_FUENTE);
    if (prog == 0) { glu::cerrar(c); SDL_Quit(); return 1; }

    // 2) LOS DATOS: 3 vértices x 3 floats, en NDC. 36 bytes subidos al GPU.
    static const float vertices[] = {
        -0.5f, -0.5f, 0.0f,   // abajo-izquierda
         0.5f, -0.5f, 0.0f,   // abajo-derecha
         0.0f,  0.5f, 0.0f,   // arriba
    };

    // 3) VAO: en core profile SIEMPRE hay un VAO enlazado para dibujar; si
    //    no lo hay, glDraw* no dibuja nada (queda warning en el debug output
    //    de la lección 17 si está activo). El VAO memoriza QUÉ atributos hay
    //    y de qué buffer vienen.
    GLuint vao = 0, vbo = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // 4) VBO + subida. glBufferData COPIA el array: después de esta llamada
    //    el C++ puede tocar su copia sin afectar al GPU.
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 5) Conectar 'aPos' (location 0, vec3) con los bytes del VBO:
    //    - 3: componentes por atributo (vec3)
    //    - GL_FLOAT + GL_FALSE: tipo float, no normalizado (lección 08)
    //    - 3*sizeof(float): STRIDE: bytes de un vértice al siguiente (12)
    //    - (void*)0: OFFSET del atributo dentro del vértice
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          3 * sizeof(float), (const void*)0);

    // 6) Render por frame: clear -> glUseProgram -> bind -> draw.
    bool corriendo = true;
    while (corriendo) {
        corriendo = glu::eventosOk();
        glClearColor(0.06f, 0.07f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(prog);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        SDL_GL_SwapWindow(c.ventana);
    }

    // Cierre en orden inverso a la creación (RAII manual, módulo 07).
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(prog);
    glu::cerrar(c);
    SDL_Quit();
    return 0;
}
