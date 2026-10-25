/*
 * MÓDULO 28: Estudio de Código Real — Arquitectura de Librerías
 * LECCIÓN 01: Criterios de Selección de Dependencias - Implementación
 */

#include "leccion_01_criterios_seleccion.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <regex>
#include <algorithm>

namespace modulo28 {

// ============================================================================
// DETECCIÓN DE LICENCIA
// ============================================================================

Licencia detectar_licencia(const std::string& repo_path) {
    namespace fs = std::filesystem;
    
    std::vector<std::string> license_files = {
        "LICENSE", "LICENSE.txt", "LICENSE.md", "COPYING", "COPYING.txt",
        "LICENSE.MIT", "LICENSE.BSD", "LICENSE.APACHE"
    };
    
    for (const auto& f : license_files) {
        fs::path p = fs::path(repo_path) / f;
        if (fs::exists(p)) {
            std::ifstream file(p);
            std::string content((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
            std::transform(content.begin(), content.end(), content.begin(), ::tolower);
            
            if (content.find("mit license") != std::string::npos ||
                content.find("permission is hereby granted") != std::string::npos) {
                return Licencia::MIT;
            }
            if (content.find("apache license") != std::string::npos &&
                content.find("version 2.0") != std::string::npos) {
                return Licencia::Apache_2_0;
            }
            if (content.find("bsd 3-clause") != std::string::npos ||
                content.find("bsd-3-clause") != std::string::npos ||
                (content.find("redistribution and use in source and binary forms") != std::string::npos &&
                 content.find("3. neither the name") != std::string::npos)) {
                return Licencia::BSD_3_Clause;
            }
            if (content.find("bsd 2-clause") != std::string::npos ||
                content.find("bsd-2-clause") != std::string::npos ||
                (content.find("redistribution and use in source and binary forms") != std::string::npos &&
                 content.find("neither the name") == std::string::npos)) {
                return Licencia::BSD_2_Clause;
            }
            if (content.find("gnu lesser general public license") != std::string::npos) {
                if (content.find("version 3") != std::string::npos) return Licencia::LGPL_3_0;
                return Licencia::LGPL_2_1;
            }
            if (content.find("gnu general public license") != std::string::npos) {
                if (content.find("version 3") != std::string::npos) return Licencia::GPL_3_0;
                return Licencia::GPL_2_0;
            }
        }
    }
    return Licencia::Desconocida;
}

// ============================================================================
// EVALUACIÓN DE MADUREZ
// ============================================================================

Madurez evaluar_madurez(const std::string& version, const std::string& repo_path) {
    // Parsear versión semver
    std::regex semver(R"(^v?(\d+)\.(\d+)\.(\d+))");
    std::smatch match;
    
    int major = 0, minor = 0, patch = 0;
    if (std::regex_search(version, match, semver)) {
        major = std::stoi(match[1]);
        minor = std::stoi(match[2]);
        patch = std::stoi(match[3]);
    }
    
    // Verificar tags en git para historia
    namespace fs = std::filesystem;
    int tag_count = 0;
    // En implementación real: `git tag -l` y contar
    
    if (major == 0) return Madurez::Experimental;
    if (major == 1) return Madurez::Estable;
    if (major >= 2) return Madurez::Madura;
    
    return Madurez::Experimental;
}

// ============================================================================
// EVALUACIÓN DE ESTILO C++
// ============================================================================

EstiloCPP evaluar_estilo_cpp(const std::string& repo_path) {
    namespace fs = std::filesystem;
    
    bool has_concepts = false;
    bool has_modules = false;
    bool has_coroutines = false;
    bool has_ranges = false;
    bool has_constexpr = false;
    bool has_if_constexpr = false;
    bool has_structured_bindings = false;
    bool has_optional = false;
    bool has_smart_ptrs = false;
    bool has_move = false;
    bool has_lambda = false;
    bool is_c_only = true;
    
    for (auto& p : fs::recursive_directory_iterator(repo_path)) {
        if (!p.is_regular_file()) continue;
        auto ext = p.path().extension().string();
        if (ext != ".h" && ext != ".hpp" && ext != ".hxx" && 
            ext != ".c" && ext != ".cpp" && ext != ".cxx" && ext != ".cc") continue;
        
        std::ifstream file(p.path());
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        
        if (ext == ".c" || content.find("#include <std") != std::string::npos) {
            // Podría ser C
        } else {
            is_c_only = false;
        }
        
        // C++20 features
        if (content.find("concept ") != std::string::npos) has_concepts = true;
        if (content.find("module;") != std::string::npos || content.find("export module") != std::string::npos) has_modules = true;
        if (content.find("co_await") != std::string::npos || content.find("co_yield") != std::string::npos) has_coroutines = true;
        if (content.find("std::ranges::") != std::string::npos || content.find("views::") != std::string::npos) has_ranges = true;
        
        // C++17 features
        if (content.find("constexpr") != std::string::npos) has_constexpr = true;
        if (content.find("if constexpr") != std::string::npos) has_if_constexpr = true;
        if (content.find("auto [") != std::string::npos) has_structured_bindings = true;
        if (content.find("std::optional") != std::string::npos) has_optional = true;
        
        // C++11/14 features
        if (content.find("std::unique_ptr") != std::string::npos ||
            content.find("std::shared_ptr") != std::string::npos ||
            content.find("std::make_unique") != std::string::npos ||
            content.find("std::make_shared") != std::string::npos) has_smart_ptrs = true;
        if (content.find("std::move") != std::string::npos || content.find("&&") != std::string::npos) has_move = true;
        if (content.find("[") != std::string::npos && content.find("]") != std::string::npos && 
            content.find("()") != std::string::npos) has_lambda = true; // heuristic
    }
    
    if (is_c_only) return EstiloCPP::Legacy_C;
    if (has_concepts || has_modules || has_coroutines) return EstiloCPP::Modern_Cpp20;
    if (has_ranges || has_if_constexpr || has_structured_bindings || has_optional) return EstiloCPP::Modern_Cpp17;
    if (has_smart_ptrs || has_move || has_lambda || has_constexpr) return EstiloCPP::Cpp11_14;
    return EstiloCPP::Cpp98_03;
}

// ============================================================================
// ANÁLISIS DE DEPENDENCIAS TRANSITIVAS
// ============================================================================

std::vector<DependenciaTransitiva> analizar_dependencias(const std::string& repo_path) {
    std::vector<DependenciaTransitiva> deps;
    namespace fs = std::filesystem;
    
    // Buscar CMakeLists.txt, conanfile.txt, vcpkg.json, package.json, etc.
    fs::path cmake = fs::path(repo_path) / "CMakeLists.txt";
    if (fs::exists(cmake)) {
        std::ifstream file(cmake);
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        
        // Buscar find_package, FetchContent_Declare, add_subdirectory
        std::regex find_pkg(R"(find_package\s*\(\s*([A-Za-z0-9_]+))");
        std::regex fetchcontent(R"(FetchContent_Declare\s*\(\s*([A-Za-z0-9_]+))");
        std::regex add_subdir(R"(add_subdirectory\s*\(\s*([A-Za-z0-9_/\-]+))");
        
        std::smatch match;
        std::string::const_iterator start = content.cbegin();
        while (std::regex_search(start, content.cend(), match, find_pkg)) {
            DependenciaTransitiva d;
            d.nombre = match[1];
            d.version = "desconocida";
            d.esHeaderOnly = false;
            d.licencia = Licencia::Desconocida;
            deps.push_back(d);
            start = match.suffix().first;
        }
        start = content.begin();
        while (std::regex_search(start, content.cend(), match, fetchcontent)) {
            DependenciaTransitiva d;
            d.nombre = match[1];
            d.version = "desconocida";
            d.esHeaderOnly = false;
            d.licencia = Licencia::Desconocida;
            deps.push_back(d);
            start = match.suffix().first;
        }
    }
    
    // vcpkg.json
    fs::path vcpkg = fs::path(repo_path) / "vcpkg.json";
    if (fs::exists(vcpkg)) {
        // Parsear JSON para dependencias
    }
    
    return deps;
}

// ============================================================================
// SCORING
// ============================================================================

int calcular_score_madurez(const CriteriosEvaluacion& c) {
    switch (c.madurez) {
        case Madurez::Madura: return 100;
        case Madurez::Estable: return 75;
        case Madurez::Experimental: return 40;
        case Madurez::Legado: return 20;
    }
    return 0;
}

int calcular_score_licencia(const CriteriosEvaluacion& c) {
    // Para proyecto propietario/cerrado
    switch (c.licencia) {
        case Licencia::MIT:
        case Licencia::BSD_2_Clause:
        case Licencia::BSD_3_Clause:
        case Licencia::Apache_2_0:
            return 100;
        case Licencia::LGPL_2_1:
        case Licencia::LGPL_3_0:
            return 60; // Requiere linking dinámico
        case Licencia::GPL_2_0:
        case Licencia::GPL_3_0:
            return 10; // Solo si todo el proyecto es GPL
        default:
            return 0; // Desconocida = riesgo legal
    }
}

int calcular_score_dependencias(const CriteriosEvaluacion& c) {
    int score = 100;
    for (const auto& d : c.dependencias_transitivas) {
        if (!d.esHeaderOnly) score -= 10; // Cada dep compilada resta
        if (d.licencia == Licencia::GPL_2_0 || d.licencia == Licencia::GPL_3_0) score -= 30;
        else if (d.licencia == Licencia::LGPL_2_1 || d.licencia == Licencia::LGPL_3_0) score -= 15;
    }
    return std::max(0, score);
}

int calcular_score_estilo(const CriteriosEvaluacion& c) {
    switch (c.estilo) {
        case EstiloCPP::Modern_Cpp20: return 100;
        case EstiloCPP::Modern_Cpp17: return 90;
        case EstiloCPP::Cpp11_14: return 70;
        case EstiloCPP::Cpp98_03: return 40;
        case EstiloCPP::Legacy_C: return 50; // C bien hecho puede ser bueno
    }
    return 0;
}

int calcular_score_actividad(const CriteriosEvaluacion& c) {
    int score = 0;
    // Commits último año (máx 30 pts)
    score += std::min(30, c.commits_ultimo_anio / 10);
    // Contributors (máx 20 pts)
    score += std::min(20, c.contributors_activos * 2);
    // Issues ratio (máx 20 pts)
    if (c.issues_abiertos + c.issues_cerrados > 0) {
        double ratio = double(c.issues_cerrados) / (c.issues_abiertos + c.issues_cerrados);
        score += int(ratio * 20);
    }
    // CI/Sanitizers/Fuzzing (máx 30 pts)
    if (c.tiene_ci) score += 10;
    if (c.usa_sanitizers) score += 10;
    if (c.tiene_fuzzing) score += 10;
    return std::min(100, score);
}

int calcular_score_plataformas(const CriteriosEvaluacion& c) {
    int score = 0;
    if (c.soporta_linux) score += 20;
    if (c.soporta_windows) score += 20;
    if (c.soporta_macos) score += 20;
    if (c.soporta_wasm) score += 15;
    if (c.soporta_android) score += 15;
    if (c.soporta_ios) score += 10;
    return score;
}

int calcular_score_abi(const CriteriosEvaluacion& c) {
    if (c.abi_estable) return 100;
    if (c.versionado_abi == "semver") return 70;
    if (c.versionado_abi == "soname") return 60;
    return 30; // Sin promesa de ABI
}

int calcular_score_build(const CriteriosEvaluacion& c) {
    int score = 0;
    if (c.tiene_cmake) score += 30;
    if (c.fetchcontent_listo) score += 30;
    if (c.tiene_pkgconfig) score += 20;
    if (c.es_header_only || c.es_single_header) score += 20; // Fácil integración
    return score;
}

int calcular_score_testing(const CriteriosEvaluacion& c) {
    int score = 0;
    if (c.num_tests > 0) score += 30;
    if (c.cobertura_estimada > 0.8) score += 40;
    else if (c.cobertura_estimada > 0.5) score += 25;
    else if (c.cobertura_estimada > 0.0) score += 10;
    if (c.tiene_ejemplos) score += 15;
    if (c.tiene_docs) score += 15;
    return std::min(100, score);
}

// ============================================================================
// EVALUACIÓN COMPLETA
// ============================================================================

CriteriosEvaluacion evaluar_libreria(const std::string& nombre, const std::string& repo_path) {
    CriteriosEvaluacion c;
    c.nombre = nombre;
    // En implementación real: obtener version desde CMakeLists.txt, git describe, etc.
    c.version = "desconocida";
    c.repo_url = repo_path;
    
    c.licencia = detectar_licencia(repo_path);
    c.madurez = evaluar_madurez(c.version, repo_path);
    c.estilo = evaluar_estilo_cpp(repo_path);
    c.dependencias_transitivas = analizar_dependencias(repo_path);
    
    // Calcular scores
    c.score_madurez = calcular_score_madurez(c);
    c.score_licencia = calcular_score_licencia(c);
    c.score_dependencias = calcular_score_dependencias(c);
    c.score_estilo = calcular_score_estilo(c);
    c.score_actividad = calcular_score_actividad(c);
    c.score_plataformas = calcular_score_plataformas(c);
    c.score_abi = calcular_score_abi(c);
    c.score_build = calcular_score_build(c);
    c.score_testing = calcular_score_testing(c);
    
    // Ponderación total
    c.score_total = 
        c.score_madurez * 0.20 +
        c.score_licencia * 0.20 +
        c.score_dependencias * 0.10 +
        c.score_estilo * 0.10 +
        c.score_actividad * 0.15 +
        c.score_plataformas * 0.05 +
        c.score_abi * 0.05 +
        c.score_build * 0.10 +
        c.score_testing * 0.05;
    
    return c;
}

void imprimir_reporte(const CriteriosEvaluacion& c) {
    std::cout << "========================================\n";
    std::cout << "REPORTE DE EVALUACIÓN: " << c.nombre << " " << c.version << "\n";
    std::cout << "========================================\n";
    std::cout << "Licencia: " << static_cast<int>(c.licencia) << "\n";
    std::cout << "Madurez: " << static_cast<int>(c.madurez) << "\n";
    std::cout << "Estilo C++: " << static_cast<int>(c.estilo) << "\n";
    std::cout << "Deps transitivas: " << c.dependencias_transitivas.size() << "\n";
    std::cout << "----------------------------------------\n";
    std::cout << "Score Madurez:        " << c.score_madurez << "/100 (20%)\n";
    std::cout << "Score Licencia:       " << c.score_licencia << "/100 (20%)\n";
    std::cout << "Score Dependencias:   " << c.score_dependencias << "/100 (10%)\n";
    std::cout << "Score Estilo C++:     " << c.score_estilo << "/100 (10%)\n";
    std::cout << "Score Actividad:      " << c.score_actividad << "/100 (15%)\n";
    std::cout << "Score Plataformas:    " << c.score_plataformas << "/100 (5%)\n";
    std::cout << "Score ABI:            " << c.score_abi << "/100 (5%)\n";
    std::cout << "Score Build:          " << c.score_build << "/100 (10%)\n";
    std::cout << "Score Testing:        " << c.score_testing << "/100 (5%)\n";
    std::cout << "----------------------------------------\n";
    std::cout << "SCORE TOTAL:          " << c.score_total << "/100\n";
    std::cout << "========================================\n";
}

bool recomendar_adopcion(const CriteriosEvaluacion& c, std::string& razon) {
    if (c.score_total >= 75) {
        razon = "Score total >= 75: candidata fuerte";
        return true;
    }
    if (c.score_licencia < 50) {
        razon = "Licencia incompatible con proyecto cerrado";
        return false;
    }
    if (c.score_madurez < 40) {
        razon = "Madurez insuficiente (experimental/legacy)";
        return false;
    }
    if (c.score_dependencias < 50) {
        razon = "Demasiadas dependencias transitivas problemáticas";
        return false;
    }
    razon = "Score total " + std::to_string(c.score_total) + " < 75: evaluar alternativas";
    return false;
}

} // namespace modulo28


// ============================================================================
// MAIN PARA TESTING MANUAL
// ============================================================================

#ifdef MODULO28_LECCION01_STANDALONE
int main() {
    using namespace modulo28;
    
    // Test con directorio actual
    CriteriosEvaluacion c = evaluar_libreria("TestLib", ".");
    imprimir_reporte(c);
    
    std::string razon;
    bool ok = recomendar_adopcion(c, razon);
    std::cout << "Recomendación: " << (ok ? "SÍ" : "NO") << " - " << razon << "\n";
    
    return 0;
}
#endif