/* =============================================================================
   MÓDULO 18 - SOLUCIÓN 03: Mini-librería de rects (semántica SDL_FRect)
   =============================================================================
   MISMO ENUNCIADO que ejercicios/ejercicio_03_rects.cpp.
   SOLUCIÓN COMENTADA: qué hace cada pieza y por qué.

   PUNTOS CLAVE:
   - El criterio "right/bottom EXCLUSIVO" es EXACTAMENTE el de SDL
     (SDL_HasRectIntersectionFloat, SDL_GetRectIntersectionFloat):
     [x, x+w). Si todo fuera inclusivo, los rects "pegados" (10,0 al lado
     de un rect que empieza en 10) colisionarían: wall-slide del módulo
     16 se rompería.
   - esVacio() con w<=0 o h<=0: SDL trata rects "negativos" igual
     (comprobar en SDL_rect.h: los casos se expresan min/max y si el
     min >= max no hay área).
   - rasterX("punto" a pixel) es half-up: composición pixel-art sin
     "wiggle" de subpixel (idéntico al layout de la L11).
   ============================================================================= */
#include <optional>
#include <cmath>
#include <algorithm>
#include "../../include/test_framework.hpp"

// ============================ MODELO (no tocar) ==============================
struct Rect { float x = 0, y = 0, w = 0, h = 0; };

constexpr Rect vacio() { return Rect{0, 0, 0, 0}; }

// Helper: áreas con w/h <= 0 cuentan como VACÍAS (sigue el criterio SDL)
static inline bool esVacio(const Rect& r) { return r.w <= 0.0f || r.h <= 0.0f; }

static inline float fmin(float a, float b) { return a < b ? a : b; }
static inline float fmax(float a, float b) { return a > b ? a : b; }

// ============================ SOLUCIÓN ======================================
// resuelto: intersección con borde exclusivo en right/bottom
static bool seTocan(const Rect& a, const Rect& b) {
    if (esVacio(a) || esVacio(b)) return false;   // sin área no hay toca
    // à-b a la derecha? / arriba?  -> XOR con la otra dirección: si ninguno
    // está separado, se tocan (los bordes [x, x+w) son abiertos a la derecha)
    return !(a.x + a.w <= b.x || b.x + b.w <= a.x ||
             a.y + a.h <= b.y || b.y + b.h <= a.y);
}

// resuelto: área común, vacío si no se tocan
static Rect interseccion(const Rect& a, const Rect& b) {
    if (!seTocan(a, b)) return vacio();
    const float x0 = fmax(a.x, b.x);
    const float y0 = fmax(a.y, b.y);
    const float x1 = fmin(a.x + a.w, b.x + b.w);
    const float y1 = fmin(a.y + a.h, b.y + b.h);
    return Rect{x0, y0, x1 - x0, y1 - y0};
}

// resuelto: mínimo rect que contiene ambos
static Rect union_(const Rect& a, const Rect& b) {
    const float x0 = fmin(a.x, b.x);
    const float y0 = fmin(a.y, b.y);
    const float x1 = fmax(a.x + a.w, b.x + b.w);
    const float y1 = fmax(a.y + a.h, b.y + b.h);
    return Rect{x0, y0, x1 - x0, y1 - y0};
}

// resuelto: (contains:) El test de BorderExclusivo borde exclusivo igual que intersección
static bool contiene(const Rect& r, float px, float py) {
    if (esVacio(r)) return false;
    return px >= r.x && px < r.x + r.w && py >= r.y && py < r.y + r.h;
}

// TODO 5 resuelto: expand con pad; si se colapsa, vacío
static Rect expande(const Rect& r, float pad) {
    Rect out{r.x - pad, r.y - pad, r.w + 2.0f * pad, r.h + 2.0f * pad};
    if (out.w <= 0.0f || out.h <= 0.0f) return vacio();
    return out;
}

// resuelto: rasterX half-up
static int rasterX(float x) {
    // trunc(x + 0.5) SIN float-shift: float->int trunc hacia el cero
    // (x = -0.5 -> trunc(0.0) = 0; NO -1)
    return static_cast<int>(x + 0.5f);
}

// ============================ TESTS (no tocar) ==============================
TEST_CASE(BordeExclusivoEnTocarYContener) {
    Rect a{0, 0, 10, 10}, b{10, 0, 5, 5};
    ASSERT_FALSE(seTocan(a, b));           // x+w == bx NO es colisión
    Rect b2{9.9f, 0, 5, 5};
    ASSERT_TRUE(seTocan(a, b2));
    ASSERT_FALSE(contiene(a, 10.0f, 5.0f));  // right exclusiva
    ASSERT_TRUE(contiene(a, 9.9f, 5.0f));    // pero un float interior sí
    ASSERT_FALSE(contiene(a, 5.0f, 10.0f));  // bottom exclusiva
}

TEST_CASE(InterseccionComunDisjuntosConteniendo) {
    Rect a{0, 0, 10, 10}, b{5, 5, 10, 10};
    Rect c = interseccion(a, b);
    ASSERT_EQ(c.x, 5.0f); ASSERT_EQ(c.y, 5.0f);
    ASSERT_EQ(c.w, 5.0f); ASSERT_EQ(c.h, 5.0f);
    ASSERT_TRUE(esVacio(interseccion(a, Rect{50, 50, 2, 2})));
    // contenidos: b está DENTRO de a -> intersección es b
    Rect in{2, 2, 3, 3};
    Rect r = interseccion(a, in);
    ASSERT_EQ(r.w, 3.0f);
    ASSERT_EQ(r.h, 3.0f);
}

TEST_CASE(UnionContieneAmbos) {
    Rect a{0, 0, 10, 10}, b{20, 20, 5, 5};
    Rect u = union_(a, b);
    ASSERT_EQ(u.x, 0.0f);
    ASSERT_EQ(u.y, 0.0f);
    ASSERT_EQ(u.w, 25.0f);   // 0..25 en x
    ASSERT_EQ(u.h, 25.0f);
    ASSERT_TRUE(contiene(u, 20.0f, 20.0f));   // la esquina de b incluida
}

TEST_CASE(ExpandeConClampVacío) {
    Rect a{10, 10, 4, 4};
    Rect e = expande(a, 2.0f);
    ASSERT_EQ(e.x, 8.0f); ASSERT_EQ(e.y, 8.0f);
    ASSERT_EQ(e.w, 8.0f); ASSERT_EQ(e.h, 8.0f);
    // pad negativo (encoge): si el "encogimiento" deja w o h <= 0, vacío
    Rect s = expande(a, -1.0f);
    ASSERT_EQ(s.x, 11.0f); ASSERT_EQ(s.w, 2.0f);
    ASSERT_TRUE(esVacio(expande(a, -3.0f)));   // 4 - 2*3 < 0 -> vacío
}

TEST_CASE(RasterHalfUp) {
    ASSERT_EQ(rasterX(3.4f), 3);
    ASSERT_EQ(rasterX(3.5f), 4);   // half-up
    ASSERT_EQ(rasterX(-0.5f), 0);  // -0.5 + 0.5 = 0 (no -1)
    ASSERT_EQ(rasterX(13.51f), 14);
}

int main() { return RUN_ALL_TESTS(); }
