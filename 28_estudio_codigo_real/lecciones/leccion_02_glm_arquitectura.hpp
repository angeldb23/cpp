/*
 * MÓDULO 28: Estudio de Código Real — Arquitectura de Librerías
 * LECCIÓN 02: Estudio de GLM — Arquitectura Header-Only, Templates y SIMD
 *
 * TEMAS:
 * - GLM (OpenGL Mathematics): header-only, C++98/11/14/17/20 compatible
 * - Diseño column-major (compatibilidad OpenGL)
 * - Tipos: vec2/3/4, mat2/3/4, quat, dualquat
 * - SIMD: SSE/AVX/NEON con dispatch en tiempo de compilación
 * - constexpr: evaluación en compile-time (C++14/17/20)
 * - Extensiones: gtx/* (transform, quaternion, matrix, color, etc.)
 * - Testing: test suite propio, cobertura, benchmarks
 * - Integración: CMake FetchContent, vcpkg, Conan, submodule
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <type_traits>

namespace modulo28_glm {

// Qualifier tags de GLM (packed/aligned) para el layout de tipos SIMD.
enum class qualifier { packed, aligned };

// ============================================================================
// ARQUITECTURA HEADER-ONLY
// ============================================================================

/*
 * GLM es header-only: todo en .hpp/.inl, sin .cpp
 * Ventajas:
 *   - Zero-cost abstraction: inline todo, optimización agresiva
 *   - Fácil integración: solo include path
 *   - Compile-time evaluation: constexpr en funciones puras
 *   - Template metaprogramming: tipos genéricos, SFINAE, concepts
 * 
 * Desventajas:
 *   - Tiempos de compilación mayores (mitigado: unity builds, PCH)
 *   - Code bloat si no se usa inline/constexpr cuidadosamente
 *   - Dependencias transitivas expuestas en headers
 */

// ============================================================================
// COLUMNA-MAJOR Y LAYOUT DE MEMORIA
// ============================================================================

/*
 * OpenGL usa column-major: mat4 = 4 column vectors (vec4)
 * GLM sigue esta convención: mat4[0] = primera columna
 * 
 * Layout en memoria (mat4):
 *   m[0][0] m[1][0] m[2][0] m[3][0]  <- col 0
 *   m[0][1] m[1][1] m[2][1] m[3][1]  <- col 1
 *   m[0][2] m[1][2] m[2][2] m[3][2]  <- col 2
 *   m[0][3] m[1][3] m[2][3] m[3][3]  <- col 3
 * 
 * value_ptr(mat) devuelve &m[0][0] = puntero a memoria contigua column-major
 * Compatible directo con glUniformMatrix4fv(..., GL_FALSE, value_ptr(mat))
 * 
 * NOTA: Row-major (C/C++ nativo) sería transpuesta. GLM proporciona
 * transpose() y constructores row-major explícitos.
 */

// Simulación simplificada de vec3 para demostrar layout
template<typename T, qualifier Q = qualifier::packed>
struct vec3 {
    T x, y, z;
    
    constexpr vec3() : x(0), y(0), z(0) {}
    constexpr vec3(T v) : x(v), y(v), z(v) {}
    constexpr vec3(T x_, T y_, T z_) : x(x_), y(y_), z(z_) {}
    
    constexpr T& operator[](size_t i) { return (&x)[i]; }
    constexpr const T& operator[](size_t i) const { return (&x)[i]; }
};

// mat3 como array de 3 vec3 (columnas)
template<typename T, qualifier Q = qualifier::packed>
struct mat3 {
    vec3<T, Q> value[3];
    
    constexpr mat3() : value{vec3<T,Q>(1,0,0), vec3<T,Q>(0,1,0), vec3<T,Q>(0,0,1)} {}
    constexpr mat3(T diagonal) : value{vec3<T,Q>(diagonal,0,0), vec3<T,Q>(0,diagonal,0), vec3<T,Q>(0,0,diagonal)} {}
    constexpr mat3(const vec3<T,Q>& c0, const vec3<T,Q>& c1, const vec3<T,Q>& c2) : value{c0, c1, c2} {}
    
    constexpr vec3<T,Q>& operator[](size_t i) { return value[i]; }
    constexpr const vec3<T,Q>& operator[](size_t i) const { return value[i]; }
    
    // value_ptr compatible OpenGL
    constexpr const T* value_ptr() const { return &value[0].x; }
    constexpr T* value_ptr() { return &value[0].x; }
};

// ============================================================================
// DISPATCH SIMD (COMPILE-TIME)
// ============================================================================

/*
 * GLM detecta capacidades CPU en compile-time via macros:
 *   #if GLM_ARCH & GLM_ARCH_SSE2_BIT
 *   #if GLM_ARCH & GLM_ARCH_AVX_BIT
 *   #if GLM_ARCH & GLM_ARCH_NEON_BIT
 * 
 * Implementa múltiples versiones de funciones críticas (dot, cross, mul, etc.)
 * y selecciona la mejor via template specialization / tag dispatch.
 * 
 * Ejemplo conceptual:
 */

struct simd_sse_tag {};
struct simd_avx_tag {};
struct simd_neon_tag {};
struct simd_scalar_tag {};

// Detectar mejor SIMD disponible (simplificado)
#if defined(__AVX2__)
using best_simd_tag = simd_avx_tag;
#elif defined(__SSE2__)
using best_simd_tag = simd_sse_tag;
#elif defined(__ARM_NEON)
using best_simd_tag = simd_neon_tag;
#else
using best_simd_tag = simd_scalar_tag;
#endif

// Dispatch genérico
template<typename Tag>
struct dot_impl;

template<>
struct dot_impl<simd_scalar_tag> {
    template<typename T>
    static constexpr T dot(const T* a, const T* b, size_t n) {
        T result = T(0);
        for (size_t i = 0; i < n; ++i) result += a[i] * b[i];
        return result;
    }
};

// Especializaciones SIMD estarían en .inl separados con intrinsics

// ============================================================================
// CONSTEXPR Y COMPILE-TIME EVALUATION
// ============================================================================

/*
 * GLM marca funciones puras como constexpr (C++14+)
 * Permite: mat4 view = lookAt(...) evaluado en compile-time si args son constexpr
 * 
 * Requisitos para constexpr:
 *   - Sin side effects
 *   - Sin asm inline no constexpr
 *   - Loops bounded (C++14) / sin restricción (C++20)
 *   - Recursion depth limitado
 */

constexpr float radians(float degrees) {
    return degrees * 0.017453292519943295f; // pi/180
}

constexpr float degrees(float radians) {
    return radians * 57.29577951308232f; // 180/pi
}

// lookAt simplificado constexpr (C++20)
constexpr mat3<float> lookAt_constexpr(const vec3<float>& eye, 
                                        const vec3<float>& center, 
                                        const vec3<float>& up) {
    // Implementación simplificada - real usa Gram-Schmidt
    vec3<float> f = {center.x - eye.x, center.y - eye.y, center.z - eye.z};
    float flen = std::sqrt(f.x*f.x + f.y*f.y + f.z*f.z);
    f = {f.x/flen, f.y/flen, f.z/flen};
    
    vec3<float> s = {f.y*up.z - f.z*up.y, f.z*up.x - f.x*up.z, f.x*up.y - f.y*up.x};
    float slen = std::sqrt(s.x*s.x + s.y*s.y + s.z*s.z);
    s = {s.x/slen, s.y/slen, s.z/slen};
    
    vec3<float> u = {s.y*f.z - s.z*f.y, s.z*f.x - s.x*f.z, s.x*f.y - s.y*f.x};
    
    return mat3<float>{
        vec3<float>(s.x, u.x, -f.x),
        vec3<float>(s.y, u.y, -f.y),
        vec3<float>(s.z, u.z, -f.z)
    };
}

// ============================================================================
// EXTENSIONES GTX (MÓDULOS OPCIONALES)
// ============================================================================

/*
 * GLM core: tipos base + operaciones esenciales
 * GLM gtx/*: extensiones opcionales (include explícito)
 * 
 * Principales:
 *   gtx/transform: translate, rotate, scale, lookAt, perspective, ortho
 *   gtx/quaternion: quat ops, slerp, squad, euler angles
 *   gtx/matrix_decompose: decompose TRS
 *   gtx/euler_angles: yawPitchRoll, orientate
 *   gtx/color_space: sRGB/linear, HSV, HSL
 *   gtx/compatibility: glu functions
 *   gtx/norm: length2, distance2 (sin sqrt)
 *   gtx/fast_*: aproximaciones rápidas (fastNormalize, fastInverseSqrt)
 *   gtx/hash: hash para unordered_map/unordered_set
 *   gtx/io: streaming << >>
 *   gtx/string_cast: to_string
 */

// ============================================================================
// TESTING EN GLM
// ============================================================================

/*
 * GLM incluye test suite propio (glm/test/)
 * - Tests de corrección numérica (ULP comparison)
 * - Tests de constexpr (static_assert)
 * - Tests de SIMD (comparar scalar vs SIMD results)
 * - Tests de extensiones
 * - Benchmarks de rendimiento
 * 
 * Ejecutado via CMake: add_subdirectory(glm) -> enable_testing()
 * Cobertura objetivo: >90% en core, >80% en gtx
 */

// ============================================================================
// EJEMPLO DE USO MODERNO (C++20)
// ============================================================================

/*
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>

// Compile-time matrix
constexpr auto view = glm::lookAt(
    glm::vec3(0, 0, 5),
    glm::vec3(0, 0, 0),
    glm::vec3(0, 1, 0)
);

constexpr auto proj = glm::perspective(glm::radians(45.0f), 16.0f/9.0f, 0.1f, 100.0f);
constexpr auto mvp = proj * view;

// Runtime con SIMD automático
glm::vec3 pos(1, 2, 3);
glm::quat rot = glm::angleAxis(glm::radians(90.0f), glm::vec3(0, 1, 0));
glm::vec3 rotated = rot * pos; // Usa SIMD si disponible
*/

} // namespace modulo28_glm