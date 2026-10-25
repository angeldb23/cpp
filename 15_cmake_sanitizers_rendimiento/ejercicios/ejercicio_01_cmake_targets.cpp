/* =============================================================================
   MÓDULO 15 - EJERCICIO 01: CMake targets, PUBLIC/PRIVATE/INTERFACE
   =============================================================================
   DIFICULTAD: Básico
   CONSIGNA:
     Completar el CMakeLists.txt para una librería "mathlib" que:
     - Tiene headers públicos en include/mathlib/
     - Tiene headers privados en src/
     - Depende de fmt (PUBLIC) y spdlog (PRIVATE)
     - Exporta targets para instalación

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra -I../../include ejercicio_01_cmake_targets.cpp -o ej01 && ./ej01
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

// TODO: Escribir el contenido del CMakeLists.txt como string
std::string cmake_lists = R"(
cmake_minimum_required(VERSION 3.20)
project(mathlib VERSION 1.0.0 LANGUAGES CXX)

# TODO 1: add_library para mathlib con src/mathlib.cpp

# TODO 2: target_include_directories con PUBLIC include/ y PRIVATE src/

# TODO 3: target_link_libraries PUBLIC fmt::fmt y PRIVATE spdlog::spdlog

# TODO 4: install(TARGETS ... EXPORT mathlib-targets ...)

# TODO 5: install(DIRECTORY include/ DESTINATION include)
#        install(EXPORT mathlib-targets FILE mathlib-config.cmake NAMESPACE mathlib:: DESTINATION lib/cmake/mathlib)
)";

// Test: verificar que el string contiene las palabras clave
TEST_CASE(CMakeListsContieneTargets) {
    ASSERT_TRUE(cmake_lists.find("add_library(mathlib") != std::string::npos);
    ASSERT_TRUE(cmake_lists.find("target_include_directories") != std::string::npos);
    ASSERT_TRUE(cmake_lists.find("PUBLIC") != std::string::npos);
    ASSERT_TRUE(cmake_lists.find("PRIVATE") != std::string::npos);
    ASSERT_TRUE(cmake_lists.find("target_link_libraries") != std::string::npos);
    ASSERT_TRUE(cmake_lists.find("fmt::fmt") != std::string::npos);
    ASSERT_TRUE(cmake_lists.find("spdlog::spdlog") != std::string::npos);
    ASSERT_TRUE(cmake_lists.find("install(TARGETS") != std::string::npos);
    ASSERT_TRUE(cmake_lists.find("EXPORT mathlib-targets") != std::string::npos);
    ASSERT_TRUE(cmake_lists.find("install(DIRECTORY include/") != std::string::npos);
    ASSERT_TRUE(cmake_lists.find("mathlib-config.cmake") != std::string::npos);
    ASSERT_TRUE(cmake_lists.find("NAMESPACE mathlib::") != std::string::npos);
}

int main() {
    return RUN_ALL_TESTS();
}