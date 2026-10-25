# =============================================================================
# Módulo 19 — CMake para probar las lecciones que usan GLM a mano.
# Solo las lecciones 01, 02, 06, 09 y 10 incluyen GLM: son los targets de acá.
# (Las demás lecciones son C++ puro con el include/mate3d.hpp del módulo y se
#  compilan directo:
#     g++ -std=c++20 -Wall -Wextra leccion_03_... .cpp -o leccion03 && ./leccion03)
#
#   cmake -B build -S . -DGLM_BUILD_LIBRARY=OFF
#   cmake --build build -j2
#   ./build/leccion_01_glm_tipos_columnas
#
# FetchContent TRAE GLM 1.0.1 CON TAG CONCRETO (nunca master), verificado
# contra el repo oficial (g-truc/glm) en la sesión de construcción.
# =============================================================================
cmake_minimum_required(VERSION 3.20)
project(modulo19_matematica LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm
    GIT_TAG        1.0.1        # verificado: refs/tags/1.0.1 en el repo oficial
)
set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)  # header-only: no compila el .cpp
FetchContent_MakeAvailable(glm)

foreach(leccion IN ITEMS
        leccion_01_glm_tipos_columnas
        leccion_02_glm_config_value_ptr
        leccion_06_mvp_orden_matrices
        leccion_09_quats_eje_angulo
        leccion_10_quats_slerp_gimbal
        leccion_11_frustum_proyeccion)
    add_executable(${leccion} ${leccion}.cpp)
    target_link_libraries(${leccion} PRIVATE glm::glm)
    target_compile_options(${leccion} PRIVATE -Wall -Wextra)
endforeach()
