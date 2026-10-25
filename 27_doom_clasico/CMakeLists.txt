# =============================================================================
# MÓDULO 27 - Doom Clásico: De C a C++ (id Software vs Source Ports)
# 14 lecciones, 10 ejercicios, 10 soluciones, QUIZ.md
#
# Dependencia externa: NINGUNA. Las lecciones son análisis ejecutable con
# estructuras propias (fixed-point, BSP, thinkers, WAD, state machines); no
# clonan ni linkean Doom, GZDoom, Odamex, EDGE ni Eternity, y no usan WADs
# comerciales. Para pruebas reales se usa Freedoom (contenido libre).
#
#   cmake -B build -S .
#   cmake --build build -j2
#   ctest --test-dir build          # corre las 10 soluciones
# =============================================================================
cmake_minimum_required(VERSION 3.20)
project(m27_doom_clasico LANGUAGES CXX)

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
# Lecciones (cada una es un ejecutable de análisis, sin assets externos)
# =============================================================================
set(LECCIONES
    leccion_01_idtech1_estructura
    leccion_02_fixed_point
    leccion_03_bsp_mapa
    leccion_04_visplanes_render
    leccion_05_thinkers_tic
    leccion_06_state_machine_ai
    leccion_07_networking
    leccion_08_wad_zone
    leccion_09_source_ports
    leccion_10_modernizacion
    leccion_11_doom3_analysis
    leccion_12_subsistemas_detalle
    leccion_13_source_ports_profundidad
    leccion_14_modernizacion_avanzada
)

foreach(leccion IN LISTS LECCIONES)
    add_executable(${leccion} ${leccion}.cpp)
    target_include_directories(${leccion} PRIVATE include)
endforeach()

# =============================================================================
# Ejercicios y soluciones (lógica pura con test_framework.hpp)
# =============================================================================
set(EJERCICIOS
    ejercicio_01_estructura
    ejercicio_02_fixed_point
    ejercicio_03_bsp
    ejercicio_04_visplanes
    ejercicio_05_thinkers
    ejercicio_06_statemachine
    ejercicio_07_networking
    ejercicio_08_wad_zone
    ejercicio_09_sourceports
    ejercicio_10_modernizacion
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
# check_all: compila las 14 lecciones + 10 ejercicios + 10 soluciones
# =============================================================================
set(TODOS_LOS_TARGETS ${LECCIONES})
foreach(ejercicio IN LISTS EJERCICIOS)
    string(REPLACE "ejercicio_" "solucion_" solucion "${ejercicio}")
    list(APPEND TODOS_LOS_TARGETS ${ejercicio} ${solucion})
endforeach()

add_custom_target(check_all
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${TODOS_LOS_TARGETS}
    COMMENT "Compilando 14 lecciones, 10 ejercicios y 10 soluciones"
    VERBATIM)
