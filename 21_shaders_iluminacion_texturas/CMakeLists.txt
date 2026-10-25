# MÓDULO 21 - Shaders Avanzados, Iluminación Phong y Texturas
# 16 lecciones, 10 ejercicios, 10 soluciones, QUIZ.md
# Requiere: SDL3 (sistema), OpenGL 3.3 core, tinyobjloader (FetchContent v2.0.0rc13)

cmake_minimum_required(VERSION 3.20)
project(m21_shaders_iluminacion_texturas LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Warnings estrictos
if(MSVC)
    add_compile_options(/W4 /permissive- /Zc:__cplusplus)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# ============================================================================
# SDL3 (sistema, pkg-config)
# ============================================================================
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED sdl3)

# ============================================================================
# tinyobjloader (FetchContent, tag v2.0.0rc13 verificado)
# ============================================================================
include(FetchContent)
FetchContent_Declare(
    tinyobjloader
    GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader
    GIT_TAG        v2.0.0rc13
)
FetchContent_GetProperties(tinyobjloader)
if(NOT tinyobjloader_POPULATED)
    FetchContent_Populate(tinyobjloader)
    message(STATUS "tinyobjloader source: ${tinyobjloader_SOURCE_DIR}")
endif()
set(TINYOBJ_INCLUDE_DIR ${tinyobjloader_SOURCE_DIR})

# ============================================================================
# Include directories
# ============================================================================
target_include_directories(${PROJECT_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${TINYOBJ_INCLUDE_DIR}
    ../../19_matematica_motores/include
    ../../20_opengl_core/include
)

# ============================================================================
# Link libraries comunes
# ============================================================================
set(COMMON_LIBS ${SDL3_LIBRARIES} OpenGL::GL)

# ============================================================================
# Lecciones (16) - compilan y linkean con SDL3 + OpenGL
# ============================================================================
add_executable(leccion_01_phong_blinn_phong leccion_01_phong_blinn_phong.cpp)
target_link_libraries(leccion_01_phong_blinn_phong ${COMMON_LIBS})

add_executable(leccion_02_gamma_srgb leccion_02_gamma_srgb.cpp)
target_link_libraries(leccion_02_gamma_srgb ${COMMON_LIBS})

add_executable(leccion_03_luces_direccional_puntual leccion_03_luces_direccional_puntual.cpp)
target_link_libraries(leccion_03_luces_direccional_puntual ${COMMON_LIBS})

add_executable(leccion_04_luces_spot_ubo leccion_04_luces_spot_ubo.cpp)
target_link_libraries(leccion_04_luces_spot_ubo ${COMMON_LIBS})

add_executable(leccion_05_materiales_texturas leccion_05_materiales_texturas.cpp)
target_link_libraries(leccion_05_materiales_texturas ${COMMON_LIBS})

add_executable(leccion_06_pbr_basico leccion_06_pbr_basico.cpp)
target_link_libraries(leccion_06_pbr_basico ${COMMON_LIBS})

add_executable(leccion_07_tangentes_tbn leccion_07_tangentes_tbn.cpp)
target_link_libraries(leccion_07_tangentes_tbn ${COMMON_LIBS})

add_executable(leccion_08_normal_mapping leccion_08_normal_mapping.cpp)
target_link_libraries(leccion_08_normal_mapping ${COMMON_LIBS})

add_executable(leccion_09_shadow_mapping leccion_09_shadow_mapping.cpp)
target_link_libraries(leccion_09_shadow_mapping ${COMMON_LIBS})

add_executable(leccion_10_pcf_bias_cascadas leccion_10_pcf_bias_cascadas.cpp)
target_link_libraries(leccion_10_pcf_bias_cascadas ${COMMON_LIBS})

add_executable(leccion_11_bloom_tonemap leccion_11_bloom_tonemap.cpp)
target_link_libraries(leccion_11_bloom_tonemap ${COMMON_LIBS})

add_executable(leccion_12_fxaa_color_grading leccion_12_fxaa_color_grading.cpp)
target_link_libraries(leccion_12_fxaa_color_grading ${COMMON_LIBS})

add_executable(leccion_13_compute_shader_concepto leccion_13_compute_shader_concepto.cpp)
target_link_libraries(leccion_13_compute_shader_concepto ${COMMON_LIBS})

add_executable(leccion_14_optimizacion_ubo_arrays leccion_14_optimizacion_ubo_arrays.cpp)
target_link_libraries(leccion_14_optimizacion_ubo_arrays ${COMMON_LIBS})

add_executable(leccion_15_camara_yaw_pitch leccion_15_camara_yaw_pitch.cpp)
target_link_libraries(leccion_15_camara_yaw_pitch ${COMMON_LIBS})

add_executable(leccion_16_mallas_grupos_materiales leccion_16_mallas_grupos_materiales.cpp)
target_link_libraries(leccion_16_mallas_grupos_materiales ${COMMON_LIBS})

# ============================================================================
# Ejercicios (10) - lógica pura, NO abren ventana, solo test_framework.hpp
# ============================================================================
add_executable(ejercicio_01_lint_phong ejercicios/ejercicio_01_lint_phong.cpp)
target_link_libraries(ejercicio_01_lint_phong)

add_executable(ejercicio_02_atenuacion_spot ejercicios/ejercicio_02_atenuacion_spot.cpp)
target_link_libraries(ejercicio_02_atenuacion_spot)

add_executable(ejercicio_03_layout_std140_luces ejercicios/ejercicio_03_layout_std140_luces.cpp)
target_link_libraries(ejercicio_03_layout_std140_luces)

add_executable(ejercicio_04_material_srgb ejercicios/ejercicio_04_material_srgb.cpp)
target_link_libraries(ejercicio_04_material_srgb)

add_executable(ejercicio_05_tbn_unpack ejercicios/ejercicio_05_tbn_unpack.cpp)
target_link_libraries(ejercicio_05_tbn_unpack)

add_executable(ejercicio_06_shadowmap_bias_pcf ejercicios/ejercicio_06_shadowmap_bias_pcf.cpp)
target_link_libraries(ejercicio_06_shadowmap_bias_pcf)

add_executable(ejercicio_07_bloom_tonemap ejercicios/ejercicio_07_bloom_tonemap.cpp)
target_link_libraries(ejercicio_07_bloom_tonemap)

add_executable(ejercicio_08_camara_yaw_pitch ejercicios/ejercicio_08_camara_yaw_pitch.cpp)
target_link_libraries(ejercicio_08_camara_yaw_pitch)

add_executable(ejercicio_09_decisiones_features ejercicios/ejercicio_09_decisiones_features.cpp)
target_link_libraries(ejercicio_09_decisiones_features)

add_executable(ejercicio_10_mallas_material_groups ejercicios/ejercicio_10_mallas_material_groups.cpp)
target_link_libraries(ejercicio_10_mallas_material_groups)

# ============================================================================
# Soluciones (10) - lógica pura, tests pasan
# ============================================================================
add_executable(sol_ejercicio_01_lint_phong soluciones/ejercicio_01_lint_phong.cpp)
target_link_libraries(sol_ejercicio_01_lint_phong)

add_executable(sol_ejercicio_02_atenuacion_spot soluciones/ejercicio_02_atenuacion_spot.cpp)
target_link_libraries(sol_ejercicio_02_atenuacion_spot)

add_executable(sol_ejercicio_03_layout_std140_luces soluciones/ejercicio_03_layout_std140_luces.cpp)
target_link_libraries(sol_ejercicio_03_layout_std140_luces)

add_executable(sol_ejercicio_04_material_srgb soluciones/ejercicio_04_material_srgb.cpp)
target_link_libraries(sol_ejercicio_04_material_srgb)

add_executable(sol_ejercicio_05_tbn_unpack soluciones/ejercicio_05_tbn_unpack.cpp)
target_link_libraries(sol_ejercicio_05_tbn_unpack)

add_executable(sol_ejercicio_06_shadowmap_bias_pcf soluciones/ejercicio_06_shadowmap_bias_pcf.cpp)
target_link_libraries(sol_ejercicio_06_shadowmap_bias_pcf)

add_executable(sol_ejercicio_07_bloom_tonemap soluciones/ejercicio_07_bloom_tonemap.cpp)
target_link_libraries(sol_ejercicio_07_bloom_tonemap)

add_executable(sol_ejercicio_08_camara_yaw_pitch soluciones/ejercicio_08_camara_yaw_pitch.cpp)
target_link_libraries(sol_ejercicio_08_camara_yaw_pitch)

add_executable(sol_ejercicio_09_decisiones_features soluciones/ejercicio_09_decisiones_features.cpp)
target_link_libraries(sol_ejercicio_09_decisiones_features)

add_executable(sol_ejercicio_10_mallas_material_groups soluciones/ejercicio_10_mallas_material_groups.cpp)
target_link_libraries(sol_ejercicio_10_mallas_material_groups)

# ============================================================================
# Compilación de verificación (sanity check: todas las lecciones + ejercicios + soluciones)
# ============================================================================
add_custom_target(check_all
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target
        leccion_01_phong_blinn_phong
        leccion_02_gamma_srgb
        leccion_03_luces_direccional_puntual
        leccion_04_luces_spot_ubo
        leccion_05_materiales_texturas
        leccion_06_pbr_basico
        leccion_07_tangentes_tbn
        leccion_08_normal_mapping
        leccion_09_shadow_mapping
        leccion_10_pcf_bias_cascadas
        leccion_11_bloom_tonemap
        leccion_12_fxaa_color_grading
        leccion_13_compute_shader_concepto
        leccion_14_optimizacion_ubo_arrays
        leccion_15_camara_yaw_pitch
        leccion_16_mallas_grupos_materiales
        ejercicio_01_lint_phong
        ejercicio_02_atenuacion_spot
        ejercicio_03_layout_std140_luces
        ejercicio_04_material_srgb
        ejercicio_05_tbn_unpack
        ejercicio_06_shadowmap_bias_pcf
        ejercicio_07_bloom_tonemap
        ejercicio_08_camara_yaw_pitch
        ejercicio_09_decisiones_features
        ejercicio_10_mallas_material_groups
        sol_ejercicio_01_lint_phong
        sol_ejercicio_02_atenuacion_spot
        sol_ejercicio_03_layout_std140_luces
        sol_ejercicio_04_material_srgb
        sol_ejercicio_05_tbn_unpack
        sol_ejercicio_06_shadowmap_bias_pcf
        sol_ejercicio_07_bloom_tonemap
        sol_ejercicio_08_camara_yaw_pitch
        sol_ejercicio_09_decisiones_features
        sol_ejercicio_10_mallas_material_groups
    COMMENT "Compilando todas las lecciones, ejercicios y soluciones del M21"
)