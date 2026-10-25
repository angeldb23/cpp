/* =============================================================================
   MÓDULO 15 - LECCIÓN 03: Estructura de proyecto — multi-proyecto, submódulos,
   CI/CD, empaquetado
   =============================================================================
   TEMA 2 de TEMAS.md (2da lección): Estructura de proyecto avanzada.

   PROYECTOS MULTI-DIRECTORIO:
   - add_subdirectory() para cada componente
   - Cada subdir tiene su CMakeLists.txt
   - Targets visibles globalmente (scope global)

   SUBMÓDULOS GIT:
   - git submodule add <url> third_party/<lib>
   - En CMake: add_subdirectory(third_party/<lib>)
   - O FetchContent para deps que no son submódulos

   MONOREPO vs MULTI-REPO:
   - Monorepo: un repo, múltiples targets, build único
   - Multi-repo: repos separados, versionado independiente
   - CMake: add_subdirectory() para monorepo, find_package() para multi-repo

   CI/CD CON CMAKE:
   - GitHub Actions / GitLab CI / Azure Pipelines
   - cmake --preset=ci
   - cmake --build --preset=ci
   - ctest --preset=ci
   - Cache ccache / vcpkg / Conan en CI

   EMPAQUETADO (CPACK):
   - include(CPack)
   - set(CPACK_PACKAGE_NAME "mi_app")
   - set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
   - set(CPACK_GENERATOR "DEB;RPM;TGZ;NSIS")
   - cpack -G DEB (genera .deb)
   - cpack -G RPM (genera .rpm)
   - cpack -G NSIS (instalador Windows)

   INSTALACIÓN COMPLETA:
   - install(TARGETS ...)
   - install(DIRECTORY include/ DESTINATION include)
   - install(FILES config.h DESTINATION include/mi_lib)
   - install(PROGRAMS script.sh DESTINATION bin)
   - install(EXPORT ... FILE mi_lib-config.cmake ...)

   VERSIONADO:
   - PROJECT_VERSION: ${PROJECT_VERSION}
   - VERSION / SOVERSION en targets
   - CPACK_PACKAGE_VERSION = ${PROJECT_VERSION}

   DOCUMENTACIÓN:
   - Doxygen + CMake: find_package(Doxygen), doxygen_add_docs()
   - install(DIRECTORY docs/html DESTINATION share/doc/mi_lib)

   SCRIPTS AUXILIARES:
   - configure_file(config.h.in config.h @ONLY)
   - configure_file(script.sh.in script.sh @ONLY)
   - install(PROGRAMS script.sh DESTINATION bin) */

#include <iostream>
#include <string>
#include <vector>

void demo_monorepo() {
    std::cout << "--- Monorepo structure ---\n";
    std::cout << R"(
project_root/
├── CMakeLists.txt              # Root: project(), options, add_subdirectory(src)
├── CMakePresets.json
├── src/
│   ├── CMakeLists.txt
│   ├── libs/
│   │   ├── CMakeLists.txt
│   │   ├── mathlib/
│   │   │   ├── CMakeLists.txt
│   │   │   ├── include/mathlib/    # PUBLIC headers
│   │   │   ├── src/                # PRIVATE headers + .cpp
│   │   │   └── tests/
│   │   └── utils/
│   └── apps/
│       ├── CMakeLists.txt
│       ├── mi_app/
│       │   ├── CMakeLists.txt
│       │   └── main.cpp
├── third_party/                # git submodules / FetchContent
├── tests/                      # Tests de integración
├── docs/
├── scripts/
├── .github/workflows/
└── CPackConfig.cmake           # Configuración CPack
)" << "\n\n";
}

void demo_submodules() {
    std::cout << "--- Git submodules vs FetchContent ---\n";
    std::cout << R"(
# Git submodule (versionado junto al proyecto)
git submodule add https://github.com/fmtlib/fmt.git third_party/fmt
# En CMakeLists.txt root:
add_subdirectory(third_party/fmt)

# FetchContent (para deps que no son submódulos)
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.5.3
)
FetchContent_MakeAvailable(Catch2)

# Diferencia:
# - submodule: versionado en .gitmodules, commit hash en repo padre
# - FetchContent: descarga en configure time, no en repo
)" << "\n\n";
}

void demo_cicd() {
    std::cout << "--- CI/CD con CMake ---\n";
    std::cout << R"(
# GitHub Actions (.github/workflows/ci.yml)
name: CI
on: [push, pull_request]
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - uses: actions/cache@v3
        with:
          path: ~/.cache/ccache
          key: ccache-${{ runner.os }}-${{ hashFiles('**/CMakeLists.txt') }}
      - name: Configure
        run: cmake --preset=ci
      - name: Build
        run: cmake --build --preset=ci
      - name: Test
        run: ctest --preset=ci --output-on-failure

# CMakePresets.json "ci" preset:
# "cacheVariables": { "BUILD_TESTS": "ON" }
# "buildPresets": [{ "name": "ci", "configurePreset": "ci", "jobs": 4 }]
)" << "\n\n";
}

void demo_cpack() {
    std::cout << "--- CPack (empaquetado) ---\n";
    std::cout << R"(
# En CMakeLists.txt root
include(CPack)

set(CPACK_PACKAGE_NAME "mi_app")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Mi aplicación")
set(CPACK_PACKAGE_VENDOR "Mi Empresa")
set(CPACK_PACKAGE_CONTACT "dev@miempresa.com")

# Generadores
set(CPACK_GENERATOR "DEB;RPM;TGZ;NSIS")

# Linux .deb
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "dev@miempresa.com")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libfmt8, libspdlog1")

# Windows NSIS
set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64\\MiApp")
set(CPACK_NSIS_DISPLAY_NAME "Mi Aplicación")

# Uso:
# cmake --preset=release
# cmake --build --preset=release
# cpack -G DEB
# cpack -G RPM
# cpack -G NSIS
)" << "\n\n";
}

void demo_configure_file() {
    std::cout << "--- configure_file() ---\n";
    std::cout << R"(
# config.h.in
#ifndef CONFIG_H
#define CONFIG_H
#define PROJECT_VERSION "@PROJECT_VERSION@"
#define ENABLE_FEATURE_X @ENABLE_FEATURE_X@
#endif

# CMakeLists.txt
set(ENABLE_FEATURE_X ON)
configure_file(config.h.in config.h @ONLY)

# Resultado config.h:
#define PROJECT_VERSION "1.0.0"
#define ENABLE_FEATURE_X 1

# Instalar
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/config.h DESTINATION include/mi_lib)
)" << "\n\n";
}

int main() {
    std::cout << "=== LECCIÓN 03: Estructura de proyecto avanzada ===\n\n";

    demo_monorepo();
    demo_submodules();
    demo_cicd();
    demo_cpack();
    demo_configure_file();

    std::cout << "=== FIN LECCIÓN 03 ===\n";
    return 0;
}