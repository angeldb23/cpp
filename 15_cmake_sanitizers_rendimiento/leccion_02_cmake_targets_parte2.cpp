/* =============================================================================
   MÓDULO 15 - LECCIÓN 02: CMake — targets avanzados, alias, generator expressions,
   install/export, testing
   =============================================================================
   TEMA 1 de TEMAS.md (2da lección): CMake targets avanzados.

   ALIAS TARGETS:
   - add_library(milib::milib ALIAS mi_lib)
   - Permite: target_link_libraries(app PRIVATE milib::milib)
   - Namespacing: evita colisiones, intención clara.
   - INTERFACE libraries: add_library(nlohmann_json INTERFACE)

   GENERATOR EXPRESSIONS ($<...>):
   - $<BUILD_INTERFACE:dir>  → solo en build tree
   - $<INSTALL_INTERFACE:dir> → solo en install tree
   - $<TARGET_PROPERTY:tgt,PROP> → leer propiedad de target
   - $<TARGET_FILE:tgt> → ruta al artefacto (.so/.exe)
   - $<CONFIG:Debug> → solo en config Debug
   - $<PLATFORM_ID:Linux> → condicional por plataforma
   - $<CXX_COMPILER_ID:GNU,Clang> → condicional por compiler

   INSTALL / EXPORT:
   - install(TARGETS mi_lib EXPORT mi_lib-targets ...)
   - install(EXPORT mi_lib-targets FILE mi_lib-config.cmake
       NAMESPACE mi_lib:: DESTINATION lib/cmake/mi_lib)
   - Genera: mi_lib-config.cmake + mi_lib-config-version.cmake
   - Cliente: find_package(mi_lib REQUIRED) → target: mi_lib::mi_lib

   VERSIONING:
   - set_target_properties(mi_lib PROPERTIES
       VERSION 1.2.3
       SOVERSION 1
   )
   - install(TARGETS mi_lib VERSION 1.2.3 SOVERSION 1 ...)

   TESTING:
   - enable_testing()
   - add_test(NAME test_nombre COMMAND test_executable)
   - set_tests_properties(test_nombre PROPERTIES TIMEOUT 30)
   - ctest --output-on-failure -j4

   CMAKEPRESETS.JSON (testing):
     "testPresets": [
       { "name": "debug", "configurePreset": "debug", "output": { "outputOnFailure": true } }
     ]
   - Uso: ctest --preset=debug --output-on-failure

   EXPORTAR TARGETS PARA CONSUMO:
   - install(EXPORT mi_lib-targets
       FILE mi_lib-config.cmake
       NAMESPACE mi_lib::
       DESTINATION lib/cmake/mi_lib
   )
   - find_package(mi_lib REQUIRED) → target: mi_lib::mi_lib

   TARGET PROPERTIES ÚTILES:
   - CXX_STANDARD, CXX_STANDARD_REQUIRED
   - POSITION_INDEPENDENT_CODE (para shared libs)
   - CXX_EXTENSIONS OFF
   - COMPILE_OPTIONS, LINK_OPTIONS
   - INSTALL_RPATH, BUILD_RPATH */

#include <iostream>
#include <string>
#include <vector>

// Demo: simular generator expressions y alias targets

struct Target {
    std::string name;
    std::string version;
    std::string soversion;
    bool is_interface = false;
    std::vector<std::string> public_includes;
    std::vector<std::string> private_includes;
    std::vector<std::string> public_deps;
    std::vector<std::string> private_deps;
    std::vector<std::string> interface_deps;
    std::string namespace_alias;

    void set_version(const std::string& v, const std::string& so = "") {
        version = v;
        soversion = so.empty() ? v.substr(0, v.find('.')) : so;
    }

    void add_alias(const std::string& alias) {
        namespace_alias = alias;
    }
};

void demo_alias() {
    std::cout << "--- Alias targets ---\n";
    std::cout << R"(
# Alias para namespacing
add_library(milib::milib ALIAS mi_lib)

# Uso en consumidor:
target_link_libraries(mi_app PRIVATE milib::milib)

# Interface library (header-only)
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE include/)
# target_link_libraries(mi_lib PUBLIC nlohmann_json::nlohmann_json)
)" << "\n\n";
}

void demo_genex() {
    std::cout << "--- Generator expressions ($<...>) ---\n";
    std::cout << R"(
# Build vs Install interface
target_include_directories(mi_lib
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Config-dependent
target_compile_options(mi_lib PRIVATE
  $<$<CONFIG:Debug>:-O0 -g>
  $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# Platform-specific
target_link_libraries(mi_lib PRIVATE
  $<$<PLATFORM_ID:Linux>:pthread rt>
  $<$<PLATFORM_ID:Windows>:ws2_32>
)

# Compiler-specific
target_compile_options(mi_lib PRIVATE
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /permissive->
)

# Target property
target_link_libraries(mi_lib PRIVATE
  $<TARGET_PROPERTY:fmt,INTERFACE_INCLUDE_DIRECTORIES>
)
)" << "\n\n";
}

void demo_install_export() {
    std::cout << "--- Install / Export ---\n";
    std::cout << R"(
# Instalación con export
install(TARGETS mi_lib
  EXPORT mi_lib-targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

install(DIRECTORY include/ DESTINATION include)

install(EXPORT mi_lib-targets
  FILE mi_lib-config.cmake
  NAMESPACE mi_lib::
  DESTINATION lib/cmake/mi_lib
)

# Versioning
set_target_properties(mi_lib PROPERTIES
  VERSION 1.2.3
  SOVERSION 1
)

# Cliente:
# find_package(mi_lib 1.2 REQUIRED)
# target_link_libraries(app PRIVATE mi_lib::mi_lib)
)" << "\n\n";
}

void demo_testing() {
    std::cout << "--- Testing con CTest ---\n";
    std::cout << R"(
# En CMakeLists.txt
enable_testing()

add_executable(test_mi_lib test.cpp)
target_link_libraries(test_mi_lib PRIVATE mi_lib::mi_lib gtest_main)

add_test(NAME test_mi_lib COMMAND test_mi_lib)
set_tests_properties(test_mi_lib PROPERTIES TIMEOUT 30)

# CMakePresets.json
# "testPresets": [
#   { "name": "debug", "configurePreset": "debug",
#     "output": { "outputOnFailure": true } }
# ]

# Ejecución:
# cmake --preset=debug
# cmake --build --preset=debug
# ctest --preset=debug --output-on-failure
# ctest -j4 --output-on-failure
)" << "\n\n";
}

void demo_target_props() {
    std::cout << "--- Target properties útiles ---\n";
    std::cout << R"(
# Estándar C++
target_compile_features(mi_lib PUBLIC cxx_std_20)
set_target_properties(mi_lib PROPERTIES
  CXX_STANDARD 20
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

# Position Independent Code (shared libs)
set_target_properties(mi_lib PROPERTIES
  POSITION_INDEPENDENT_CODE ON
)

# Versioning
set_target_properties(mi_lib PROPERTIES
  VERSION 1.2.3
  SOVERSION 1
)

# RPATH
set_target_properties(mi_lib PROPERTIES
  INSTALL_RPATH "\$ORIGIN/../lib"
  BUILD_RPATH "${CMAKE_BINARY_DIR}/lib"
)

# Compile options por config
target_compile_options(mi_lib PRIVATE
  $<$<CONFIG:Debug>:-O0 -g>
  $<$<CONFIG:Release>:-O3 -DNDEBUG>
)
)" << "\n\n";
}

int main() {
    std::cout << "=== LECCIÓN 02: CMake targets avanzados ===\n\n";

    demo_alias();
    demo_genex();
    demo_install_export();
    demo_testing();
    demo_target_props();

    std::cout << "=== FIN LECCIÓN 02 ===\n";
    return 0;
}