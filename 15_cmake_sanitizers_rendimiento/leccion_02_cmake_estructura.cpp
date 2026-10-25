/* =============================================================================
   MÓDULO 15 - LECCIÓN 02: Estructura de proyecto — librerías, ejecutables,
   tests y CMakePresets.json
   =============================================================================
   TEMA 2 de TEMAS.md (2 lecciones): Estructura de proyecto profesional.

   ESTRUCTURA RECOMENDADA (layout moderno):
   project_root/
   ├── CMakeLists.txt              # Root: project(), cmake_minimum_required, include subdirs
   ├── CMakePresets.json           # Configuración de builds (CI, local, release, debug)
   ├── src/
   │   ├── CMakeLists.txt
   │   ├── lib/                    # Librerías internas
   │   │   ├── CMakeLists.txt
   │   │   ├── mi_lib/
   │   │   │   ├── CMakeLists.txt
   │   │   │   ├── include/mi_lib/  # Headers PUBLIC
   │   │   │   ├── src/             # .cpp + headers PRIVATE
   │   │   │   └── tests/           # Tests de la lib
   │   │   └── otra_lib/
   │   └── apps/                   # Ejecutables
   │       ├── CMakeLists.txt
   │       ├── mi_app/
   │       │   ├── CMakeLists.txt
   │   │   └── main.cpp
   ├── third_party/                # Dependencias (FetchContent, submodules)
   ├── tests/                      # Tests de integración / e2e
   │   ├── CMakeLists.txt
   │   └── ...
   ├── docs/
   ├── scripts/
   └── .github/workflows/          # CI/CD

   CMAKEPRESETS.JSON (CMake 3.19+):
   - Define "presets" de configuración: compile options, generators, cache variables.
   - Permite: cmake --preset=release, cmake --preset=debug, cmake --preset=ci
   - Evita pasar -D flags a mano. Compartible en equipo/CI.
   - Sections: configurePresets, buildPresets, testPresets.

   CMAKELISTS.TXT ROOT (mínimo):
     cmake_minimum_required(VERSION 3.20)
     project(mi_proyecto VERSION 1.0.0 LANGUAGES CXX)
     set(CMAKE_CXX_STANDARD 20)
     set(CMAKE_CXX_STANDARD_REQUIRED ON)
     set(CMAKE_CXX_EXTENSIONS OFF)

     # Opciones globales
     option(BUILD_TESTS "Build tests" ON)
     option(BUILD_EXAMPLES "Build examples" OFF)

     add_subdirectory(src)

   CMAKEPRESETS.JSON (ejemplo):
   {
     "version": 6,
     "configurePresets": [
       {
         "name": "debug",
         "displayName": "Debug Build",
         "description": "Debug build with sanitizers",
         "generator": "Ninja",
         "binaryDir": "${sourceDir}/build/debug",
         "cacheVariables": {
           "CMAKE_BUILD_TYPE": "Debug",
           "CMAKE_CXX_FLAGS": "-Wall -Wextra -Wpedantic -fsanitize=address,undefined",
           "BUILD_TESTS": "ON"
         }
       },
       {
         "name": "release",
         "inherit": "debug",
         "displayName": "Release Build",
         "cacheVariables": {
           "CMAKE_BUILD_TYPE": "Release",
           "CMAKE_CXX_FLAGS": "-O3 -DNDEBUG -flto",
           "BUILD_TESTS": "OFF"
         }
       }
     ],
     "buildPresets": [
       { "name": "debug", "configurePreset": "debug" },
       { "name": "release", "configurePreset": "release" }
     ]
   }

   USO:
     cmake --preset=debug
     cmake --build --preset=debug
     ctest --preset=debug   (si hay testPresets)

   ENABLE_TESTING() + add_test() + CTest:
   - enable_testing() en root CMakeLists.txt
   - add_test(NAME test_nombre COMMAND test_executable)
   - ctest --output-on-failure
   - ctest -jN para paralelismo. */

#include <iostream>
#include <string>
#include <vector>

// Demo: simular estructura de proyecto y presets

struct Preset {
    std::string name;
    std::string inherits;
    std::string generator;
    std::string binary_dir;
    std::vector<std::pair<std::string, std::string>> cache_vars;
};

struct Project {
    std::string name;
    std::string version;
    std::vector<std::string> subdirs;
    std::vector<Preset> presets;
};

void demo_estructura() {
    std::cout << "--- Estructura de proyecto recomendada ---\n";
    std::cout << "project_root/\n";
    std::cout << "├── CMakeLists.txt\n";
    std::cout << "├── CMakePresets.json\n";
    std::cout << "├── src/\n";
    std::cout << "│   ├── CMakeLists.txt\n";
    std::cout << "│   ├── lib/\n";
    std::cout << "│   │   ├── CMakeLists.txt\n";
    std::cout << "│   │   ├── mi_lib/\n";
    std::cout << "│   │   │   ├── CMakeLists.txt\n";
    std::cout << "│   │   │   ├── include/mi_lib/   (PUBLIC)\n";
    std::cout << "│   │   │   ├── src/              (PRIVATE)\n";
    std::cout << "│   │   │   └── tests/            (tests unitarios)\n";
    std::cout << "│   │   └── otra_lib/\n";
    std::cout << "│   └── apps/\n";
    std::cout << "│       ├── CMakeLists.txt\n";
    std::cout << "│       ├── mi_app/\n";
    std::cout << "│       │   ├── CMakeLists.txt\n";
    std::cout << "│       │   └── main.cpp\n";
    std::cout << "├── tests/           (integración / e2e)\n";
    std::cout << "├── third_party/     (FetchContent / submodules)\n";
    std::cout << "├── docs/\n";
    std::cout << "└── .github/workflows/\n\n";
}

void demo_cmake_root() {
    std::cout << "--- CMakeLists.txt root (mínimo moderno) ---\n";
    std::cout << R"(cmake_minimum_required(VERSION 3.20)
project(mi_proyecto VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Políticas modernas recomendadas
cmake_policy(SET CMP0077 NEW)  # option() honorea type
cmake_policy(SET CMP0126 NEW)  # target_link_libraries() rechaza flags

option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" OFF)

# Salida de builds en build/<preset> (via CMakePresets.json)
# No forzar CMAKE_RUNTIME_OUTPUT_DIRECTORY aquí

add_subdirectory(src)

enable_testing()
)" << "\n\n";
}

void demo_presets() {
    std::cout << "--- CMakePresets.json (v6) ---\n";
    std::cout << R"({
  "version": 6,
  "cmakeMinimumRequired": { "major": 3, "minor": 20, "patch": 0 },
  "configurePresets": [
    {
      "name": "debug",
      "displayName": "Debug Build",
      "description": "Debug build with ASan/UBSan",
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/debug",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "CMAKE_CXX_FLAGS": "-Wall -Wextra -Wpedantic -fsanitize=address,undefined -fno-omit-frame-pointer",
        "BUILD_TESTS": "ON"
      }
    },
    {
      "name": "release",
      "inherits": "debug",
      "displayName": "Release Build",
      "description": "Optimized release with LTO",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release",
        "CMAKE_CXX_FLAGS": "-O3 -DNDEBUG -flto=auto",
        "BUILD_TESTS": "OFF"
      }
    },
    {
      "name": "ci",
      "inherits": "debug",
      "displayName": "CI Build",
      "cacheVariables": {
        "BUILD_TESTS": "ON"
      }
    }
  ],
  "buildPresets": [
    { "name": "debug", "configurePreset": "debug" },
    { "name": "release", "configurePreset": "release" },
    { "name": "ci", "configurePreset": "ci", "jobs": 4 }
  ],
  "testPresets": [
    { "name": "debug", "configurePreset": "debug", "output": { "outputOnFailure": true } }
  ]
}
)" << "\n\n";
}

void demo_cmake_lib() {
    std::cout << "--- src/lib/mi_lib/CMakeLists.txt (target moderno) ---\n";
    std::cout << R"(cmake_minimum_required(VERSION 3.20)

add_library(mi_lib
    src/mi_lib.cpp
)

# Headers PUBLIC (API)
target_include_directories(mi_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(mi_lib PUBLIC cxx_std_20)
target_compile_options(mi_lib PRIVATE -Wall -Wextra -Wpedantic)

# Dependencias: fmt es parte de la API (PUBLIC), spdlog solo interno (PRIVATE)
find_package(fmt REQUIRED)
target_link_libraries(mi_lib PUBLIC fmt::fmt)
find_package(spdlog REQUIRED)
target_link_libraries(mi_lib PRIVATE spdlog::spdlog)

# Tests unitarios de la lib
if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Instalación
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
)" << "\n\n";
}

void demo_cmake_app() {
    std::cout << "--- src/apps/mi_app/CMakeLists.txt ---\n";
    std::cout << R"(cmake_minimum_required(VERSION 3.20)

add_executable(mi_app main.cpp)

target_link_libraries(mi_app PRIVATE mi_lib::mi_lib)

# La app no instala librerías, solo el ejecutable
install(TARGETS mi_app
    RUNTIME DESTINATION bin
)
)" << "\n\n";
}

void demo_testing() {
    std::cout << "--- Testing con CTest ---\n";
    std::cout << R"(
# En CMakeLists.txt de tests/
add_executable(test_mi_lib test_mi_lib.cpp)
target_link_libraries(test_mi_lib PRIVATE mi_lib::mi_lib gtest_main)

add_test(NAME mi_lib_test COMMAND test_mi_lib)

# En root: enable_testing() ya llamado

# Ejecutar:
# cmake --preset=debug
# cmake --build --preset=debug
# ctest --preset=debug --output-on-failure
# ctest -j4  (paralelo)
)" << "\n\n";
}

int main() {
    std::cout << "=== LECCIÓN 02: Estructura de proyecto — libs, apps, tests, CMakePresets ===\n\n";

    demo_estructura();
    demo_cmake_root();
    demo_presets();
    demo_cmake_lib();
    demo_cmake_app();
    demo_testing();

    std::cout << "=== FIN LECCIÓN 02 ===\n";
    return 0;
}