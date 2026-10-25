/* =============================================================================
   MÓDULO 20 - include/gl_util.hpp — helpers comunes de las lecciones GL
   =============================================================================
   Funciones que repiten TODAS las lecciones que abren contexto y compilan
   shaders. Las lecciones SOLO las usan: la teoría de cada paso está en el
   .cpp de la lección correspondiente (01: contexto; 04: compilación; 06:
   buffers; 08: atributos...). Todo es OpenGL 3.3 CORE: ninguna función acá
   pertenece al legado o a versiones > 3.3 salvo las marcadas "mención".

   VERIFICADO contra headers locales el 22/10/2026:
   - SDL3 3.4.18 (SDL_video.h): SDL_GL_SetAttribute/GetAttribute -> bool;
     SDL_GL_CreateContext -> SDL_GLContext; SDL_GL_DestroyContext; flags.
   - GL/glcorearb.h de glvnd: requiere GL_GLEXT_PROTOTYPES definido ANTES
     para declarar prototipos; se linkea con -lOpenGL (libOpenGL.so.0).

   Compilar una lección (desde la carpeta del módulo):
     g++ -std=c++20 -Wall -Wextra leccion_02_... .cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec02 && ./lec02
   ============================================================================ */
#pragma once

#define GL_GLEXT_PROTOTYPES
#include <SDL3/SDL.h>
#include <GL/glcorearb.h>

#include <cstdio>
#include <cstring>
#include <string>

namespace glu {  // "gl utilitario" del módulo

/* ---------------------------------------------------------------------------
   CONTEXTO: ventana + contexto 3.3 core + cierre en orden inverso.
   La matemática pedagógica está explicada en la lección 01; acá es solo el
   boilerplate facturado para no repetirlo 17 veces.
 ------------------------------------------------------------------------- */
struct Contexto {
    SDL_Window*   ventana  = nullptr;
    SDL_GLContext contexto = nullptr;
    int           ancho    = 0;
    int           alto     = 0;
};

inline Contexto abrir(const char* titulo, int w, int h, bool msaa = false) {
    Contexto c;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    if (msaa) {   // MSAA del FRAMEBUFFER DEFAULT (la lección 16 lo desarma)
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    }
    c.ventana = SDL_CreateWindow(titulo, w, h, SDL_WINDOW_OPENGL);
    if (c.ventana == nullptr) {
        std::fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        return c;
    }
    c.contexto = SDL_GL_CreateContext(c.ventana);
    if (c.contexto == nullptr) {
        std::fprintf(stderr, "SDL_GL_CreateContext: %s\n", SDL_GetError());
        SDL_DestroyWindow(c.ventana);
        c.ventana = nullptr;
        return c;
    }
    SDL_GL_MakeCurrent(c.ventana, c.contexto);
    SDL_GL_SetSwapInterval(1);
    SDL_GetWindowSize(c.ventana, &c.ancho, &c.alto);
    return c;
}

inline void cerrar(Contexto& c) {
    if (c.contexto != nullptr) SDL_GL_DestroyContext(c.contexto);
    if (c.ventana   != nullptr) SDL_DestroyWindow(c.ventana);
    c = {};
}

/* Recorre la cola de eventos y devuelve false cuando llega el QUIT. */
inline bool eventosOk() {
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_EVENT_QUIT) return false;
    }
    return true;
}

/* ---------------------------------------------------------------------------
   COMPILACIÓN / LINKEO DE SHADERS: la mecánica completa está en la lección 04
   (acá solo la empaquetamos para que no la copien 15 veces).
 ------------------------------------------------------------------------- */

/* Devuelve la parte del log SIN el prefijo informativo del driver (Mesa
   antepone "ERROR: 0:1: ..." igual en todos lados; se reporta entero). */
inline bool compilar(GLenum tipo, const char* fuente, GLuint& fuera) {
    GLuint sh = glCreateShader(tipo);
    glShaderSource(sh, 1, &fuente, nullptr);
    glCompileShader(sh);

    GLint ok = GL_FALSE;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (ok != GL_TRUE) {                       // el log es la única lectura
        char log[1024];
        GLsizei n = 0;
        glGetShaderInfoLog(sh, sizeof log, &n, log);
        std::fprintf(stderr, "compilar: %.*s\n", (int)n, log);
        glDeleteShader(sh);
        return false;
    }
    fuera = sh;
    return true;
}

/* VS+FS -> programa linkeado (0 si falla; imprime el log). */
inline GLuint enlazar(const char* vs, const char* fs) {
    GLuint v = 0, f = 0, prog = 0;
    if (!compilar(GL_VERTEX_SHADER, vs, v))   return 0;
    if (!compilar(GL_FRAGMENT_SHADER, fs, f)) { glDeleteShader(v); return 0; }

    prog = glCreateProgram();
    glAttachShader(prog, v);
    glAttachShader(prog, f);
    glLinkProgram(prog);
    glDeleteShader(v);   // ya están dentro del programa: los adjuntos no se
    glDeleteShader(f);   // borran, solo se despegan de su vida independiente

    GLint ok = GL_FALSE;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (ok != GL_TRUE) {
        char log[1024];
        GLsizei n = 0;
        glGetProgramInfoLog(prog, sizeof log, &n, log);
        std::fprintf(stderr, "enlazar: %.*s\n", (int)n, log);
        glDeleteProgram(prog);
        return 0;
    }
    return prog;
}

/* GLSL 3.30 base: encabezado común (versión SIEMPRE primera línea no
   vacía/no-comentario; la lección 04 lo lintea). */
inline std::string vs330() {
    return "#version 330 core\n";
}

/* ---------------------------------------------------------------------------
   UNIFORM "1 float/vec": atajo de la lección 03. glGetUniformLocation llama
   al driver CADA VEZ que se consulta: en el mundo real se cachea (lección 05
   guarda el lugar en un struct al linkar).
 ------------------------------------------------------------------------- */
struct Uniform {
    GLint lugar = -1;                       // -1 si el compilador lo tiró
    void buscar(GLuint prog, const char* nombre) { lugar = glGetUniformLocation(prog, nombre); }
    bool vez() const { return lugar != -1; }
};

}  // namespace glu
