/* =============================================================================
   MÓDULO 20 - LECCIÓN 05: Shaders (2/2) — hot-reload de shaders en vivo y
   uniform blocks (UBO, layout std140)
   =============================================================================
   TEMA 3 de TEMAS.md (2 de 2): Hot-reload (watch file + recompile) y UBO.

   CÓMO CORRERLA A MANO (desde la carpeta del módulo):
     g++ -std=c++20 -Wall -Wextra leccion_05_hot_reload_ubo.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec05 && ./lec05
   (Y dejar .glsl al lado del binario para ver el reload en acción: el
   programa busca el archivo y lo recompila SOLO cuando cambió el mtime.)

   --- HOT-RELOAD: ¿qué problema resuelve? -------------------------------------
   Editar un shader NO debería exigir recompilar el ejecutable ni reiniciar:
   el shader es DATO en runtime (glShaderSource recibe un string). El ciclo:
     1. Guardás el fuente en un archivo .glsl.
     2. Cada N frames comparamos el mtime (modificación) con el valor que
        guardamos cuando cargamos la última versión.
     3. Cambió -> leer el archivo COMPLETO -> compilar+linkear (lección 04)
     4. Si compila: reemplazamos SOLO el GLuint del programa. El VAO y lo
        demás queda EXACTAMENTE IGUAL (los atributos son por-location, la
        lección 08); si agregaste uniforms nuevos, se re-buscan.
     5. Si NO compila: se conserva el programa viejo y el log se imprime
        (el error no tala el juego: es la gracia del reload).
   El mtime en std::filesystem: last_write_time (módulo 12). En un juego
   real (módulo 24) el watcher de assets hace lo mismo con texturas/wavs.

   TRAMPA del hot reload: cambiar un layout(location) o los nombres de los
   in/out puede "descomperse" el programa con el vocablo de C++ (el C++ lo
   descargaba por nombre también en glGetAttribLocation). Mantener los
   locations fijos por convención 0..N da estabilidad mientras experimentá.

   --- UNIFORM BLOCKS (UBO): el problema del uniform a mano --------------------
   Con glGetUniformLocation x cada uniform, un motor con 50 uniforms es una
   lista que DUPLICA el shader (en C++ y en GLSL). La pregunta: ¿y si el C++
   setea una Estructura entera (bloque) y el shader la declara al unísono?

     GLSL: uniform BloqueCamara { mat4 vista; mat4 proyeccion; };
     C++:  struct BloqueCamara { float vista[16]; float proyeccion[16]; };
           glBufferData(GL_UNIFORM_BUFFER, sizeof(BloqueCamara), &b, ...);

   El detalle es el LAYOUT: el driver ubica los miembros según REGLAS DE
   std140 (core 3.1; base para entender mejor estructuras de materiales en
   21). REGLAS std140 BÁSICAS (para float/vec):
     - cada miembro alineado a su tamaño redondeado a 16 bytes: float -> 16;
       vec2 -> 16; vec3 -> 16 (¡sí, 16!); vec4 -> 16; mat4 -> 16x4 = 64.
     - Total al final de UNA ESTRUCTURA redondeado a 16.
   O sea: NO es sizeof(C++struct): un vec3 seguido de un float NO son 16
   (es el vec3 alineado a 16 por SU tamaño y el float empieza en 16)...
   ¡PERO el float siguiente si hay ARRIBA del próximo vec4 va solito en
   12 bytes "gratis" (p-pack) que C++ NO permite! PASO: es fijo:
     float (4) -> vec2 (8) -> float: se ARMA: float+vec2 en 8 más 16:
     más la tabla de offsets/reglas exactas en std140 vs std430 (4.3/mención).

   VERIFICADO NECESARIO: el calculador de OFFSETS std140 C++ (puro) QUE
   COMPONEMOS ACÁ es el núcleo del QUIZ (pregunta 12) y es la razón por la
   que un material con vec3+float pesa 32 en un UBO aunque en C++ pesen 16.

   --- FUNCIONES GL DEL UBO (verificadas en glcorearb.h, core <= 3.3) ----------
     glGenBuffers/glBindBuffer(GL_UNIFORM_BUFFER, ...) / glBufferData
     glGetUniformBlockIndex(prog, "Bloque")       -> índice del bloque
     glUniformBlockBinding(prog, index, bindingPoint)  -> index<->punto 0..N
     glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, buffer) -> uv.
   --------------------------------------------------------------------------- */

#include "include/gl_util.hpp"
#include <filesystem>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <string>

namespace fs = std::filesystem;

/* ---------------------------------------------------------------------------
   NÚCLEO TESTEABLE 1: mtimes para el hot-reload (lógica pura, sin GL)
 ------------------------------------------------------------------------- */
struct Reload {
    std::string ruta;
    fs::file_time_type mtime = fs::file_time_type::min();
};

static bool actualizar(Reload& r, std::string& fuente) {
    auto mt = fs::last_write_time(r.ruta);          // puede lanzar si no existe
    if (mt == r.mtime) return false;                // nada nuevo -> nada

    std::ifstream in(r.ruta);
    if (!in) return false;
    std::ostringstream ss;
    ss << in.rdbuf();
    fuente = ss.str();
    r.mtime = mt;                                   // record = la nueva marca
    return true;                                    // se leyó y recargó
}

/* ---------------------------------------------------------------------------
   NÚCLEO TESTEABLE 2: cálculo de OFFSETS std140 (aplica las reglas del
   spec para float/vec2/vec3/vec4/mat4). NO es sizeof(C++): confundirlo es
   el error tipo del QUIZ (pregunta 12).
 ------------------------------------------------------------------------- */
enum class Std140Tipo { Float, Vec2, Vec3, Vec4, Mat4 };

static size_t std140_offset(size_t* cursor, Std140Tipo t) {
    size_t tam = 0, alineacion = 0;
    switch (t) {
        case Std140Tipo::Float: tam = 4;  alineacion = 16; break; // 4 -> a 16
        case Std140Tipo::Vec2:  tam = 8;  alineacion = 16; break;
        case Std140Tipo::Vec3:  tam = 12; alineacion = 16; break;
        case Std140Tipo::Vec4:  tam = 16; alineacion = 16; break;
        case Std140Tipo::Mat4:  tam = 64; alineacion = 16; break;
    }
    size_t resto = (*cursor) % alineacion;
    size_t offset = *cursor + (resto == 0 ? 0 : alineacion - resto);
    *cursor = offset + tam;                          // cursor avanza ACÁ
    return offset;
}

/* ---------------------------------------------------------------------------
   EL RELOAD CORRIENDO: un programa CARGADO desde disco que se recompila si
   el .glsl cambia, y UN UBO de cámara actualizable sin tocar el programa.
 ------------------------------------------------------------------------- */
int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL_Init fallo: " << SDL_GetError() << "\n";
        return 1;
    }

    glu::Contexto c = glu::abrir("Modulo 20 - L05 hot reload y UBO", 800, 600);
    if (c.contexto == nullptr) { SDL_Quit(); return 1; }

    /* Escribimos UN .glsl de ejemplo al lado (para que la lección corra
       incluso la primera vez). Podés editarlo mientras corre y ver el
       resultado: el cuadrado cambia de color cada vez que cambiás blanco.  */
    const char* rutaVs = "shader_05.vs.glsl";
    if (!fs::exists(rutaVs)) {
        std::ofstream out(rutaVs);
        out << "#version 330 core\n"
            << "layout (location = 0) in vec2 aPos;\n"
            << "void main() { gl_Position = vec4(aPos, 0.0, 1.0); }\n";
        out.close();
    }
    const char* rutaFs = "shader_05.fs.glsl";
    if (!fs::exists(rutaFs)) {
        std::ofstream out(rutaFs);
        out << "#version 330 core\n"
            << "out vec4 FragColor;\n"
            << "void main() { FragColor = vec4(1.0, 1.0, 1.0, 1.0); }\n";
        out.close();
    }

    Reload relVs{ rutaVs }, relFs{ rutaFs };
    std::string srcVs, srcFs;

    // Primera carga (mtime==min, o sea "siempre actualizar la primera vez").
    actualizar(relVs, srcVs);
    actualizar(relFs, srcFs);
    GLuint prog = glu::enlazar(srcVs.c_str(), srcFs.c_str());
    if (prog == 0) { glu::cerrar(c); SDL_Quit(); return 1; }
    std::cout << "programa inicial cargado: " << prog << "\n";

    // --- UBO de cámara ---------------------------------------------
    // Offsets std140 VERIFICADOS con el calculador de arriba:
    //   mat4 vista (0..63) + mat4 proyeccion (64..127) -> 128 bytes.
    unsigned char bloque[128] = {0};
    size_t cur = 0;
    size_t offVista = std140_offset(&cur, Std140Tipo::Mat4);
    size_t offProy  = std140_offset(&cur, Std140Tipo::Mat4);
    std::cout << "std140: vista@" << offVista << " proyeccion@" << offProy
              << " total=" << cur << "\n";

    // identidad de 4x4 column-major (módulo 19) para el ejemplo
    float ident[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
    std::memcpy(bloque + offVista, ident, sizeof ident);
    std::memcpy(bloque + offProy,  ident, sizeof ident);

    GLuint ubo = 0;
    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof bloque, bloque, GL_DYNAMIC_DRAW);

    GLuint idxBloque = glGetUniformBlockIndex(prog, "BloqueCamara");
    // (este shader demo NO declara el bloque: ejemplo referencial; el
    //  QUIZ y la lección 21 usan bloques reales)
    std::cout << "BloqueCamara index: " << idxBloque
              << " (glGetUniformBlockIndex devuelve GL_INVALID_INDEX si no está declarado)\n";

    float t;    // (el UBO reemplazaria el uniform del tiempo de la L03)
    (void)t;
    bool corriendo = true;
    SDL_Event ev;
    while (corriendo) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) corriendo = false;
        }

        // HOT RELOAD: una vez por frame (barato: un stat por archivo).
        std::string nuevoVs, nuevoFs;
        if (actualizar(relVs, nuevoVs) || actualizar(relFs, nuevoFs)) {
            std::cout << "cambio detectado; recompilando...\n";
            GLuint nuevo = glu::enlazar(nuevoVs.c_str(), nuevoFs.c_str());
            if (nuevo != 0) {                  // falla -> conservamos el viejo
                glDeleteProgram(prog);
                prog = nuevo;
                idxBloque = glGetUniformBlockIndex(prog, "BloqueCamara");
                std::cout << "recargado: " << prog << "\n";
            } else {
                std::cout << "el nuevo shader NO compila: seguimos con el viejo\n";
            }
        }

        glClearColor(0.06f, 0.07f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(prog);
        SDL_GL_SwapWindow(c.ventana);
    }

    glDeleteBuffers(1, &ubo);
    glDeleteProgram(prog);
    glu::cerrar(c);
    SDL_Quit();
    return 0;
}
