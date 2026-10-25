// =============================================================================
// mate3d.hpp — Mini librería de matemática 3D del MÓDULO 19 (header-only)
// =============================================================================
// Esta es la librería que el módulo va construyendo a lo largo del tramo:
//   - Las lecciones la usan como base de los ejemplos correibles a mano.
//   - Los ejercicios implementan (con TODOs) las funciones que cada lección
//     enseña: largo/normalizar, proyección/reflexión, matrices TRS, jerarquía,
//     cuaterniones, proyección de cámara, raycasts, splines, etc.
//
// QUÉ YA VIENE RESUELTO (infraestructura, no conceptos del módulo):
//   - Vec2/Vec3/Vec4 con aritmética y multiplicación por escalar.
//   - Mat3/Mat4 column-major (igual que GLM: la columna es la tanda contigua
//     en memoria) con identidad, producto matriz*matriz, matriz*vector y
//     transformarPunto/transformarDireccion para matrices afines.
//   - inversaRigida(M4) para matrices de rotación + traslación (ver L07).
//   - Quat con producto de Hamilton, aritmética cuaternional y casiIgualQuat
//     (que respeta la doble cobertura).
//   - Constantes y conversiones de grados/radianes.
//
// QUÉ NO VIENE (se aprende/resuelve en lecciones y ejercicios 01..12):
//   dot/cross, largo, normalizar, distancia, proyección, reflexión,
//   construcción de TRS, matrices de proyección/cámara, cuaterniones desde
//   eje-ángulo, slerp, raycasts, Catmull-Rom, noise...
//
// Convención: like GLM — columnas contiguas en memoria, right-handed,
// "aplicar primero el factor de la derecha" (M = A * B aplica B y después A).
// =============================================================================
#pragma once
#include <cmath>
#include <cstddef>

namespace mate {

constexpr float PI = 3.14159265358979f;
constexpr float TAU = PI * 2.0f;

inline float aRadianes(float grados) { return grados * (PI / 180.0f); }
inline float   aGrados(float radianes) { return radianes * (180.0f / PI); }

inline bool casiIgual(float a, float b, float epsilon = 1e-5f) {
    return std::fabs(a - b) <= epsilon;
}

// -----------------------------------------------------------------------------
// Vectores
// -----------------------------------------------------------------------------
struct Vec2 {
    float x{}, y{};
};

struct Vec3 {
    float x{}, y{}, z{};

    Vec3& operator+=(Vec3 const& o) { x += o.x; y += o.y; z += o.z; return *this; }
    Vec3& operator-=(Vec3 const& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
    Vec3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
    Vec3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }
    float  operator[](int i) const { return (&x)[i]; }
};

inline Vec3 operator+(Vec3 a, Vec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
inline Vec3 operator-(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
inline Vec3 operator-(Vec3 a)         { return {-a.x, -a.y, -a.z}; }
inline Vec3 operator*(Vec3 a, float s) { return {a.x * s, a.y * s, a.z * s}; }
inline Vec3 operator*(float s, Vec3 a) { return a * s; }
inline Vec3 operator/(Vec3 a, float s) { return {a.x / s, a.y / s, a.z / s}; }

struct Vec4 {
    float x{}, y{}, z{}, w{};

    Vec4& operator+=(Vec4 const& o) { x += o.x; y += o.y; z += o.z; w += o.w; return *this; }
    float  operator[](int i) const { return (&x)[i]; }
};

// AYUDA DADA (infra): comparar vectores con tolerancia — para tests y demos.
inline bool casiIgualVec2(Vec2 a, Vec2 b, float eps = 1e-5f) {
    return casiIgual(a.x, b.x, eps) && casiIgual(a.y, b.y, eps);
}
inline bool casiIgualVec3(Vec3 a, Vec3 b, float eps = 1e-5f) {
    return casiIgual(a.x, b.x, eps) && casiIgual(a.y, b.y, eps) && casiIgual(a.z, b.z, eps);
}
inline bool casiIgualVec4(Vec4 a, Vec4 b, float eps = 1e-5f) {
    return casiIgual(a.x, b.x, eps) && casiIgual(a.y, b.y, eps)
        && casiIgual(a.z, b.z, eps) && casiIgual(a.w, b.w, eps);
}

// Ejes unitarios útiles.
inline constexpr Vec3 V3X{1.0f, 0.0f, 0.0f};
inline constexpr Vec3 V3Y{0.0f, 1.0f, 0.0f};
inline constexpr Vec3 V3Z{0.0f, 0.0f, 1.0f};
inline constexpr Vec3 V3CERO{0.0f, 0.0f, 0.0f};

// -----------------------------------------------------------------------------
// Mat3 (column-major, como GLM: m[c*3 + r])
// -----------------------------------------------------------------------------
struct Mat3 {
    float m[9]{};

    static Mat3 identidad() {
        Mat3 r{};
        r.m[0] = r.m[4] = r.m[8] = 1.0f;
        return r;
    }
    float at(int col, int row) const { return m[col * 3 + row]; }
    void  set(int col, int row, float v) { m[col * 3 + row] = v; }
    Vec3  columna(int c) const {
        return {m[c * 3 + 0], m[c * 3 + 1], m[c * 3 + 2]};
    }
    void  ponerColumna(int c, Vec3 v) {
        m[c * 3 + 0] = v.x; m[c * 3 + 1] = v.y; m[c * 3 + 2] = v.z;
    }
};

inline Mat3 operator*(Mat3 const& a, Mat3 const& b) {
    Mat3 r{};
    for (int c = 0; c < 3; ++c) {
        for (int row = 0; row < 3; ++row) {
            float s = 0.0f;
            for (int k = 0; k < 3; ++k) s += a.at(k, row) * b.at(c, k);
            r.set(c, row, s);
        }
    }
    return r;
}

inline Vec3 operator*(Mat3 const& m, Vec3 v) {
    Vec3 r{};
    for (int c = 0; c < 3; ++c)
        r += m.columna(c) * (c == 0 ? v.x : (c == 1 ? v.y : v.z));
    return r;
}

// -----------------------------------------------------------------------------
// Mat4 (column-major, como GLM: m[c*4 + r]; m[0..3] es la PRIMERA columna)
// -----------------------------------------------------------------------------
struct Mat4 {
    float m[16]{};

    static Mat4 identidad() {
        Mat4 r{};
        r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
        return r;
    }
    float at(int col, int row) const { return m[col * 4 + row]; }
    void  set(int col, int row, float v) { m[col * 4 + row] = v; }
    Vec4  columna(int c) const {
        return {m[c * 4 + 0], m[c * 4 + 1], m[c * 4 + 2], m[c * 4 + 3]};
    }
    void  ponerColumna4(int c, Vec4 v) {
        m[c * 4 + 0] = v.x; m[c * 4 + 1] = v.y; m[c * 4 + 2] = v.z; m[c * 4 + 3] = v.w;
    }
    void  ponerColumna3(int c, Vec3 v, float w) {
        m[c * 4 + 0] = v.x; m[c * 4 + 1] = v.y; m[c * 4 + 2] = v.z; m[c * 4 + 3] = w;
    }
    // Columna de ROTACIÓN (dirección de un eje local): w = 0.
    void  ponerColumna(int c, Vec3 v) {
        m[c * 4 + 0] = v.x; m[c * 4 + 1] = v.y; m[c * 4 + 2] = v.z; m[c * 4 + 3] = 0.0f;
    }
};

// a * b significa: primero se aplica b, después a (columnas de a son la misma
// convención de GLM). Es un triple lazo por columnas y filas.
inline Mat4 operator*(Mat4 const& a, Mat4 const& b) {
    Mat4 r{};
    for (int c = 0; c < 4; ++c) {
        for (int row = 0; row < 4; ++row) {
            float s = 0.0f;
            for (int k = 0; k < 4; ++k) s += a.at(k, row) * b.at(c, k);
            r.set(c, row, s);
        }
    }
    return r;
}

// m * v con v en coordenadas homogéneas (w = 1 punto, w = 0 dirección).
inline Vec4 operator*(Mat4 const& m, Vec4 v) {
    Vec4 r{};
    float k[4] = {v.x, v.y, v.z, v.w};
    for (int row = 0; row < 4; ++row) {
        float s = 0.0f;
        for (int c = 0; c < 4; ++c) s += m.at(c, row) * k[c];
        (&r.x)[row] = s;
    }
    return r;
}

// Punto en transform: "estira" a w=1, hace m*v y devuelve el trío xyz.
// Solo para matrices afines (model/world). No divide por w.
inline Vec3 transformarPunto(Mat4 const& m, Vec3 p) {
    Vec4 r = m * Vec4{p.x, p.y, p.z, 1.0f};
    return {r.x, r.y, r.z};
}

// Dirección en transform: w=0 para que la traslación NO la mueva.
inline Vec3 transformarDireccion(Mat4 const& m, Vec3 d) {
    Vec4 r = m * Vec4{d.x, d.y, d.z, 0.0f};
    return {r.x, r.y, r.z};
}

// Inversa de una matriz RIGIDA: *rotación ortogonal + traslación*, sin escala.
//   inv = [ R^T | -R^T * t ]  (fila inferior intacta: 0 0 0 1)
// Si hay escala o cortan (shear), NO sirve: ver L08 (descomposición).
inline Mat4 inversaRigida(Mat4 const& om) {
    Mat4 inv{};
    // Traspuesta del bloque rotacional 3x3: inv[c][r] = om[r][c]
    for (int c = 0; c < 3; ++c)
        for (int row = 0; row < 3; ++row)
            inv.set(c, row, om.at(row, c));
    // t = columna 3 de om; nueva traslación = -(R^T · t)
    //   (R^T·t)[c] = dot(columna c de R, t)  —  columna(c) acá es DE om, no de inv.
    Vec3 ts{om.m[12], om.m[13], om.m[14]};
    Vec3 t2;
    float tc[3] = {ts.x, ts.y, ts.z};
    for (int c = 0; c < 3; ++c)
        (&t2.x)[c] = -(om.at(c, 0) * tc[0] + om.at(c, 1) * tc[1] + om.at(c, 2) * tc[2]);
    inv.ponerColumna3(3, t2, 1.0f);
    return inv;
}

// -----------------------------------------------------------------------------
// Cuaterniones (Hamilton, double cover manejado en las funciones de uso)
// -----------------------------------------------------------------------------
struct Quat {
    float x{}, y{}, z{}, w{1.0f};
};

inline Quat quatIdentidad() { return {0.0f, 0.0f, 0.0f, 1.0f}; }

// Producto de Hamilton: q1 * q2 aplica primero la rotación de q2, luego q1
// (mismo sentido de orden que el producto de matrices).
inline Quat operator*(Quat const& a, Quat const& b) {
    return {
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
    };
}

// Aritmética cuaternional (necesaria para las mezclas — L10/L14).
inline Quat operator+(Quat const& a, Quat const& b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}
inline Quat operator-(Quat const& a, Quat const& b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}
inline Quat operator-(Quat const& a) { return {-a.x, -a.y, -a.z, -a.w}; }
inline Quat operator*(Quat const& a, float s) {
    return {a.x * s, a.y * s, a.z * s, a.w * s};
}
inline Quat operator/(Quat const& a, float s) {
    return {a.x / s, a.y / s, a.z / s, a.w / s};
}

// Comparación de rotaciones: q y -q representan la MISMA rotación (doble
// cobertura), así que comparamos por el producto escalar |dot| ~ 1.
inline bool casiIgualQuat(Quat a, Quat b, float eps = 1e-4f) {
    float d = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    return std::fabs(d) >= 1.0f - eps;
}

} // namespace mate
