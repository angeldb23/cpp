# =============================================================================
# MÓDULO 25 - Audio y Empaquetado de Assets
# 11 lecciones, 11 ejercicios, 11 soluciones, QUIZ.md
#
# Dependencia externa: miniaudio (solo la demo real de la lección 01). Se trae
# con FetchContent desde el tag INMUTABLE 0.11.25 (verificado con
# `git ls-remote --tags`; es el más nuevo del repo al día de escribir esto).
#
# IMPORTANTE (regla del curso): el código REAL de miniaudio (ma_context_init,
# ma_device_init, ma_engine_init, ...) se COMPARTE con un único target
# library; las lecciones compilan headless SIN miniaudio y son ejecutables
# sin hardware de audio (así las verifica verificar.sh). Solo un target,
# demo_01_miniaudio_real, define DEMO_MINIAUDIO y linkea la librería: ese
# target requiere un device de audio (se NUNCA ejecuta en CI: solo compila).
#
#   cmake -B build -S .
#   cmake --build build -j2
#   ctest --test-dir build          # corre las 11 soluciones
# =============================================================================
cmake_minimum_required(VERSION 3.20)
project(m25_audio_y_empaquetado LANGUAGES C CXX)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive- /Zc:__cplusplus)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ---- miniaudio 0.11.25 (tag fijo, NO master) --------------------------------
include(FetchContent)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# El CMakeLists propio de miniaudio ya deja examples/tests/tools en OFF por
# defecto (verificado en el tag 0.11.25): con FetchContent solamente creamos
# el target "miniaudio" (estático, miniaudio.c) y los extra nodes
# (miniaudio_reverb_node, etc.) si los extras compilan solos.
FetchContent_Declare(
    miniaudio
    GIT_REPOSITORY https://github.com/mackron/miniaudio
    GIT_TAG        0.11.25
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(miniaudio)

# =============================================================================
# Lecciones (headless: sin miniaudio y sin hardware de audio)
# =============================================================================
set(LECCIONES
    leccion_01_miniaudio_contexto_device
    leccion_02_callback_mezcla_conversiones
    leccion_03_streaming_vs_decoding
    leccion_04_engine_voces_pool
    leccion_05_buses_pan_ducking
    leccion_06_delay
    leccion_07_filtros_freeverb_distorsion
    leccion_08_pack_assets_indice
    leccion_09_pack_vfs_crc32
    leccion_10_pipeline_assets_manifest
    leccion_11_localizacion_bancos
)

foreach(leccion IN LISTS LECCIONES)
    add_executable(${leccion} ${leccion}.cpp)
    target_include_directories(${leccion} PRIVATE include)
endforeach()

# =============================================================================
# Demo REAL con hardware de audio: SOLO lección 01. Este target define
# DEMO_MINIAUDIO (compile definition del target) y linkea la librería
# miniaudio. Abrir el device REQUIERE un backend presente
# (PipeWire/ALSA en Arch): NO se ejecuta nunca desde CI, es solo para
# compilar el path real (y para el dueño de la máquina: correrlo a mano).
# El resto de las lecciones deja su bloque #if DEMO_MINIAUDIO
# apagado cuando compila por verificar.sh (g++ sin -DDEMO_MINIAUDIO).
# =============================================================================
add_executable(demo_01_miniaudio_real leccion_01_miniaudio_contexto_device.cpp)
target_include_directories(demo_01_miniaudio_real PRIVATE include)
target_link_libraries(demo_01_miniaudio_real PRIVATE miniaudio)
target_compile_definitions(demo_01_miniaudio_real PRIVATE DEMO_MINIAUDIO)

# =============================================================================
# Ejercicios y soluciones (lógica pura: no abren ventana ni hardware)
# =============================================================================
set(EJERCICIOS
    ejercicio_01_formato_muestras
    ejercicio_02_mezclador
    ejercicio_03_streamer
    ejercicio_04_pool_voces
    ejercicio_05_buses_ducking
    ejercicio_06_delay
    ejercicio_07_biquad
    ejercicio_08_pack_writer
    ejercicio_09_crc32
    ejercicio_10_manifest
    ejercicio_11_localizador
)

foreach(ejercicio IN LISTS EJERCICIOS)
    add_executable(${ejercicio} ejercicios/${ejercicio}.cpp)

    string(REPLACE "ejercicio_" "solucion_" solucion "${ejercicio}")
    add_executable(${solucion} soluciones/${solucion}.cpp)

    # Solo las soluciones son tests que deben pasar. Los ejercicios fallan a
    # propósito hasta que se resuelvan, así que no se registran en CTest.
    add_test(NAME ${solucion} COMMAND ${solucion})
endforeach()

# =============================================================================
# check_all: compila TODO (lecciones + demo real + ejercicios + soluciones)
# =============================================================================
set(TODOS_LOS_TARGETS ${LECCIONES} demo_01_miniaudio_real)
foreach(ejercicio IN LISTS EJERCICIOS)
    string(REPLACE "ejercicio_" "solucion_" solucion "${ejercicio}")
    list(APPEND TODOS_LOS_TARGETS ${ejercicio} ${solucion})
endforeach()

add_custom_target(check_all
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${TODOS_LOS_TARGETS}
    COMMENT "Compilando las 11 lecciones, la demo real, 11 ejercicios y 11 soluciones del módulo 25"
    VERBATIM
)
