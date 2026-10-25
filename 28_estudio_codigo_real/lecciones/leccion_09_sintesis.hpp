/*
 * MÓDULO 28: Estudio de Código Real — Arquitectura de Librerías
 * LECCIÓN 09: Síntesis — Patrones Recurrentes, Checklist Evaluación, Informe Técnico
 *
 * TEMAS:
 * - Patrones recurrentes en librerías C/C++ modernas
 * - Checklist unificado de evaluación de dependencias
 * - Trade-offs: header-only vs compiled, C API vs C++, static vs dynamic
 * - Versionado ABI, semver, compatibilidad
 * - Testing: unit, integration, fuzzing, property-based
 * - Build systems: CMake, FetchContent, vcpkg, Conan, pkg-config
 * - CI/CD: sanitizers, coverage, benchmarks
 * - Documentación: API reference, guías, ejemplos, changelog
 * - Informe técnico comparativo: tabla de decisión, 15 min presentation
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>

namespace modulo28_synthesis {

// ============================================================================
// PATRONES RECURRENTES EN LIBRERÍAS MODERNAS
// ============================================================================

/*
 * 1. OPAQUE HANDLES / Pimpl Idiom
 *    - C: typedef struct Foo* FooHandle;  // forward declaration
 *    - C++: class Foo { std::unique_ptr<Impl> pimpl; };
 *    - Beneficio: ABI stable, header limpio, compile-time decoupling
 *    - Ejemplos: SDL_Window*, SDL_GPUDevice*, ImGuiContext*, ma_device*
 * 
 * 2. ARENA / BUMP ALLOCATORS
 *    - Pool grande, allocation = pointer bump, free = reset todo
 *    - Frame allocators: reset cada frame (ImGui, game engines)
 *    - Scratch buffers: temp allocations sin fragmentation
 *    - Ejemplos: ImGui mem alloc, miniaudio, Box2D block allocator
 * 
 * 3. DATA-ORIENTED DESIGN (DOD)
 *    - SoA (Structure of Arrays) vs AoS (Array of Structures)
 *    - Cache-friendly: procesar arrays contiguos
 *    - SIMD-friendly: mismo operación en múltiples datos
 *    - ECS: componentes en arrays separados
 *    - Ejemplos: Box2D solver arrays, miniaudio ring buffers, GLM SIMD
 * 
 * 4. PROPERTIES / KEY-VALUE SYSTEM
 *    - Sistema unificado para metadata extensible
 *    - SDL_PropertiesID, Vulkan VkObjectSetDebugUtilsObjectNameEXT
 *    - Evita añadir campos a structs públicos
 *    - Serialization-friendly
 * 
 * 5. BACKEND ABSTRACTION / STRATEGY PATTERN
 *    - Interfaz común + implementaciones por plataforma
 *    - SDL: video/audio/input backends
 *    - miniaudio: WASAPI/CoreAudio/ALSA/PulseAudio
 *    - SDL_GPU: Vulkan/Metal/DX12
 *    - Box2D: broad-phase pluggable (SAP, tree, grid)
 * 
 * 6. IMMEDIATE MODE vs RETAINED MODE
 *    - Immediate: reconstruye cada frame (ImGui, Nuklear)
 *    - Retained: scene graph persistente (Qt, GTK, game engines)
 *    - Híbrido: immediate para UI, retained para scene
 * 
 * 6. LOCK-FREE / WAIT-FREE CONCURRENCY
 *    - SPSC ring buffers (audio, job queues)
 *    - Atomic reference counting
 *    - Hazard pointers / epoch-based reclamation
 *    - Ejemplos: miniaudio ring buffer, job systems
 * 
 * 7. CUSTOM ALLOCATORS / MEMORY TRACKING
 *    - Override new/delete, ImGuiMemAlloc, SDL_malloc
 *    - Categories: temp, persistent, streaming, audio, graphics
 *    - Tracking: peak, current, leaks, fragmentation
 *    - Integration con malloc hooks, valgrind, ASan
 * 
 * 8. VERSIONING & ABI STABILITY
 *    - Semver: MAJOR.MINOR.PATCH
 *    - ABI: soname (libfoo.so.1.2.3), symbol versioning
 *    - Compat: forward (nuevo compila con viejo), backward (viejo corre con nuevo)
 *    - Deprecation macros, feature test macros
 * 
 * 9. PLUGIN / EXTENSION ARCHITECTURE
 *    - Dynamic loading: dlopen/LoadLibrary
 *    - Interface versioning: query version, function table
 *    - Hot-reload: unload/reload sin restart
 *    - Ejemplos: ImGui backends, SDL render drivers, audio backends
 * 
 * 10. ERROR HANDLING PATTERNS
 *     - C: return codes + global error string (SDL_GetError)
 *     - C++: exceptions (opt-in), std::expected (C++23), Result<T,E>
 *     - Errno-style: thread-local error state
 *     - Rich errors: context, stack trace, recovery hints
 */

// ============================================================================
// CHECKLIST UNIFICADO DE EVALUACIÓN
// ============================================================================

enum class LicenseType {
    MIT, BSD2, BSD3, Apache2, Zlib, Unlicense,
    LGPL2, LGPL3, GPL2, GPL3, AGPL,
    Custom, Unknown
};

enum class MaturityLevel {
    Experimental,  // 0.x, breaking changes
    Stable,        // 1.x, semver
    Mature,        // 2.x+, años en producción
    Legacy         // maintenance only
};

enum class CppStandard {
    C89, C99, C11, C17, C23,
    Cpp98, Cpp03, Cpp11, Cpp14, Cpp17, Cpp20, Cpp23
};

enum class BuildSystem {
    CMake, Meson, Bazel, Premake, Autotools, 
    HeaderOnly, SingleHeader, Custom
};

enum class PackageManager {
    None, Vcpkg, Conan, PkgConfig, System, FetchContent, GitSubmodule
};

struct DependencyProfile {
    std::string name;
    std::string version;
    std::string repo_url;
    
    // Legal
    LicenseType license = LicenseType::Unknown;
    std::vector<std::string> license_files;
    bool license_compatible_closed = false;
    bool license_compatible_gpl = false;
    
    // Technical
    MaturityLevel maturity = MaturityLevel::Experimental;
    CppStandard min_standard = CppStandard::Cpp11;
    bool is_header_only = false;
    bool is_single_header = false;
    bool has_c_api = false;
    
    // Dependencies
    std::vector<std::string> transitive_deps;
    bool has_circular_deps = false;
    size_t dep_count = 0;
    
    // Platform
    bool linux = true, windows = true, macos = true;
    bool wasm = false, android = false, ios = false;
    bool console = false; // PS5, Xbox, Switch
    
    // Build
    BuildSystem build_system = BuildSystem::Custom;
    PackageManager package_manager = PackageManager::None;
    bool fetchcontent_ready = false;
    bool cmake_targets_exported = false;
    bool pkgconfig_generated = false;
    
    // Quality
    bool has_ci = false;
    bool uses_sanitizers = false;
    bool has_fuzzing = false;
    bool has_unit_tests = false;
    bool has_integration_tests = false;
    double test_coverage = 0.0;
    bool has_benchmarks = false;
    
    // Documentation
    bool has_api_docs = false;
    bool has_tutorial = false;
    bool has_examples = false;
    bool has_changelog = false;
    bool has_migration_guide = false;
    
    // ABI
    bool abi_stable = false;
    bool semver_followed = false;
    std::string versioning_scheme;
    
    // Activity (last 12 months)
    int commits = 0;
    int contributors = 0;
    int open_issues = 0;
    int closed_issues = 0;
    int open_prs = 0;
    int merged_prs = 0;
    std::string last_release;
    bool security_audit = false;
    
    // Scoring (0-100)
    struct Scores {
        int legal = 0;
        int maturity = 0;
        int technical = 0;
        int dependencies = 0;
        int platforms = 0;
        int build = 0;
        int quality = 0;
        int docs = 0;
        int abi = 0;
        int activity = 0;
        int total = 0;
    } scores;
    
    bool recommended = false;
    std::vector<std::string> concerns;
    std::vector<std::string> strengths;
};

struct EvaluationWeights {
    double legal = 0.25;
    double maturity = 0.15;
    double technical = 0.15;
    double dependencies = 0.10;
    double platforms = 0.05;
    double build = 0.10;
    double quality = 0.10;
    double docs = 0.05;
    double abi = 0.05;
    double activity = 0.10;
    
    // Presets
    static EvaluationWeights GameEngineClosed() {
        return {0.30, 0.15, 0.15, 0.10, 0.05, 0.05, 0.10, 0.05, 0.05, 0.10};
    }
    static EvaluationWeights GameEngineOpen() {
        return {0.15, 0.15, 0.15, 0.10, 0.05, 0.10, 0.10, 0.05, 0.05, 0.10};
    }
    static EvaluationWeights Library() {
        return {0.20, 0.10, 0.20, 0.15, 0.10, 0.10, 0.10, 0.10, 0.05, 0.05};
    }
};

DependencyProfile evaluate_library(const std::string& name, const std::string& repo_path);
DependencyProfile evaluate_from_github(const std::string& owner_repo);
void calculate_scores(DependencyProfile& profile, const EvaluationWeights& weights);
std::string generate_report(const DependencyProfile& profile);
std::string generate_comparison_table(const std::vector<DependencyProfile>& profiles);

// ============================================================================
// TRADE-OFFS DECISION MATRIX
// ============================================================================

struct TradeoffAnalysis {
    std::string dimension;
    std::string option_a, option_b;
    std::string recommendation;
    std::vector<std::string> pros_a, cons_a;
    std::vector<std::string> pros_b, cons_b;
};

std::vector<TradeoffAnalysis> analyze_tradeoffs(const DependencyProfile& a, const DependencyProfile& b);

// Common trade-offs:
/*
 * HEADER-ONLY vs COMPILED LIBRARY:
 *   Header-only: +easy integrate, +compile-time opts, +no ABI issues
 *                -compile time, -code bloat, -no binary distribution
 *   Compiled:    +fast compile, +binary dist, +stable ABI
 *              -build complexity, -ABI management, -linking issues
 * 
 * C API vs C++ API:
 *   C: +universal binding, +stable ABI, +any language
 *      -no RAII, -no templates, -manual memory
 *   C++: +RAII, +templates, +type safety, +expressiveness
 *        -ABI fragile, -compile time, -binding complexity
 * 
 * STATIC vs DYNAMIC LINKING:
 *   Static: +self-contained, +dead code elimination, +no DLL hell
 *           -larger binary, -no shared memory, -LGPL issues
 *   Dynamic: +smaller binary, +shared memory, +hot update
 *            -dependency hell, -version conflicts, -startup time
 * 
 * SYNC vs ASYNC API:
 *   Sync: +simple, +predictable, +easy debug
 *         -blocks thread, -no overlap I/O/compute
 *   Async: +throughput, +overlap, +responsive
 *          -complexity, -callback hell, -cancellation
 * 
 * IMMEDIATE vs RETAINED MODE UI:
 *   Immediate: +simple, +no sync, +hot reload
 *              -CPU per frame, -latency
 *   Retained: +GPU-driven, +animations, +accessibility
 *             -complexity, -state sync, -memory
 */

// ============================================================================
// INFORME TÉCNICO: ESTRUCTURA
// ============================================================================

struct TechnicalReport {
    std::string title;
    std::string date;
    std::string author;
    
    struct LibraryAnalysis {
        std::string name;
        std::string version;
        std::string summary;
        int score;
        bool recommended;
        std::vector<std::string> key_findings;
        std::vector<std::string> risks;
        std::string adoption_strategy; // adopt, wrap, reimplement, defer
    };
    
    std::vector<LibraryAnalysis> libraries;
    std::vector<TradeoffAnalysis> tradeoffs;
    std::string executive_summary;
    std::string decision_matrix; // markdown table
    std::string conclusion;
    std::string presentation_outline; // 15 min structure
};

TechnicalReport generate_technical_report(const std::vector<DependencyProfile>& profiles);

// Presentation structure (15 min):
/*
 * Slide 1: Title / Context (1 min)
 * Slide 2: Evaluation Criteria & Weights (1 min)
 * Slide 3-7: Per-library analysis (5 min) - score, key findings, risks
 * Slide 8: Comparison matrix (2 min)
 * Slide 9: Trade-offs deep dive (2 min)
 * Slide 10: Recommendations per library (2 min)
 * Slide 11: Adoption strategy (1 min)
 * Slide 12: Risks & Mitigation (1 min)
 * Slide 13: Next Steps / Action Items (1 min)
 * Slide 14: Q&A (1 min)
 */

// ============================================================================
// DECISION DOCUMENT TEMPLATE
// ============================================================================

struct DecisionRecord {
    std::string decision_id;
    std::string title;
    std::string status; // proposed, accepted, rejected, superseded
    std::string date;
    std::vector<std::string> deciders;
    
    std::string context;
    std::string decision;
    std::string consequences; // positive, negative, neutral
    std::vector<std::string> alternatives_considered;
    std::string related_decisions;
};

std::string generate_adr(const DecisionRecord& record); // Architecture Decision Record

// ============================================================================
// EJEMPLO DE USO
// ============================================================================

/*
int main() {
    // Evaluar 5 librerías objetivo
    std::vector<std::string> targets = {
        "https://github.com/g-truc/glm",
        "https://github.com/mackron/miniaudio",
        "https://github.com/erincatto/box2d",
        "https://github.com/ocornut/imgui",
        "https://github.com/libsdl-org/SDL"
    };
    
    std::vector<DependencyProfile> profiles;
    for (auto& url : targets) {
        auto profile = evaluate_from_github(url);
        profiles.push_back(profile);
    }
    
    // Generar informe
    auto report = generate_technical_report(profiles);
    
    // Guardar markdown
    std::ofstream("evaluation_report.md") << report.executive_summary 
                                          << "\n\n" << report.decision_matrix
                                          << "\n\n" << report.conclusion;
    
    // Guardar ADR para cada decisión
    for (auto& lib : report.libraries) {
        DecisionRecord dr;
        dr.decision_id = "ADR-" + lib.name;
        dr.title = "Adopt " + lib.name + " for " + (lib.recommended ? "core" : "evaluation");
        dr.status = lib.recommended ? "accepted" : "deferred";
        dr.context = "Need " + lib.name + " for engine module";
        dr.decision = lib.adoption_strategy;
        dr.consequences = "Score: " + std::to_string(lib.score);
        std::ofstream("decisions/" + dr.decision_id + ".md") << generate_adr(dr);
    }
    
    return 0;
}
*/

} // namespace modulo28_synthesis