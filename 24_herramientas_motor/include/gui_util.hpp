/* =============================================================================
   MÓDULO 24 - CABECERA COMÚN (header-only, SIN Dear ImGui)
   =============================================================================
   Todo lo que se puede probar sin abrir una ventana vive acá: matemática de
   gizmos, hashing de IDs, rectángulos, colores y utilidades de texto. Los
   ejercicios y las soluciones incluyen SOLO esta cabecera y el test framework,
   así que compilan y corren en cualquier máquina (incluso headless).

   Regla del módulo: el código que toca ImGui/SDL/OpenGL se verifica por
   compilación; la lógica se verifica con tests. Esta cabecera es la frontera.
   ============================================================================= */
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace gui {

inline constexpr float kPi = 3.14159265358979323846f;

inline float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

inline float lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

// smoothstep clásico: 0 en t=0, 1 en t=1 y derivada 0 en ambos extremos.
inline float smooth01(float t) {
    t = clampf(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;
};

inline Vec2 operator+(Vec2 a, Vec2 b) { return {a.x + b.x, a.y + b.y}; }
inline Vec2 operator-(Vec2 a, Vec2 b) { return {a.x - b.x, a.y - b.y}; }
inline Vec2 operator*(Vec2 a, float s) { return {a.x * s, a.y * s}; }
inline float dot(Vec2 a, Vec2 b) { return a.x * b.x + a.y * b.y; }

struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

inline Vec3 operator+(Vec3 a, Vec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
inline Vec3 operator-(Vec3 a, Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
inline Vec3 operator*(Vec3 a, float s) { return {a.x * s, a.y * s, a.z * s}; }
inline float dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

inline Vec3 cross(Vec3 a, Vec3 b) {
    return {a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
}

inline float length(Vec3 v) { return std::sqrt(dot(v, v)); }

inline Vec3 normalize(Vec3 v) {
    const float len = length(v);
    return len > 0.0f ? v * (1.0f / len) : Vec3{};
}

struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 0.0f;

    float right() const { return x + w; }
    float bottom() const { return y + h; }
    bool empty() const { return w <= 0.0f || h <= 0.0f; }

    // Borde inclusivo: un punto justo en x+w pertenece al rectángulo.
    bool contains(Vec2 p) const {
        return p.x >= x && p.x <= right() && p.y >= y && p.y <= bottom();
    }

    bool overlaps(const Rect& o) const {
        return x <= o.right() && right() >= o.x && y <= o.bottom() && bottom() >= o.y;
    }
};

struct Color {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};

// -----------------------------------------------------------------------------
// IDs estables estilo Dear ImGui.
// ImGui calcula el ID de un widget como hash(ventana + stack de PushID + label).
// Usamos FNV-1a de 32 bits: determinista, barato y suficiente para una demo.
// Dos widgets con el mismo label en scopes distintos DEBEN dar IDs distintos.
// -----------------------------------------------------------------------------
inline std::uint32_t hashId(std::string_view text, std::uint32_t seed = 2166136261u) {
    std::uint32_t h = seed;
    for (unsigned char c : text) {
        h ^= static_cast<std::uint32_t>(c);
        h *= 16777619u;  // primo FNV de 32 bits
    }
    return h;
}

// -----------------------------------------------------------------------------
// Texto: minúsculas ASCII y búsqueda sin distinguir mayúsculas.
// Para Unicode de verdad haría falta una librería; el curso se queda en ASCII
// a propósito para que el comportamiento sea determinista y testeable.
// -----------------------------------------------------------------------------
inline std::string toLower(std::string_view s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        out.push_back((c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c);
    }
    return out;
}

inline bool containsNoCase(std::string_view haystack, std::string_view needle) {
    if (needle.empty()) return true;
    return toLower(haystack).find(toLower(needle)) != std::string::npos;
}

inline std::string trim(std::string_view s) {
    std::size_t b = 0;
    std::size_t e = s.size();
    while (b < e && (s[b] == ' ' || s[b] == '\t' || s[b] == '\r' || s[b] == '\n')) ++b;
    while (e > b && (s[e - 1] == ' ' || s[e - 1] == '\t' || s[e - 1] == '\r' || s[e - 1] == '\n')) --e;
    return std::string(s.substr(b, e - b));
}

inline std::vector<std::string> split(std::string_view s, char sep) {
    std::vector<std::string> out;
    std::size_t start = 0;
    for (std::size_t i = 0; i <= s.size(); ++i) {
        if (i == s.size() || s[i] == sep) {
            out.emplace_back(s.substr(start, i - start));
            start = i + 1;
        }
    }
    return out;
}

} // namespace gui
