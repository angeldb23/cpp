/* =============================================================================
   MÓDULO 15 - EJERCICIO 06: Cache y velocidad — ccache, PCH, Unity builds
   =============================================================================
   DIFICULTAD: Intermedio
   CONSIGNA (tema 6 de TEMAS.md: ccache, precompiled headers, builds paralelos):
     1. ccache_cmake_flags(): string con flags para habilitar ccache en CMake
        (CMAKE_CXX_COMPILER_LAUNCHER=ccache)
     2. pch_target(lib_name, headers...): string con target_precompile_headers
        para librería con headers públicos (INTERFACE) y privados (PRIVATE)
     3. unity_build_config(batch_size, unique_id): string con set_target_properties
        para UNITY_BUILD ON, batch_size, UNIQUE_ID
     4. mold_linker_flags(): string con flags para usar mold linker

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../../include ejercicio_06_cache_speed.cpp -o ej06 && ./ej06
   ============================================================================= */
#include <iostream>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

// TODO 1: ccache flags para CMake
std::string ccache_cmake_flags() {
    // TU CÓDIGO: CMAKE_CXX_COMPILER_LAUNCHER=ccache
    return "";
}

// TODO 2: PCH transitivo para target
std::string pch_target([[maybe_unused]] const std::string& lib_name,
                        [[maybe_unused]] const std::vector<std::string>& public_headers,
                        [[maybe_unused]] const std::vector<std::string>& private_headers) {
    // TU CÓDIGO: target_precompile_headers con PUBLIC e INTERFACE
    return "";
}

// TODO 3: Unity build configuración
std::string unity_build_config([[maybe_unused]] int batch_size,
                                [[maybe_unused]] const std::string& unique_id) {
    // TU CÓDIGO: set_target_properties UNITY_BUILD ON, BATCH_SIZE, UNIQUE_ID
    return "";
}

// TODO 4: mold linker flags
std::string mold_linker_flags() {
    // TU CÓDIGO: -fuse-ld=mold para exe, shared, module
    return "";
}

// ============= TESTS =============
TEST_CASE(CcacheFlagsContieneLauncher) {
    std::string flags = ccache_cmake_flags();
    ASSERT_TRUE(flags.find("CMAKE_CXX_COMPILER_LAUNCHER") != std::string::npos);
    ASSERT_TRUE(flags.find("ccache") != std::string::npos);
}

TEST_CASE(PchTargetContienePublicPrivate) {
    std::string pch = pch_target("mi_lib", {"fmt/format.h"}, {"internal.h"});
    ASSERT_TRUE(pch.find("target_precompile_headers") != std::string::npos);
    ASSERT_TRUE(pch.find("PRIVATE") != std::string::npos);
    ASSERT_TRUE(pch.find("INTERFACE") != std::string::npos);
    ASSERT_TRUE(pch.find("fmt/format.h") != std::string::npos);
}

TEST_CASE(UnityBuildConfigContieneProps) {
    std::string ub = unity_build_config(16, "mi_lib_");
    ASSERT_TRUE(ub.find("UNITY_BUILD") != std::string::npos);
    ASSERT_TRUE(ub.find("ON") != std::string::npos);
    ASSERT_TRUE(ub.find("UNITY_BUILD_BATCH_SIZE") != std::string::npos);
    ASSERT_TRUE(ub.find("16") != std::string::npos);
    ASSERT_TRUE(ub.find("UNITY_BUILD_UNIQUE_ID") != std::string::npos);
    ASSERT_TRUE(ub.find("mi_lib_") != std::string::npos);
}

TEST_CASE(MoldLinkerFlagsContieneFuseLd) {
    std::string flags = mold_linker_flags();
    ASSERT_TRUE(flags.find("fuse-ld=mold") != std::string::npos);
    ASSERT_TRUE(flags.find("CMAKE_EXE_LINKER_FLAGS") != std::string::npos);
    ASSERT_TRUE(flags.find("CMAKE_SHARED_LINKER_FLAGS") != std::string::npos);
    ASSERT_TRUE(flags.find("CMAKE_MODULE_LINKER_FLAGS") != std::string::npos);
}

int main() {
    return RUN_ALL_TESTS();
}