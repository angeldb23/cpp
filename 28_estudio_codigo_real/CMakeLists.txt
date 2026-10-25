# =============================================================================
# MÓDULO 28 - Estudio de Código Real: Arquitectura de Librerías
# 9 lecciones, 8 ejercicios, 8 soluciones, QUIZ.md, cierre/INFORME_TECNICO.md
#
# Dependencia externa: NINGUNA. Las cinco librerías estudiadas (GLM,
# miniaudio, Box2D, Dear ImGui, SDL3) se analizan con código propio y
# simplificado: las lecciones y los tests son headless y no necesitan
# headers, ventanas, GPU ni dispositivo de audio.
#
#   cmake -B build -S .
#   cmake --build build -j2
#   ctest --test-dir build          # corre las 8 soluciones
# =============================================================================
cmake_minimum_required(VERSION 3.20)
project(m28_estudio_codigo_real LANGUAGES CXX)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive- /Zc:__cplusplus)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# =============================================================================
# Lección 01: es la única con main propio (detrás de un define)
# Las lecciones 02-09 son headers de estudio: se validan con -fsyntax-only.
# =============================================================================
add_executable(leccion_01_criterios_seleccion
    lecciones/leccion_01_criterios_seleccion.cpp)
target_include_directories(leccion_01_criterios_seleccion PRIVATE include)
target_compile_definitions(leccion_01_criterios_seleccion PRIVATE
    MODULO28_LECCION01_STANDALONE)

set(LECCIONES_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/lecciones/leccion_02_glm_arquitectura.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lecciones/leccion_03_glm_simd_constexpr.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lecciones/leccion_04_miniaudio.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lecciones/leccion_05_box2d_core.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lecciones/leccion_06_box2d_solver.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lecciones/leccion_07_imgui.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lecciones/leccion_08_sdl3.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lecciones/leccion_09_sintesis.hpp
)

if(UNIX)
    add_custom_target(check_lecciones
        COMMAND ${CMAKE_COMMAND} -E echo "Verificando sintaxis de las lecciones 02-09"
        COMMAND ${CMAKE_CXX_COMPILER} -std=c++20 -Wall -Wextra -Wpedantic
                -I${CMAKE_CURRENT_SOURCE_DIR}/include -fsyntax-only
                ${LECCIONES_HEADERS}
        COMMENT "Sintaxis OK: 8 headers de lecciones"
        VERBATIM)
endif()

# =============================================================================
# Ejercicios y soluciones (lógica pura con test_framework.hpp)
# =============================================================================
set(EJERCICIOS
    ejercicio_01_criterios
    ejercicio_02_glm
    ejercicio_03_miniaudio
    ejercicio_04_box2d
    ejercicio_05_imgui
    ejercicio_06_sdl3
    ejercicio_07_patrones
    ejercicio_08_informe
)

foreach(ejercicio IN LISTS EJERCICIOS)
    add_executable(${ejercicio} ejercicios/${ejercicio}.cpp)
    target_include_directories(${ejercicio} PRIVATE include)

    string(REPLACE "ejercicio_" "solucion_" solucion "${ejercicio}")
    add_executable(${solucion} soluciones/${solucion}.cpp)
    target_include_directories(${solucion} PRIVATE include)

    # Solo las soluciones son tests que deben pasar (CTest).
    add_test(NAME ${solucion} COMMAND ${solucion})
endforeach()

# =============================================================================
# check_all: compila lecciones + ejercicios + soluciones y valida headers
# =============================================================================
set(TODOS_LOS_TARGETS leccion_01_criterios_seleccion)
foreach(ejercicio IN LISTS EJERCICIOS)
    string(REPLACE "ejercicio_" "solucion_" solucion "${ejercicio}")
    list(APPEND TODOS_LOS_TARGETS ${ejercicio} ${solucion})
endforeach()

add_custom_target(check_all
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${TODOS_LOS_TARGETS}
    COMMENT "Compilando 1 lección ejecutable, 8 ejercicios y 8 soluciones"
    VERBATIM)

if(UNIX)
    add_dependencies(check_all check_lecciones)
endif()
