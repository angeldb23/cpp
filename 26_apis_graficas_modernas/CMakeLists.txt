# =============================================================================
# MÓDULO 26 - APIs Gráficas Modernas: Vulkan, WebGPU, SDL_GPU
# 10 lecciones, 10 ejercicios, 10 soluciones, QUIZ.md, 3 demos, DECISION_API.md
#
# Dependencias externas (FetchContent con tags INMUTABLES verificados):
#   - Vulkan-Headers  v1.4.366  (KhronosGroup/Vulkan-Headers)    -- verificado con git ls-remote
#   - VMA             v3.4.0      (GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) -- verificado
#   - wgpu-native     A VERIFICAR (gfx-rs/wgpu-native)            -- tag pendiente confirmar
#   - SDL3            sistema (3.4.18 en Arch, pkg-config sdl3)  -- headers + libs
#
# Regla del curso: demos gráficas (01/02 SDL_GPU, 03 Vulkan manual) se COMPILAN
# y LINKEAN contra librerías reales pero NUNCA se ejecutan en CTest/verificar.sh
# (los tests son headless: test_framework.hpp puro). Targets demo_* son solo
# para compilar el path real; el dueño los corre a mano si quiere.
#
#   cmake -B build -S .
#   cmake --build build -j2
#   ctest --test-dir build          # corre las 10 soluciones
# =============================================================================
cmake_minimum_required(VERSION 3.20)
project(m26_apis_graficas_modernas LANGUAGES C CXX)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive- /Zc:__cplusplus)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ---- SDL3 del sistema (pkg-config) -------------------------------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED sdl3)

# ---- FetchContent: Vulkan-Headers v1.4.366 (tag verificado) -------------------
include(FetchContent)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    VulkanHeaders
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers
    GIT_TAG        v1.4.366
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(VulkanHeaders)
# target: Vulkan::Headers (interface, include dirs)

# ---- FetchContent: VMA v3.4.0 (tag verificado) --------------------------------
FetchContent_Declare(
    VMA
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
    GIT_TAG        v3.4.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(VMA)
# target: VMA::VMA (estático, vk_mem_alloc.c)

# ---- FetchContent: wgpu-native (A VERIFICAR - tag pendiente) ------------------
# Descomentar y fijar GIT_TAG cuando se confirme el tag estable en gfx-rs/wgpu-native
# FetchContent_Declare(
#     WGPUNative
#     GIT_REPOSITORY https://github.com/gfx-rs/wgpu-native
#     GIT_TAG        <TAG_PENDIENTE_VERIFICAR>
#     GIT_SHALLOW    TRUE
# )
# FetchContent_MakeAvailable(WGPUNative)
# target: wgpu_native (C API, lib + headers en ffi/wgpu.h)

# =============================================================================
# Lecciones (headless: lógica pura, test_framework.hpp, sin hardware gráfico)
# =============================================================================
set(LECCIONES
    leccion_01_por_que_apis_modernas
    leccion_02_vulkan_instancia_queues
    leccion_03_vulkan_comandos_sync
    leccion_04_sdl_gpu_device_pipeline
    leccion_05_sdl_gpu_texturas_quad
    leccion_06_webgpu_wgsl_bindgroups
    leccion_07_spv_glslang_reflection
    leccion_08_vma_memoria_budget
    leccion_09_render_graph
    leccion_10_decision_api
)

foreach(leccion IN LISTS LECCIONES)
    add_executable(${leccion} ${leccion}.cpp)
    target_include_directories(${leccion} PRIVATE include)
    # Vulkan headers disponibles para lecciones 02, 03, 07, 08, 09
    target_link_libraries(${leccion} PRIVATE Vulkan::Headers)
    # VMA headers para lección 08
    if(${leccion} MATCHES "08")
        target_link_libraries(${leccion} PRIVATE VMA::VMA)
    endif()
endforeach()

# =============================================================================
# Demos REALES (gráficos): compilan + linkean contra librerías del sistema
# NO se registran en CTest (requieren GPU/ventana). Solo para `check_all`.
# =============================================================================

# Demo 1: Triángulo SDL_GPU (lección 04/05)
add_executable(demo_01_triangulo_sdl_gpu demos/01_triangulo_sdl_gpu.cpp)
target_include_directories(demo_01_triangulo_sdl_gpu PRIVATE include)
target_link_libraries(demo_01_triangulo_sdl_gpu PRIVATE ${SDL3_LIBRARIES})
target_compile_options(demo_01_triangulo_sdl_gpu PRIVATE ${SDL3_CFLAGS_OTHER})

# Demo 2: Quad texturizado SDL_GPU (lección 05)
add_executable(demo_02_quad_sdl_gpu demos/02_quad_sdl_gpu.cpp)
target_include_directories(demo_02_quad_sdl_gpu PRIVATE include)
target_link_libraries(demo_02_quad_sdl_gpu PRIVATE ${SDL3_LIBRARIES})
target_compile_options(demo_02_quad_sdl_gpu PRIVATE ${SDL3_CFLAGS_OTHER})

# Demo 3: Instancia mínima Vulkan (lección 02/03)
add_executable(demo_03_vulkan_instancia_minima demos/03_vulkan_instancia_minima.cpp)
target_include_directories(demo_03_vulkan_instancia_minima PRIVATE include)
target_link_libraries(demo_03_vulkan_instancia_minima PRIVATE vulkan)

# =============================================================================
# Ejercicios y soluciones (lógica pura: test_framework.hpp, sin hardware)
# =============================================================================
set(EJERCICIOS
    ejercicio_01_costo_api
    ejercicio_02_vulkan_swapchain
    ejercicio_03_frame_ring
    ejercicio_04_pipeline_sdl_gpu
    ejercicio_05_quad_texturizado
    ejercicio_06_wgsl_lint
    ejercicio_07_spv_parser
    ejercicio_08_vma_memoria
    ejercicio_09_render_graph
    ejercicio_10_decision_api
)

foreach(ejercicio IN LISTS EJERCICIOS)
    add_executable(${ejercicio} ejercicios/${ejercicio}.cpp)
    target_include_directories(${ejercicio} PRIVATE include)

    string(REPLACE "ejercicio_" "solucion_" solucion "${ejercicio}")
    add_executable(${solucion} soluciones/${solucion}.cpp)
    target_include_directories(${solucion} PRIVATE include)

    # Vulkan headers para ejercicios/soluciones que los usan (02, 03, 07, 08, 09)
    if(${ejercicio} MATCHES "0[23789]")
        target_link_libraries(${ejercicio} PRIVATE Vulkan::Headers)
        target_link_libraries(${solucion} PRIVATE Vulkan::Headers)
    endif()
    # VMA para 08
    if(${ejercicio} MATCHES "08")
        target_link_libraries(${ejercicio} PRIVATE VMA::VMA)
        target_link_libraries(${solucion} PRIVATE VMA::VMA)
    endif()

    # Solo las soluciones son tests que deben pasar (CTest)
    add_test(NAME ${solucion} COMMAND ${solucion})
endforeach()

# =============================================================================
# check_all: compila TODO (lecciones + demos + ejercicios + soluciones)
# =============================================================================
set(TODOS_LOS_TARGETS ${LECCIONES}
    demo_01_triangulo_sdl_gpu
    demo_02_quad_sdl_gpu
    demo_03_vulkan_instancia_minima
)

foreach(ejercicio IN LISTS EJERCICIOS)
    string(REPLACE "ejercicio_" "solucion_" solucion "${ejercicio}")
    list(APPEND TODOS_LOS_TARGETS ${ejercicio} ${solucion})
endforeach()

add_custom_target(check_all
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target ${TODOS_LOS_TARGETS}
    COMMENT "Compilando 10 lecciones, 3 demos, 10 ejercicios y 10 soluciones del módulo 26"
    VERBATIM
)