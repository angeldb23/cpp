/* =============================================================================
   MÓDULO 15 - LECCIÓN 04: Warnings y modos de build — -Wall -Wextra -Werror,
   Debug/Release, LTO
   =============================================================================
   TEMA 4 de TEMAS.md (1 lección): Configuración de warnings y modos de build.

   WARNINGS RECOMENDADOS (mínimo vital):
   - -Wall -Wextra: activan la mayoría de warnings útiles.
   - -Wpedantic: estándar estricto (ISO C++).
   - -Werror: convierte warnings en errores (CI obligatorio).
   - -Wconversion: conversiones implícitas que pierden datos.
   - -Wshadow: variables que ocultan otras (bug común).
   - -Wnon-virtual-dtor: clase con virtual functions pero dtor no virtual.
   - -Wold-style-cast: casts estilo C (usar static_cast, etc.).
   - -Wnull-dereference, -Wdouble-promotion, -Wformat=2.

   CMAKE: target_compile_options() POR TARGET (no global add_compile_options):
     target_compile_options(mi_lib PRIVATE
       -Wall -Wextra -Wpedantic
       -Wconversion -Wshadow
       -Wnon-virtual-dtor -Wold-style-cast
     )
   - PRIVATE: solo para compilar ESTE target.
   - PUBLIC/INTERFACE: propaga a consumidores (raro para warnings).

   MODO DE BUILD (CMAKE_BUILD_TYPE):
   - Debug:   -O0 -g, asserts ON, sanitizers, sin optimizar.
   - Release: -O3 -DNDEBUG, LTO, asserts OFF, optimizado.
   - RelWithDebInfo: -O2 -g, símbolos de debug + optimización.
   - MinSizeRel: -Os, optimizar tamaño.

   CMAKE: set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fsanitize=address,undefined")
   CMAKE: set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -flto=auto")

   LTO (Link Time Optimization):
   - -flto: permite optimizaciones cross-TU (inlining cross-file, devirtualization).
   - -flto=auto: usa thin LTO si el linker lo soporta (más rápido).
   - ThinLTO: escalable para proyectos grandes.
   - Requiere linker con soporte (lld, gold, mold).
   - CMake: target_link_options(mi_lib INTERFACE -flto=auto)

   SANITIZERS EN BUILD TYPES:
   - Debug: -fsanitize=address,undefined (ASan + UBSan)
   - Release: SIN sanitizers (performance)
   - Opcional: build type "Asan" o "Tsan" dedicado.

   COMPILER-SPECIFIC:
   - GCC: -Wall -Wextra -Wpedantic -Wconversion -Wshadow...
   - Clang: mismo + -Weverything (luego desactivar ruido con -Wno-...)
   - MSVC: /W4 /WX /permissive- /external:W3

   CMAKE: generator expressions para flags por compiler:
     target_compile_options(mi_lib PRIVATE
       $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
       $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /permissive->
     ) */

#include <iostream>
#include <string>

void demo_warnings() {
    std::cout << "--- Warnings recomendados ---\n";
    std::cout << R"(
# GCC/Clang (target_compile_options por target)
target_compile_options(mi_lib PRIVATE
  -Wall -Wextra -Wpedantic          # Base
  -Wconversion                      # Conversiones implícitas que pierden info
  -Wshadow                          # Variables que ocultan otras
  -Wnon-virtual-dtor                # Clase polimórfica sin dtor virtual
  -Wold-style-cast                  # Casts estilo C
  -Wnull-dereference                # Dereference de null
  -Wdouble-promotion                # float -> double implícito
  -Wformat=2                        # printf/scanf format strings
  -Wmisleading-indentation          # Indentación engañosa (if sin llaves)
  -Wduplicated-branches             # Branches idénticos en if/else
  -Wduplicated-cond                 # Condiciones duplicadas
  -Wlogical-op                      # Operaciones lógicas sospechosas
)

# Clang: -Weverything y desactivar ruido
# -Weverything -Wno-c++98-compat -Wno-c++11-compat -Wno-padded...

# MSVC
# /W4 /WX /permissive- /external:W3 /external:anglebrackets
)" << "\n\n";
}

void demo_build_types() {
    std::cout << "--- Build types (CMAKE_BUILD_TYPE) ---\n";
    std::cout << R"(
# Configuración típica en CMakeLists.txt o toolchain:

# Debug: máximo debug, sanitizers
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fno-omit-frame-pointer -fsanitize=address,undefined")

# Release: máxima optimización
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -flto=auto")

# RelWithDebInfo: optimizado con debug info
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")

# MinSizeRel: optimizar tamaño
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Os -DNDEBUG")

# En CMakePresets.json (recomendado):
# "cacheVariables": {
#   "CMAKE_BUILD_TYPE": "Debug",
#   "CMAKE_CXX_FLAGS_DEBUG": "-O0 -g -fsanitize=address,undefined"
# }
)" << "\n\n";
}

void demo_lto() {
    std::cout << "--- LTO (Link Time Optimization) ---\n";
    std::cout << R"(
# LTO permite optimizaciones cross-TU:
# - Inlining cross-file
# - Devirtualization
# - Whole-program optimization

# GCC/Clang:
# -flto              # Full LTO (lento, memoria)
# -flto=auto         # ThinLTO (recomendado, escalable)
# -flto=thin         # Explícito thin

# CMake moderno (3.18+):
target_link_options(mi_lib INTERFACE
  $<$<CXX_COMPILER_ID:GNU,Clang>:-flto=auto>
)

# O global (menos recomendado):
# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)  # CMake 3.9+

# Requiere linker con soporte:
# - lld (LLVM): -fuse-ld=lld
# - gold: -fuse-ld=gold
# - mold: -fuse-ld=mold (más rápido)

# ThinLTO: paralelizable, usa cache (.thinlto.bc)
# -flto=thin -fthinlto-cache-dir=/tmp/lto_cache
)" << "\n\n";
}

void demo_cmake_integration() {
    std::cout << "--- Integración completa en CMakeLists.txt ---\n";
    std::cout << R"(
cmake_minimum_required(VERSION 3.20)
project(mi_proyecto VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Políticas modernas
cmake_policy(SET CMP0077 NEW)  # option() type
cmake_policy(SET CMP0126 NEW)  # target_link_libraries() strict

# Opciones
option(BUILD_TESTS "Build tests" ON)
option(ENABLE_LTO "Enable LTO" ON)

# Build types flags
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fno-omit-frame-pointer -fsanitize=address,undefined")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")

# LTO opcional
if (ENABLE_LTO)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto=auto")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto=auto")
endif()

# Warnings por compiler (generator expressions)
add_compile_options(
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX /permissive->
)
# O mejor por target (ver lección 01)

add_subdirectory(src)
enable_testing()
)" << "\n\n";
}

int main() {
    std::cout << "=== LECCIÓN 04: Warnings y modos de build ===\n\n";

    demo_warnings();
    demo_build_types();
    demo_lto();
    demo_cmake_integration();

    std::cout << "=== FIN LECCIÓN 04 ===\n";
    return 0;
}