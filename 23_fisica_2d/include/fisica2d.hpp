#pragma once
/* =============================================================================
   MÓDULO 23 - Cabecera común de Física 2D (namespace f2d)

   Solo contiene los tipos y operaciones que COMPARTEN lecciones, ejercicios y
   soluciones: Vec2, AABB, Pair y helpers numéricos. La física de verdad
   (SAT, impulsos, joints, broad-phase, etc.) se implementa en cada archivo:
   ese es el ejercicio.

   Convención de ejes: +x a la derecha, +y hacia arriba (como en Box2D y en
   la matemática del módulo 19, NO como la pantalla de SDL/raylib).
   ============================================================================= */
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>

namespace f2d {

constexpr float kEps = 1e-6f;

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2() = default;
    Vec2(float x_, float y_) : x(x_), y(y_) {}

    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator-() const { return {-x, -y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
    Vec2 operator/(float s) const { return {x / s, y / s}; }
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
    Vec2& operator-=(const Vec2& o) { x -= o.x; y -= o.y; return *this; }
    Vec2& operator*=(float s) { x *= s; y *= s; return *this; }
    bool operator==(const Vec2& o) const {
        return std::fabs(x - o.x) <= kEps && std::fabs(y - o.y) <= kEps;
    }
};

inline float dot(const Vec2& a, const Vec2& b) { return a.x * b.x + a.y * b.y; }
// cross 2D: componente z del producto vectorial (a x b).
inline float cross(const Vec2& a, const Vec2& b) { return a.x * b.y - a.y * b.x; }
inline float lengthSq(const Vec2& v) { return dot(v, v); }
inline float length(const Vec2& v) { return std::sqrt(lengthSq(v)); }

inline Vec2 normalize(const Vec2& v) {
    const float len = length(v);
    if (len <= kEps) return {0.0f, 0.0f};
    return v / len;
}

// Perpendicular antihoraria: (x,y) -> (-y,x). Sirve para normales de aristas.
inline Vec2 perp(const Vec2& v) { return {-v.y, v.x}; }

inline float clampf(float v, float lo, float hi) {
    return std::max(lo, std::min(hi, v));
}

struct AABB {
    float minX = 0.0f;
    float minY = 0.0f;
    float maxX = 0.0f;
    float maxY = 0.0f;

    AABB() = default;
    AABB(float minX_, float minY_, float maxX_, float maxY_)
        : minX(minX_), minY(minY_), maxX(maxX_), maxY(maxY_) {}

    // Para broad-phase, tocarse cuenta como solaparse (bordes inclusivos).
    bool overlaps(const AABB& o) const {
        return minX <= o.maxX && maxX >= o.minX &&
               minY <= o.maxY && maxY >= o.minY;
    }
};

// Par de IDs de entidades, siempre normalizado a (a < b) y comparable.
struct Pair {
    int a = 0;
    int b = 0;

    bool operator==(const Pair& o) const { return a == o.a && b == o.b; }
    bool operator<(const Pair& o) const {
        return a != o.a ? a < o.a : b < o.b;
    }
};

// División entera hacia abajo: necesaria para celdas con coordenadas negativas.
inline int floorDiv(float value, float cellSize) {
    return static_cast<int>(std::floor(value / cellSize));
}

} // namespace f2d
