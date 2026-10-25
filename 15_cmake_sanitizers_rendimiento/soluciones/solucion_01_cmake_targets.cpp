/* =============================================================================
   MÓDULO 15 - SOLUCIÓN 01: CMake targets
   ============================================================================= */
#include <iostream>
#include <string>
#include "../../include/test_framework.hpp"

std::string cmake_lists = R"(
cmake_minimum_required(VERSION 3.20)
project(mathlib VERSION 1.0.0 LANGUAGES CXX)

add_library(mathlib src/mathlib.cpp)

target_include_directories(mathlib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(mathlib
    PUBLIC fmt::fmt
    PRIVATE spdlog::spdlog
)

install(TARGETS mathlib
    EXPORT mathlib-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/ DESTINATION include)
install(EXPORT mathlib-targets
    FILE mathlib-config.cmake
    NAMESPACE mathlib::
    DESTINATION lib/cmake/mathlib
)
)";

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