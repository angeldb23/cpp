/* =============================================================================
   MÓDULO 19 - EJERCICIO 11: Intersecciones II — rayo vs TRIÁNGULO (Möller-
   Trumbore) y el raycast contra una malla entera (el menor t gana)
   =============================================================================
   DIFICULTAD: Avanzado
   APOYO: Lección 13 (el algoritmo completo paso a paso y las coordenadas
   baricéntricas para INTERPOLAR lo que sale del vértice).

   CONSIGNA:

     struct ImpactoTri { bool toca; float t; float u; float v; }

     ImpactoTri rayoTrianguloMT(Vec3 O, Vec3 D, Vec3 p0, Vec3 p1, Vec3 p2)
        -> e1 = p1 - p0;     e2 = p2 - p0
           h  = cross(D, e2);   a = dot(e1, h)
           si |a| < EPS: PARALELO / sin área => no toca
           f  = 1 / a;     s = O - p0
           u  = f * dot(s, h);   si u < 0 || u > 1: no toca
           q  = cross(s, e1)
           v  = f * dot(D, q);   si v < 0 || u + v > 1: no toca
           t  = f * dot(e2, q);  si t <= EPS: el triángulo está DETRÁS
           toca: devolvé {true, t, u, v}

     ImpactoTri raycastMalla(Vec3 O, Vec3 D, std::vector<Vec3> const& tris)
        -> la escena real es una NUBE de triángulos (tris trae SIEMPRE
           grupos de 3 consecutivos: p0,p1,p2 / p0,p1,p2...). Probá cada
           triángulo y devolvé el que dé MENOR t (el más CERCANO a la
           cámara). Si nada cruza: {false, 0, 0, 0}.

     Vec3 interpoloBaricentricas(Vec3 a, Vec3 b, Vec3 c, float w0, float w1,
                                 float w2)
        -> la mezcla a*w0 + b*w1 + c*w2 (los pesos SUMAN 1 y provienen de
           (u, v): w0 = 1-(u+v), w1 = u, w2 = v — es lo que interpola
           normales/UVs/colores en el vertex pipeline)

   TESTS (no tocar): triangúlate con el MISMO triángulo de la lección 13
   (plano z=0 con vértice en (0,0), (2,0) y (0,2)).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_11_... -o ej11 && ./ej11
   ============================================================================= */
#include <cmath>
#include <vector>
#include "../../include/test_framework.hpp"
#include "../include/mate3d.hpp"

using namespace mate;

constexpr float EPS = 1e-8f;

// Ayudas DADA (del ejercicio 01 / lección 03):
static float productoEscalar(Vec3 a, Vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
static Vec3 productoVectorial(Vec3 a, Vec3 b) {
    return { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
}

struct ImpactoTri { bool toca; float t; float u; float v; };

// ===== TU CÓDIGO (mirá la LECCIÓN 13) =====
ImpactoTri rayoTrianguloMT(Vec3 O, Vec3 D, Vec3 p0, Vec3 p1, Vec3 p2) {
    (void)O; (void)D; (void)p0; (void)p1; (void)p2;
    return {false, 0.0f, 0.0f, 0.0f};   // TODO 1: determinante h, f, u, q, v, t
}

ImpactoTri raycastMalla(Vec3 O, Vec3 D, std::vector<Vec3> const& tris) {
    (void)O; (void)D; (void)tris;
    return {false, 0.0f, 0.0f, 0.0f};   // TODO 2: el MENOR t de todos los tris
}

Vec3 interpoloBaricentricas(Vec3 a, Vec3 b, Vec3 c,
                            float w0, float w1, float w2) {
    (void)a; (void)b; (void)c; (void)w0; (void)w1; (void)w2;
    return {0.0f, 0.0f, 0.0f};          // TODO 3: la mezcla ponderada
}

// ============================= TESTS (no tocar) =============================
TEST_CASE(ElCentroDelTriangulo) {
    // el triángulo de la lección: p0(0,0,0), p1(2,0,0), p2(0,2,0) en z=0;
    // el rayo viene desde (0.5, 0.5, -5) hacia +z. Su impacto: t=5 y
    // baricéntricas u = v = 0.25 (el punto cae EXACTO en (0.5, 0.5, 0)):
    Vec3 p0{0.0f, 0.0f, 0.0f}, p1{2.0f, 0.0f, 0.0f}, p2{0.0f, 2.0f, 0.0f};
    ImpactoTri h = rayoTrianguloMT(Vec3{0.5f, 0.5f, -5.0f}, V3Z, p0, p1, p2);
    ASSERT_TRUE(h.toca);
    ASSERT_TRUE(casiIgual(h.t, 5.0f, 1e-3f));
    ASSERT_TRUE(casiIgual(h.u, 0.25f, 1e-3f));
    ASSERT_TRUE(casiIgual(h.v, 0.25f, 1e-3f));
    // con los pesos (w0 = 1-(u+v), w1 = u, w2 = v): reconstituyen el punto:
    float w0 = 1.0f - (h.u + h.v), w1 = h.u, w2 = h.v;
    Vec3 punto = interpoloBaricentricas(p0, p1, p2, w0, w1, w2);
    ASSERT_TRUE(casiIgualVec3(punto, Vec3{0.5f, 0.5f, 0.0f}, 1e-3f));
}

TEST_CASE(ElVerticeEsUnPuntoValido) {
    // el rayo pega EXACTO en el vértice p1 = (2,0,0): u=1, v=0:
    ImpactoTri h = rayoTrianguloMT(Vec3{2.0f, 0.0f, -5.0f}, V3Z,
                                   Vec3{0.0f, 0.0f, 0.0f},
                                   Vec3{2.0f, 0.0f, 0.0f},
                                   Vec3{0.0f, 2.0f, 0.0f});
    ASSERT_TRUE(h.toca);
    ASSERT_TRUE(casiIgual(h.t, 5.0f, 1e-3f));
    ASSERT_TRUE(casiIgual(h.u, 1.0f, 1e-3f));
    ASSERT_TRUE(casiIgual(h.v, 0.0f, 1e-3f));
}

TEST_CASE(FueraDelTriangulo) {
    // (1.9, 1.9) con u+v > 1: CAE FUERA del triángulo:
    ImpactoTri h1 = rayoTrianguloMT(Vec3{1.9f, 1.9f, -5.0f}, V3Z,
                                    Vec3{0.0f, 0.0f, 0.0f},
                                    Vec3{2.0f, 0.0f, 0.0f},
                                    Vec3{0.0f, 2.0f, 0.0f});
    ASSERT_FALSE(h1.toca);
    // el rayo PARALELO al plano del triángulo (dir = x): |a| < EPS:
    ImpactoTri h2 = rayoTrianguloMT(Vec3{0.5f, 0.5f, -5.0f}, V3X,
                                    Vec3{0.0f, 0.0f, 0.0f},
                                    Vec3{2.0f, 0.0f, 0.0f},
                                    Vec3{0.0f, 2.0f, 0.0f});
    ASSERT_FALSE(h2.toca);
    // el triángulo está DETRÁS del origen del rayo:
    ImpactoTri h3 = rayoTrianguloMT(Vec3{0.5f, 0.5f, 5.0f}, V3Z,
                                    Vec3{0.0f, 0.0f, 0.0f},
                                    Vec3{2.0f, 0.0f, 0.0f},
                                    Vec3{0.0f, 2.0f, 0.0f});
    ASSERT_FALSE(h3.toca);
}

TEST_CASE(LaMallaDevuelveElMasCercano) {
    // TRES triángulos apilados en z: uno a t=5 (z=0), otro a t=12 (z=7) y
    // el más "lejos" del impacto... al revés: el MÁS CERCANO es el de z=0:
    Vec3 p0{0.0f, 0.0f, 0.0f}, p1{2.0f, 0.0f, 0.0f}, p2{0.0f, 2.0f, 0.0f};
    std::vector<Vec3> malla = {
        {0.0f, 0.0f, 7.0f}, {2.0f, 0.0f, 7.0f}, {0.0f, 2.0f, 7.0f},   // z=7
        {0.0f, 0.0f, 3.0f}, {2.0f, 0.0f, 3.0f}, {0.0f, 2.0f, 3.0f},   // z=3
        p0, p1, p2,                                                   // z=0
    };
    ImpactoTri h = raycastMalla(Vec3{0.5f, 0.5f, -5.0f}, V3Z, malla);
    ASSERT_TRUE(h.toca);
    ASSERT_TRUE(casiIgual(h.t, 5.0f, 1e-3f));   // el MENOR t (z=0, de -5 a 0)
    // si NADA cruza (más atrás que el ORIGEN del rayo: el t sale <= 0):
    std::vector<Vec3> mallaDetras = {
        {0.0f, 0.0f, -9.0f}, {2.0f, 0.0f, -9.0f}, {0.0f, 2.0f, -9.0f},
    };
    ImpactoTri h2 = raycastMalla(Vec3{0.5f, 0.5f, -5.0f}, V3Z, mallaDetras);
    ASSERT_FALSE(h2.toca);
}

TEST_CASE(BasicosDeLaMezcla) {
    // ¿y si el punto cae sobre una ARISTA o un VÉRTICE? los pesos eligen:
    Vec3 m = interpoloBaricentricas(V3X, V3Y, V3Z, 1.0f, 0.0f, 0.0f);
    ASSERT_TRUE(casiIgualVec3(m, V3X, 1e-5f));
    m = interpoloBaricentricas(V3X, V3Y, V3Z, 0.0f, 0.0f, 1.0f);
    ASSERT_TRUE(casiIgualVec3(m, V3Z, 1e-5f));
    m = interpoloBaricentricas(V3X, V3Y, V3Z, 0.0f, 0.5f, 0.5f);
    ASSERT_TRUE(casiIgualVec3(m, Vec3{0.0f, 0.5f, 0.5f}, 1e-5f));
}

int main() {
    std::cout << "EJERCICIO 11: ray-triángulo (Möller-Trumbore) y malla (lección 13)\n";
    int r = RUN_ALL_TESTS();
    if (r != 0) std::cout << "Sin resolver: los tests están fallando (OK).\n";
    return r;
}
