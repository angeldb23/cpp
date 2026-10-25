/* =============================================================================
   MÓDULO 20 - SOLUCIÓN 01: Lint de shaders (versión con in/out y uniforms)
   =============================================================================
   DIFICULTAD: Baja-media
   APOYO: Lección 04. Resuelve el ejercicio_01_shader_lint.cpp.

   La idea del lint (C++ puro antes de llamar al driver):
     - atrapa los errores tontos (versión, main del FS, llaves) sin gastar
       un ciclo de compilación GL;
     - matchea out/in por nombre y tipo (la causa típica del LINK error que
       parece venir "de la nada" en el log del driver).
   ============================================================================ */
#include "../../include/test_framework.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

struct Reporte {
    bool ok = true;
    std::vector<std::string> problemas;
};

// trims exteriores
static std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r");
    return s.substr(a, b - a + 1);
}

static bool esVersion(const std::string& linea) {
    return linea.rfind("#version", 0) == 0;
}

// extrae "Tipo Nombre" de una línea que empieza con prefijo ("in " o "out ")
static void extraerVariables(const std::string& fuente, const std::string& prefijo,
                             std::vector<std::pair<std::string, std::string>>& out) {
    std::istringstream jet(fuente);
    std::string linea;
    while (std::getline(jet, linea)) {
        std::string core = trim(linea);
        if (core.rfind(prefijo, 0) != 0) continue;
        core = core.substr(prefijo.size());
        // "Tipo Nombre;" con espacios variables: parse sencillo
        std::istringstream pasa(core);
        std::string tipo, nombre;
        if (pasa >> tipo) {
            if (tipo.ends_with(';')) tipo.pop_back();
            if (pasa >> nombre) {
                size_t p = nombre.find(';');
                if (p != std::string::npos) nombre = nombre.substr(0, p);
                out.push_back({tipo, nombre});
            }
        }
    }
}

// ¿la línea declara main()? (la detección exacta "void main" del curso)
static bool lineaTieneMain(const std::string& linea) {
    return linea.find("void main") != std::string::npos;
}

// TODO 1
static std::string primeraLineaEfectiva(const std::string& fuente) {
    std::istringstream jet(fuente);
    std::string linea;
    while (std::getline(jet, linea)) {
        std::string core = trim(linea);
        if (core.empty()) continue;
        if (core.rfind("//", 0) == 0) continue;
        return core;
    }
    return "";
}

// TODO 2
static Reporte lintearShader(const std::string& fuente, bool esVertex) {
    (void)esVertex;   // regla del enunciado: el VS sin main NO se marca
    Reporte r;

    std::string primera = primeraLineaEfectiva(fuente);
    if (primera.empty() || !esVersion(primera)) {
        r.problemas.push_back("sin #version al inicio");
    } else if (primera.find(" core") == std::string::npos) {
        r.problemas.push_back("#version sin core");
    }

    int llaves = 0;
    std::istringstream jet(fuente);
    std::string linea;
    bool mainVisto = false;
    while (std::getline(jet, linea)) {
        // comentarios de línea: VAN al conteo de llaves (superfuncion { } en
        // un comentario es MAGIA DEL DRIVER de la lección 04: no cuentan)
        std::string sinComentario = linea;
        size_t pos = sinComentario.find("//");
        if (pos != std::string::npos) sinComentario = sinComentario.substr(0, pos);
        llaves += (int)std::count(sinComentario.begin(), sinComentario.end(), '{');
        llaves -= (int)std::count(sinComentario.begin(), sinComentario.end(), '}');
        if (lineaTieneMain(sinComentario)) mainVisto = true;
    }
    if (llaves != 0) {
        r.problemas.push_back("llaves sin cerrar (" + std::to_string(llaves) + ")");
    }
    if (!esVertex && !mainVisto) {
        r.problemas.push_back("falta void main() del fragment shader");
    }
    r.ok = r.problemas.empty();
    return r;
}

// TODO 3
static Reporte linteandoPar(const std::string& vs, const std::string& fs) {
    Reporte r;

    Reporte rv = lintearShader(vs, true);
    for (const std::string& p : rv.problemas) r.problemas.push_back("VS: " + p);
    Reporte rf = lintearShader(fs, false);
    for (const std::string& p : rf.problemas) r.problemas.push_back("FS: " + p);

    std::vector<std::pair<std::string, std::string>> outsVs, insFs;
    extraerVariables(vs, "out ", outsVs);
    extraerVariables(fs, "in ",  insFs);

    // cada in del FS debe estar provisto por un out del VS (nombre == y tipo ==)
    for (const auto& [tipoIn, nombreIn] : insFs) {
        bool provisto = false;
        for (const auto& [tipoOut, nombreOut] : outsVs) {
            if (nombreOut == nombreIn && tipoOut == tipoIn) { provisto = true; break; }
        }
        if (!provisto) {
            r.problemas.push_back("FS dice 'in " + nombreIn +
                                  "' que el VS no provee con el mismo tipo");
        }
    }
    r.ok = r.problemas.empty();
    return r;
}

/* --------------------------- TESTS (no tocar) -------------------------- */
static const char* VS_OK = "#version 330 core\n"
    "layout(location=0) in vec2 aPos;\n"
    "out vec2 vUV;\n"
    "void main() { vUV = aPos * 0.5 + 0.5; gl_Position = vec4(aPos, 0.0, 1.0); }\n";

static const char* FS_OK = "  \n"
    "// comentario antes de la version (permitido)\n"
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
    Reporte rv = lintearShader("#version 330 core\n", true);
    ASSERT_TRUE(rv.ok);   // el VS sin main NO se marca (regla del enunciado)
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
