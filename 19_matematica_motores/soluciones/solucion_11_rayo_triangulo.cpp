/* =============================================================================
   SOLUCIÓN 11: rayo vs triángulo (Möller-Trumbore) + malla + baricéntricas
   =============================================================================
   - El algoritmo sale del plano del triángulo resuelto con la REGLA DE
     CRAMER (el determinante del sistema 3x3): de ahí salen t, u, v en una
     sola pasada, SIN planos ni puntos aparte. u y v son las coordenadas
     baricéntricas: son las que interpolan en el vertex pipeline.
   - raycastMalla: recorro los triángulos de 3 en 3 y me quedo el MENOR t
     (el más cercano a la cámara). Un for bruto con mejor t local.
   - interpoloBaricentricas: la mezcla ponderada a*w0 + b*w1 + c*w2 (el
     atributo se mueve LINEALMENTE en el triángulo).
   ============================================================================= */
#include <cmath>
#include <vector>
#include "../../include/test_framework.hpp"
#include "../include/mate3d.hpp"

using namespace mate;

constexpr float EPS = 1e-8f;

static float productoEscalar(Vec3 a, Vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
static Vec3 productoVectorial(Vec3 a, Vec3 b) {
    return { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
}

struct ImpactoTri { bool toca; float t; float u; float v; };

// ===== SOLUCIÓN =====
ImpactoTri rayoTrianguloMT(Vec3 O, Vec3 D, Vec3 p0, Vec3 p1, Vec3 p2) {
    ImpactoTri out{false, 0.0f, 0.0f, 0.0f};
    Vec3 e1 = p1 - p0;
    Vec3 e2 = p2 - p0;

    Vec3 h = productoVectorial(D, e2);       // el "helper" del determinante
    float a = productoEscalar(e1, h);
    if (std::fabs(a) < EPS) return out;      // paralelo / triángulo sin área

    float f = 1.0f / a;
    float u = f * productoEscalar(O - p0, h);
    if (u < 0.0f || u > 1.0f) return out;    // fuera por el lado e1

    Vec3 q = productoVectorial(O - p0, e1);
    float v = f * productoEscalar(D, q);
    if (v < 0.0f || u + v > 1.0f) return out;  // fuera por e2 / la suma

    float t = f * productoEscalar(e2, q);
    if (t <= EPS) return out;                // el triángulo está DETRÁS
    return {true, t, u, v};
}

ImpactoTri raycastMalla(Vec3 O, Vec3 D, std::vector<Vec3> const& tris) {
    ImpactoTri mejor{false, 0.0f, 0.0f, 0.0f};
    // recorro la nube de 3 EN 3 (la malla apoya sus vértices así):
    for (size_t i = 0; i + 2 < tris.size(); i += 3) {
        ImpactoTri h = rayoTrianguloMT(O, D, tris[i], tris[i + 1], tris[i + 2]);
        if (h.toca && (!mejor.toca || h.t < mejor.t)) mejor = h;
    }
    return mejor;
}

Vec3 interpoloBaricentricas(Vec3 a, Vec3 b, Vec3 c, float w0, float w1, float w2) {
    return a*w0 + b*w1 + c*w2;
}

// ============================= TESTS (del ejercicio — sin cambios) ==========
TEST_CASE(ElCentroDelTriangulo) {
    Vec3 p0{0.0f, 0.0f, 0.0f}, p1{2.0f, 0.0f, 0.0f}, p2{0.0f, 2.0f, 0.0f};
    ImpactoTri h = rayoTrianguloMT(Vec3{0.5f, 0.5f, -5.0f}, V3Z, p0, p1, p2);
    ASSERT_TRUE(h.toca);
    ASSERT_TRUE(casiIgual(h.t, 5.0f, 1e-3f));
    ASSERT_TRUE(casiIgual(h.u, 0.25f, 1e-3f));
    ASSERT_TRUE(casiIgual(h.v, 0.25f, 1e-3f));
    float w0 = 1.0f - (h.u + h.v), w1 = h.u, w2 = h.v;
    Vec3 punto = interpoloBaricentricas(p0, p1, p2, w0, w1, w2);
    ASSERT_TRUE(casiIgualVec3(punto, Vec3{0.5f, 0.5f, 0.0f}, 1e-3f));
}

TEST_CASE(ElVerticeEsUnPuntoValido) {
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
    ImpactoTri h1 = rayoTrianguloMT(Vec3{1.9f, 1.9f, -5.0f}, V3Z,
                                    Vec3{0.0f, 0.0f, 0.0f},
                                    Vec3{2.0f, 0.0f, 0.0f},
                                    Vec3{0.0f, 2.0f, 0.0f});
    ASSERT_FALSE(h1.toca);
    ImpactoTri h2 = rayoTrianguloMT(Vec3{0.5f, 0.5f, -5.0f}, V3X,
                                    Vec3{0.0f, 0.0f, 0.0f},
                                    Vec3{2.0f, 0.0f, 0.0f},
                                    Vec3{0.0f, 2.0f, 0.0f});
    ASSERT_FALSE(h2.toca);
    ImpactoTri h3 = rayoTrianguloMT(Vec3{0.5f, 0.5f, 5.0f}, V3Z,
                                    Vec3{0.0f, 0.0f, 0.0f},
                                    Vec3{2.0f, 0.0f, 0.0f},
                                    Vec3{0.0f, 2.0f, 0.0f});
    ASSERT_FALSE(h3.toca);
}

TEST_CASE(LaMallaDevuelveElMasCercano) {
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
    Vec3 m = interpoloBaricentricas(V3X, V3Y, V3Z, 1.0f, 0.0f, 0.0f);
    ASSERT_TRUE(casiIgualVec3(m, V3X, 1e-5f));
    m = interpoloBaricentricas(V3X, V3Y, V3Z, 0.0f, 0.0f, 1.0f);
    ASSERT_TRUE(casiIgualVec3(m, V3Z, 1e-5f));
    m = interpoloBaricentricas(V3X, V3Y, V3Z, 0.0f, 0.5f, 0.5f);
    ASSERT_TRUE(casiIgualVec3(m, Vec3{0.0f, 0.5f, 0.5f}, 1e-5f));
}

int main() {
    std::cout << "SOLUCIÓN 11: ray-triángulo y malla (todos DEBEN pasar)\n";
    int r = RUN_ALL_TESTS();
    if (r != 0) std::cout << "ERROR: la solución debe pasar todos los tests.\n";
    return r;
}
