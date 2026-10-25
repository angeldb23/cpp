/*
 * MÓDULO 28: Estudio de Código Real — Arquitectura de Librerías
 * EJERCICIO 01: Criterios de Selección - Implementar evaluador
 *
 * OBJETIVO: Completar las funciones de scoring para que pasen los tests.
 */

#include <string>
#include <vector>
#include <optional>
#include "../../include/test_framework.hpp"

// ============================================================================
// TIPOS (copiados de la lección - NO MODIFICAR)
// ============================================================================

enum class Licencia {
    MIT, BSD_2_Clause, BSD_3_Clause, Apache_2_0,
    LGPL_2_1, LGPL_3_0, GPL_2_0, GPL_3_0, Custom, Desconocida
};

enum class Madurez { Experimental, Estable, Madura, Legado };
enum class EstiloCPP { Legacy_C, Cpp98_03, Cpp11_14, Modern_Cpp17, Modern_Cpp20, Bleeding_Edge };

struct DependenciaTransitiva {
    std::string nombre;
    std::string version;
    bool esHeaderOnly;
    Licencia licencia;
};

struct CriteriosEvaluacion {
    std::string nombre, version, repo_url;
    Madurez madurez = Madurez::Experimental;
    Licencia licencia = Licencia::Desconocida;
    EstiloCPP estilo = EstiloCPP::Legacy_C;
    std::vector<DependenciaTransitiva> dependencias_transitivas;
    int commits_ultimo_anio = 0, contributors_activos = 0;
    int issues_abiertos = 0, issues_cerrados = 0, prs_abiertos = 0, prs_mergeados = 0;
    std::optional<std::string> ultimo_release;
    bool tiene_ci = false, usa_sanitizers = false, tiene_fuzzing = false;
    bool tiene_docs = false, tiene_ejemplos = false;
    bool soporta_linux = true, soporta_windows = true, soporta_macos = true;
    bool soporta_wasm = false, soporta_android = false, soporta_ios = false;
    bool abi_estable = false; std::string versionado_abi;
    bool tiene_cmake = false, tiene_pkgconfig = false, fetchcontent_listo = false;
    bool es_header_only = false, es_single_header = false;
    int num_tests = 0; double cobertura_estimada = 0.0;
    int score_madurez = 0, score_licencia = 0, score_dependencias = 0;
    int score_estilo = 0, score_actividad = 0, score_plataformas = 0;
    int score_abi = 0, score_build = 0, score_testing = 0, score_total = 0;
};

// ============================================================================
// FUNCIONES A IMPLEMENTAR
// ============================================================================

// TODO: Implementar scoring por madurez
int calcular_score_madurez(const CriteriosEvaluacion& c) { return 0; }

// TODO: Implementar scoring por licencia (proyecto cerrado)
int calcular_score_licencia(const CriteriosEvaluacion& c) { return 0; }

// TODO: Implementar scoring por dependencias transitivas
int calcular_score_dependencias(const CriteriosEvaluacion& c) { return 0; }

// TODO: Implementar scoring por estilo C++
int calcular_score_estilo(const CriteriosEvaluacion& c) { return 0; }

// TODO: Implementar scoring por actividad
int calcular_score_actividad(const CriteriosEvaluacion& c) { return 0; }

// TODO: Implementar scoring por plataformas
int calcular_score_plataformas(const CriteriosEvaluacion& c) { return 0; }

// TODO: Implementar scoring por ABI
int calcular_score_abi(const CriteriosEvaluacion& c) { return 0; }

// TODO: Implementar scoring por build system
int calcular_score_build(const CriteriosEvaluacion& c) { return 0; }

// TODO: Implementar scoring por testing
int calcular_score_testing(const CriteriosEvaluacion& c) { return 0; }

// ============================================================================
// COMPATIBILIDAD DE LICENCIAS
// ============================================================================

// TODO: true si licencia permite uso en proyecto propietario/cerrado
bool licencia_compatible_proyecto_cerrado(Licencia l) { return false; }

// TODO: true si licencia compatible con GPL
bool licencia_compatible_gpl(Licencia l) { return false; }

// ============================================================================
// TESTS (NO MODIFICAR)
// ============================================================================

TEST_CASE(TestScoreMadurez) {
    CriteriosEvaluacion c;
    c.madurez = Madurez::Madura; ASSERT_EQ(calcular_score_madurez(c), 100);
    c.madurez = Madurez::Estable; ASSERT_EQ(calcular_score_madurez(c), 75);
    c.madurez = Madurez::Experimental; ASSERT_EQ(calcular_score_madurez(c), 40);
    c.madurez = Madurez::Legado; ASSERT_EQ(calcular_score_madurez(c), 20);
}

TEST_CASE(TestScoreLicenciaProyectoCerrado) {
    CriteriosEvaluacion c;
    c.licencia = Licencia::MIT; ASSERT_EQ(calcular_score_licencia(c), 100);
    c.licencia = Licencia::BSD_3_Clause; ASSERT_EQ(calcular_score_licencia(c), 100);
    c.licencia = Licencia::Apache_2_0; ASSERT_EQ(calcular_score_licencia(c), 100);
    c.licencia = Licencia::LGPL_2_1; ASSERT_EQ(calcular_score_licencia(c), 60);
    c.licencia = Licencia::LGPL_3_0; ASSERT_EQ(calcular_score_licencia(c), 60);
    c.licencia = Licencia::GPL_2_0; ASSERT_EQ(calcular_score_licencia(c), 10);
    c.licencia = Licencia::GPL_3_0; ASSERT_EQ(calcular_score_licencia(c), 10);
    c.licencia = Licencia::Desconocida; ASSERT_EQ(calcular_score_licencia(c), 0);
}

TEST_CASE(TestScoreDependencias) {
    CriteriosEvaluacion c;
    c.dependencias_transitivas.clear();
    ASSERT_EQ(calcular_score_dependencias(c), 100);
    
    c.dependencias_transitivas.push_back({"dep1", "1.0", true, Licencia::MIT});
    ASSERT_EQ(calcular_score_dependencias(c), 90); // -10 por dep compilada (header-only=true no resta)
    
    c.dependencias_transitivas.push_back({"dep2", "2.0", false, Licencia::MIT});
    ASSERT_EQ(calcular_score_dependencias(c), 80); // -10 por dep compilada
    
    c.dependencias_transitivas.push_back({"dep3", "3.0", false, Licencia::GPL_3_0});
    ASSERT_EQ(calcular_score_dependencias(c), 50); // -30 por GPL
    
    c.dependencias_transitivas.push_back({"dep4", "4.0", false, Licencia::LGPL_2_1});
    ASSERT_EQ(calcular_score_dependencias(c), 35); // -15 por LGPL
}

TEST_CASE(TestScoreEstilo) {
    CriteriosEvaluacion c;
    c.estilo = EstiloCPP::Modern_Cpp20; ASSERT_EQ(calcular_score_estilo(c), 100);
    c.estilo = EstiloCPP::Modern_Cpp17; ASSERT_EQ(calcular_score_estilo(c), 90);
    c.estilo = EstiloCPP::Cpp11_14; ASSERT_EQ(calcular_score_estilo(c), 70);
    c.estilo = EstiloCPP::Cpp98_03; ASSERT_EQ(calcular_score_estilo(c), 40);
    c.estilo = EstiloCPP::Legacy_C; ASSERT_EQ(calcular_score_estilo(c), 50);
}

TEST_CASE(TestScoreActividad) {
    CriteriosEvaluacion c;
    // Sin actividad
    ASSERT_EQ(calcular_score_actividad(c), 0);
    
    // Con actividad media
    c.commits_ultimo_anio = 100; // 10 pts
    c.contributors_activos = 5;  // 10 pts
    c.issues_abiertos = 10; c.issues_cerrados = 90; // 18 pts (90%)
    c.tiene_ci = true; c.usa_sanitizers = true; c.tiene_fuzzing = false; // 20 pts
    int score = calcular_score_actividad(c);
    ASSERT_TRUE(score >= 50 && score <= 60);
    
    // Máximo
    c.commits_ultimo_anio = 500; c.contributors_activos = 20;
    c.issues_abiertos = 0; c.issues_cerrados = 100;
    c.tiene_fuzzing = true;
    ASSERT_EQ(calcular_score_actividad(c), 100);
}

TEST_CASE(TestScorePlataformas) {
    CriteriosEvaluacion c;
    c.soporta_linux = c.soporta_windows = c.soporta_macos = true;
    c.soporta_wasm = c.soporta_android = c.soporta_ios = false;
    ASSERT_EQ(calcular_score_plataformas(c), 60);
    
    c.soporta_wasm = c.soporta_android = true;
    ASSERT_EQ(calcular_score_plataformas(c), 90);
    
    c.soporta_ios = true;
    ASSERT_EQ(calcular_score_plataformas(c), 100);
}

TEST_CASE(TestScoreABI) {
    CriteriosEvaluacion c;
    c.abi_estable = false; c.versionado_abi = "ninguno";
    ASSERT_EQ(calcular_score_abi(c), 30);
    
    c.versionado_abi = "semver";
    ASSERT_EQ(calcular_score_abi(c), 70);
    
    c.versionado_abi = "soname";
    ASSERT_EQ(calcular_score_abi(c), 60);
    
    c.abi_estable = true;
    ASSERT_EQ(calcular_score_abi(c), 100);
}

TEST_CASE(TestScoreBuild) {
    CriteriosEvaluacion c;
    c.tiene_cmake = c.fetchcontent_listo = c.tiene_pkgconfig = false;
    c.es_header_only = c.es_single_header = false;
    ASSERT_EQ(calcular_score_build(c), 0);
    
    c.tiene_cmake = true; ASSERT_EQ(calcular_score_build(c), 30);
    c.fetchcontent_listo = true; ASSERT_EQ(calcular_score_build(c), 60);
    c.tiene_pkgconfig = true; ASSERT_EQ(calcular_score_build(c), 80);
    c.es_header_only = true; ASSERT_EQ(calcular_score_build(c), 100);
}

TEST_CASE(TestScoreTesting) {
    CriteriosEvaluacion c;
    ASSERT_EQ(calcular_score_testing(c), 0);
    
    c.num_tests = 10; c.cobertura_estimada = 0.9; c.tiene_ejemplos = true; c.tiene_docs = true;
    ASSERT_EQ(calcular_score_testing(c), 100); // 30+40+15+15
    
    c.cobertura_estimada = 0.6;
    ASSERT_EQ(calcular_score_testing(c), 85); // 30+25+15+15
    
    c.cobertura_estimada = 0.3;
    ASSERT_EQ(calcular_score_testing(c), 70); // 30+10+15+15
}

TEST_CASE(TestLicenciaCompatibleProyectoCerrado) {
    ASSERT_TRUE(licencia_compatible_proyecto_cerrado(Licencia::MIT));
    ASSERT_TRUE(licencia_compatible_proyecto_cerrado(Licencia::BSD_2_Clause));
    ASSERT_TRUE(licencia_compatible_proyecto_cerrado(Licencia::BSD_3_Clause));
    ASSERT_TRUE(licencia_compatible_proyecto_cerrado(Licencia::Apache_2_0));
    ASSERT_FALSE(licencia_compatible_proyecto_cerrado(Licencia::LGPL_2_1));
    ASSERT_FALSE(licencia_compatible_proyecto_cerrado(Licencia::LGPL_3_0));
    ASSERT_FALSE(licencia_compatible_proyecto_cerrado(Licencia::GPL_2_0));
    ASSERT_FALSE(licencia_compatible_proyecto_cerrado(Licencia::GPL_3_0));
    ASSERT_FALSE(licencia_compatible_proyecto_cerrado(Licencia::Desconocida));
}

TEST_CASE(TestLicenciaCompatibleGPL) {
    ASSERT_TRUE(licencia_compatible_gpl(Licencia::MIT));
    ASSERT_TRUE(licencia_compatible_gpl(Licencia::BSD_3_Clause));
    ASSERT_TRUE(licencia_compatible_gpl(Licencia::Apache_2_0));
    ASSERT_TRUE(licencia_compatible_gpl(Licencia::LGPL_2_1));
    ASSERT_TRUE(licencia_compatible_gpl(Licencia::LGPL_3_0));
    ASSERT_TRUE(licencia_compatible_gpl(Licencia::GPL_2_0));
    ASSERT_TRUE(licencia_compatible_gpl(Licencia::GPL_3_0));
    ASSERT_FALSE(licencia_compatible_gpl(Licencia::Desconocida));
}

int main() { return RUN_ALL_TESTS(); }