/* =============================================================================
   MÓDULO 20 - LECCIÓN 04: Shaders (1/2) — compilación, linkeo, error log y
   fuente "linteada" antes de mandarla al GPU
   =============================================================================
   TEMA 3 de TEMAS.md (1 de 2): Compilación, linkeo, error log, hot-reload y
   uniform blocks. (Hot-reload y UBO van en la lección 05.)

   CÓMO CORRERLA A MANO (desde la carpeta del módulo):
     g++ -std=c++20 -Wall -Wextra leccion_04_compilacion_linking.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec04 && ./lec04

   --- EL MODELO DE COMPILACIÓN (diferente al de C++, y de ahí el dolor) -------
   GLSL compila EN RUNTIME, desde C++:
     1. glCreateShader(GL_VERTEX_SHADER) -> nombre de shader ( como "Objeto"
        de GL: un tipo puntero a funciones, la tabla del driver).
     2. glShaderSource + glCompileShader: lo FUENTE lo manda el driver.
     3. glGetShaderiv(shader, GL_COMPILE_STATUS, &ok): SIN MIRAR ok el
        compile falla SILENCIOSAMENTE en muchos drivers (bug clásico
        reportado por la comunidad desde hace años). SIEMPRE mirarlo.
     4. glGetShaderInfoLog: devuelve el log del compilador COMO STRING, con
        la línea del error (0:N formato: N = línea DEL FUENTE, no del
        archivo .glsl completo). Cada driver escribe a su manera (Mesa:
        "0:5(2): error: xxx"; el contenido de la línea es la clave).
     5. glCreateProgram + glAttachShader x2 + glLinkProgram.
     6. glGetProgramiv(prog, GL_LINK_STATUS): ¿insiste dónde el embedding?
        (link falla si un out del VS no matchea un in del FS, o si el FS
        no tiene out). Mirarlo SIEMPRE TAMBIÉN: el link SEGUIRÍA andando
        con el programa "medio" o `prog = 0` si se borra mal.
     7. glDetachShader/glDeleteShader: adjuntos al programa NO viven
        "haciéndolo", solo se copian en él (borrar el shader NO borra el
        programa; pero fuga de nombres GL: el GPU le gusta austeros).
   Y geneque el "GLSL lint" hecho C++ (este módulo tiene ejercicio 01) porque
   el mensaje del driver muchas veces no dice QUÉ la fuente tiene mal:

   --- EL LINT EN C++ (complemento, no reemplazo) --------------------------------
   Antes de mandar el fuente al GPU, un "lint" en C++ puro atrapa los
   errores tontos:
     - validar que la PRIMERA línea no vacía sea "#version 330 core";
     - llaves balanceadas;
     - nunca subir al GPU una fuente corrupta (ahorra un ciclo de log del
       driver confuso).
   El lint de esta lección ES el núcleo del ejercicio 01 (allí lo completás
   vos un paso más: in/out matcheados y uniforms duplicados). Acá está la
   versión base, completa, y cómo se usa junto al compilador real del GPU.
    ****************************************************************************/

#include "include/gl_util.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

/* ---------------------------------------------------------------------------
   NÚCLEO TESTEABLE: lint de un fuente GLSL EN C++ (lo usamos ANTES de
   pegarle al driver; el esqueleto del ejercicio 01 lo completa).
 ------------------------------------------------------------------------- */
struct ReporteLint {
    bool ok = false;
    std::vector<std::string> problemas;
};

static bool esVersion(const std::string& linea) {
    // #version 330 core | #version 100 es (la core la usamos en el curso)
    return linea.rfind("#version", 0) == 0;
}

static ReporteLint lintearShader(const char* fuente) {
    ReporteLint r;
    std::istringstream jet(fuente);
    std::string linea;
    int numeroLinea = 0, llaves = 0;
    bool vistoVersion = false;

    while (std::getline(jet, linea)) {
        ++numeroLinea;
        // trim sencillo: sin bordes ni espacios al inicio
        size_t a = linea.find_first_not_of(" \t\r");
        if (a == std::string::npos) continue;         // línea vacía
        std::string core = linea.substr(a);
        if (core.rfind("//", 0) == 0) continue;       // comentario corto

        if (!vistoVersion) {                          // 1ra línea efectiva
            if (!esVersion(core)) {
                r.problemas.push_back("linea " + std::to_string(numeroLinea) +
                                      ": falta #version al inicio");
            } else {
                vistoVersion = true;
                if (core.find(" core") == std::string::npos)
                    r.problemas.push_back("linea " +
                        std::to_string(numeroLinea) + ": #version sin 'core'");
            }
        }
        llaves += (int)std::count(core.begin(), core.end(), '{');
        llaves -= (int)std::count(core.begin(), core.end(), '}');
    }

    if (llaves != 0) {
        r.problemas.push_back("las llaves no cierran (" +
                              std::to_string(llaves) + " sin cerrar)");
    }
    if (!vistoVersion)
        r.problemas.push_back("no hay ningun #version");
    r.ok = r.problemas.empty();
    return r;
}

/* ---------------------------------------------------------------------------
   El camión mínimo de compilación humanizada: devuelve 0 y guarda el log.
   Los errores capturan el mensaje "importante" del driver: línea + causa.
 ------------------------------------------------------------------------- */
static GLuint compilarLogeado(GLenum tipo, const char* fuente,
                              std::string& logDeError) {
    ReporteLint lint = lintearShader(fuente);
    logDeError.clear();
    if (!lint.ok) {
        // EARLY EXIT: con el lint roto NI INTENTAMOS el GPU (ahorra confusión:
        // un shader sin #version compila "por resolución de_Profile" en
        // algunos drivers legacy y después se rompe el link "por nada").
        for (const std::string& p : lint.problemas) logDeError = p + " | ";
        return 0;
    }

    GLuint sh = glCreateShader(tipo);
    glShaderSource(sh, 1, &fuente, nullptr);
    glCompileShader(sh);

    GLint ok = GL_FALSE;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);       // NO OMITIR
    GLint log = 0;
    glGetShaderiv(sh, GL_INFO_LOG_LENGTH, &log);
    if (log > 0) {
        logDeError.resize((size_t)log);
        glGetShaderInfoLog(sh, log, nullptr, logDeError.data());
    }
    if (ok != GL_TRUE) {
        glDeleteShader(sh);
        return 0;
    }
    return sh;   // el caller lo attacha y lo borra (lección 02/03 helpers)
}

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL_Init fallo: " << SDL_GetError() << "\n";
        return 1;
    }

    glu::Contexto c = glu::abrir("Modulo 20 - L04 compilacion y logs", 640, 480);
    if (c.contexto == nullptr) { SDL_Quit(); return 1; }

    // Demostración: un buen shader y dos MALOS con logs comparables.
    const char* vsBien = "#version 330 core\n"
        "layout(location=0) in vec2 aPos;\n"
        "void main() { gl_Position = vec4(aPos, 0.0, 1.0); }\n";

    std::string logBien, logMal;
    GLuint sh = compilarLogeado(GL_VERTEX_SHADER, vsBien, logBien);
    std::cout << "buen shader -> nombre " << sh
              << " | log: '" << logBien << "'\n";
    if (sh != 0) glDeleteShader(sh);

    const char* vsSinVersion = "layout(location=0) in vec2 aPos;\n"
        "void main() { gl_Position = vec4(aPos, 0.0, 1.0); }\n";
    sh = compilarLogeado(GL_VERTEX_SHADER, vsSinVersion, logMal);
    std::cout << "sin #version -> nombre " << sh
              << " | lint: '" << logMal << "'\n";

    const char* vsTriple = "#version 330 core\nvoid main() {\n";
    sh = compilarLogeado(GL_VERTEX_SHADER, vsTriple, logMal);
    std::cout << "llaver sucio -> nombre " << sh
              << " | lint: '" << logMal << "'\n";

    glu::cerrar(c);
    SDL_Quit();
    return 0;
    /* Siguiente lección (05): use el log del LINT para hot-reload — sin
       reinicio del programa; también los UBO (uniform BLOques std140). */
}
