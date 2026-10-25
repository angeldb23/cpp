/*
 * MÓDULO 28: Estudio de Código Real — Arquitectura de Librerías
 * LECCIÓN 03: GLM — SIMD Internals, Constexpr Profundo y Testing
 *
 * TEMAS:
 * - Implementación SIMD: SSE2/AVX/AVX2/NEON intrinsics
 * - Tag dispatch y template metaprogramming para dispatch
 * - Constexpr en C++14/17/20: límites y capacidades
 * - Testing numérico: ULP comparison, relative error
 * - Benchmarking: nanobench, Google Benchmark
 * - Análisis de assembly generado (Godbolt, objdump)
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <immintrin.h> // SSE/AVX intrinsics (disponible en x86)

namespace modulo28_glm_detail {

// ============================================================================
// SIMD INTRINSICS: DOT PRODUCT VEC4 (SSE2)
// ============================================================================

/*
 * Dot product de 4 floats con SSE2:
 *   _mm_dp_ps(a, b, 0xFF) = a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w
 *   Requiere SSE4.1 (dpps). En SSE2 puro: mul + hadd + hadd
 * 
 * GLM usa hadd (horizontal add) para SSE2:
 *   __m128 mul = _mm_mul_ps(a, b);
 *   __m128 hadd1 = _mm_hadd_ps(mul, mul); // [x+y, z+w, x+y, z+w]
 *   __m128 hadd2 = _mm_hadd_ps(hadd1, hadd1); // [x+y+z+w, ...]
 *   return _mm_cvtss_f32(hadd2);
 */

inline float dot_sse2(const float* a, const float* b) {
    __m128 va = _mm_load_ps(a);
    __m128 vb = _mm_load_ps(b);
    __m128 mul = _mm_mul_ps(va, vb);
    __m128 hadd1 = _mm_hadd_ps(mul, mul);
    __m128 hadd2 = _mm_hadd_ps(hadd1, hadd1);
    return _mm_cvtss_f32(hadd2);
}

// Versión AVX (256-bit = 8 floats a la vez)
inline float dot_avx(const float* a, const float* b, size_t n) {
    __m256 sum = _mm256_setzero_ps();
    size_t i = 0;
    for (; i + 7 < n; i += 8) {
        __m256 va = _mm256_loadu_ps(a + i);
        __m256 vb = _mm256_loadu_ps(b + i);
        sum = _mm256_fmadd_ps(va, vb, sum); // FMA: a*b + sum
    }
    // Horizontal sum de 8 elementos
    __m128 sum128 = _mm256_castps256_ps128(sum);
    __m128 sum_high = _mm256_extractf128_ps(sum, 1);
    sum128 = _mm_add_ps(sum128, sum_high);
    __m128 hadd1 = _mm_hadd_ps(sum128, sum128);
    __m128 hadd2 = _mm_hadd_ps(hadd1, hadd1);
    float result = _mm_cvtss_f32(hadd2);
    
    // Resto scalar
    for (; i < n; ++i) result += a[i] * b[i];
    return result;
}

// Versión NEON (ARM)
#if defined(__ARM_NEON)
#include <arm_neon.h>
inline float dot_neon(const float* a, const float* b, size_t n) {
    float32x4_t sum = vdupq_n_f32(0.0f);
    size_t i = 0;
    for (; i + 3 < n; i += 4) {
        float32x4_t va = vld1q_f32(a + i);
        float32x4_t vb = vld1q_f32(b + i);
        sum = vfmaq_f32(sum, va, vb); // FMA
    }
    // Horizontal sum
    float result = vaddvq_f32(sum); // ARMv8.2+
    for (; i < n; ++i) result += a[i] * b[i];
    return result;
}
#endif

// ============================================================================
// TAG DISPATCH PARA SELECCIÓN DE IMPLEMENTACIÓN
// ============================================================================

struct simd_scalar_tag {};
struct simd_sse2_tag {};
struct simd_avx_tag {};
struct simd_neon_tag {};

// Trait para detectar mejor SIMD
template<typename T>
struct best_simd_tag {
    using type = simd_scalar_tag;
};

template<>
struct best_simd_tag<float> {
#if defined(__AVX2__)
    using type = simd_avx_tag;
#elif defined(__SSE2__)
    using type = simd_sse2_tag;
#elif defined(__ARM_NEON)
    using type = simd_neon_tag;
#else
    using type = simd_scalar_tag;
#endif
};

// Implementaciones por tag
template<typename Tag>
struct dot_dispatcher;

template<>
struct dot_dispatcher<simd_scalar_tag> {
    static float dot(const float* a, const float* b, size_t n) {
        float result = 0.0f;
        for (size_t i = 0; i < n; ++i) result += a[i] * b[i];
        return result;
    }
};

template<>
struct dot_dispatcher<simd_sse2_tag> {
    static float dot(const float* a, const float* b, size_t n) {
        float result = 0.0f;
        size_t i = 0;
        for (; i + 3 < n; i += 4) {
            result += dot_sse2(a + i, b + i);
        }
        for (; i < n; ++i) result += a[i] * b[i];
        return result;
    }
};

template<>
struct dot_dispatcher<simd_avx_tag> {
    static float dot(const float* a, const float* b, size_t n) {
        return dot_avx(a, b, n);
    }
};

#if defined(__ARM_NEON)
template<>
struct dot_dispatcher<simd_neon_tag> {
    static float dot(const float* a, const float* b, size_t n) {
        return dot_neon(a, b, n);
    }
};
#endif

// API unificada
inline float dot_simd(const float* a, const float* b, size_t n) {
    using tag = typename best_simd_tag<float>::type;
    return dot_dispatcher<tag>::dot(a, b, n);
}

// ============================================================================
// CONSTEXPR PROFUNDO (C++20)
// ============================================================================

/*
 * C++20 permite:
 *   - Virtual constexpr (C++20)
 *   - Try/catch constexpr
 *   - Dynamic allocation constexpr (new/delete)
 *   - std::vector constexpr (C++20 con allocator)
 *   - Loops sin límite de recursion
 * 
 * GLM aprovecha para:
 *   - Matrix inverse constexpr (Gauss-Jordan)
 *   - Quaternion slerp constexpr
 *   - Decompose TRS constexpr
 *   - Eigenvalues constexpr (Jacobi)
 */

// Matrix inverse 3x3 constexpr (C++20)
constexpr bool inverse_constexpr(const float m[9], float out[9]) {
    // Calcular determinante
    float det = m[0]*(m[4]*m[8] - m[5]*m[7])
              - m[1]*(m[3]*m[8] - m[5]*m[6])
              + m[2]*(m[3]*m[7] - m[4]*m[6]);
    
    if (det == 0.0f) return false;
    
    float inv_det = 1.0f / det;
    
    out[0] =  (m[4]*m[8] - m[5]*m[7]) * inv_det;
    out[1] = -(m[1]*m[8] - m[2]*m[7]) * inv_det;
    out[2] =  (m[1]*m[5] - m[2]*m[4]) * inv_det;
    out[3] = -(m[3]*m[8] - m[5]*m[6]) * inv_det;
    out[4] =  (m[0]*m[8] - m[2]*m[6]) * inv_det;
    out[5] = -(m[0]*m[5] - m[2]*m[3]) * inv_det;
    out[6] =  (m[3]*m[7] - m[4]*m[6]) * inv_det;
    out[7] = -(m[0]*m[7] - m[1]*m[6]) * inv_det;
    out[8] =  (m[0]*m[4] - m[1]*m[3]) * inv_det;
    
    return true;
}

// Quaternion slerp constexpr (C++20)
constexpr void slerp_constexpr(const float q1[4], const float q2[4], float t, float out[4]) {
    float dot = q1[0]*q2[0] + q1[1]*q2[1] + q1[2]*q2[2] + q1[3]*q2[3];
    
    float q2_corrected[4] = {q2[0], q2[1], q2[2], q2[3]};
    if (dot < 0.0f) {
        dot = -dot;
        q2_corrected[0] = -q2_corrected[0];
        q2_corrected[1] = -q2_corrected[1];
        q2_corrected[2] = -q2_corrected[2];
        q2_corrected[3] = -q2_corrected[3];
    }
    
    constexpr float DOT_THRESHOLD = 0.9995f;
    if (dot > DOT_THRESHOLD) {
        // Linear interpolation para ángulos pequeños
        for (int i = 0; i < 4; ++i) {
            out[i] = q1[i] + t * (q2_corrected[i] - q1[i]);
        }
        // Normalize
        float len = sqrtf(out[0]*out[0] + out[1]*out[1] + out[2]*out[2] + out[3]*out[3]);
        for (int i = 0; i < 4; ++i) out[i] /= len;
        return;
    }
    
    float theta_0 = acosf(dot);
    float theta = theta_0 * t;
    float sin_theta = sinf(theta);
    float sin_theta_0 = sinf(theta_0);
    
    float s0 = cosf(theta) - dot * sin_theta / sin_theta_0;
    float s1 = sin_theta / sin_theta_0;
    
    for (int i = 0; i < 4; ++i) {
        out[i] = s0 * q1[i] + s1 * q2_corrected[i];
    }
}

// ============================================================================
// TESTING NUMÉRICO: ULP COMPARISON
// ============================================================================

/*
 * Comparación de floats para testing:
 * - Absolute epsilon: insuficiente para valores grandes
 * - Relative epsilon: insuficiente para valores cercanos a 0
 * - ULP (Units in Last Place): distancia en representaciones IEEE 754
 * 
 * ULP difference = |reinterpret_int(a) - reinterpret_int(b)|
 * Para floats normales: 1 ULP = epsilon * 2^exponent
 */

inline int32_t float_to_int_bits(float f) {
    static_assert(sizeof(float) == sizeof(int32_t));
    int32_t i;
    std::memcpy(&i, &f, sizeof(float));
    return i;
}

inline uint32_t ulp_distance(float a, float b) {
    // Manejar signos: hacer que -0.0 == 0.0
    int32_t ia = float_to_int_bits(a);
    int32_t ib = float_to_int_bits(b);
    
    // Convertir a ordered integers (two's complement -> sign-magnitude ordered)
    if (ia < 0) ia = 0x80000000 - ia;
    if (ib < 0) ib = 0x80000000 - ib;
    
    return (ia > ib) ? uint32_t(ia - ib) : uint32_t(ib - ia);
}

inline bool almost_equal_ulp(float a, float b, uint32_t max_ulp = 4) {
    if (a == b) return true; // Incluye inf, -inf, NaN != NaN
    if (std::isnan(a) || std::isnan(b)) return false;
    if (std::isinf(a) || std::isinf(b)) return a == b;
    
    uint32_t ulps = ulp_distance(a, b);
    return ulps <= max_ulp;
}

// Relative + absolute combo (Google Test style)
inline bool almost_equal(float a, float b, float abs_eps = 1e-6f, float rel_eps = 1e-6f) {
    float diff = std::abs(a - b);
    if (diff <= abs_eps) return true;
    float max_ab = std::max(std::abs(a), std::abs(b));
    return diff <= rel_eps * max_ab;
}

// ============================================================================
// BENCHMARKING PATTERN
// ============================================================================

/*
 * Pattern para benchmarking tipo Google Benchmark / nanobench:
 * 
 * void BM_DotProduct(benchmark::State& state) {
 *     size_t n = state.range(0);
 *     std::vector<float> a(n, 1.0f), b(n, 2.0f);
 *     for (auto _ : state) {
 *         benchmark::DoNotOptimize(dot_simd(a.data(), b.data(), n));
 *     }
 *     state.SetItemsProcessed(state.iterations() * n);
 * }
 * BENCHMARK(BM_DotProduct)->Arg(4)->Arg(16)->Arg(1024)->Arg(10000);
 */

} // namespace modulo28_glm_detail