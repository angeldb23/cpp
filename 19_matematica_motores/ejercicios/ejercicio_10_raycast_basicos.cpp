/* =============================================================================
   MÓDULO 19 - EJERCICIO 10: Intersecciones I — rayo vs esfera, AABB y plano
   =============================================================================
   DIFICULTAD: Avanzado
   APOYO: Lección 12 (el discriminante de la esfera, los SLABS de la caja con
   1/0 = infinito de IEEE, y el rayo-plano por el coseno entre dir y normal).
   NOTA: la dirección se asume NORMALIZADA (t = unidades de mundo).

   CONSIGNA:

     struct Impacto     { bool toca; float t; }
     struct ImpactoCaja { bool toca; float tEntrada; float tSalida; }

     Impacto rayoEsfera(Vec3 origen, Vec3 dirUnit, Vec3 centro, float radio)
        -> oc = origen - centro
           a = dot(d, d);  b = 2*dot(oc, d);  c = dot(oc, oc) - r²
           disc = b² - 4ac
           - disc < 0: NO toca
           - devolvé el MENOR t con t > EPS (raíz de (-b - sqrt(disc))/(2a));
             si ESA es <= 0 el origen está DENTRO: devolvé la otra
             (si las dos son <= 0, la esfera quedó detrás del rayo)

     ImpactoCaja rayoAABB(Vec3 origen, Vec3 dirUnit, Vec3 minCaja, Vec3 maxCaja)
        -> por cada eje: inv = 1/d[eje] (d==0 => infinito);
           t0 = (min - o) * inv;  t1 = (max - o) * inv
           si t0 > t1: swap (el rayo puede entrar "del otro lado")
           tEntrada = max(tEntrada, t0);  tSalida = min(tSalida, t1)
           si tEntrada > tSalida en ALGÚN punto: NO toca
           (acumulá desde -inf y +inf; la retorno: toca solo si tSalida >=
            max(tEntrada, EPS))

     Impacto rayoPlano(Vec3 origen, Vec3 dirUnit, Vec3 normalUnit, Vec3 punto)
        -> den = dot(dir, normal);  si |den| < EPS: PARALELO (no toca)
           t = dot(punto - origen, normal) / den
           toca solo si t > EPS

   TESTS (no tocar): fallan hasta las 3 funciones. EPS = 1e-6 como en la
   lección (con t=0 exacto no cuenta: el origen YA está ahí).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_10_... -o ej10 && ./ej10
   ============================================================================= */
#include <cmath>
#include <limits>
#include "../../include/test_framework.hpp"
#include "../include/mate3d.hpp"

using namespace mate;

constexpr float EPS = 1e-6f;

// Ayuda DADA (del ejercicio 01 / lección 03):
static float productoEscalar(Vec3 a, Vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
static float largoDe(Vec3 a) {
    return std::sqrt(productoEscalar(a, a));
}

struct Impacto     { bool toca; float t; };
struct ImpactoCaja { bool toca; float tEntrada; float tSalida; };

// ===== TU CÓDIGO (mirá la LECCIÓN 12) =====
Impacto rayoEsfera(Vec3 origen, Vec3 dirUnit, Vec3 centro, float radio) {
    (void)origen; (void)dirUnit; (void)centro; (void)radio;
    return {false, 0.0f};        // TODO 1: el discriminante y las DOS raíces
}

ImpactoCaja rayoAABB(Vec3 origen, Vec3 dirUnit, Vec3 minCaja, Vec3 maxCaja) {
    (void)origen; (void)dirUnit; (void)minCaja; (void)maxCaja;
    return {false, 0.0f, 0.0f};  // TODO 2: los SLABS por eje (x, y, z)
}

Impacto rayoPlano(Vec3 origen, Vec3 dirUnit, Vec3 normalUnit, Vec3 puntoDelPlano) {
    (void)origen; (void)dirUnit; (void)normalUnit; (void)puntoDelPlano;
    return {false, 0.0f};        // TODO 3: den casi 0 => paralelo; el cociente
}

// ============================= TESTS (no tocar) =============================
// DADA: el punto de impacto de un rayo con su t (lo vas a querer SIEMPRE):
static Vec3 puntoDelRayo(Vec3 origen, Vec3 dirUnit, float t) {
    return origen + dirUnit * t;
}

TEST_CASE(EsferaDelante) {
    // origen (0,0,0), dir +z, esfera en (0,0,5) radio 1: entra por z=4:
    Impacto h = rayoEsfera(V3CERO, V3Z, Vec3{0.0f, 0.0f, 5.0f}, 1.0f);
    ASSERT_TRUE(h.toca);
    ASSERT_TRUE(casiIgual(h.t, 4.0f, 1e-3f));
    // el punto de impacto queda en la PIEL de la esfera:
    Vec3 p = puntoDelRayo(V3CERO, V3Z, h.t);
    Vec3 delta = p - Vec3{0.0f, 0.0f, 5.0f};
    ASSERT_TRUE(casiIgual(largoDe(delta), 1.0f, 1e-3f));
}

TEST_CASE(EsferaConOrigenAdentro) {
    // origen DENTRO de esfera de radio 10 centrada en (0,0,5): sale por z=15:
    Impacto h = rayoEsfera(V3CERO, V3Z, Vec3{0.0f, 0.0f, 5.0f}, 10.0f);
    ASSERT_TRUE(h.toca);
    ASSERT_TRUE(casiIgual(h.t, 15.0f, 1e-3f));
}

TEST_CASE(EsferaQueNoCruza) {
    // detrás del rayo (z=-5 con dir +z):
    Impacto h1 = rayoEsfera(V3CERO, V3Z, Vec3{0.0f, 0.0f, -5.0f}, 1.0f);
    ASSERT_FALSE(h1.toca);
    // al costado (x=10, radio 1: el rayo pasa de largo):
    Impacto h2 = rayoEsfera(V3CERO, V3Z, Vec3{10.0f, 0.0f, 5.0f}, 1.0f);
    ASSERT_FALSE(h2.toca);
    // apenas ROZA (distancia centro-recta = radio exacto):
    Impacto h3 = rayoEsfera(V3CERO, V3Z, Vec3{1.0f, 0.0f, 5.0f}, 1.0f);
    ASSERT_TRUE(h3.toca);
    ASSERT_TRUE(casiIgual(h3.t, 5.0f, 1e-3f));
}

TEST_CASE(AABBDelante) {
    // caja x:[-1..1] y:[-1..1] z:[4..6] delante del rayo +z:
    ImpactoCaja c = rayoAABB(V3CERO, V3Z, Vec3{-1.0f, -1.0f, 4.0f},
                                          Vec3{ 1.0f,  1.0f, 6.0f});
    ASSERT_TRUE(c.toca);
    ASSERT_TRUE(casiIgual(c.tEntrada, 4.0f, 1e-4f));
    ASSERT_TRUE(casiIgual(c.tSalida,  6.0f, 1e-4f));
}

TEST_CASE(AABBQueNoCruza) {
    // DETRÁS del rayo:
    ImpactoCaja c1 = rayoAABB(V3CERO, V3Z, Vec3{-1.0f, -1.0f, -9.0f},
                                           Vec3{ 1.0f,  1.0f, -6.0f});
    ASSERT_FALSE(c1.toca);
    // al COSTADO (la slab-x nunca se cruza):
    ImpactoCaja c2 = rayoAABB(V3CERO, V3Z, Vec3{4.0f, -1.0f, 4.0f},
                                           Vec3{6.0f,  1.0f, 6.0f});
    ASSERT_FALSE(c2.toca);
}

TEST_CASE(AABBDiagonal) {
    // rayo en diagonal (1,0,1)/√2 contra caja x:[4..6] z:[4..6]:
    // entra por la ESQUINA-TRONCO en t=4 (x=4 y z=4 al mismo tiempo),
    // sale en t=6 (x=6 y z=6 al mismo tiempo):
    Vec3 d = V3X + V3Z;
    float l = std::sqrt(productoEscalar(d, d));
    d = d / l;
    ImpactoCaja c = rayoAABB(V3CERO, d, Vec3{4.0f, -1.0f, 4.0f},
                                        Vec3{6.0f,  1.0f, 6.0f});
    ASSERT_TRUE(c.toca);
    ASSERT_TRUE(casiIgual(c.tEntrada, 4.0f * l, 1e-3f));   // en unidades mundo
    ASSERT_TRUE(casiIgual(c.tSalida,  6.0f * l, 1e-3f));
}

TEST_CASE(PlanoNormalDelante) {
    // plano z=3, la normal mirando hacia la cámara (-z), rayo +z: t=3:
    Impacto p = rayoPlano(V3CERO, V3Z, Vec3{0.0f, 0.0f, -1.0f},
                          Vec3{0.0f, 0.0f, 3.0f});
    ASSERT_TRUE(p.toca);
    ASSERT_TRUE(casiIgual(p.t, 3.0f, 1e-4f));
}

TEST_CASE(PlanoParaleloODetras) {
    // rayo PARALELO al plano (dir por el plano): NO cruza:
    Impacto p1 = rayoPlano(V3CERO, V3Z, Vec3{0.0f, -1.0f, 0.0f},
                           Vec3{0.0f, 3.0f, 0.0f});
    ASSERT_FALSE(p1.toca);
    // el plano DETRÁS del rayo: t negativo, no cuenta:
    Impacto p2 = rayoPlano(V3CERO, V3Z, Vec3{0.0f, 0.0f, -1.0f},
                           Vec3{0.0f, 0.0f, -3.0f});
    ASSERT_FALSE(p2.toca);
}

int main() {
    std::cout << "EJERCICIO 10: raycast contra esfera/AABB/plano (lección 12)\n";
    int r = RUN_ALL_TESTS();
    if (r != 0) std::cout << "Sin resolver: los tests están fallando (OK).\n";
    return r;
}
