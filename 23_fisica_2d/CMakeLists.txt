# =============================================================================
# MÓDULO 23 - Física 2D
# 14 lecciones, 11 ejercicios, 11 soluciones, QUIZ.md
#
# Dependencia externa: Box2D (solo lecciones 10 y 11). Se trae con FetchContent
# desde el tag INMUTABLE v2.4.2 (verificado con `git ls-remote --tags`).
# El resto del módulo es lógica pura y no necesita librerías gráficas.
#
#   cmake -B build -S .
#   cmake --build build -j2
#   ctest --test-dir build          # opcional: corre todos los tests
# =============================================================================
cmake_minimum_required(VERSION 3.20)
project(m23_fisica_2d LANGUAGES CXX)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive- /Zc:__cplusplus)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ---- Box2D v2.4.2 (tag fijo, NO master) -------------------------------------
include(FetchContent)
set(BOX2D_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_TESTBED OFF CACHE BOOL "" FORCE)
set(BOX2D_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    box2d
    GIT_REPOSITORY https://github.com/erincatto/box2d
    GIT_TAG        v2.4.2
)
FetchContent_MakeAvailable(box2d)

# =============================================================================
# Lecciones
# =============================================================================
set(LECCIONES
    leccion_01_timestep_fijo
    leccion_02_spatial_hash
    leccion_03_sweep_and_prune_quadtree
    leccion_04_sat_poligonos
    leccion_05_circle_poly_gjk
    leccion_06_impulsos_restitucion_friccion
    leccion_07_baumgarte_warm_starting
    leccion_08_cuerpos_masa_inercia
    leccion_09_joints
    leccion_10_box2d_mundo_cuerpos
    leccion_11_box2d_contactos_filtros
    leccion_12_ecs_integracion_fisica
    leccion_13_ecs_eventos_fisica
    leccion_14_debug_draw_fisica
)

foreach(leccion IN LISTS LECCIONES)
    add_executable(${leccion} ${leccion}.cpp)
    target_include_directories(${leccion} PRIVATE include)
    # Solo las lecciones de Box2D linkean la librería.
    if(leccion MATCHES "box2d")
        target_link_libraries(${leccion} PRIVATE box2d)
    endif()
endforeach()

# =============================================================================
# Ejercicios y soluciones (lógica pura: no abren ventana ni linkean Box2D)
# =============================================================================
set(EJERCICIOS
    ejercicio_01_timestep_fijo
    ejercicio_02_spatial_hash
    ejercicio_03_sweep_and_prune
    ejercicio_04_sat
    ejercicio_05_circle_poly
    ejercicio_06_impulsos
    ejercicio_07_baumgarte
    ejercicio_08_cuerpos
    ejercicio_09_joints
    ejercicio_10_filtros_debug
    ejercicio_11_ecs_fisica
)

foreach(ejercicio IN LISTS EJERCICIOS)
    add_executable(${ejercicio} ejercicios/${ejercicio}.cpp)
    target_include_directories(${ejercicio} PRIVATE include)

    string(REPLACE "ejercicio_" "solucion_" solucion "${ejercicio}")
    add_executable(${solucion} soluciones/${solucion}.cpp)
    target_include_directories(${solucion} PRIVATE include)

    # Solo las soluciones son tests que deben pasar. Los ejercicios fallan a
    # propósito hasta que se resuelvan, así que no se registran en CTest.
    add_test(NAME ${solucion} COMMAND ${solucion})
endforeach()

# =============================================================================
# check_all: compila TODO (lecciones + ejercicios + soluciones)
# =============================================================================
set(TODOS_LOS_TARGETS ${LECCIONES})
foreach(ejercicio IN LISTS EJERCICIOS)
    string(REPLACE "ejercicio_" "solucion_" solucion "${ejercicio}")
    list(APPEND TODOS_LOS_TARGETS ${ejercicio} ${solucion})
endforeach()

add_custom_target(check_all
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${TODOS_LOS_TARGETS}
    COMMENT "Compilando las 14 lecciones, 11 ejercicios y 11 soluciones del módulo 23"
    VERBATIM
)
