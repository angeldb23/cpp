/* =============================================================================
   MÓDULO 20 - LECCIÓN 01: Contexto OpenGL 3.3 Core — SDL_GL_SetAttribute,
   profile, forward-compat y validación de versión
   =============================================================================
   TEMA 1 (de 12 en TEMAS.md): Contexto OpenGL 3.3 Core.

   CÓMO CORRERLA A MANO (verificar.sh NO la corre: abre una ventana):
     g++ -std=c++20 -Wall -Wextra leccion_01_contexto_core.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec01 && ./lec01

   --- LOS DOS CAMINOS GRÁFICOS DE TU CARRERA ----------------------------------
   En 16/17/18 le pediste a raylib o SDL "dibujame ese sprite" y listo: la
   librería usó OpenGL por debajo sin mostrarte nada. De acá en más le hablás
   al GPU en su propio idioma: funciones gl* + shaders que escribís vos. Más
   trabajo, pero es el lenguaje real de todos los motores (Unity, Unreal,
   Godot) y te da control de cada paso.

   --- QUÉ ES UN "CONTEXTO" (y por qué no es opcional) -------------------------
   Un contexto GL es TODO el estado global vigente del GPU para un hilo: qué
   shaders, texturas y buffers tiene enlazados, qué blend/cull usa. Antes de
   poder llamar UNA sola función gl* tenés que crear el contexto y decirle al
   driver en qué modo lo querés:
     - PERFIL: CORE (este curso) o COMPATIBILITY (mantiene todo el legado).
     - VERSIÓN: 3.3. Nada del curso exige más; las funciones de 4.x que
       aparezcan se marcan "solo mención" en su lección.
     - FORWARD_COMPATIBLE: confiesa "core puro, nada del legado". El flag es
       de seguridad CLI para drivers raros; con Mesa 3.3 core anda sin drama.
   El orden de siempre: pedís atributos -> la ventana -> el contexto. Todo
   gl_FACTORY ciudadano de bien. TODA llamada gl* sin contexto vivo es
   comportamiento indefinido (generalmente segfault en el driver).

   --- LOS HEADERS (verificado contra los headers GL locales de glvnd/Mesa) ---
   OpenGL no es una librería: es una lista de funciones que Khronos congeló y
   que cada driver implementa. En Linux con glvnd:
     #define GL_GLEXT_PROTOTYPES
     #include <GL/glcorearb.h>
   (1) glcorearb.h SOLO declara prototipos de gl* si GL_GLEXT_PROTOTYPES está
       definido ANTES de incluirlo (verificado: sin el define falla con
       "glGenBuffers was not declared").
   (2) glcorearb.h no debe incluirse junto a <GL/gl.h> ni <GL/glext.h> en el
       mismo archivo (lo dice su propio encabezado).
   (3) -lOpenGL: glvnd's libOpenGL.so exporta los entrypoints y los despacha
       por la tabla del driver (Mesa en tu máquina). Ya está instalado.
       A VERIFICAR en Windows/macOS: ahí sí hace falta un loader (GLAD/GL3W)
       que declare PUNTEROS A FUNCIÓN y cargue con SDL_GL_GetProcAddress; el
       mecanismo lo muestra la lección 17 (KHR_debug) cuando carga por
       puntero.

   --- APIS VERIFICADAS contra SDL3 3.4.18 (SDL_video.h local) ----------------
   - SDL_GL_SetAttribute(attr, int) -> bool (true = ok). EN SDL2 ERA int
     0 = ok: trampa clásica del port (if (!SDL_GL_SetAttribute(...))).
   - SDL_GL_GetAttribute(attr, int*) -> bool: te CONFIRMA lo que el driver
     aceptó (puede diferir de lo que pediste).
   - SDL_CreateWindow(titulo, w, h, SDL_WINDOW_OPENGL).
   - SDL_GL_CreateContext(SDL_Window*) -> SDL_GLContext (puntero opaco).
   - SDL_GL_MakeCurrent(win, ctx), SDL_GL_SwapWindow(win) -> bool.
   - SDL_GL_DestroyContext(ctx) (en SDL2 se llamaba SDL_GL_DeleteContext) y
     SDL_DestroyWindow: cierre en orden INVERSO al de creacion (regla RAII
     que ya conocés del módulo 07).
   ============================================================================
 */

#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>
#include <SDL3/SDL.h>

#include <iostream>

/* Boilerplate ESPEJO: esta pareja abrir/cerrar se repite en cada lección con
   GL para que cada .cpp se corra a mano de forma autónoma. */
struct ContextoGL {
  SDL_Window *ventana = nullptr;
  SDL_GLContext contexto = nullptr;
};

static ContextoGL abrirContexto(const char *titulo, int w, int h) {
  ContextoGL c;

  // 1) El PLANO del contexto se declara ANTES de crearlo.
  if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                           SDL_GL_CONTEXT_PROFILE_CORE))
    std::cout << "perfil core no seteable: " << SDL_GetError() << "\n";
  if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3))
    std::cout << "major no seteable: " << SDL_GetError() << "\n";
  if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3))
    std::cout << "minor no seteable: " << SDL_GetError() << "\n";
  // Forward-compat NO es un atributo aparte: es una FLAG dentro del
  // atributo SDL_GL_CONTEXT_FLAGS (verificado en SDL_video.h).
  int flags = 0;
  SDL_GL_GetAttribute(SDL_GL_CONTEXT_FLAGS, &flags);
  if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
                           flags | SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG))
    std::cout << "forward-compat no seteable: " << SDL_GetError() << "\n";

  // 2) Doble buffer: pintar directo a la ventana parpadea; en cambio se
  //    pinta un backbuffer invisible y se intercambia fin de frame con
  //    SDL_GL_SwapWindow. El swap respeta el vsync de SDL_GL_SetSwapInterval
  //    (1 = esperar vblank, 0 = libre, -1 = adaptive).
  if (!SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1))
    std::cout << "double buffer no seteable: " << SDL_GetError() << "\n";

  c.ventana = SDL_CreateWindow(titulo, w, h, SDL_WINDOW_OPENGL);
  if (c.ventana == nullptr) {
    std::cout << "SDL_CreateWindow fallo: " << SDL_GetError() << "\n";
    return c;
  }

  // 3) Con ventana + plano, YA se puede crear el contexto real.
  c.contexto = SDL_GL_CreateContext(c.ventana);
  if (c.contexto == nullptr) {
    std::cout << "SDL_GL_CreateContext fallo: " << SDL_GetError() << "\n";
    SDL_DestroyWindow(c.ventana);
    c.ventana = nullptr;
    return c;
  }

  SDL_GL_MakeCurrent(c.ventana, c.contexto);
  SDL_GL_SetSwapInterval(1);
  return c;
}

static void cerrarContexto(ContextoGL &c) {
  if (c.contexto != nullptr)
    SDL_GL_DestroyContext(c.contexto);
  if (c.ventana != nullptr)
    SDL_DestroyWindow(c.ventana);
}

/* VALIDACIÓN DE VERSIÓN: pedimos 3.3 core, PERO el driver es la verdad.
   SDL_GL_GetAttribute confirma lo aceptado y glGetString la cadena canónica,
   p. ej. "3.3 (Core Profile) Mesa 24.x" en tu Intel UHD 600. glGetString se
   puede llamar UNICAMENTE con el contexto ya activo. */
static void imprimirInfoGL() {
  int mayor = 0, menor = 0, perfil = 0;
  SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &mayor);
  SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &menor);
  SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &perfil);

  const unsigned char *version = glGetString(GL_VERSION); // exige ctx vivo
  const unsigned char *glsl = glGetString(GL_SHADING_LANGUAGE_VERSION);
  const unsigned char *vendor = glGetString(GL_VENDOR);
  const unsigned char *render = glGetString(GL_RENDERER);

  std::cout << "pediste     : " << mayor << "." << menor << "\n";
  std::cout << "GL_VERSION  : " << (version ? (const char *)version : "NULL")
            << "\n";
  std::cout << "GLSL_VERSION: " << (glsl ? (const char *)glsl : "NULL") << "\n";
  std::cout << "GL_VENDOR   : " << (vendor ? (const char *)vendor : "NULL")
            << "\n";
  std::cout << "GL_RENDERER : " << (render ? (const char *)render : "NULL")
            << "\n";
  std::cout << "perfil      : "
            << (perfil == SDL_GL_CONTEXT_PROFILE_CORE ? "CORE" : "OTRO")
            << "\n";
}

int main() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cout << "SDL_Init fallo: " << SDL_GetError() << "\n";
    return 1;
  }

  ContextoGL c = abrirContexto("Modulo 20 - L01 contexto core 3.3", 800, 600);
  if (c.contexto == nullptr) {
    SDL_Quit();
    return 1;
  }

  imprimirInfoGL();

  glClearColor(0.1f, 0.1f, 0.15f, 1.0f); // fija el color de borrado
  /* Bucle mínimo: clear + swap + cerrar con la X. Es el esqueleto del
     render de todas las lecciones siguientes. */
  bool corriendo = true;
  SDL_Event ev;
  while (corriendo) {
    while (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_EVENT_QUIT)
        corriendo = false;
    }
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(c.ventana);
  }

  cerrarContexto(c);
  SDL_Quit();
  return 0;
}
