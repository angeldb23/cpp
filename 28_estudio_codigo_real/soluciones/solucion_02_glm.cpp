/*
 * MÓDULO 28: Estudio de Código Real — Arquitectura de Librerías
 * SOLUCIÓN 02: GLM — Column-Major, SIMD Dispatch, Constexpr, ULP Testing
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

enum class qualifier { packed, aligned };

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

template<typename T, qualifier Q>
constexpr T dot(const vec2<T,Q>& a, const vec2<T,Q>& b) {
    return a.x * b.x + a.y * b.y;
}

template<typename T, qualifier Q>
constexpr T dot(const vec3<T,Q>& a, const vec3<T,Q>& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

template<typename T, qualifier Q>
constexpr T dot(const vec4<T,Q>& a, const vec4<T,Q>& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

template<typename T, qualifier Q>
constexpr vec3<T,Q> cross(const vec3<T,Q>& a, const vec3<T,Q>& b) {
    return vec3<T,Q>(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

template<typename T, qualifier Q>
constexpr T length(const vec2<T,Q>& v) {
    return std::sqrt(dot(v, v));
}

template<typename T, qualifier Q>
constexpr T length(const vec3<T,Q>& v) {
    return std::sqrt(dot(v, v));
}

template<typename T, qualifier Q>
constexpr T length(const vec4<T,Q>& v) {
    return std::sqrt(dot(v, v));
}

template<typename T, qualifier Q>
constexpr vec3<T,Q> normalize(const vec3<T,Q>& v) {
    T len = length(v);
    return len > T(0) ? v / len : vec3<T,Q>(0);
}

template<typename T, qualifier Q>
constexpr vec3<T,Q> operator/(const vec3<T,Q>& v, T s) {
    return vec3<T,Q>(v.x / s, v.y / s, v.z / s);
}

template<typename T, qualifier Q>
constexpr vec3<T,Q> operator*(const vec3<T,Q>& v, T s) {
    return vec3<T,Q>(v.x * s, v.y * s, v.z * s);
}

template<typename T, qualifier Q>
constexpr vec3<T,Q> operator*(T s, const vec3<T,Q>& v) {
    return v * s;
}

template<typename T, qualifier Q>
constexpr vec4<T,Q> operator*(const vec4<T,Q>& v, T s) {
    return vec4<T,Q>(v.x * s, v.y * s, v.z * s, v.w * s);
}

template<typename T, qualifier Q>
constexpr vec4<T,Q> operator*(T s, const vec4<T,Q>& v) {
    return v * s;
}

template<typename T, qualifier Q>
constexpr vec4<T,Q> operator+(const vec4<T,Q>& a, const vec4<T,Q>& b) {
    return vec4<T,Q>(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

template<typename T, qualifier Q>
constexpr vec4<T,Q> operator-(const vec4<T,Q>& a, const vec4<T,Q>& b) {
    return vec4<T,Q>(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

template<typename T, qualifier Q>
constexpr vec3<T,Q> operator-(const vec3<T,Q>& a, const vec3<T,Q>& b) {
    return vec3<T,Q>(a.x - b.x, a.y - b.y, a.z - b.z);
}

// ============================================================================
// MATRIX-VECTOR MULTIPLICATION (COLUMN-MAJOR)
// ============================================================================

template<typename T, qualifier Q>
constexpr vec3<T,Q> operator*(const mat3<T,Q>& m, const vec3<T,Q>& v) {
    return vec3<T,Q>(
        m[0].x * v.x + m[1].x * v.y + m[2].x * v.z,
        m[0].y * v.x + m[1].y * v.y + m[2].y * v.z,
        m[0].z * v.x + m[1].z * v.y + m[2].z * v.z
    );
}

template<typename T, qualifier Q>
constexpr vec4<T,Q> operator*(const mat4<T,Q>& m, const vec4<T,Q>& v) {
    return vec4<T,Q>(
        m[0].x * v.x + m[1].x * v.y + m[2].x * v.z + m[3].x * v.w,
        m[0].y * v.x + m[1].y * v.y + m[2].y * v.z + m[3].y * v.w,
        m[0].z * v.x + m[1].z * v.y + m[2].z * v.z + m[3].z * v.w,
        m[0].w * v.x + m[1].w * v.y + m[2].w * v.z + m[3].w * v.w
    );
}

// ============================================================================
// MATRIX-MATRIX MULTIPLICATION
// ============================================================================

template<typename T, qualifier Q>
constexpr mat3<T,Q> operator*(const mat3<T,Q>& a, const mat3<T,Q>& b) {
    mat3<T,Q> result;
    for (int col = 0; col < 3; ++col) {
        for (int row = 0; row < 3; ++row) {
            result[col][row] = a[0][row] * b[col][0] + a[1][row] * b[col][1] + a[2][row] * b[col][2];
        }
    }
    return result;
}

// ============================================================================
// TRANSFORMACIONES
// ============================================================================

template<typename T, qualifier Q>
constexpr mat4<T,Q> translate(const mat4<T,Q>& m, const vec3<T,Q>& v) {
    mat4<T,Q> result = m;
    result[3] = m[0] * v.x + m[1] * v.y + m[2] * v.z + m[3];
    return result;
}

template<typename T, qualifier Q>
constexpr mat4<T,Q> scale(const mat4<T,Q>& m, const vec3<T,Q>& v) {
    mat4<T,Q> result;
    result[0] = m[0] * v.x;
    result[1] = m[1] * v.y;
    result[2] = m[2] * v.z;
    result[3] = m[3];
    return result;
}

template<typename T, qualifier Q>
constexpr mat4<T,Q> rotate(const mat4<T,Q>& m, T angle, const vec3<T,Q>& axis) {
    T c = std::cos(angle);
    T s = std::sin(angle);
    vec3<T,Q> u = normalize(axis);
    
    mat4<T,Q> r;
    r[0] = vec4<T,Q>(
        c + u.x*u.x*(1-c), u.y*u.x*(1-c) + u.z*s, u.z*u.x*(1-c) - u.y*s, 0
    );
    r[1] = vec4<T,Q>(
        u.x*u.y*(1-c) - u.z*s, c + u.y*u.y*(1-c), u.z*u.y*(1-c) + u.x*s, 0
    );
    r[2] = vec4<T,Q>(
        u.x*u.z*(1-c) + u.y*s, u.y*u.z*(1-c) - u.x*s, c + u.z*u.z*(1-c), 0
    );
    r[3] = vec4<T,Q>(0, 0, 0, 1);
    
    return m * r;
}

template<typename T, qualifier Q>
constexpr mat4<T,Q> lookAt(const vec3<T,Q>& eye, const vec3<T,Q>& center, const vec3<T,Q>& up) {
    vec3<T,Q> f = normalize(center - eye);
    vec3<T,Q> s = normalize(cross(f, up));
    vec3<T,Q> u = cross(s, f);
    
    mat4<T,Q> result;
    result[0] = vec4<T,Q>(s.x, u.x, -f.x, 0);
    result[1] = vec4<T,Q>(s.y, u.y, -f.y, 0);
    result[2] = vec4<T,Q>(s.z, u.z, -f.z, 0);
    result[3] = vec4<T,Q>(-dot(s, eye), -dot(u, eye), dot(f, eye), 1);
    return result;
}

template<typename T, qualifier Q>
constexpr mat4<T,Q> perspective(T fovy, T aspect, T zNear, T zFar) {
    T tan_half_fovy = std::tan(fovy / 2);
    mat4<T,Q> result(0);
    
    result[0][0] = 1 / (aspect * tan_half_fovy);
    result[1][1] = 1 / tan_half_fovy;
    result[2][2] = -(zFar + zNear) / (zFar - zNear);
    result[2][3] = -1;
    result[3][2] = -(2 * zFar * zNear) / (zFar - zNear);
    return result;
}

// ============================================================================
// SIMD DISPATCH
// ============================================================================

struct simd_scalar_tag {};
struct simd_sse2_tag {};

template<typename T>
struct best_simd_tag { using type = simd_scalar_tag; };

template<>
struct best_simd_tag<float> {
#if defined(__AVX2__)
    using type = simd_sse2_tag;
#elif defined(__SSE2__)
    using type = simd_sse2_tag;
#else
    using type = simd_scalar_tag;
#endif
};

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
        return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
    }
};

inline float dot_simd_vec4(const float* a, const float* b) {
    using tag = typename best_simd_tag<float>::type;
    return dot_dispatcher<tag>::dot4(a, b);
}

// ============================================================================
// ULP COMPARISON
// ============================================================================

inline int32_t float_to_int_bits(float f) {
    int32_t i;
    std::memcpy(&i, &f, sizeof(float));
    return i;
}

inline uint32_t ulp_distance(float a, float b) {
    int32_t ia = float_to_int_bits(a);
    int32_t ib = float_to_int_bits(b);
    if (ia < 0) ia = 0x80000000 - ia;
    if (ib < 0) ib = 0x80000000 - ib;
    return (ia > ib) ? uint32_t(ia - ib) : uint32_t(ib - ia);
}

inline bool almost_equal_ulp(float a, float b, uint32_t max_ulp = 4) {
    if (a == b) return true;
    if (std::isnan(a) || std::isnan(b)) return false;
    if (std::isinf(a) || std::isinf(b)) return a == b;
    return ulp_distance(a, b) <= max_ulp;
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
    ASSERT_EQ(m[0][0], 1);
    ASSERT_EQ(m[1][1], 1);
    ASSERT_EQ(m[2][2], 1);
}

TEST_CASE(TestMatColumnMajor) {
    mat4<float> m = mat4<float>(2.0f);
    ASSERT_EQ(m[0][0], 2);
    ASSERT_EQ(m[1][1], 2);
    ASSERT_EQ(m[2][2], 2);
    ASSERT_EQ(m[3][3], 2);
    ASSERT_EQ(m[0][1], 0);
}

TEST_CASE(TestDotConstexpr) {
    constexpr vec3<float> a{1, 2, 3};
    constexpr vec3<float> b{4, 5, 6};
    constexpr float d = dot(a, b);
    ASSERT_EQ(d, 32.0f);
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
    mat3<float> m = mat3<float>(2.0f);
    vec3<float> v{1, 2, 3};
    vec3<float> r = m * v;
    ASSERT_EQ(r.x, 2);
    ASSERT_EQ(r.y, 4);
    ASSERT_EQ(r.z, 6);
    
    mat3<float> m2;
    m2[0] = vec3<float>{1, 0, 0};
    m2[1] = vec3<float>{0, 0, -1};
    m2[2] = vec3<float>{0, 1, 0};
    vec3<float> v2{1, 2, 3};
    vec3<float> r2 = m2 * v2;
    ASSERT_EQ(r2.x, 1);
    ASSERT_EQ(r2.y, 3);   // 1*0 + 2*0 + 3*1 = 3
    ASSERT_EQ(r2.z, -2);  // 1*0 + 2*(-1) + 3*0 = -2
}

TEST_CASE(TestTransformations) {
    mat4<float> t = translate(mat4<float>(), vec3<float>{1, 2, 3});
    ASSERT_EQ(t[3].x, 1);
    ASSERT_EQ(t[3].y, 2);
    ASSERT_EQ(t[3].z, 3);
    ASSERT_EQ(t[3].w, 1);
    
    mat4<float> s = scale(mat4<float>(), vec3<float>{2, 3, 4});
    ASSERT_EQ(s[0][0], 2);
    ASSERT_EQ(s[1][1], 3);
    ASSERT_EQ(s[2][2], 4);
    
    mat4<float> p = perspective<float, qualifier::packed>(1.5708f, 1.0f, 0.1f, 100.0f);
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
    ASSERT_TRUE(view[2][2] > 0);  // -f.z = -(-1) = 1
    ASSERT_TRUE(view[0][0] > 0);
    ASSERT_TRUE(view[1][1] > 0);
}

TEST_CASE(TestSIMDDispatch) {
    float a[4] = {1, 2, 3, 4};
    float b[4] = {5, 6, 7, 8};
    float d = dot_simd_vec4(a, b);
    ASSERT_EQ(d, 70.0f);
}

TEST_CASE(TestULPComparison) {
    ASSERT_TRUE(almost_equal_ulp(1.0f, 1.0f));
    ASSERT_TRUE(almost_equal_ulp(-0.0f, 0.0f));
    ASSERT_FALSE(almost_equal_ulp(std::numeric_limits<float>::quiet_NaN(), 1.0f));
    ASSERT_TRUE(almost_equal_ulp(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()));
    ASSERT_FALSE(almost_equal_ulp(std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()));
    
    float f1 = 1.0f;
    float f2 = std::nextafterf(1.0f, 2.0f);
    ASSERT_TRUE(almost_equal_ulp(f1, f2, 1));
    ASSERT_TRUE(almost_equal_ulp(f1, f2, 4));
    
    float f3 = 1.0f;
    float f4 = 1.0001f;
    ASSERT_FALSE(almost_equal_ulp(f3, f4, 4));
}

int main() { return RUN_ALL_TESTS(); }