/* =============================================================================
   SOLUCIÓN 10: raycast vs esfera, AABB y plano (lección 12)
   =============================================================================
   - rayoEsfera: sustituyo la recta en |p - centro|² = r²: sale una cuadrática
     en t. disc < 0 => nada; si NO, la MENOR raíz con t > EPS (y si esa es
     <= 0, el origen está adentro => la otra raíz).
   - rayoAABB: los SLABS: cada eje aporta un intervalo [t0, t1]; el rayo toca
     la caja si la INTERSECCIÓN de los tres intervalos es no vacía. 1/0 = ±
     infinito de IEEE hace que las comparaciones sigan funcionando con
     dirección 0 (almorzar eso a pisa es la trampa).
   - rayoPlano: den = dot(dir, normal) == 0 => paralelo; el resto es el
     cociente del "undershoot" del punto del plano contra el rayo.
   ============================================================================= */
#include <cmath>
#include <limits>
#include "../../include/test_framework.hpp"
#include "../include/mate3d.hpp"

using namespace mate;

constexpr float EPS = 1e-6f;

static float productoEscalar(Vec3 a, Vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
static float largoDe(Vec3 a) {
    return std::sqrt(productoEscalar(a, a));
}

struct Impacto     { bool toca; float t; };
struct ImpactoCaja { bool toca; float tEntrada; float tSalida; };

static Vec3 puntoDelRayo(Vec3 origen, Vec3 dirUnit, float t) {
    return origen + dirUnit * t;
}

// ===== SOLUCIÓN =====
Impacto rayoEsfera(Vec3 origen, Vec3 dirUnit, Vec3 centro, float radio) {
    Vec3 oc = origen - centro;
    float a = productoEscalar(dirUnit, dirUnit);   // 1 si d normalizada
    float b = 2.0f * productoEscalar(oc, dirUnit);
    float c = productoEscalar(oc, oc) - radio * radio;
    float disc = b*b - 4.0f*a*c;
    if (disc < 0.0f) return {false, 0.0f};
    float raiz = std::sqrt(disc);
    float t1 = (-b - raiz) / (2.0f * a);           // la MÁS CERCANA
    float t2 = (-b + raiz) / (2.0f * a);
    if (t1 > EPS) return {true, t1};               // saliendo de afuera
    if (t2 > EPS) return {true, t2};               // origen ADENTRO
    return {false, 0.0f};                          // la esfera quedó atrás
}

ImpactoCaja rayoAABB(Vec3 origen, Vec3 dirUnit, Vec3 minCaja, Vec3 maxCaja) {
    constexpr float INF = std::numeric_limits<float>::infinity();
    float tEnt = -INF, tSal = INF;
    float o[3]  = {origen.x,  origen.y,  origen.z};
    float d[3]  = {dirUnit.x, dirUnit.y, dirUnit.z};
    float mn[3] = {minCaja.x, minCaja.y, minCaja.z};
    float mx[3] = {maxCaja.x, maxCaja.y, maxCaja.z};
    for (int eje = 0; eje < 3; ++eje) {
        float inv = (d[eje] != 0.0f) ? 1.0f / d[eje] : INF;
        float t0 = (mn[eje] - o[eje]) * inv;
        float t1 = (mx[eje] - o[eje]) * inv;
        if (t0 > t1) { float tmp = t0; t0 = t1; t1 = tmp; }  // puedo entrar
                                                             // "del otro lado"
        tEnt = std::fmax(tEnt, t0);   // la INTERSECCIÓN de los 3 rangos
        tSal = std::fmin(tSal, t1);
        if (tEnt > tSal) return {false, 0.0f, 0.0f};
    }
    return {tSal >= std::fmax(tEnt, EPS), std::fmax(tEnt, 0.0f), tSal};
}

Impacto rayoPlano(Vec3 origen, Vec3 dirUnit, Vec3 normalUnit,
                  Vec3 puntoDelPlano) {
    float den = productoEscalar(dirUnit, normalUnit);
    if (std::fabs(den) < EPS) return {false, 0.0f};   // paralelo
    float t = productoEscalar(puntoDelPlano - origen, normalUnit) / den;
    return {t > EPS, t};
}

// ============================= TESTS (del ejercicio — sin cambios) ==========
TEST_CASE(EsferaDelante) {
    Impacto h = rayoEsfera(V3CERO, V3Z, Vec3{0.0f, 0.0f, 5.0f}, 1.0f);
    ASSERT_TRUE(h.toca);
    ASSERT_TRUE(casiIgual(h.t, 4.0f, 1e-3f));
    Vec3 p = puntoDelRayo(V3CERO, V3Z, h.t);
    Vec3 delta = p - Vec3{0.0f, 0.0f, 5.0f};
    ASSERT_TRUE(casiIgual(largoDe(delta), 1.0f, 1e-3f));
}

TEST_CASE(EsferaConOrigenAdentro) {
    Impacto h = rayoEsfera(V3CERO, V3Z, Vec3{0.0f, 0.0f, 5.0f}, 10.0f);
    ASSERT_TRUE(h.toca);
    ASSERT_TRUE(casiIgual(h.t, 15.0f, 1e-3f));
}

TEST_CASE(EsferaQueNoCruza) {
    Impacto h1 = rayoEsfera(V3CERO, V3Z, Vec3{0.0f, 0.0f, -5.0f}, 1.0f);
    ASSERT_FALSE(h1.toca);
    Impacto h2 = rayoEsfera(V3CERO, V3Z, Vec3{10.0f, 0.0f, 5.0f}, 1.0f);
    ASSERT_FALSE(h2.toca);
    Impacto h3 = rayoEsfera(V3CERO, V3Z, Vec3{1.0f, 0.0f, 5.0f}, 1.0f);
    ASSERT_TRUE(h3.toca);
    ASSERT_TRUE(casiIgual(h3.t, 5.0f, 1e-3f));
}

TEST_CASE(AABBDelante) {
    ImpactoCaja c = rayoAABB(V3CERO, V3Z, Vec3{-1.0f, -1.0f, 4.0f},
                                          Vec3{ 1.0f,  1.0f, 6.0f});
    ASSERT_TRUE(c.toca);
    ASSERT_TRUE(casiIgual(c.tEntrada, 4.0f, 1e-4f));
    ASSERT_TRUE(casiIgual(c.tSalida,  6.0f, 1e-4f));
}

TEST_CASE(AABBQueNoCruza) {
    ImpactoCaja c1 = rayoAABB(V3CERO, V3Z, Vec3{-1.0f, -1.0f, -9.0f},
                                           Vec3{ 1.0f,  1.0f, -6.0f});
    ASSERT_FALSE(c1.toca);
    ImpactoCaja c2 = rayoAABB(V3CERO, V3Z, Vec3{4.0f, -1.0f, 4.0f},
                                           Vec3{6.0f,  1.0f, 6.0f});
    ASSERT_FALSE(c2.toca);
}

TEST_CASE(AABBDiagonal) {
    Vec3 d = V3X + V3Z;
    float l = std::sqrt(productoEscalar(d, d));
    d = d / l;
    ImpactoCaja c = rayoAABB(V3CERO, d, Vec3{4.0f, -1.0f, 4.0f},
                                        Vec3{6.0f,  1.0f, 6.0f});
    ASSERT_TRUE(c.toca);
    ASSERT_TRUE(casiIgual(c.tEntrada, 4.0f * l, 1e-3f));
    ASSERT_TRUE(casiIgual(c.tSalida,  6.0f * l, 1e-3f));
}

TEST_CASE(PlanoNormalDelante) {
    Impacto p = rayoPlano(V3CERO, V3Z, Vec3{0.0f, 0.0f, -1.0f},
                          Vec3{0.0f, 0.0f, 3.0f});
    ASSERT_TRUE(p.toca);
    ASSERT_TRUE(casiIgual(p.t, 3.0f, 1e-4f));
}

TEST_CASE(PlanoParaleloODetras) {
    Impacto p1 = rayoPlano(V3CERO, V3Z, Vec3{0.0f, -1.0f, 0.0f},
                           Vec3{0.0f, 3.0f, 0.0f});
    ASSERT_FALSE(p1.toca);
    Impacto p2 = rayoPlano(V3CERO, V3Z, Vec3{0.0f, 0.0f, -1.0f},
                           Vec3{0.0f, 0.0f, -3.0f});
    ASSERT_FALSE(p2.toca);
}

int main() {
    std::cout << "SOLUCIÓN 10: raycast básicos (todos los tests DEBEN pasar)\n";
    int r = RUN_ALL_TESTS();
    if (r != 0) std::cout << "ERROR: la solución debe pasar todos los tests.\n";
    return r;
}
