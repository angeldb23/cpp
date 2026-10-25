/* =============================================================================
   MÓDULO 15 - LECCIÓN 06: Caché y velocidad — ccache, precompiled headers,
   builds paralelos
   =============================================================================
   TEMA 6 de TEMAS.md (2 lecciones): Acelerar builds.

   CCACHE — COMPILER CACHE:
   - Cachea resultados de compilación (preprocesador + compilación).
   - Reutiliza objetos si inputs idénticos (source + flags + compiler version).
   - Velocidad: 5-10x en rebuilds incrementales.
   - Instalación: apt install ccache / brew install ccache
   - Integración CMake (3.18+):
     set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
     # O en toolchain / CMakePresets.json
   - Config: ccache --max-size=10G --max-files=100000
   - Stats: ccache -s
   - Comprimir cache: ccache --compress

   PRECOMPILED HEADERS (PCH):
   - Headers pesados (Boost, Qt, stdc++.h) se precompilan una vez.
   - CMake 3.16+: target_precompile_headers()
     target_precompile_headers(mi_lib PRIVATE
       <vector> <string> <memory>  # headers comunes
       "pch.h"                     # header propio con includes pesados
     )
   - GCC: -Winvalid-pch -include pch.h
   - Clang: similar
   - MSVC: /Yc /Yu
   - CMake 3.20+: COTRE (Compile Options Target Result Export) para PCH transitivos.

   UNITY BUILDS (Jumbo / Unity builds):
   - Une múltiples .cpp en uno solo -> menos overhead de frontend.
   - CMake 3.16+: target_unity_build()
     set_target_properties(mi_lib PROPERTIES
       UNITY_BUILD ON
       UNITY_BUILD_BATCH_SIZE 8   # .cpp por archivo unificado
     )
   - OJO: puede romper ODR si hay static/anonymous namespace conflicts.
   - No usar en librerías header-only ni templates pesados.

   BUILDS PARALELOS:
   - cmake --build --parallel N  (CMake 3.12+)
   - make -jN / ninja -jN
   - Ninja por defecto usa todos los cores.
   - cmake --build --preset=release --parallel 8
   - En CI: limitar jobs para no OOM (ej. 4-8).

   DISTRIBUTED BUILDS (icecc, distcc):
   - icecc (Icecream): distribuye compilación en cluster.
   - distcc: más simple, menos features.
   - Útil para proyectos grandes (LLVM, Chromium, Qt).

   CMAKE: CONFIGURACIÓN DE PARALELISMO:
     set(CMAKE_BUILD_PARALLEL_LEVEL 8)  # CMake 3.12+
     # O en CMakePresets.json: "jobs": 8

   CACHE DE DEPENDENCIAS (vcpkg/Conan cache):
   - vcpkg: cache binario global (~/.cache/vcpkg)
   - Conan: cache de paquetes (~/.conan2/cache)
   - En CI: cachear ~/.cache/vcpkg y ~/.conan2/cache

   INCREMENTAL LINKING:
   - /INCREMENTAL (MSVC) / -Wl,--incremental (ld)
   - Solo para Debug, desactivar en Release.

   MOLD LINKER (más rápido que lld/gold):
   - mold: linker ultra-rápido (Rust)
   - -fuse-ld=mold
   - En CMake: set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=mold")

   RESUMEN CHECKLIST BUILD RÁPIDO:
   [ ] ccache activado
   [ ] PCH para headers pesados
   [ ] Unity builds (si aplica)
   [ ] Ninja generator
   [ ] Paralelismo: -j$(nproc)
   [ ] mold linker
   [ ] ccache cache en CI
   [ ] PCH transitivos (CMake 3.20+) */

#include <iostream>
#include <string>

void demo_ccache() {
    std::cout << "--- ccache ---\n";
    std::cout << R"(
# Instalación
# Ubuntu: apt install ccache
# Arch: pacman -S ccache
# macOS: brew install ccache

# Integración CMake (3.18+)
set(CMAKE_CXX_COMPILER_LAUNCHER ccache)

# O en CMakePresets.json:
# "cacheVariables": { "CMAKE_CXX_COMPILER_LAUNCHER": "ccache" }

# Configuración recomendada
ccache --max-size=10G
ccache --max-files=100000
ccache --compress              # Comprime cache (ahorra espacio)
ccache --compression-level=6   # Zstd level 6 (default)

# Stats
ccache -s
# Cache hit rate > 90% es bueno

# En CI: cachear ~/.cache/ccache
# GitHub Actions:
# - uses: actions/cache@v3
#   with:
#     path: ~/.cache/ccache
#     key: ccache-${{ runner.os }}-${{ hashFiles('**/CMakeLists.txt') }}
)" << "\n\n";
}

void demo_pch() {
    std::cout << "--- Precompiled Headers (PCH) ---\n";
    std::cout << R"(
# CMake 3.16+: target_precompile_headers()
target_precompile_headers(mi_lib PRIVATE
  <vector> <string> <memory> <algorithm>  # STL común
  <fmt/format.h> <spdlog/spdlog.h>        # Deps pesadas
  "pch.h"                                 # Header propio con includes pesados
)

# pch.h ejemplo:
#ifndef PCH_H
#define PCH_H
#include <vector>
#include <string>
#include <memory>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#endif

# GCC/Clang flags automáticos:
# -Winvalid-pch -include pch.h

# CMake 3.20+: PCH transitivos (COTRE)
# target_precompile_headers(mi_lib INTERFACE <header>)
# Se propaga a consumidores automáticamente.

# OJO: PCH debe ser compatible (mismo compiler, flags, standard).
# Cambiar flags invalida PCH.
)" << "\n\n";
}

void demo_unity() {
    std::cout << "--- Unity / Jumbo Builds ---\n";
    std::cout << R"(
# CMake 3.16+: target_unity_build()
set_target_properties(mi_lib PROPERTIES
  UNITY_BUILD ON
  UNITY_BUILD_BATCH_SIZE 8    # 8 .cpp por archivo unificado
  UNITY_BUILD_UNIQUE_ID mi_lib_  # Prefijo para símbolos internos
)

# Ventajas: menos overhead frontend, mejor optimización cross-TU
# Desventajas:
# - Rompe anonymous namespace / static en .cpp
# - Menos paralelismo (menos archivos para compilar en paralelo)
# - Rebuild completo si cambia un header
# - No usar en: header-only libs, templates pesados, código con static internos

# Recomendado: solo para targets grandes (>50 .cpp), no librerías pequeñas.
)" << "\n\n";
}

void demo_parallel() {
    std::cout << "--- Builds paralelos ---\n";
    std::cout << R"(
# Ninja (default en CMake 3.20+):
cmake -G Ninja -B build
cmake --build build --parallel 8    # -j8
cmake --build --preset=release --parallel 8

# Make:
make -j$(nproc)

# CMake 3.12+:
set(CMAKE_BUILD_PARALLEL_LEVEL 8)  # Default global

# En CMakePresets.json:
# "buildPresets": [
#   { "name": "release", "configurePreset": "release", "jobs": 8 }
# ]

# En CI: limitar jobs para no OOM
# jobs: 4  (en runners compartidos)
)" << "\n\n";
}

void demo_mold() {
    std::cout << "--- mold linker ---\n";
    std::cout << R"(
# mold: linker ultra-rápido (Rust, compatible ELF/Mach-O/COFF)
# Instalación:
# Ubuntu 22.04+: apt install mold
# Arch: pacman -S mold
# macOS: brew install mold
# O bajar release de github.com/rui314/mold

# Uso:
# -fuse-ld=mold

# CMake:
set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=mold")
set(CMAKE_SHARED_LINKER_FLAGS "-fuse-ld=mold")
set(CMAKE_MODULE_LINKER_FLAGS "-fuse-ld=mold")

# En CMakePresets.json:
# "cacheVariables": {
#   "CMAKE_EXE_LINKER_FLAGS": "-fuse-ld=mold",
#   "CMAKE_SHARED_LINKER_FLAGS": "-fuse-ld=mold"
# }

# Velocidad: 2-5x más rápido que lld/gold
# Soporte: ELF (Linux), Mach-O (macOS), COFF (Windows)
# Open Source, MIT license
)" << "\n\n";
}

void demo_checklist() {
    std::cout << "--- Checklist build rápido ---\n";
    std::cout << R"(
[ ] ccache activado (CMAKE_CXX_COMPILER_LAUNCHER=ccache)
[ ] PCH para headers pesados (target_precompile_headers)
[ ] Unity builds para targets grandes (>50 .cpp)
[ ] Ninja generator (cmake -G Ninja)
[ ] Paralelismo: -j$(nproc) o cmake --build --parallel
[ ] mold linker (-fuse-ld=mold)
[ ] ccache cache en CI (cachear ~/.cache/ccache)
[ ] PCH transitivos (CMake 3.20+ COTRE)
[ ] LTO en Release (-flto=auto)
[ ] mold en CI (si disponible)
[ ] Cache vcpkg/Conan en CI
)" << "\n\n";
}

int main() {
    std::cout << "=== LECCIÓN 06: Caché y velocidad — ccache, PCH, builds paralelos ===\n\n";

    demo_ccache();
    demo_pch();
    demo_unity();
    demo_parallel();
    demo_mold();
    demo_checklist();

    std::cout << "=== FIN LECCIÓN 06 ===\n";
    return 0;
}