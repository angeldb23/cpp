/* =============================================================================
   MÓDULO 15 - EJERCICIO 05: FetchContent y dependencias
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA (tema 3 de TEMAS.md: find_package, FetchContent, vcpkg/Conan):
     1. fetchcontent_basic(dep_name, git_repo, git_tag): retorna string con
        FetchContent_Declare + FetchContent_MakeAvailable para dep
     2. fetchcontent_shallow(dep_name, git_repo, git_tag): igual que arriba
        pero con GIT_SHALLOW ON y GIT_PROGRESS ON
     3. fetchcontent_url(dep_name, url, sha256): FetchContent con URL download
        (no Git), con URL_HASH SHA256
     4. find_package_moderno(pkg, version): string con find_package moderno
        que usa targets namespaceados (pkg::pkg)

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../../include ejercicio_05_fetchcontent.cpp -o ej05 && ./ej05
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

// TODO 1: FetchContent básico
std::string fetchcontent_basic([[maybe_unused]] const std::string& dep_name,
                                [[maybe_unused]] const std::string& git_repo,
                                [[maybe_unused]] const std::string& git_tag) {
    // TU CÓDIGO: retorna string con FetchContent_Declare y MakeAvailable
    return "";
}

// TODO 2: FetchContent con shallow clone
std::string fetchcontent_shallow([[maybe_unused]] const std::string& dep_name,
                                  [[maybe_unused]] const std::string& git_repo,
                                  [[maybe_unused]] const std::string& git_tag) {
    // TU CÓDIGO: igual que básico + GIT_SHALLOW ON + GIT_PROGRESS ON
    return "";
}

// TODO 3: FetchContent con URL download
std::string fetchcontent_url([[maybe_unused]] const std::string& dep_name,
                              [[maybe_unused]] const std::string& url,
                              [[maybe_unused]] const std::string& sha256) {
    // TU CÓDIGO: FetchContent_Declare con URL + URL_HASH
    return "";
}

// TODO 4: find_package moderno con targets namespaceados
std::string find_package_moderno([[maybe_unused]] const std::string& pkg,
                                  [[maybe_unused]] const std::string& version) {
    // TU CÓDIGO: find_package(pkg version REQUIRED) + target_link_libraries
    return "";
}

// ============= TESTS =============
TEST_CASE(FetchContentBasicoContieneClaves) {
    std::string fc = fetchcontent_basic("Catch2",
        "https://github.com/catchorg/Catch2.git", "v3.5.3");
    ASSERT_TRUE(fc.find("FetchContent_Declare") != std::string::npos);
    ASSERT_TRUE(fc.find("FetchContent_MakeAvailable") != std::string::npos);
    ASSERT_TRUE(fc.find("Catch2") != std::string::npos);
    ASSERT_TRUE(fc.find("GIT_REPOSITORY") != std::string::npos);
    ASSERT_TRUE(fc.find("GIT_TAG") != std::string::npos);
}

TEST_CASE(FetchContentShallowContieneOpciones) {
    std::string fc = fetchcontent_shallow("fmt",
        "https://github.com/fmtlib/fmt.git", "10.2.1");
    ASSERT_TRUE(fc.find("FetchContent_Declare") != std::string::npos);
    ASSERT_TRUE(fc.find("GIT_SHALLOW") != std::string::npos);
    ASSERT_TRUE(fc.find("GIT_PROGRESS") != std::string::npos);
}

TEST_CASE(FetchContentURLContieneHash) {
    std::string fc = fetchcontent_url("lyra",
        "https://github.com/bfgroup/lyra/archive/refs/tags/v1.6.0.zip",
        "SHA256=abc123...");
    ASSERT_TRUE(fc.find("URL") != std::string::npos);
    ASSERT_TRUE(fc.find("URL_HASH") != std::string::npos);
    ASSERT_TRUE(fc.find("SHA256") != std::string::npos);
}

TEST_CASE(FindPackageModernoUsaTargets) {
    std::string fp = find_package_moderno("fmt", "10.2.1");
    ASSERT_TRUE(fp.find("find_package") != std::string::npos);
    ASSERT_TRUE(fp.find("fmt") != std::string::npos);
    ASSERT_TRUE(fp.find("REQUIRED") != std::string::npos);
    ASSERT_TRUE(fp.find("fmt::fmt") != std::string::npos);
    ASSERT_TRUE(fp.find("target_link_libraries") != std::string::npos);
}

int main() {
    return RUN_ALL_TESTS();
}