/* =============================================================================
   MÓDULO 20 - EJERCICIO 01: Lint de shaders (versión con in/out y uniforms)
   =============================================================================
   DIFICULTAD: Baja-media
   APOYO: Lección 04 (compilación, linkeo, logs y el lint base que mostró
          la lección).

   CONSIGNA:
   La lección 04 lintea #version y llaves de un fuente GLSL. Completá el
   lint MEJORADO que atrapa los errores que después explotan en el LINK:

     struct Reporte { bool ok; std::vector<std::string> problemas; }

     1. `primeraLineaEfectiva(fs)`: la primera línea que no es vacía ni
        comentario "//". De VOLVERELS "#version 330 core" o similar.
     2. `lintearShader(fuente, esVertex)`:
        - si la primera línea efectiva no es "#version ...": problema
          "sin #version al inicio" (piggyback NÚMERO de la línea over).
        - si el #version no contiene " core": problema "#version sin core".
        - llaves desbalanceadas (base de la lección).
        - SOLO en fragment shader (esVertex == false): si NO aparece
          "void main" (ningun şirinº), problema "falta void main()".
     3. `linteandoPar(vsFuente, fsFuente)`:
        - lintea ambos (con esVertex correcto):
        - EXTRA del link: los OUT del VS (líneas "out Tipo nombre;") deben
          tener MATCH con los IN del FS ("in Tipo nombre;") por nombre y
          tipo; cada par que matchea se descarta; los que no:
           - un out del FS sin in del VS <- problema
             "FS dice 'in NOMBRE' que el VS no provee"
           - un out del VS sin in del FS: NO es problema (el varying
             no consumido es válido: IGUAL que vertex-only output).
        - extra objeto: los detalles de los problemas sintácticos del FS
          y del VS (por otras pruebas) VENE en ese orden: VS primero,
          luego FS, luego los cross-check de link. (reporte finalización)

   TESTS (no tocar). COMPILACIÓN Y CORRIDA:
     g++ -std=c++20 -Wall -Wextra ejercicio_01_shader_lint.cpp -o ej && ./ej
   (Sin GL: el lint es C++ puro: la lección muestra cuándo se conecta).
   ============================================================================ */
#include "../../include/test_framework.hpp"
#include <string>
#include <vector>

struct Reporte {
    bool ok = true;
    std::vector<std::string> problemas;
};

/* ------------------------------ TU CÓDIGO AQUÍ ---------------------------- */

// TODO 1: primera línea efectiva (no vacía, no comentario //)
static std::string primeraLineaEfectiva(const std::string& fuente) {
    (void)fuente;
    return "";   // stub: falla el test de versión
}

// TODO 2: lint de UN shader (línea de version, core, llaves, main del fs)
static Reporte lintearShader(const std::string& fuente, bool esVertex) {
    (void)fuente; (void)esVertex;
    return Reporte{};   // stub vacío: ok en falso: fallan los tests
}

// TODO 3: lint del PAR VS+FS: mismatches in/out de multiples
static Reporte linteandoPar(const std::string& vs, const std::string& fs) {
    (void)vs; (void)fs;
    return Reporte{};
}

/* --------------------------- TESTS (no tocar) -------------------------- */
static const char* VS_OK = "#version 330 core\n"
    "layout(location=0) in vec2 aPos;\n"
    "out vec2 vUV;\n"
    "void main() { vUV = aPos * 0.5 + 0.5; gl_Position = vec4(aPos, 0.0, 1.0); }\n";

static const char* FS_OK = "  \n"
    "// comentario antes de la version (permitido: es lineal)\n"
    "#version 330 core\n"
    "in vec2 vUV;\n"
    "out vec4 FragColor;\n"
    "void main() { FragColor = vec4(vUV, 0.0, 1.0); }\n";

static const char* FS_SIN_VERSION = "in vec2 vUV;\n"
    "out vec4 FragColor;\n"
    "void main() { FragColor = vec4(vUV, 0, 1); }\n";

static const char* FS_MAL_IN = "#version 330 core\n"
    "in vec3 vOtraCosa;\n"           // el VS solo provee vUV (vec2)
    "out vec4 FragColor;\n"
    "void main() { FragColor = vec4(0.0); }\n";

TEST_CASE(PrimeraLineaEfectiva) {
    ASSERT_EQ(primeraLineaEfectiva(VS_OK), std::string("#version 330 core"));
    // lee el FS_OK: primera línea es "  " (blanco) ignorada;
    // la segunda es comentario: IGNORA. La tercera es #version:
    ASSERT_EQ(primeraLineaEfectiva(FS_OK), std::string("#version 330 core"));
}

TEST_CASE(LintBasico) {
    Reporte rVs = lintearShader(VS_OK, true);
    ASSERT_TRUE(rVs.ok);

    Reporte rFs = lintearShader(FS_OK, false);
    ASSERT_TRUE(rFs.ok);

    Reporte mal = lintearShader(FS_SIN_VERSION, false);
    ASSERT_FALSE(mal.ok);
    ASSERT_EQ(mal.problemas.size(), (size_t)1);
    ASSERT_TRUE(mal.problemas[0].find("sin #version") != std::string::npos);
}

TEST_CASE(FaltaMainDelFS) {
    const std::string fs = "#version 330 core\n"
        "in vec2 vUV; out vec4 FragColor;\n";
    Reporte r = lintearShader(fs, false);
    ASSERT_FALSE(r.ok);
    ASSERT_TRUE(r.problemas[0].find("falta void main") != std::string::npos);
    // en el vertex shader main podría faltar... convención del curso: SI
    Reporte rv = lintearShader("#version 330 core\n", true);
    ASSERT_TRUE(rv.ok);   // el VS sin main NO se marca (regl del enunciado)
}

TEST_CASE(LinkMatchInOut) {
    Reporte bien = linteandoPar(VS_OK, FS_OK);
    ASSERT_TRUE(bien.ok);

    Reporte mal = linteandoPar(VS_OK, FS_MAL_IN);
    ASSERT_FALSE(mal.ok);
    bool hayIn = false;
    for (const std::string& p : mal.problemas)
        if (p.find("vOtraCosa") != std::string::npos) hayIn = true;
    ASSERT_TRUE(hayIn);
}

int main() { return RUN_ALL_TESTS(); }
