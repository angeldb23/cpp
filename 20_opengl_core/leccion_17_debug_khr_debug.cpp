/* =============================================================================
   MÓDULO 20 - LECCIÓN 17: Debug — glDebugMessageCallback (KHR_debug,
   A VERIFICAR soporte en tu Mesa), validación de estado y GPU timer queries
   =============================================================================
   TEMA 11 de TEMAS.md: glDebugMessageCallback, GL_KHR_debug (extensión),
   validación de estado y GPU timestamps (timer queries, OpenGL 3.3 core).

   CÓMO CORRERLA A MANO (desde la carpeta del módulo):
     g++ -std=c++20 -Wall -Wextra leccion_17_debug_khr_debug.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec17 && ./lec17

   --- LAS TRES HERRAMIENTAS DE "QUÉ ESTÁ PASANDO" -----------------------------

   1) glGetError: el clásico. Devuelve UNO de los errores acumulados
      (cada consulta consume uno; GL_NO_ERROR cuando la cinta está vacía).
      Regla del curso: chequeo post-glCall en debug en las secciones que
      se escriben or primera vez (los errores se acumulan: el siguiente
      glCall hereda el error del anterior y confunde la causa).

   2) KHR_debug (GL_KHR_debug): la descriptiva: el driver TE LLAMA cuando:
      - guardías "incompleto" ("malla de mesh sin atributos"); 
      - OR issues ("BufferStaticView Usedana draw con VISUALE defecto"); 
      - performance warning ("atributo duplicado"); Y labels que VOS ponés
        (glObjectLabel: "BUENAS PRÁCTICAS de equipo").
      A VERIFICAR EN TU MÁQUINA (Mesa 3.3 core): es una EXTENSIÓN:
        - en el RUNTIME: glGetString(GL_EXTENSIONS) tiene que contener
          "GL_KHR_debug" (Intel/Mesa si la soporta: dependiente por
          versión; si no supportá lo DELEGamos un "no debug callback" y
          el juego sigue configurable).
        - el FUNCTION POINTER: core 3.3 NO lo declara como gl*: si está
          la ext, la obtenés con SDL_GL_GetProcAddress("glDebugMessage-
          Callback") y la llamás con el typedef PFNGLDEBUGMESSAGE-
          CALLBACKPROC (verificado en glcorearb.h, con GL_KHR_debug def).
        Además: como función CORE existe recién en GL 4.3; en 3.3 SIEMPRE
          se pasa por SDL_GL_GetProcAddress (portable).

   3) TIMER QUERIES: el cronometro del GPU (core 3.3: verificado en el
      glcorearb.h: glQueryCounter(GL_TIMESTAMP) y GL_TIME_ELAPSED son de
      la sección del core GL_VERSION_3_3 (concurrencia con ARB_timer_query
      ya promosivida a core en 3.3)):
        glBeginQuery(GL_TIME_ELAPSED, id) ... glEndQuery(GL_TIME_ELAPSED);
        glGetQueryObjectui64v(id, GL_QUERY_RESULT, &ns)  -> NS del GPU
      ATENCIÓN: el resultado NO está listo al termina el glEndQuery (el
      GPU es asíncrono): se espera/pollea con glGetQueryObjectiv(...,
      GL_QUERY_RESULT_AVAILABLE) y suele listarse 1-2 frames después
      (no bloquees el loop esperando: cronometré y leé cuadros más tarde,
      como los frame graphs del 24).
   --------------------------------------------------------------------------- */

#include "include/gl_util.hpp"
#include <iostream>
#include <cstring>
#include <string>
#include <vector>

/* Hay una función callback de debug: es LLAMADA por el driver (thread
   del driver, ojo SI la globalstate está shared: ofta)
     src: GL_DEBUG_SOURCE_API o las fuentes de context 通常 etc.
     type: ERROR, DEPRECATED, UNDEFINED, PERFORMANCE, MARKER, GROUP...
     sev : HIGH (bug/de séjour), MEDIUM**, LOW, NOTIFICATION.
   El driver puede repetir el mismo msgID muchas veces; glDebugMessageControl
   (con GL_DEBUG_OUTPUT) recorta categorías si el log molesta.           */
static void APIENTRY callbackDebug(GLenum fuente, GLenum tipo, GLuint id,
                                   GLenum severidad, GLsizei len,
                                   const GLchar* mensaje,
                                   const void* /*user*/) {
    (void)tipo;   // (el tipo también se loggea en el mundo real: acá no)
    (void)id;
    const char* src = "<no constante>";
    switch (fuente) {
        case GL_DEBUG_SOURCE_API:               src = "API"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:   src = "shader-compiler"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:     src = "window-system"; break;
        default: break;
    }
    const char* sev = "otros";
    if (severidad == GL_DEBUG_SEVERITY_HIGH)         sev = "HIGH";
    else if (severidad == GL_DEBUG_SEVERITY_MEDIUM)  sev = "MEDIUM";
    else if (severidad == GL_DEBUG_SEVERITY_LOW)     sev = "LOW";
    std::fprintf(stderr, "[KHR_debug][%s][%s] %.*s\n",
                 src, sev, (int)len, mensaje);
}

/* glGetError: checekgl con nombre: el debug del curso: los asserts + el
   valor del enum en pantalla SIEMPRE en compilación de DEBUG.          */
static bool chequearGL(const char* donde) {
    GLenum e = glGetError();
    if (e == GL_NO_ERROR) return true;
    const char* nombre = "otros";
    switch (e) {
        case GL_INVALID_ENUM:                  nombre = "INVALID_ENUM"; break;
        case GL_INVALID_VALUE:                 nombre = "INVALID_VALUE"; break;
        case GL_INVALID_OPERATION:             nombre = "INVALID_OPERATION"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: nombre = "INVALID_FRAMEBUFFER"; break;
        case GL_OUT_OF_MEMORY:                 nombre = "OUT_OF_MEMORY"; break;
        default: break;
    }
    std::fprintf(stderr, "[glGetError] %s: %s\n", donde, nombre);
    return false;
}

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL_Init fallo: " << SDL_GetError() << "\n";
        return 1;
    }

    // A VERIFICAR 0 (conectado): un contexto de debug CAPTUREA los mensajes
    // -(como el pylint del debug): con SDL_GL_CONTEXT_DEBUG_FLAG añadía la
    // bandera; al manejar la alternativa: la función callback con laLibrary:
    glu::Contexto c = glu::abrir("Modulo 20 - L17 debug KHR y timers", 800, 600);
    if (c.contexto == nullptr) { SDL_Quit(); return 1; }

    // A VERIFICAR 1: ¿MI MESA soporta KHR_debug? (glGetString usa el :
    // extension list: con core 3.3 va leído igual)
    std::string exts = (const char*)glGetString(GL_EXTENSIONS);
    bool hayKhrDebug = exts.find("GL_KHR_debug") != std::string::npos;
    std::cout << "GL_KHR_debug: " << (hayKhrDebug ? "SI" : "NO") << "\n";

    if (hayKhrDebug) {
        // Obtención por proc address: la forma PORTABLE en 3.3 (el typedef
        // PFNGLDEBUGMESSAGECALLBACKPROC viene del glcorearb.h verificado).
        auto glDebugMessageCallbackFn =
            (PFNGLDEBUGMESSAGECALLBACKPROC)SDL_GL_GetProcAddress("glDebugMessageCallback");
        if (glDebugMessageCallbackFn != nullptr) {
            glDebugMessageCallbackFn(callbackDebug, nullptr);
            glEnable(GL_DEBUG_OUTPUT);          // optimizado (seal: los mensajes
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // de error "salen ya"
            std::cout << "callback de debug instan :: synchrone ON\n";
        }
    }

    /* VALIDACIÓN de errores GL: un error driv condicionel (el ejemplo
       del "glEnable(GL_NOT_A_FLAG)" con un 0xdeadbeef: INVALID_ENUM).  */
    glEnable((GLenum)0xdeadbeef);           // malo por imitaciones de prueba
    chequearGL("glEnable(malo)");           // imprimiría INVALID_ENUM

    /* TIMER QUERY: GPU time de UN CLEAR (ejǥ de 980-ish). En 3.3 core:
       GL_TIME_ELAPSED (verificado: sección GL_VERSION_3_3 de glcorearb)  */
    GLuint q = 0;
    glGenQueries(1, &q);
    glFinish();                              // resetear ruido del GPU previo
    glBeginQuery(GL_TIME_ELAPSED, q);
    glClear(GL_COLOR_BUFFER_BIT);
    glEndQuery(GL_TIME_ELAPSED);

    GLint listo = 0;
    std::cout << "esperando en poll el resultado del GPU...\n";
    do {
        glGetQueryObjectiv(q, GL_QUERY_RESULT_AVAILABLE, &listo);
    } while (!listo);
    GLuint64 ns = 0;
    glGetQueryObjectui64v(q, GL_QUERY_RESULT, &ns);
    std::cout << "duracion del clear en el GPU: " << ns / 1000.0
              << " microseg (en el UHD 600 suele quedar muy por debajo del ms).\n";

    glDeleteQueries(1, &q);
    glu::cerrar(c);
    SDL_Quit();
    return 0;
}
