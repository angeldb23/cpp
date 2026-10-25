/*
 * MÓDULO 28: Estudio de Código Real — Arquitectura de Librerías
 * LECCIÓN 01: Criterios de Selección de Dependencias
 *
 * TEMAS:
 * - Madurez y estabilidad (versión, semver, changelog)
 * - Licencia (MIT, BSD, Apache 2.0, LGPL, GPL — implicaciones)
 * - Dependencias transitivas (cuántas, cuáles, versión pinneada)
 * - Estilo C++ (modern C++17/20 vs legacy, templates, RAII, noexcept)
 * - Actividad del proyecto (commits, issues, PRs, releases, bus factor)
 * - Calidad: tests, CI, sanitizers, fuzzing, docs, ejemplos
 * - ABI stability y versionado
 * - Soporte de plataformas (Linux, Windows, macOS, WebAssembly, móviles)
 * - Tamaño: header-only, single-header, biblioteca compilada
 * - Integración: CMake FetchContent, pkg-config, vcpkg, Conan, submodule
 *
 * CHECKLIST DE EVALUACIÓN (para cada candidata):
 */

#pragma once

#include <string>
#include <vector>
#include <optional>

namespace modulo28 {

// ============================================================================
// ESTRUCTURAS DE EVALUACIÓN
// ============================================================================

enum class Licencia {
    MIT,
    BSD_2_Clause,
    BSD_3_Clause,
    Apache_2_0,
    LGPL_2_1,
    LGPL_3_0,
    GPL_2_0,
    GPL_3_0,
    Custom,
    Desconocida
};

enum class Madurez {
    Experimental,    // 0.x, pre-1.0, breaking changes frecuentes
    Estable,         // 1.x+, semver respetado, releases regulares
    Madura,          // 2.x+, años en producción, adopción amplia
    Legado           // Sin mantenimiento activo, solo bugfixes críticos
};

enum class EstiloCPP {
    Legacy_C,        // C puro, sin RAII, manual memory management
    Cpp98_03,        // C++98/03, raw pointers, sin move semantics
    Cpp11_14,        // C++11/14, smart pointers, move, lambda
    Modern_Cpp17,    // C++17, structured bindings, if constexpr, std::optional
    Modern_Cpp20,    // C++20, concepts, modules, ranges, coroutines
    Bleeding_Edge    // C++23+, experimental features
};

struct DependenciaTransitiva {
    std::string nombre;
    std::string version;
    bool esHeaderOnly;
    Licencia licencia;
    // Para detectar conflictos de licencia en cadena
};

struct CriteriosEvaluacion {
    std::string nombre;
    std::string version;
    std::string repo_url;
    
    Madurez madurez = Madurez::Experimental;
    Licencia licencia = Licencia::Desconocida;
    EstiloCPP estilo = EstiloCPP::Legacy_C;
    
    std::vector<DependenciaTransitiva> dependencias_transitivas;
    
    // Métricas de actividad (últimos 12 meses)
    int commits_ultimo_anio = 0;
    int contributors_activos = 0;
    int issues_abiertos = 0;
    int issues_cerrados = 0;
    int prs_abiertos = 0;
    int prs_mergeados = 0;
    std::optional<std::string> ultimo_release; // fecha
    bool tiene_ci = false;
    bool usa_sanitizers = false;
    bool tiene_fuzzing = false;
    bool tiene_docs = false;
    bool tiene_ejemplos = false;
    
    // Compatibilidad
    bool soporta_linux = true;
    bool soporta_windows = true;
    bool soporta_macos = true;
    bool soporta_wasm = false;
    bool soporta_android = false;
    bool soporta_ios = false;
    
    // ABI
    bool abi_estable = false;          // Promesa de ABI stable
    std::string versionado_abi;        // ej: "semver", "soname", "ninguno"
    
    // Build system
    bool tiene_cmake = false;
    bool tiene_pkgconfig = false;
    bool fetchcontent_listo = false;   // Tiene CMakeLists.txt compatible FetchContent
    bool es_header_only = false;
    bool es_single_header = false;
    
    // Testing
    int num_tests = 0;
    double cobertura_estimada = 0.0;   // 0.0 - 1.0
    
    // Scoring ponderado (0-100 cada uno)
    int score_madurez = 0;
    int score_licencia = 0;
    int score_dependencias = 0;
    int score_estilo = 0;
    int score_actividad = 0;
    int score_plataformas = 0;
    int score_abi = 0;
    int score_build = 0;
    int score_testing = 0;
    int score_total = 0;
};

// ============================================================================
// FUNCIONES DE EVALUACIÓN
// ============================================================================

Licencia detectar_licencia(const std::string& repo_path);
Madurez evaluar_madurez(const std::string& version, const std::string& repo_path);
EstiloCPP evaluar_estilo_cpp(const std::string& repo_path);
std::vector<DependenciaTransitiva> analizar_dependencias(const std::string& repo_path);
int calcular_score_madurez(const CriteriosEvaluacion& c);
int calcular_score_licencia(const CriteriosEvaluacion& c);
int calcular_score_dependencias(const CriteriosEvaluacion& c);
int calcular_score_estilo(const CriteriosEvaluacion& c);
int calcular_score_actividad(const CriteriosEvaluacion& c);
int calcular_score_plataformas(const CriteriosEvaluacion& c);
int calcular_score_abi(const CriteriosEvaluacion& c);
int calcular_score_build(const CriteriosEvaluacion& c);
int calcular_score_testing(const CriteriosEvaluacion& c);

CriteriosEvaluacion evaluar_libreria(const std::string& nombre, const std::string& repo_path);
void imprimir_reporte(const CriteriosEvaluacion& c);
bool recomendar_adopcion(const CriteriosEvaluacion& c, std::string& razon);

// ============================================================================
// TABLA DE COMPATIBILIDAD DE LICENCIAS (simplificada)
// ============================================================================

/*
 * Tu proyecto -> Licencia candidata -> ¿Compatible?
 * 
 * MIT/BSD/Apache 2.0 (permisivas) -> TODO: SÍ
 * LGPL (dinámico) -> MIT/BSD/Apache/LGPL/GPL: SÍ (si linking dinámico)
 * LGPL (estático) -> Requiere objeto relinkable o excepción
 * GPL -> Solo GPL compatible (contamina todo el binario)
 * 
 * REGLAS PRÁCTICAS:
 * - Motor propietario/cerrado: SOLO MIT, BSD, Apache 2.0, zlib, Unlicense
 * - Motor open source (GPL): Cualquier licencia compatible GPL
 * - Motor open source (MIT/BSD/Apache): MIT, BSD, Apache, LGPL (dinámico)
 * - LGPL estático en proyecto cerrado: EVITAR (requiere proporcionar .o para relink)
 */

inline bool licencia_compatible_proyecto_cerrado(Licencia l) {
    switch (l) {
        case Licencia::MIT:
        case Licencia::BSD_2_Clause:
        case Licencia::BSD_3_Clause:
        case Licencia::Apache_2_0:
            return true;
        case Licencia::LGPL_2_1:
        case Licencia::LGPL_3_0:
            return false; // Solo si linking dinámico + proveer objetos
        case Licencia::GPL_2_0:
        case Licencia::GPL_3_0:
            return false; // Contamina todo
        default:
            return false; // Desconocida = riesgo
    }
}

inline bool licencia_compatible_gpl(Licencia l) {
    // GPLv3 compatible con: GPLv3, LGPLv3, Apache 2.0, MIT, BSD
    // GPLv2 compatible con: GPLv2, LGPLv2, MIT, BSD (NO Apache 2.0 sin cláusula)
    switch (l) {
        case Licencia::MIT:
        case Licencia::BSD_2_Clause:
        case Licencia::BSD_3_Clause:
        case Licencia::Apache_2_0:
        case Licencia::LGPL_2_1:
        case Licencia::LGPL_3_0:
        case Licencia::GPL_2_0:
        case Licencia::GPL_3_0:
            return true;
        default:
            return false;
    }
}

// ============================================================================
// EJEMPLO DE USO
// ============================================================================

/*
int main() {
    // Evaluar GLM
    auto glm = evaluar_libreria("GLM", "/path/to/glm");
    imprimir_reporte(glm);
    
    std::string razon;
    if (recomendar_adopcion(glm, razon)) {
        std::cout << "RECOMENDADA: " << razon << "\n";
    } else {
        std::cout << "NO RECOMENDADA: " << razon << "\n";
    }
    
    return 0;
}
*/

} // namespace modulo28