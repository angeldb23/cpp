/* =============================================================================
   MÓDULO 20 - LECCIÓN 08: Formatos de vértice — glVertexAttribPointer, stride,
   offset y sizeof(Vertex)
   =============================================================================
   TEMA 5 de TEMAS.md: glVertexAttribPointer, glEnableVertexAttribArray,
   stride, offset y sizeof(Vertex).

   CÓMO CORRERLA A MANO (desde la carpeta del módulo):
     g++ -std=c++20 -Wall -Wextra leccion_08_formato_vertices.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec08 && ./lec08

   --- LA FUNCIÓN MÁS "INGERIDA" POR EL VAO -----------------------------------
   glVertexAttribPointer(location, cantidad, tipo, normalizar, stride, offset)
     location  : el ÍNDICE de layout en el shader (lección 03).
     cantidad  : 1..4 componentes del tipo básico.
     tipo      : GL_FLOAT, GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT... GL_DOUBLE
                 existe (no lo usamos: 2x el bus por nada).
     normalizar: GL_TRUE convierte el entero a [0,1] (caso de uso real:
                 color como 4 GLubyte = 4 bytes en vez de 16 de floats y
                 shader ve 0..1 como si fuera float). Con datos float
                 SIEMPRE GL_FALSE.
     stride    : bytes ENTRE vértices (de un vértice al siguiente). ¡OJO!
                 stride 0 significa "tight packing", NO 0 bytes: el driver
                 deduce cantidad*sizeof(tipo).
     offset    : bytes DONDE empieza el atributo dentro del vértice. La C++
                 signatura pide const void* pero ES UN OFFSET, no un puntero
                 de memoria real (el GPU suma el offset del VBO enlazado).
   Y LA REGLA DE ORO: el buffer que queda grabado en el VAO para cada
   location es el VBO ENLAZADO (GL_ARRAY_BUFFER) en el MOMENTO de la
   llamada a glVertexAttribPointer. Es por eso que el patrón del VAO es:
     bind VAO -> bind VBO -> bufferData -> (por cada atributo) pointer.

   --- DOS ARREGLOS CLÁSICOS (opuestos en la mesa) ----------------------------
   INTERLEAVED (un VBO, vértice con pos+uv+color uno al lado del otro):
     mejor localidad de caché: una línea de caché trae los bytes de TODOS
     los atributos del mismo vértice.
   SEPARADO (un VBO por atributo): mejor cuando un sistema actualiza SOLO
     un atributo (las partículas mueven pos: el buffer de color no se
     re-suben cada frame, sección streaming de la L07).
   POSTURA DEL CURSO: interleaved por defecto (la localidad gana);
   separado SOLO si un sistema de update por-atributo lo justifica (y
   decídelo con perf, módulo 15: profiler antes, arquitectura después).

   --- sizeof(VERTEX): la cuenta que se hace UNA vez y se verifica ------------
   struct Vertex { float pos[3]; float uv[2]; float color[4]; };
     pos   : offset 0,  12 bytes
     uv    : offset 12,  8 bytes
     color : offset 20, 16 bytes
     stride = 36 = sizeof(Vertex)  (tight packing: alineación 4 no deja gap)
   El static_assert de abajo VERIFICA la cuenta en tiempo de compilación:
   si en el futuro editás el struct y olvidás re-configurar los offsets del
   VAO, el error aparece AL COMPILAR (mucho mejor que una geometría roja en
   pantalla sin explicar). MENCION 4.3+: mientras std140 da offsets "de
   fábrica" para UBOs (lección 05), para atributos por vértice el layout
   queda a tu cargo; herramientas tipo std430/SSBO también van fuera de
   core 3.3.
   --------------------------------------------------------------------------- */

#include "include/gl_util.hpp"
#include <cstddef>
#include <iostream>
#include <string>
#include <vector>

/* ---------------------------------------------------------------------------
   NÚCLEO TESTEABLE (es la materia del Ejercicio 02): layout de atributos
   calculado EN C++, para luego garantizar que glPointer use EXACTO ese.
 ------------------------------------------------------------------------- */
enum class AtributoTipo { Float, Vec2, Vec3, Vec4, Ubyte4 };

static int compsDe(AtributoTipo t, bool& normalizar) {
    normalizar = false;
    switch (t) {
        case AtributoTipo::Float:  return 1;
        case AtributoTipo::Vec2:   return 2;
        case AtributoTipo::Vec3:   return 3;
        case AtributoTipo::Vec4:   return 4;
        case AtributoTipo::Ubyte4: normalizar = true; return 4;
    }
    return 0;
}

static int bytesBaseDe(AtributoTipo t) {     // sizeof del tipo básico
    return (t == AtributoTipo::Ubyte4) ? 1 : 4;   // GLubyte o float
}

struct Campo {
    std::string  nombre;              // documentación para humanos
    AtributoTipo tipo;
};

struct Layout {                       // resultado del cálculo
    int stride = 0;                   // bytes por vértice
    std::vector<int> offsets;         // en el orden de los campos
};

static Layout calcular(const std::vector<Campo>& campos) {
    Layout l;
    l.offsets.reserve(campos.size());
    int cursor = 0;
    for (const Campo& c : campos) {
        bool normalizar = false;
        int comps = compsDe(c.tipo, normalizar);
        l.offsets.push_back(cursor);
        cursor += comps * bytesBaseDe(c.tipo);   // tight: sin gap
    }
    l.stride = cursor;
    return l;
}

/* ------------------------------------------------------------------------- */
struct Vertex {                        // la geometría BASE del curso
    float pos[3];                      // offset 0,  12 bytes
    float uv[2];                       // offset 12,  8 bytes
    float color[4];                    // offset 20, 16 bytes
};
// VERIFICACIÓN DE COMPILACIÓN: si tocás el struct, la cuenta EXPLOTA (gana
// elPrograma de check en vez de "dibujar verde"): TU GLM no iguala std140.
static_assert(offsetof(Vertex, pos)   == 0,  "pos offset correcto");
static_assert(offsetof(Vertex, uv)    == 12, "uv sabe offset en 12");
static_assert(offsetof(Vertex, color) == 20, "color sabe offset en 20");
static_assert(sizeof(Vertex) == 36,       "stride total = 36 bytes");

/* Shaders mínimos: la demo de abajo solo verifica que el VAO con esos
   offsets se enlaza sin dramas. Nada superfluo.                          */
static const char* VS_FUENTE = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
void main() { gl_Position = vec4(aPos, 0.0, 1.0); }
)";

static const char* FS_FUENTE = R"(
#version 330 core
out vec4 FragColor;
void main() { FragColor = vec4(0.4, 0.7, 1.0, 1.0); }
)";

int main() {
    // 1) Núcleo: el layout de Vertex (36 de stride) es el que verificar
    std::vector<Campo> campoST = {
        {"pos",   AtributoTipo::Vec3},
        {"uv",    AtributoTipo::Vec2},
        {"color", AtributoTipo::Vec4},
    };
    Layout l = calcular(campoST);
    std::cout << "layout: stride=" << l.stride << " offsets:";
    for (int o : l.offsets) std::cout << " " << o;
    std::cout << " | sizeof(Vertex)=" << sizeof(Vertex) << "\n";

    /* 2) La parte GL: el mismo Vertex subido al GPU y enlazado con los
          offsets que calcular() produjo (stride = sizeof(Vertex) = 36).
          Dos vértices para el patrón: la parte interesante NO es dibujar
          sino que stride/offset deriven del layout y NO de números a mano. */
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL_Init fallo: " << SDL_GetError() << "\n";
        return 1;
    }
    glu::Contexto c = glu::abrir("Modulo 20 - L08 formato vertices", 640, 480);
    if (c.contexto == nullptr) { SDL_Quit(); return 1; }

    GLuint prog = glu::enlazar(VS_FUENTE, FS_FUENTE);
    if (prog == 0) { glu::cerrar(c); SDL_Quit(); return 1; }

    static const Vertex verts[2] = {
        { {-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.5f, 0.2f, 1.0f} },
        { { 0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.5f, 0.2f, 1.0f} },
    };
    GLuint vao = 0, vbo = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof verts, verts, GL_STATIC_DRAW);

    // los offsets VIENEN DEL LAYOUT CALCULADO (no a mano):
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (const void*)(size_t)l.offsets[0]);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (const void*)(size_t)l.offsets[1]);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (const void*)(size_t)l.offsets[2]);
    glBindVertexArray(0);

    glClearColor(0.06f, 0.07f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(prog);
    glBindVertexArray(vao);
    glDrawArrays(GL_LINES, 0, 2);      // solo 2 puntos: demostración mínima
    SDL_GL_SwapWindow(c.ventana);

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(prog);
    glu::cerrar(c);
    SDL_Quit();
    return 0;
}
