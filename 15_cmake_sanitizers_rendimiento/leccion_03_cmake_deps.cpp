/* =============================================================================
   MÓDULO 15 - LECCIÓN 03: Dependencias — find_package, FetchContent,
   vcpkg/Conan
   =============================================================================
   TEMA 3 de TEMAS.md (2 lecciones): Gestión de dependencias en CMake.

   TRES ESTRATEGIAS PRINCIPALES:

   1. find_package() — Sistema instalado / CMake config packages
      - Busca paquetes instalados en el sistema o en CMAKE_PREFIX_PATH.
      - Requiere que la librería provea <nombre>Config.cmake o <nombre>-config.cmake.
      - Moderna: targets namespaceados (fmt::fmt, nlohmann_json::nlohmann_json).
      - Versiones: find_package(fmt 10.0 REQUIRED)
      - Componentes: find_package(Boost REQUIRED COMPONENTS filesystem system)

   2. FetchContent (CMake 3.14+) — Descarga en configure time
      - Descarga fuentes (Git, URL, archive) y los añade como subdirectorio.
      - Útil para deps que no tienen paquetes de sistema o para pinnear versiones.
      - MakeAvailable: popula el source dir y hace add_subdirectory automático.
      - OJO: aumenta configure time; no usar para deps gigantes (llvm, qt).
      - populate() vs MakeAvailable(): MakeAvailable = populate + add_subdirectory.

   3. Gestores de paquetes externos (vcpkg, Conan) — Ecosistema completo
      - vcpkg: Microsoft, ports CMake, integra con CMake toolchain file.
        vcpkg install fmt spdlog
        cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
      - Conan: Python-based, más maduro para C++, perfiles, lockfiles.
        conan install . --output-folder=build --build=missing
        cmake -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake
      - Ambos resuelven transitive deps, versiones, ABI compatibility.

   COMPARACIÓN RÁPIDA:
   | Estrategia      | Pros                              | Contras                        |
   |-----------------|-----------------------------------|--------------------------------|
   | find_package    | Sistema limpio, targets modernos  | Requiere instalación previa    |
   | FetchContent    | Zero-install, version pinneada    | Configure time lento, no cache |
   | vcpkg/Conan     | Resolución completa, lockfiles    | Toolchain file, setup inicial  |

   BUENAS PRÁCTICAS:
   - find_package() para deps de sistema establecidas (fmt, spdlog, gtest, Boost).
   - FetchContent para deps header-only pequeñas o sin paquetes (Catch2, lyra, etc.).
   - vcpkg/Conan para proyectos grandes con muchas deps transitivas.
   - SIEMPRE usar targets namespaceados (fmt::fmt, no ${FMT_LIBRARIES}).
   - Evitar find_library() manual; dejar que find_package() haga el trabajo.

   EJEMPLO FETCHCONTENT (Catch2):
     include(FetchContent)
     FetchContent_Declare(
       Catch2
       GIT_REPOSITORY https://github.com/catchorg/Catch2.git
       GIT_TAG        v3.5.3
     )
     FetchContent_MakeAvailable(Catch2)
     # Target: Catch2::Catch2WithMain

   TOOLCHAIN FILE (vcpkg/Conan):
   - Archivo .cmake que setea CMAKE_TOOLCHAIN_FILE, CMAKE_PREFIX_PATH, etc.
   - Se pasa en configure: -DCMAKE_TOOLCHAIN_FILE=... */

#include <iostream>
#include <string>
#include <vector>

// Demo: simular estrategias de dependencias

struct Dependency {
    std::string name;
    std::string version;
    std::string strategy;  // "find_package", "FetchContent", "vcpkg", "Conan"
};

struct ProjectDeps {
    std::vector<Dependency> deps;

    void add_find_package(const std::string& name, const std::string& ver = "") {
        deps.push_back({name, ver, "find_package"});
    }

    void add_fetchcontent(const std::string& name, const std::string& git_url, const std::string& tag) {
        deps.push_back({name, tag, "FetchContent"});
    }

    void add_vcpkg(const std::string& name) {
        deps.push_back({name, "", "vcpkg"});
    }

    void print() const {
        for (const auto& d : deps) {
            std::cout << "  " << d.strategy << ": " << d.name;
            if (!d.version.empty()) std::cout << " @" << d.version;
            std::cout << "\n";
        }
    }
};

void demo_find_package() {
    std::cout << "--- find_package() moderno ---\n";
    std::cout << R"(
# En CMakeLists.txt
find_package(fmt 10.0 REQUIRED)           # Config package, target: fmt::fmt
find_package(spdlog 1.11 REQUIRED)        # target: spdlog::spdlog
find_package(GTest REQUIRED)              # target: GTest::GTest, GTest::Main
find_package(Boost 1.83 REQUIRED COMPONENTS filesystem system)  # Boost components

# Uso con targets namespaceados (moderno)
target_link_libraries(mi_lib PUBLIC fmt::fmt spdlog::spdlog)
target_link_libraries(test_mi_lib PRIVATE GTest::GTest GTest::Main)

# Variables legacy (EVITAR): ${FMT_LIBRARIES}, ${SPDLOG_LIBRARIES}, etc.
# CMake moderno usa SOLO targets: fmt::fmt, spdlog::spdlog, etc.
)" << "\n\n";
}

void demo_fetchcontent() {
    std::cout << "--- FetchContent (CMake 3.14+) ---\n";
    std::cout << R"(
include(FetchContent)

# Catch2 para tests
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.5.3  # Pin version!
)
FetchContent_MakeAvailable(Catch2)
# Targets: Catch2::Catch2, Catch2::Catch2WithMain

# lyra (CLI parser header-only)
FetchContent_Declare(
  lyra
  GIT_REPOSITORY https://github.com/bfgroup/lyra.git
  GIT_TAG        v1.6.0
)
FetchContent_MakeAvailable(lyra)
# Target: lyra::lyra (INTERFACE)

# OJO: FetchContent corre en CONFIGURE TIME.
# No abusar para deps grandes (llvm, qt, opencv).
# Para deps grandes: find_package + vcpkg/Conan.
)" << "\n\n";
}

void demo_vcpkg() {
    std::cout << "--- vcpkg (Microsoft) ---\n";
    std::cout << R"(
# Instalación (una vez)
git clone https://github.com/microsoft/vcpkg
cd vcpkg && ./bootstrap-vcpkg.sh
./vcpkg install fmt spdlog gtest nlohmann-json

# Uso en CMake (toolchain file)
cmake -B build -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake

# O setear en CMakePresets.json:
# "cacheVariables": {
#   "CMAKE_TOOLCHAIN_FILE": "/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"
# }

# vcpkg crea targets automáticamente:
# find_package(fmt CONFIG REQUIRED) -> target: fmt::fmt
# find_package(spdlog CONFIG REQUIRED) -> target: spdlog::spdlog

# Manifest mode (vcpkg.json) para version pinning:
# {
#   "name": "mi-proyecto",
#   "version": "1.0.0",
#   "dependencies": ["fmt", "spdlog", "gtest"]
# }
)" << "\n\n";
}

void demo_conan() {
    std::cout << "--- Conan (v2) ---\n";
    std::cout << R"(
# conanfile.txt (simple) o conanfile.py (avanzado)
# conanfile.txt
[requires]
fmt/10.2.1
spdlog/1.12.0
gtest/1.14.0

[generators]
CMakeToolchain
CMakeDeps

# Instalar
conan install . --output-folder=build --build=missing

# Uso en CMake
cmake -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake
cmake --build build

# conanfile.py para más control:
# class MiProyecto(ConanFile):
#     settings = "os", "compiler", "build_type", "arch"
#     generators = "CMakeToolchain", "CMakeDeps"
#     def requirements(self):
#         self.requires("fmt/10.2.1")
#         self.requires("spdlog/1.12.0")

# Ventajas: lockfiles (conan.lock), perfiles, cross-compile, private repos.
)" << "\n\n";
}

void demo_hibrido() {
    std::cout << "--- Estrategia híbrida recomendada ---\n";
    std::cout << R"(
# 1. find_package para deps de sistema establecidas
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(GTest REQUIRED)

# 2. FetchContent para header-only pequeñas sin paquete
include(FetchContent)
FetchContent_Declare(lyra GIT_REPOSITORY ... GIT_TAG v1.6.0)
FetchContent_MakeAvailable(lyra)

# 3. vcpkg/Conan para proyectos con muchas deps transitivas
# En CI: cachear vcpkg/Conan cache para velocidad

# Ejemplo CMakeLists.txt híbrido:
cmake_minimum_required(VERSION 3.20)
project(mi_proyecto)

# 1. Sistema / find_package
find_package(fmt 10 REQUIRED)
find_package(spdlog 1.11 REQUIRED)

# 2. FetchContent para deps sin paquete
include(FetchContent)
FetchContent_Declare(lyra GIT_REPOSITORY ... GIT_TAG v1.6.0)
FetchContent_MakeAvailable(lyra)

# 4. Targets namespaceados SIEMPRE
add_library(mi_lib src/mi_lib.cpp)
target_link_libraries(mi_lib PUBLIC fmt::fmt spdlog::spdlog lyra::lyra)
)" << "\n\n";
}

int main() {
    std::cout << "=== LECCIÓN 03: Dependencias — find_package, FetchContent, vcpkg/Conan ===\n\n";

    demo_find_package();
    demo_fetchcontent();
    demo_vcpkg();
    demo_conan();
    demo_hibrido();

    std::cout << "=== FIN LECCIÓN 03 ===\n";
    return 0;
}