/* =============================================================================
   MÓDULO 15 - LECCIÓN 04: Dependencias avanzadas — FetchContent avanzado,
   vcpkg/Conan en detalle, lockfiles, toolchain files
   =============================================================================
   TEMA 3 de TEMAS.md (2da lección): Dependencias avanzadas.

   FETCHCONTENT AVANZADO:
   - FetchContent_Declare() con SOURCE_DIR / BINARY_DIR custom
   - FetchContent_Populate() vs MakeAvailable():
     - Populate: solo descarga/configura, NO add_subdirectory()
     - MakeAvailable: populate + add_subdirectory() automático
   - POPULATE_ONLY: para inspeccionar antes de add_subdirectory()
   - Override: FETCHCONTENT_SOURCE_DIR_<name> / BINARY_DIR_<name>
   - Git: GIT_REPOSITORY, GIT_TAG, GIT_SHALLOW, GIT_PROGRESS
   - URL: URL, URL_HASH (SHA256), DOWNLOAD_NO_EXTRACT

   VCPKG DETALLADO:
   - vcpkg.json (manifest mode):
     {
       "name": "mi-proyecto",
       "version": "1.0.0",
       "dependencies": [
         "fmt", "spdlog", "gtest",
         { "name": "boost", "version>=": "1.83", "features": ["filesystem", "system"] }
       ],
       "builtin-baseline": "abc123..."
     }
   - vcpkg install (lee vcpkg.json)
   - Toolchain: -DCMAKE_TOOLCHAIN_FILE=/path/vcpkg/scripts/buildsystems/vcpkg.cmake
   - Overlay ports: vcpkg overlay-ports=/path/to/custom/ports
   - Registry: built-in + custom (git repo con ports)
   - Binary caching: vcpkg --binarycache

   CONAN v2 DETALLADO:
   - conanfile.py (recomendado sobre conanfile.txt):
     class MiProyecto(ConanFile):
         settings = "os", "compiler", "build_type", "arch"
         generators = "CMakeToolchain", "CMakeDeps"
         def requirements(self):
             self.requires("fmt/10.2.1")
             self.requires("spdlog/1.12.0")
         def build_requirements(self):
             self.tool_requires("cmake/3.28.1")
   - Lockfile: conan.lock (conan install --lockfile=conan.lock)
   - Perfiles: conan profile detect, conan profile list
   - build=missing / build=never / build=outdated
   - Conan Center Index (CCI) + repos privados (Artifactory, Nexus)

   TOOLCHAIN FILES:
   - vcpkg: scripts/buildsystems/vcpkg.cmake
   - Conan: conan_toolchain.cmake (generado)
   - Propio: para cross-compilation, flags globales
   - Uso: -DCMAKE_TOOLCHAIN_FILE=/path/to/toolchain.cmake

   LOCKFILES / REPRODUCIBILIDAD:
   - vcpkg: vcpkg.lock (generado por manifest mode)
   - Conan: conan.lock
   - CMake: CMakeDeps/Toolchain generados son determinísticos
   - CI: cachear lockfiles + cache binario

   DEPENDENCIAS TRANSITIVAS:
   - find_package() resuelve transitivas automáticamente (targets)
   - FetchContent: MakeAvailable resuelve transitivas
   - vcpkg/Conan: resuelven grafo completo
   - Evitar: find_library() manual, link_directories()

   CONFLICTOS DE VERSIONES:
   - vcpkg: baseline fija versiones (builtin-baseline)
   - Conan: lockfile fija versiones exactas
   - CMake: find_package(x.y REQUIRED) falla si versión incompatible
   - Alias targets: add_library(foo::foo ALIAS foo_v1) para migración

   BUENAS PRÁCTICAS:
   1. find_package() para deps de sistema (fmt, Boost, GTest)
   2. FetchContent para header-only sin paquete (lyra, Catch2)
   3. vcpkg/Conan para proyectos con muchas deps transitivas
   4. SIEMPRE targets namespaceados (fmt::fmt, no ${FMT_LIBRARIES})
   5. Lockfiles en CI (commit conan.lock / vcpkg.lock) */

#include <iostream>
#include <string>
#include <vector>

void demo_fetchcontent_adv() {
    std::cout << "--- FetchContent avanzado ---\n";
    std::cout << R"(
# Populate solo (sin add_subdirectory)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.5.3
  GIT_SHALLOW ON          # Solo último commit (rápido)
  GIT_PROGRESS ON
)
FetchContent_Populate(Catch2)  # Solo descarga, NO add_subdirectory

# Inspeccionar antes de usar
if (Catch2_POPULATED)
  message(STATUS "Catch2 source: ${catch2_SOURCE_DIR}")
  # add_subdirectory(${catch2_SOURCE_DIR})  # Manual

# MakeAvailable = Populate + add_subdirectory
FetchContent_MakeAvailable(Catch2)

# Override local (desarrollo)
# cmake -DFETCHCONTENT_SOURCE_DIR_Catch2=/path/local/Catch2
# cmake -DFETCHCONTENT_BINARY_DIR_Catch2=/path/build/Catch2

# URL download (no Git)
FetchContent_Declare(
  lyra
  URL https://github.com/bfgroup/lyra/archive/refs/tags/v1.6.0.zip
  URL_HASH SHA256=abc123...
)
FetchContent_MakeAvailable(lyra)
)" << "\n\n";
}

void demo_vcpkg_adv() {
    std::cout << "--- vcpkg avanzado ---\n";
    std::cout << R"(
# vcpkg.json (manifest mode)
{
  "name": "mi-proyecto",
  "version": "1.0.0",
  "dependencies": [
    "fmt",
    "spdlog",
    "gtest",
    { "name": "boost", "version>=": "1.83", "features": ["filesystem", "system"] },
    { "name": "opencv", "version>=": "4.8", "default-features": false }
  ],
  "builtin-baseline": "d5a1b2c3...",  # Commit hash de vcpkg
  "overlay-ports": [ "custom-ports" ]  # Puertos personalizados
}

# Instalar
vcpkg install

# Toolchain en CMake
cmake -B build -DCMAKE_TOOLCHAIN_FILE=/path/vcpkg/scripts/buildsystems/vcpkg.cmake

# Binary caching
vcpkg install --binarycache  # Usa binarios precompilados
vcpkg fetch --binarycache    # Solo descarga binarios

# Registry custom (git repo con ports)
# vcpkg.json:
# "registries": [
#   { "kind": "git", "repository": "https://github.com/mi-org/vcpkg-registry", "baseline": "abc123" }
# ]
)" << "\n\n";
}

void demo_conan_adv() {
    std::cout << "--- Conan v2 avanzado ---\n";
    std::cout << R"(
# conanfile.py (recomendado)
from conan import ConanFile
from conan.tools.cmake import cmake_layout

class MiProyecto(ConanFile):
    name = "mi_proyecto"
    version = "1.0.0"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"

    def requirements(self):
        self.requires("fmt/10.2.1")
        self.requires("spdlog/1.12.0")

    def build_requirements(self):
        self.tool_requires("cmake/3.28.1")

    def layout(self):
        cmake_layout(self)

# Instalar con lockfile
conan install . --output-folder=build --build=missing --lockfile=conan.lock

# Usar lockfile en CI
conan install . --lockfile=conan.lock --build=missing

# Perfiles
conan profile detect --name=default
conan profile list

# Build policies
conan install . --build=missing      # Compila si no hay binario
conan install . --build=never        # Falla si no hay binario
conan install . --build=outdated     # Recompila si receta cambió
)" << "\n\n";
}

void demo_toolchain() {
    std::cout << "--- Toolchain files ---\n";
    std::cout << R"(
# vcpkg toolchain
cmake -B build -DCMAKE_TOOLCHAIN_FILE=/path/vcpkg/scripts/buildsystems/vcpkg.cmake

# Conan toolchain (generado)
cmake -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake

# Cross-compilation toolchain propio (arm64)
# arm64-toolchain.cmake:
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
set(CMAKE_FIND_ROOT_PATH /opt/arm64-sysroot)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Uso
cmake -B build -DCMAKE_TOOLCHAIN_FILE=arm64-toolchain.cmake
)" << "\n\n";
}

void demo_lockfiles() {
    std::cout << "--- Lockfiles y reproducibilidad ---\n";
    std::cout << R"(
# vcpkg.lock (generado automáticamente en manifest mode)
# {
#   "dependencies": [
#     { "name": "fmt", "version": "10.2.1", "hash": "abc123" }
#   ]
# }

# Conan lockfile
conan install . --lockfile=conan.lock --build=missing
# conan.lock commitado en repo

# CI con lockfiles
# - Cachear ~/.cache/vcpkg y ~/.conan2/cache
# - Restaurar conan.lock / vcpkg.lock
# - conan install . --lockfile=conan.lock --build=missing
# - vcpkg install (usa vcpkg.lock automáticamente en manifest mode)

# CMake determinístico
# - CMakeDeps/Toolchain generados son determinísticos
# - find_package() con versión fija = reproducible
)" << "\n\n";
}

int main() {
    std::cout << "=== LECCIÓN 04: Dependencias avanzadas ===\n\n";

    demo_fetchcontent_adv();
    demo_vcpkg_adv();
    demo_conan_adv();
    demo_toolchain();
    demo_lockfiles();

    std::cout << "=== FIN LECCIÓN 04 ===\n";
    return 0;
}