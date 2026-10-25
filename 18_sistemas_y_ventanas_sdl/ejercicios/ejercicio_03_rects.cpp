/* =============================================================================
   MÓDULO 18 - EJERCICIO 03: Mini-librería de rects (semántica SDL_FRect)
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 06 (texturas/rects/FRect), Módulo 16 (colisiones AABB) y
          Módulo 19 (se viene: ray vs AABB usa estas mismas primitivas)

   CONSIGNA:
   El renderer fino de SDL3 usa SDL_FRect (lección 06) y trae PROPIAS
   funciones de intersección (verificadas en SDL_rect.h):
     SDL_HasRectIntersectionFloat(A, B)                  -> bool
     SDL_GetRectIntersectionFloat(A, B, &result)         -> bool
     SDL_GetRectUnionFloat(A, B, &result)                -> bool (fusión)
   Tu mini-librería Rect (misma semántica) es la que vas a usar en la
   SIMULACIÓN (los tests no tocan SDL) y más adelante en colisiones.

   ESTRUCTURA (copiar exactamente, no tocar):
     struct Rect { float x, y, w, h; }   // como SDL_FRect: w/h pueden ser 0;
                                         // negativos: seguí el criterio SDL:
                                         // w<0 o h<0 -> rect vacío sin área
   TU PARTE (funciones libres, todas PURAS):
     1.	bool seTocan(a, b): intersección con borde EXCLUSIVO (el criterio
        SDL / "right-bottom no incluido": tocar en x+w == b.x NO es
        colisión; un rect con w<=0 o h<=0 no colisiona nada).
     2.	Rect interseccion(a, b): el rect resultante (área común; si no
        se tocan, devuelve un rect vacío {0,0,0,0}).
     3.	Rect union_(a, b): rect mínimo que contiene ambos.
     4.	bool contiene(r, px, py): punto DENTRO (borde exclusivo).
     5.	Rect expande(r, pad): agranda el rect pad en CADA lado (el
        "broad phase" inflado que después usa la lección de grids del
        módulo 16); si w o h quedan <= 0, el resultado es vacío.
     6.	int rasterX(float x): redondeo half-up para pintar al raster
        (el mismo criterio del layout de la L11: v*scale+0.5 truncado).

   TESTS (no tocar): borde/exclusividad, disjuntos/contenidos, union,
   contención de puntos con borde, expansión con colapso a vacío, rounding.

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_03_rects.cpp -o ej && ./ej
   ============================================================================= */
#include <optional>
#include <cmath>
#include "../../include/test_framework.hpp"

// ============================ MODELO (no tocar) ==============================
struct Rect { float x = 0, y = 0, w = 0, h = 0; };

constexpr Rect vacio() { return Rect{0, 0, 0, 0}; }

// Helper: áreas con w/h <= 0 cuentan como VACÍAS (sigue el criterio SDL)
static inline bool esVacio(const Rect& r) { return r.w <= 0.0f || r.h <= 0.0f; }

static inline float fmin(float a, float b) { return a < b ? a : b; }
static inline float fmax(float a, float b) { return a > b ? a : b; }

// ============================ TU CÓDIGO AQUÍ ================================
// TODO 1: seTocan (borde exclusivo en right/bottom)
static bool seTocan(const Rect& a, const Rect& b) {
    (void)a; (void)b;
    return false;   // reemplazá esto
}

// TODO 2: interseccion (área común, vacío si no se tocan)
static Rect interseccion(const Rect& a, const Rect& b) {
    (void)a; (void)b;
    return vacio();
}

// TODO 3: union_ (mínimo rect que contiene ambos)
static Rect union_(const Rect& a, const Rect& b) {
    (void)a; (void)b;
    return vacio();
}

// TODO 4: contiene (borde exclusivo: el right/bottom NO cuenta)
static bool contiene(const Rect& r, float px, float py) {
    (void)r; (void)px; (void)py;
    return false;
}

// TODO 5: expande: agrega pad en los 4 lados; si w o h quedan <= 0,
// el resultado es vacío (misma regla que intersección)
static Rect expande(const Rect& r, float pad) {
    (void)r; (void)pad;
    return vacio();
}

// TODO 6: rasterX: redondeo half-up (x de espaciado float -> pixel entero)
static int rasterX(float x) {
    (void)x;
    return 0;
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
