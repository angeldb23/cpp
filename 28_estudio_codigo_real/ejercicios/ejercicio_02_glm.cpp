/*
 * MÓDULO 28: Estudio de Código Real — Arquitectura de Librerías
 * EJERCICIO 02: GLM — Column-Major, SIMD Dispatch, Constexpr, ULP Testing
 *
 * OBJETIVO: Implementar tipos matemáticos core estilo GLM con arquitectura header-only.
 */

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <cstring>
#include <limits>
#include "../../include/test_framework.hpp"

// ============================================================================
// TIPOS BASE ESTILO GLM
// ============================================================================

// TODO: qualifier tag (para SIMD alignment futuro)
enum class qualifier { packed, aligned };

// TODO: vec2, vec3, vec4 con layout contiguo y operator[]
template<typename T, qualifier Q = qualifier::packed>
struct vec2 {
    T x, y;
    constexpr vec2() : x(0), y(0) {}
    constexpr vec2(T v) : x(v), y(v) {}
    constexpr vec2(T x_, T y_) : x(x_), y(y_) {}
    constexpr T& operator[](size_t i) { return (&x)[i]; }
    constexpr const T& operator[](size_t i) const { return (&x)[i]; }
};

template<typename T, qualifier Q = qualifier::packed>
struct vec3 {
    T x, y, z;
    constexpr vec3() : x(0), y(0), z(0) {}
    constexpr vec3(T v) : x(v), y(v), z(v) {}
    constexpr vec3(T x_, T y_, T z_) : x(x_), y(y_), z(z_) {}
    constexpr T& operator[](size_t i) { return (&x)[i]; }
    constexpr const T& operator[](size_t i) const { return (&x)[i]; }
};

template<typename T, qualifier Q = qualifier::packed>
struct vec4 {
    T x, y, z, w;
    constexpr vec4() : x(0), y(0), z(0), w(0) {}
    constexpr vec4(T v) : x(v), y(v), z(v), w(v) {}
    constexpr vec4(T x_, T y_, T z_, T w_) : x(x_), y(y_), z(z_), w(w_) {}
    constexpr vec4(const vec3<T,Q>& v, T w_) : x(v.x), y(v.y), z(v.z), w(w_) {}
    constexpr T& operator[](size_t i) { return (&x)[i]; }
    constexpr const T& operator[](size_t i) const { return (&x)[i]; }
};

// TODO: mat2, mat3, mat4 column-major (array de column vectors)
template<typename T, qualifier Q = qualifier::packed>
struct mat2 {
    vec2<T,Q> value[2];
    constexpr mat2() : value{vec2<T,Q>(1,0), vec2<T,Q>(0,1)} {}
    constexpr mat2(T diag) : value{vec2<T,Q>(diag,0), vec2<T,Q>(0,diag)} {}
    constexpr vec2<T,Q>& operator[](size_t i) { return value[i]; }
    constexpr const vec2<T,Q>& operator[](size_t i) const { return value[i]; }
    constexpr const T* value_ptr() const { return &value[0].x; }
    constexpr T* value_ptr() { return &value[0].x; }
};

template<typename T, qualifier Q = qualifier::packed>
struct mat3 {
    vec3<T,Q> value[3];
    constexpr mat3() : value{vec3<T,Q>(1,0,0), vec3<T,Q>(0,1,0), vec3<T,Q>(0,0,1)} {}
    constexpr mat3(T diag) : value{vec3<T,Q>(diag,0,0), vec3<T,Q>(0,diag,0), vec3<T,Q>(0,0,diag)} {}
    constexpr vec3<T,Q>& operator[](size_t i) { return value[i]; }
    constexpr const vec3<T,Q>& operator[](size_t i) const { return value[i]; }
    constexpr const T* value_ptr() const { return &value[0].x; }
    constexpr T* value_ptr() { return &value[0].x; }
};

template<typename T, qualifier Q = qualifier::packed>
struct mat4 {
    vec4<T,Q> value[4];
    constexpr mat4() : value{vec4<T,Q>(1,0,0,0), vec4<T,Q>(0,1,0,0), vec4<T,Q>(0,0,1,0), vec4<T,Q>(0,0,0,1)} {}
    constexpr mat4(T diag) : value{vec4<T,Q>(diag,0,0,0), vec4<T,Q>(0,diag,0,0), vec4<T,Q>(0,0,diag,0), vec4<T,Q>(0,0,0,diag)} {}
    constexpr vec4<T,Q>& operator[](size_t i) { return value[i]; }
    constexpr const vec4<T,Q>& operator[](size_t i) const { return value[i]; }
    constexpr const T* value_ptr() const { return &value[0].x; }
    constexpr T* value_ptr() { return &value[0].x; }
};

// ============================================================================
// OPERACIONES BÁSICAS (CONSTEXPR)
// ============================================================================

// TODO: dot product constexpr
template<typename T, qualifier Q>
constexpr T dot(const vec2<T,Q>& a, const vec2<T,Q>& b) {
    (void)a; (void)b;
    return T(0);
}

template<typename T, qualifier Q>
constexpr T dot(const vec3<T,Q>& a, const vec3<T,Q>& b) {
    (void)a; (void)b;
    return T(0);
}

template<typename T, qualifier Q>
constexpr T dot(const vec4<T,Q>& a, const vec4<T,Q>& b) {
    (void)a; (void)b;
    return T(0);
}

// TODO: cross product constexpr (solo vec3)
template<typename T, qualifier Q>
constexpr vec3<T,Q> cross(const vec3<T,Q>& a, const vec3<T,Q>& b) {
    (void)a; (void)b;
    return vec3<T,Q>(0);
}

// TODO: length, normalize constexpr
template<typename T, qualifier Q>
constexpr T length(const vec2<T,Q>& v) {
    (void)v;
    return T(0);
}

template<typename T, qualifier Q>
constexpr T length(const vec3<T,Q>& v) {
    (void)v;
    return T(0);
}

template<typename T, qualifier Q>
constexpr T length(const vec4<T,Q>& v) {
    (void)v;
    return T(0);
}

template<typename T, qualifier Q>
constexpr vec3<T,Q> normalize(const vec3<T,Q>& v) {
    (void)v;
    return vec3<T,Q>(0);
}

// Operator / para vec3
template<typename T, qualifier Q>
constexpr vec3<T,Q> operator/(const vec3<T,Q>& v, T s) {
    return vec3<T,Q>(v.x / s, v.y / s, v.z / s);
}

// Helpers de vec3/vec4 usados por las transformaciones
template<typename T, qualifier Q>
constexpr vec3<T,Q> operator*(const vec3<T,Q>& v, T s) {
    return vec3<T,Q>(v.x * s, v.y * s, v.z * s);
}

template<typename T, qualifier Q>
constexpr vec4<T,Q> operator*(const vec4<T,Q>& v, T s) {
    return vec4<T,Q>(v.x * s, v.y * s, v.z * s, v.w * s);
}

template<typename T, qualifier Q>
constexpr vec4<T,Q> operator+(const vec4<T,Q>& a, const vec4<T,Q>& b) {
    return vec4<T,Q>(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

template<typename T, qualifier Q>
constexpr vec3<T,Q> operator-(const vec3<T,Q>& a, const vec3<T,Q>& b) {
    return vec3<T,Q>(a.x - b.x, a.y - b.y, a.z - b.z);
}

// ============================================================================
// MATRIX-VECTOR MULTIPLICATION (COLUMN-MAJOR)
// ============================================================================

// TODO: mat3 * vec3 (column-major: M * v = sum(v[i] * col_i))
template<typename T, qualifier Q>
constexpr vec3<T,Q> operator*(const mat3<T,Q>& m, const vec3<T,Q>& v) {
    (void)m; (void)v;
    return vec3<T,Q>(0);
}

template<typename T, qualifier Q>
constexpr vec4<T,Q> operator*(const mat4<T,Q>& m, const vec4<T,Q>& v) {
    (void)m; (void)v;
    return vec4<T,Q>(0);
}

// ============================================================================
// MATRIX-MATRIX MULTIPLICATION
// ============================================================================

// TODO: mat3 * mat3
template<typename T, qualifier Q>
constexpr mat3<T,Q> operator*(const mat3<T,Q>& a, const mat3<T,Q>& b) {
    (void)a; (void)b;
    return mat3<T,Q>();
}

// ============================================================================
// TRANSFORMACIONES (translate, rotate, scale, lookAt, perspective)
// ============================================================================

// TODO: translate mat4
template<typename T, qualifier Q>
constexpr mat4<T,Q> translate(const mat4<T,Q>& m, const vec3<T,Q>& v) {
    (void)m; (void)v;
    return mat4<T,Q>();
}

// TODO: scale mat4
template<typename T, qualifier Q>
constexpr mat4<T,Q> scale(const mat4<T,Q>& m, const vec3<T,Q>& v) {
    (void)m; (void)v;
    return mat4<T,Q>();
}

// TODO: rotate mat4 (axis-angle, radians)
template<typename T, qualifier Q>
constexpr mat4<T,Q> rotate(const mat4<T,Q>& m, T angle, const vec3<T,Q>& axis) {
    (void)m; (void)angle; (void)axis;
    return mat4<T,Q>();
}

// TODO: lookAt (view matrix)
template<typename T, qualifier Q>
constexpr mat4<T,Q> lookAt(const vec3<T,Q>& eye, const vec3<T,Q>& center, const vec3<T,Q>& up) {
    (void)eye; (void)center; (void)up;
    return mat4<T,Q>();
}

// TODO: perspective projection
template<typename T, qualifier Q>
constexpr mat4<T,Q> perspective(T fovy, T aspect, T zNear, T zFar) {
    (void)fovy; (void)aspect; (void)zNear; (void)zFar;
    return mat4<T,Q>();
}

// ============================================================================
// SIMD DISPATCH (TAG-BASED)
// ============================================================================

// Tags
struct simd_scalar_tag {};
struct simd_sse2_tag {};

// Trait para mejor SIMD
template<typename T>
struct best_simd_tag { using type = simd_scalar_tag; };

template<>
struct best_simd_tag<float> {
#if defined(__AVX2__)
    using type = simd_sse2_tag; // Simplificado: usamos SSE2 como proxy
#elif defined(__SSE2__)
    using type = simd_sse2_tag;
#else
    using type = simd_scalar_tag;
#endif
};

// Dispatcher
template<typename Tag>
struct dot_dispatcher;

template<>
struct dot_dispatcher<simd_scalar_tag> {
    static float dot4(const float* a, const float* b) {
        return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
    }
};

template<>
struct dot_dispatcher<simd_sse2_tag> {
    static float dot4(const float* a, const float* b) {
        // SSE2: mul + hadd + hadd
        // Simulamos sin intrinsics para portabilidad del test
        return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
    }
};

// API unificada
inline float dot_simd_vec4(const float* a, const float* b) {
    (void)a; (void)b;
    return 0.0f;
}

// ============================================================================
// ULP COMPARISON PARA TESTING
// ============================================================================

// TODO: float_to_int_bits
inline int32_t float_to_int_bits(float f) {
    (void)f;
    return 0;
}

// TODO: ulp_distance
inline uint32_t ulp_distance(float a, float b) {
    (void)a; (void)b;
    return 0;
}

// TODO: almost_equal_ulp
inline bool almost_equal_ulp(float a, float b, uint32_t max_ulp = 4) {
    (void)a; (void)b; (void)max_ulp;
    return false;
}

// ============================================================================
// TESTS (NO MODIFICAR)
// ============================================================================

TEST_CASE(TestVecLayout) {
    vec3<float> v{1, 2, 3};
    ASSERT_EQ(v[0], 1);
    ASSERT_EQ(v[1], 2);
    ASSERT_EQ(v[2], 3);
    ASSERT_EQ(sizeof(vec3<float>), 3 * sizeof(float));
    
    mat3<float> m;
    ASSERT_EQ(m.value_ptr(), &m[0].x);
    ASSERT_EQ(m[0][0], 1); // identity
    ASSERT_EQ(m[1][1], 1);
    ASSERT_EQ(m[2][2], 1);
}

TEST_CASE(TestMatColumnMajor) {
    mat4<float> m = mat4<float>(2.0f); // diagonal 2
    // Column-major: m[col][row]
    ASSERT_EQ(m[0][0], 2); // col 0, row 0
    ASSERT_EQ(m[1][1], 2); // col 1, row 1
    ASSERT_EQ(m[2][2], 2);
    ASSERT_EQ(m[3][3], 2);
    ASSERT_EQ(m[0][1], 0); // off-diagonal
}

TEST_CASE(TestDotConstexpr) {
    constexpr vec3<float> a{1, 2, 3};
    constexpr vec3<float> b{4, 5, 6};
    constexpr float d = dot(a, b);
    ASSERT_EQ(d, 32.0f); // 1*4 + 2*5 + 3*6 = 4+10+18=32
}

TEST_CASE(TestCrossConstexpr) {
    constexpr vec3<float> a{1, 0, 0};
    constexpr vec3<float> b{0, 1, 0};
    constexpr vec3<float> c = cross(a, b);
    ASSERT_EQ(c.x, 0);
    ASSERT_EQ(c.y, 0);
    ASSERT_EQ(c.z, 1);
}

TEST_CASE(TestMatVecMul) {
    mat3<float> m = mat3<float>(2.0f); // 2*I
    vec3<float> v{1, 2, 3};
    vec3<float> r = m * v;
    ASSERT_EQ(r.x, 2);
    ASSERT_EQ(r.y, 4);
    ASSERT_EQ(r.z, 6);
    
    // Non-diagonal
    mat3<float> m2;
    m2[0] = vec3<float>{1, 0, 0};
    m2[1] = vec3<float>{0, 0, -1};
    m2[2] = vec3<float>{0, 1, 0};
    vec3<float> v2{1, 2, 3};
    vec3<float> r2 = m2 * v2; // Should rotate y<->z with flip
    ASSERT_EQ(r2.x, 1);
    ASSERT_EQ(r2.y, 3);   // 1*0 + 2*0 + 3*1 = 3
    ASSERT_EQ(r2.z, -2);  // 1*0 + 2*(-1) + 3*0 = -2
}

TEST_CASE(TestTransformations) {
    // Translate
    mat4<float> t = translate(mat4<float>(), vec3<float>{1, 2, 3});
    ASSERT_EQ(t[3].x, 1);
    ASSERT_EQ(t[3].y, 2);
    ASSERT_EQ(t[3].z, 3);
    ASSERT_EQ(t[3].w, 1);
    
    // Scale
    mat4<float> s = scale(mat4<float>(), vec3<float>{2, 3, 4});
    ASSERT_EQ(s[0][0], 2);
    ASSERT_EQ(s[1][1], 3);
    ASSERT_EQ(s[2][2], 4);
    
    // Perspective
    mat4<float> p = perspective<float, qualifier::packed>(1.5708f, 1.0f, 0.1f, 100.0f); // 90 deg, square
    ASSERT_TRUE(p[0][0] > 0);
    ASSERT_TRUE(p[1][1] > 0);
    ASSERT_TRUE(p[2][2] < 0);
    ASSERT_EQ(p[2][3], -1.0f);
    ASSERT_TRUE(p[3][2] < 0);
}

TEST_CASE(TestLookAt) {
    mat4<float> view = lookAt(
        vec3<float>{0, 0, 5},
        vec3<float>{0, 0, 0},
        vec3<float>{0, 1, 0}
    );
    // Camera at (0,0,5) looking at origin
    // Forward should be -Z, so -f.z = 1 > 0
    ASSERT_TRUE(view[2][2] > 0); // -f.z = -(-1) = 1
    // Right should be +X
    ASSERT_TRUE(view[0][0] > 0); // s.x
    // Up should be +Y
    ASSERT_TRUE(view[1][1] > 0); // u.y
}

TEST_CASE(TestSIMDDispatch) {
    // Verify dispatch compiles and runs
    float a[4] = {1, 2, 3, 4};
    float b[4] = {5, 6, 7, 8};
    float d = dot_simd_vec4(a, b);
    ASSERT_EQ(d, 70.0f); // 1*5 + 2*6 + 3*7 + 4*8 = 5+12+21+32=70
}

TEST_CASE(TestULPComparison) {
    // Same value
    ASSERT_TRUE(almost_equal_ulp(1.0f, 1.0f));
    ASSERT_TRUE(almost_equal_ulp(-0.0f, 0.0f));
    
    // NaN/Inf
    ASSERT_FALSE(almost_equal_ulp(std::numeric_limits<float>::quiet_NaN(), 1.0f));
    ASSERT_TRUE(almost_equal_ulp(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()));
    ASSERT_FALSE(almost_equal_ulp(std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()));
    
    // Small ULP difference
    float f1 = 1.0f;
    float f2 = std::nextafterf(1.0f, 2.0f); // 1 ULP away
    ASSERT_TRUE(almost_equal_ulp(f1, f2, 1));
    ASSERT_TRUE(almost_equal_ulp(f1, f2, 4));
    
    // Large ULP difference
    float f3 = 1.0f;
    float f4 = 1.0001f; // ~100 ULPs away
    ASSERT_FALSE(almost_equal_ulp(f3, f4, 4));
}

int main() { return RUN_ALL_TESTS(); }