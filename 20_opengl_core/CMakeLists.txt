# =============================================================================
# Módulo 20 — CMake para construir las lecciones GL a mano.
# TODAS las lecciones usan SDL3 + OpenGL (via include/gl_util.hpp): acá tienen
# targets las 18. La L09 (stb_image) y la L18 (tinyobjloader) además llevan
# sus headers vía FetchContent.
#
#   cmake -B build -S .
#   cmake --build build -j2
#   ./build/leccion_01_contexto_core
#
# VERSIONES PINNEADAS (verificadas contra los repos oficiales en la sesión
# de construcción, NUNCA master):
#   - SDL3: release-3.4.18 — se usa la del SISTEMA via pkg-config (en Arch
#     es exactamente esa versión). Si no la tenés instalada, el bloque
#     comentado más abajo la trae y compila con FetchContent mismo.
#   - stb: NO usa tags; está pinneado al COMMIT INMUTABLE
#     a7d4c2175630e1ea5b6f8dd00ab96fed0b97b96d (master es movedizo).
#   - tinyobjloader: tag v2.0.0rc13 (el tag "rc13" real del repo oficial).
# =============================================================================
cmake_minimum_required(VERSION 3.20)
project(modulo20_opengl_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# ---- SDL3 del sistema (pkg-config) -------------------------------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED IMPORTED_TARGET sdl3)

# ---- Alternativa: SDL3 por FetchContent si no está instalada -----------------
# FetchContent_Declare(
#     sdl3
#     GIT_REPOSITORY https://github.com/libsdl-org/SDL
#     GIT_TAG        release-3.4.18     # verificado: refs/tags/release-3.4.18
# )
# set(SDL_SHARED OFF CACHE BOOL "" FORCE)   # la lección linkea estática o dinámica: eligé vos
# FetchContent_MakeAvailable(sdl3)

# ---- stb (header-only; commit inmutable, sin tags en el repo) ----------------
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb
    GIT_TAG        a7d4c2175630e1ea5b6f8dd00ab96fed0b97b96d
)
FetchContent_GetProperties(stb)
if(NOT stb_POPULATED)
    FetchContent_Populate(stb)
endif()
set(STB_INCLUDE_DIR ${stb_SOURCE_DIR})   # header-only: no se compila NADA

# ---- tinyobjloader (tag real v2.0.0rc13) --------------------------------------
FetchContent_Declare(
    tinyobjloader
    GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader
    GIT_TAG        v2.0.0rc13
)
FetchContent_GetProperties(tinyobjloader)
if(NOT tinyobjloader_POPULATED)
    FetchContent_Populate(tinyobjloader)
endif()
set(TINYOBJ_INCLUDE_DIR ${tinyobjloader_SOURCE_DIR})  # header-only vía tiny_obj_loader.h

# ---- El vertex source de GL 3.3 core: glvnd entrega el target OpenGL ---------
find_package(OpenGL REQUIRED COMPONENTS OpenGL)

# ---- Targets: las 18 lecciones (todas usan SDL3 + GL) -------------------------
foreach(leccion IN ITEMS
        leccion_01_contexto_core
        leccion_02_pipeline_basico
        leccion_03_varyings_uniforms
        leccion_04_compilacion_linking
        leccion_05_hot_reload_ubo
        leccion_06_vbo_ebo_vao
        leccion_07_buffer_usages
        leccion_08_formato_vertices
        leccion_09_stb_image
        leccion_10_texturas_mipmaps
        leccion_11_srgb_atlas
        leccion_12_estado_global
        leccion_13_estado_cache
        leccion_14_draw_calls_instancing
        leccion_15_fbo_render_texture
        leccion_16_fbo_blit_msaa
        leccion_17_debug_khr_debug
        leccion_18_modelos_tinyobjloader)
    add_executable(${leccion} ${leccion}.cpp)
    target_link_libraries(${leccion} PRIVATE PkgConfig::SDL3 OpenGL::OpenGL)
    target_compile_options(${leccion} PRIVATE -Wall -Wextra)
endforeach()

# La L09 necesita stb_image.h; la L18, tiny_obj_loader.h
target_include_directories(leccion_09_stb_image PRIVATE ${STB_INCLUDE_DIR})
target_include_directories(leccion_18_modelos_tinyobjloader PRIVATE ${TINYOBJ_INCLUDE_DIR})
