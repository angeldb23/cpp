/* =============================================================================
   MÓDULO 15 - LECCIÓN 07: Cache y velocidad avanzados — PCH transitivos,
   unity builds avanzados, distributed builds, linkers, CI caching
   =============================================================================
   TEMA 6 de TEMAS.md (2da lección): Cache y velocidad avanzados.

   PCH TRANSITIVOS (COTRE - CMake 3.20+):
   - target_precompile_headers(mi_lib INTERFACE <header>)
   - Se propaga a consumidores automáticamente
   - COTRE: Compile Options Target Result Export
   - Requiere: mismo compiler, flags, standard en consumidor
   - target_precompile_headers(mi_lib INTERFACE <vector> "pch.h")

   UNITY BUILDS AVANZADOS:
   - UNITY_BUILD_BATCH_SIZE: .cpp por archivo unificado (default 8)
   - UNITY_BUILD_UNIQUE_ID: prefijo para símbolos internos
   - UNITY_BUILD_CODE_BEFORE/AFTER: código inyectado
   - set_target_properties(lib PROPERTIES
       UNITY_BUILD ON
       UNITY_BUILD_BATCH_SIZE 16
       UNITY_BUILD_UNIQUE_ID mi_lib_
       UNITY_BUILD_CODE_BEFORE "#include \"pch.h\""
     )
   - UNITY_BUILD_MODE: SEPARATE (default) vs BATCH
   - OJO: symbols internos colisionan -> UNIQUE_ID
   - No usar en: header-only, templates pesados, static internos

   DISTRIBUTED BUILDS:
   - icecc (Icecream): daemon + scheduler + monitor
     - icecc --daemon (en cada nodo)
     - icecc-scheduler (uno solo)
     - cmake -DCMAKE_CXX_COMPILER_LAUNCHER=icecc
   - distcc: más simple, menos features
   - sccache: cache distribuido (Mozilla/Rust)
     - s3/redis/gcs backend
     - sccache --start-server
     - export RUSTC_WRAPPER=sccache (Rust) / CC=sccache

   LINKERS MODERNOS:
   - mold: -fuse-ld=mold (2-5x lld)
   - lld: -fuse-ld=lld (default en Clang)
   - gold: -fuse-ld=gold
   - ld.bfd: clásico, lento
   - LTO + mold: combinación ganadora

   INCREMENTAL LINKING:
   - MSVC: /INCREMENTAL (default Debug)
   - ld: -Wl,--incremental
   - mold: no soporta incremental (por diseño)
   - Desactivar en Release: /INCREMENTAL:NO / -Wl,--no-incremental

   CI CACHING AVANZADO:
   - ccache: cachear ~/.cache/ccache
   - sccache: cache distribuido (S3/GCS/Redis)
   - vcpkg binary cache: --binarycache
   - Conan: ~/.conan2/cache
   - pip/uv cache: ~/.cache/pip
   - npm cache: ~/.npm

   BUILD CACHE KEYS (CI):
   // GitHub Actions cache keys (template syntax):
   // - key: build-${{ runner.os }}-${{ hashFiles }}
   // - restore-keys: build-${{ runner.os }}-

   PARALELISMO ÓPTIMO:
   - ninja -j$(nproc) (default)
   - cmake --build --parallel $(nproc)
   - En CI: limitar a 4-8 para no OOM
   - Override: cmake --build --parallel 8

   METRICS Y MONITOREO:
   - ninja -t stats (build time, commands)
   - ninja -t graph | dot -Tpng > graph.png
   - cmake --build --target help
   - ccache -s (stats)
   - sccache --show-stats
   - time cmake --build build --target mi_lib

   PROFILING DEL BUILD:
   - cmake --build --verbose
   - CMAKE_BUILD_PARALLEL_LEVEL
   - ninja -t targets | wc -l
   - hyperfine 'cmake --build build' --warmup 3 --runs 10 */

#include <iostream>
#include <string>

void demo_pch_transitivo() {
    std::cout << "--- PCH transitivos (COTRE, CMake 3.20+) ---\n";
    std::cout << R"(
# Librería con PCH transitivo
add_library(mi_lib src/mi_lib.cpp)

# PCH propio (PRIVATE)
target_precompile_headers(mi_lib PRIVATE
  <vector> <string> <memory>
  "pch.h"
)

# PCH transitivo (INTERFACE) - se propaga a consumidores
target_precompile_headers(mi_lib INTERFACE
  <fmt/format.h>
  <spdlog/spdlog.h>
)

# Consumidor:
add_executable(mi_app main.cpp)
target_link_libraries(mi_app PRIVATE mi_lib::mi_lib)
# Automáticamente hereda PCH INTERFACE de mi_lib

# Requisitos para PCH transitivo:
# - Mismo compiler (GCC/Clang/MSVC)
# - Mismos flags relevantes (-std, -D, -I)
# - Mismo standard (C++20)
# - CMake 3.20+ (COTRE feature)
)" << "\n\n";
}

void demo_unity_avanzado() {
    std::cout << "--- Unity builds avanzados ---\n";
    std::cout << R"(
set_target_properties(mi_lib PROPERTIES
  UNITY_BUILD ON
  UNITY_BUILD_BATCH_SIZE 16          # 16 .cpp por archivo unificado
  UNITY_BUILD_UNIQUE_ID mi_lib_      # Prefijo símbolos internos
  UNITY_BUILD_CODE_BEFORE "#include \"pch.h\""  # Inyectar PCH
  UNITY_BUILD_MODE BATCH             # BATCH vs SEPARATE
)

# UNITY_BUILD_MODE:
# - SEPARATE (default): cada batch compila separado, linkea después
# - BATCH: un solo archivo unificado gigante (más rápido, menos paralelismo)

# Problemas comunes:
# - anonymous namespace en .cpp -> colisión en unity
#   Solución: UNITY_BUILD_UNIQUE_ID prefijo único
# - static variables en .cpp -> compartidas entre .cpp unificados
#   Solución: evitar static en .cpp o usar UNITY_BUILD_UNIQUE_ID
# - Templates pesados -> compilación lenta en unity
# - Headers con #pragma once -> OK

# Cuándo NO usar:
# - Header-only libs
# - Templates pesados (STL heavy)
# - Código con static/anonymous namespace en .cpp
# - Librerías pequeñas (<20 .cpp)
)" << "\n\n";
}

void demo_distributed() {
    std::cout << "--- Distributed builds ---\n";
    std::cout << R"(
# icecc (Icecream) - Distribuido
# 1. En cada nodo:
#    icecc --daemon
# 2. Un nodo scheduler:
#    icecc-scheduler -n "Mi Cluster"
# 3. En build:
#    export CMAKE_CXX_COMPILER_LAUNCHER=icecc
#    cmake -B build && cmake --build build -j$(icecc --jobs)

# distcc - más simple
# distccd --daemon --allow 192.168.1.0/24
# export CC="distcc gcc" CXX="distcc g++"
# make -j$(distcc -j)

# sccache - Cache distribuido (Rust/Mozilla)
# Backend: S3, GCS, Redis, Azure Blob, local disk
# sccache --start-server
# export CC="sccache gcc" CXX="sccache g++"
# export RUSTC_WRAPPER=sccache  # Para Rust
# sccache --start-server
# sccache --show-stats
)" << "\n\n";
}

void demo_linkers() {
    std::cout << "--- Linkers modernos ---\n";
    std::cout << R"(
# mold (más rápido, Rust)
# -fuse-ld=mold
# 2-5x más rápido que lld
# Soporte: ELF, Mach-O, COFF

# lld (LLVM linker)
# -fuse-ld=lld
# Default en Clang
# ThinLTO nativo

# gold (GNU gold)
# -fuse-ld=gold
# Más rápido que bfd, pero deprecated

# ld.bfd (clásico)
# -fuse-ld=bfd
# Lento, compatible

# LTO + mold = combo ganador
# -flto=auto -fuse-ld=mold

# CMake:
set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=mold")
set(CMAKE_SHARED_LINKER_FLAGS "-fuse-ld=mold")
set(CMAKE_MODULE_LINKER_FLAGS "-fuse-ld=mold")

# En CMakePresets.json:
# "cacheVariables": {
#   "CMAKE_EXE_LINKER_FLAGS": "-fuse-ld=mold",
#   "CMAKE_SHARED_LINKER_FLAGS": "-fuse-ld=mold"
# }
)" << "\n\n";
}

void demo_incremental() {
    std::cout << "--- Incremental linking ---\n";
    std::cout << R"(
# MSVC (default en Debug):
# /INCREMENTAL  (default Debug)
# /INCREMENTAL:NO  (Release)

# ld (GNU):
# -Wl,--incremental          # Habilitar
# -Wl,--no-incremental       # Deshabilitar

# mold: NO soporta incremental (por diseño)
# -fuse-ld=mold  -> siempre full link

# LTO + incremental = NO compatible
# -flto + /INCREMENTAL = conflict

# Recomendación:
# Debug: /INCREMENTAL (o -Wl,--incremental)
# Release: /INCREMENTAL:NO + LTO + mold
)" << "\n\n";
}

void demo_ci_caching() {
    std::cout << "--- CI Caching avanzado ---\n";
    std::cout << R"(
# GitHub Actions ccache
- uses: actions/cache@v3
  with:
    path: ~/.cache/ccache
    key: ccache-$ runner.os }}-$ hashFiles('**/CMakeLists.txt', '**/vcpkg.json', '**/conan.lock') }}
    restore-keys: |
      ccache-$ runner.os }}-

# sccache (distribuido)
# - Backend: S3, GCS, Redis, Azure Blob, local
# sccache --start-server
# export CC="sccache gcc" CXX="sccache g++"
# export RUSTC_WRAPPER=sccache
# sccache --show-stats

# vcpkg binary cache
# vcpkg install --binarycache
# vcpkg fetch --binarycache

# Conan cache
# ~/.conan2/cache
# actions/cache@v3 con path: ~/.conan2/cache

# Build cache keys (GitHub Actions)
# key: build-$ runner.os }}-$ hashFiles('**/CMakeLists.txt', '**/vcpkg.json', '**/conan.lock', '**/vcpkg.json') }}
# restore-keys: build-$ runner.os }}-
)" << "\n\n";
}

void demo_metrics() {
    std::cout << "--- Metrics y monitoreo del build ---\n";
    std::cout << R"(
# Ninja stats
ninja -t stats
# 1234 commands, 1.23s, 45.6% cache hit

# Build graph
ninja -t graph | dot -Tpng > graph.png
ninja -t targets | wc -l

# ccache stats
ccache -s
# cache hit rate: 95.2%

# sccache stats
sccache --show-stats

# Build timing
time cmake --build build --target mi_lib
# real 0m12.345s

# Ninja verbose
cmake --build build -- VERBOSE=1

# CMake build parallel level
set(CMAKE_BUILD_PARALLEL_LEVEL 8)  # CMake 3.12+

# Profiling del build
hyperfine 'cmake --build build' --warmup 3 --runs 10
)" << "\n\n";
}

int main() {
    std::cout << "=== LECCIÓN 07: Cache y velocidad avanzados (parte 2) ===\n\n";

    demo_pch_transitivo();
    demo_unity_avanzado();
    demo_distributed();
    demo_linkers();
    demo_incremental();
    demo_ci_caching();
    demo_metrics();

    std::cout << "=== FIN LECCIÓN 07 (Cache parte 2) ===\n";
    return 0;
}