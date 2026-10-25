/* =============================================================================
   MÓDULO 19 - LECCIÓN 12: Intersecciones I — rayo vs esfera, AABB y plano
   =============================================================================
   TEMA 7 de TEMAS.md (1ra lección): ray-AABB (slab), ray-esfera, ray-plano.

   == EL RAYO ================================================================

   Un rayo es la recta "congelada" del espacio:

       P(t) = origen + t * direccion       (t > 0 hacia adelante del ORIGEN)

   Reglas del curso: la dirección viene NORMALIZADA (así t es la distancia
   en unidades del mundo — útil para los raycast: el menor t gana).
   Y quedan los límites del test aparte (p. ej. near/far de cámara, o el
   alcance del láser): no confiar solo en "t > 0".

   == ESFERA -------------------------------------------------------------------

   Resolver |origen + t*d - centro|² = radio²:
       oc = origen - centro
       a = dot(d, d); b = 2 * dot(oc, d); c = dot(oc, oc) - r²
       discriminante = b² - 4*a*c
   - < 0: NO toca   - = 0: roza tangente (1 solución)  - > 0: DOS
   intersecciones: se devuelve el MENOR con t > epsilon (si el origen está
   DENTRO de la esfera el menor t es ≤ 0: se devuelve el segundo).

   == LOS "SLABS" (ray-AABB) ================================================

   La caja es el caso más barato de tocar: por cada EJE el rayo entra por
   una pared (x = min) y sale por la otra (x = max); cada par genera un
   rango [t0, t1]. El rayo toca la caja si TODOS los rangos se superponen:

   para eje en {x, y, z}:
       inv_d = 1 / d[eje]         (con d == 0: queda infinito de signo;
                                   el rayo es PARALELO a esas dos paredes)
       t0 = (min[eje] - o[eje]) * inv_d
       t1 = (max[eje] - o[eje]) * inv_d
       if (t0 > t1) swap(t0, t1)
       tEntrada = max(tEntrada, t0)
       tSalida  = min(tSalida,  t1)
       si tEntrada > tSalida → FUERA (no se cruzan los rangos)

   En C++: la división por 0 se maneja con
   std::numeric_limits<float>::infinity(); por IEEE 1/0 = ±inf y las
   comparaciones siguen funcionando. Cuidado con -0.0f (signo invertido) —
   en la práctica: manejar el caso 0 aparte si tu formato puede darlo.

   == PLANO -------------------------------------------------------------------

   Plano definido por normal unitaria n y un punto q del plano:
       dot(d, n) == 0 → PARALELO (sin intersección o infinita: ver caso)
       t = dot(q - origen, n) / dot(d, n)
   Solo si t > epsilon.

   == COMO PROBAR ESTO =======================================================

   g++ -std=c++20 -Wall -Wextra leccion_12_... -o leccion12 && ./leccion12
   (La DEMO muestra las tres: esfera con origen dentro/fuera, AABB esquinas,
   plano paralelo.)
   ============================================================================= */

#include "include/mate3d.hpp"
#include <cmath>
#include <cstdio>
#include <iostream>
#include <limits>
#include <vector>

using namespace mate;

constexpr float EPS = 1e-6f;

// ==== PRIMITIVAS LOCALES ===================================================
float productoEscalar(Vec3 a, Vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
// ===========================================================================

struct Impacto { bool toca; float t; };   // el "t" del primer choque

// ESFERA: centro + radio. t válido > EPS (o el segundo si el origen está adentro).
Impacto rayoEsfera(Vec3 origen, Vec3 dirUnit, Vec3 centro, float radio) {
    Vec3 oc = origen - centro;
    float a = productoEscalar(dirUnit, dirUnit);      // 1 si d normalizado
    float b = 2.0f * productoEscalar(oc, dirUnit);
    float c = productoEscalar(oc, oc) - radio * radio;
    float disc = b*b - 4.0f*a*c;
    if (disc < 0.0f) return {false, 0.0f};
    float raiz = std::sqrt(disc);
    float t1 = (-b - raiz) / (2.0f * a);
    float t2 = (-b + raiz) / (2.0f * a);
    if (t1 > EPS)    return {true, t1};   // el MÁS CERCANO desde afuera
    if (t2 > EPS)    return {true, t2 };  // el origen está DENTRO: el segundo
    return {false, 0.0f};                 // la esfera quedó "detrás" del rayo
}

struct ImpactoCaja { bool toca; float tEntrada; float tSalida; };

ImpactoCaja rayoAABB(Vec3 origen, Vec3 dirUnit, Vec3 minCaja, Vec3 maxCaja) {
    constexpr float INF = std::numeric_limits<float>::infinity();
    float tEnt = -INF, tSal = INF;
    float o[3] = {origen.x, origen.y, origen.z};
    float d[3] = {dirUnit.x, dirUnit.y, dirUnit.z};
    float mn[3] = {minCaja.x, minCaja.y, minCaja.z};
    float mx[3] = {maxCaja.x, maxCaja.y, maxCaja.z};
    for (int eje = 0; eje < 3; ++eje) {
        float inv = (d[eje] != 0.0f) ? 1.0f / d[eje] : INF;
        float t0 = (mn[eje] - o[eje]) * inv;
        float t1 = (mx[eje] - o[eje]) * inv;
        if (t0 > t1) { float tmp = t0; t0 = t1; t1 = tmp; }
        tEnt = std::max(tEnt, t0);
        tSal = std::min(tSal, t1);
        if (tEnt > tSal) return {false, 0.0f, 0.0f};   // los rangos no se cruzan
    }
    return {tSal >= std::max(tEnt, EPS), std::max(tEnt, 0.0f), tSal};
}

Impacto rayoPlano(Vec3 origen, Vec3 dirUnit, Vec3 normalUnit, Vec3 puntoDelPlano) {
    float den = productoEscalar(dirUnit, normalUnit);
    if (std::fabs(den) < EPS) return {false, 0.0f};        // paralelo
    float t = productoEscalar(puntoDelPlano - origen, normalUnit) / den;
    return {t > EPS, t};
}

// ==== DEMO ================================================================
int main() {
    std::cout << "== ESFERA ==\n";
    Vec3 origen{0.0f, 0.0f, 0.0f}, dir{0.0f, 0.0f, 1.0f};
    Impacto h = rayoEsfera(origen, dir, Vec3{0.0f, 0.0f, 5.0f}, 1.0f);
    std::printf("esfera en (0,0,5) radio 1: toca=%d t=%.3f (t = 5 - 1 = 4)\n",
                (int)h.toca, h.t);
    Impacto h2 = rayoEsfera(origen, dir, Vec3{0.0f, 0.0f, 5.0f}, 10.0f);
    std::printf("esfera radio 10 (origen adentro): toca=%d t=%.3f "
                "(sale por z=15)\n", (int)h2.toca, h2.t);
    Impacto h3 = rayoEsfera(origen, dir, Vec3{10.0f, 0.0f, 0.0f}, 1.0f);
    std::printf("esfera lejos (a un costado): toca=%d ( esperado 0 )\n",
                (int)h3.toca);

    std::cout << "\n== AABB (slab) ==\n";
    ImpactoCaja c = rayoAABB(origen, dir, Vec3{-1.0f, -1.0f, 4.0f},
                                         Vec3{ 1.0f,  1.0f, 6.0f});
    std::printf("caja z:[4..6] dir +z: toca=%d entra=%.1f sale=%.1f\n",
                (int)c.toca, c.tEntrada, c.tSalida);
    ImpactoCaja c2 = rayoAABB(origen, dir, Vec3{-1.0f, -1.0f, -9.0f},
                                          Vec3{ 1.0f,  1.0f, -6.0f});
    std::printf("caja DETRÁS del rayo (z:[-9..-6], dir +z): toca=%d\n",
                (int)c2.toca);

    std::cout << "\n== PLANO ==\n";
    Impacto p = rayoPlano(origen, dir, Vec3{0.0f, 0.0f, -1.0f},
                          Vec3{0.0f, 0.0f, 3.0f});
    std::printf("plano z=3, rayo +z: toca=%d t=%.1f\n", (int)p.toca, p.t);
    Impacto p2 = rayoPlano(origen, dir, Vec3{0.0f, -1.0f, 0.0f},
                           Vec3{0.0f, 3.0f, 0.0f});
    std::printf("plano y=3 (paralelo): toca=%d\n", (int)p2.toca);

    std::cout << "=== FIN LECCIÓN 12 ===\n";
    return 0;
}
