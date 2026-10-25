# =============================================================================
# MÓDULO 24 - Herramientas de Motor: Dear ImGui, Profiling y UI
# 12 lecciones, 11 ejercicios, 11 soluciones, QUIZ.md
#
# Dependencias:
#   - SDL3 + OpenGL 3.3 core (backend de la ventana). Se usa la SDL3 del
#     sistema vía pkg-config o find_package; si no está, se puede activar el
#     bloque FetchContent comentado.
#   - Dear ImGui, rama docking, tag INMUTABLE v1.91.5-docking (verificado con
#     `git ls-remote --tags https://github.com/ocornut/imgui.git`). El tag
#     existe y trae DockSpaceOverViewport + backends SDL3/OpenGL3 + viewports.
#     NUNCA master: master no tiene docking.
#
#   cmake -B build -S .
#   cmake --build build -j2
#   ctest --test-dir build
#
# Las 11 soluciones son lógica pura (no linkean ImGui ni abren ventana); sus
# tests se registran en CTest. Los 11 ejercicios fallan a propósito hasta que
# el dueño los resuelva, así que no se registran.
# =============================================================================
cmake_minimum_required(VERSION 3.20)
project(m24_herramientas_motor LANGUAGES CXX)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive- /Zc:__cplusplus)
else()
    add_compile_options(-Wall -Wextra)
endif()

include(FetchContent)

# ---- SDL3 (sistema primero; FetchContent como alternativa) -------------------
find_package(SDL3 CONFIG QUIET)
if(SDL3_FOUND)
    set(SDL3_TARGET SDL3::SDL3)
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL3 REQUIRED IMPORTED_TARGET sdl3)
    set(SDL3_TARGET PkgConfig::SDL3)
endif()

# Alternativa si no tenés SDL3 instalada (descomentá):
# FetchContent_Declare(
#     sdl3
#     GIT_REPOSITORY https://github.com/libsdl-org/SDL
#     GIT_TAG        release-3.4.18
# )
# set(SDL_SHARED OFF CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(sdl3)
# set(SDL3_TARGET SDL3::SDL3)

find_package(OpenGL REQUIRED COMPONENTS OpenGL)

# ---- Dear ImGui v1.91.5-docking (tag fijo, NO master) ------------------------
if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)  # ImGui no trae CMakeLists: hay que poblarlo
endif()
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG        v1.91.5-docking
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC ${SDL3_TARGET} OpenGL::OpenGL)
target_compile_features(imgui PUBLIC cxx_std_17)
if(NOT MSVC)
    target_compile_options(imgui PRIVATE -w)  # código de terceros: no ensuciar el build
endif()

# =============================================================================
# Lecciones (usan ImGui + SDL3 + OpenGL 3.3)
# =============================================================================
set(LECCIONES
    leccion_01_imgui_contexto_ciclo
    leccion_02_widgets_ids_layout
    leccion_03_entity_inspector
    leccion_04_debug_windows_hierarchy
    leccion_05_profiling_frame_graph
    leccion_06_memory_tracker
    leccion_07_gizmo_math
    leccion_08_property_grid_curve_color
    leccion_09_consola_logging
    leccion_10_hot_reload_assets
    leccion_11_layout_persistente_docking
    leccion_12_testing_visual
)

foreach(leccion IN LISTS LECCIONES)
    add_executable(${leccion} ${leccion}.cpp)
    target_link_libraries(${leccion} PRIVATE imgui)
    target_include_directories(${leccion} PRIVATE include)
endforeach()

# =============================================================================
# Ejercicios y soluciones (lógica pura: sin ventana, sin ImGui)
# =============================================================================
set(EJERCICIOS
    ejercicio_01_frame_ids
    ejercicio_02_consola
    ejercicio_03_property_grid
    ejercicio_04_frame_profiler
    ejercicio_05_memory_tracker
    ejercicio_06_gizmo
    ejercicio_07_curve_editor
    ejercicio_08_dock_layout
    ejercicio_09_golden_frames
    ejercicio_10_asset_watcher
    ejercicio_11_entity_inspector
)

foreach(ejercicio IN LISTS EJERCICIOS)
    add_executable(${ejercicio} ejercicios/${ejercicio}.cpp)
    target_include_directories(${ejercicio} PRIVATE include)

    string(REPLACE "ejercicio_" "solucion_" solucion "${ejercicio}")
    add_executable(${solucion} soluciones/${solucion}.cpp)
    target_include_directories(${solucion} PRIVATE include)

    add_test(NAME ${solucion} COMMAND ${solucion})
endforeach()

# =============================================================================
# check_all: compila lecciones + ejercicios + soluciones
# =============================================================================
set(TODOS_LOS_TARGETS ${LECCIONES})
foreach(ejercicio IN LISTS EJERCICIOS)
    string(REPLACE "ejercicio_" "solucion_" solucion "${ejercicio}")
    list(APPEND TODOS_LOS_TARGETS ${ejercicio} ${solucion})
endforeach()

add_custom_target(check_all
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${TODOS_LOS_TARGETS}
    COMMENT "Compilando 12 lecciones, 11 ejercicios y 11 soluciones del módulo 24"
    VERBATIM
)
