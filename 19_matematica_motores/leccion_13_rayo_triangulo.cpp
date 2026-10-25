/* =============================================================================
   MÓDULO 19 - LECCIÓN 13: Intersecciones II — rayo vs triángulo (Möller-Trumbore)
   =============================================================================
   TEMA 7 de TEMAS.md (2da lección): ray-triángulo con Möller-Trumbore y
   coordenadas baricéntricas.

   == POR QUÉ ESTE ALGORITMO =================================================

   Toda malla (un OBJ del módulo 20, un tile del módulo 21, un enemigo del
   módulo 27) es una colección de TRIÁNGULOS. El ray-triangle es el building
   del raycast "contra una escena": probás contra cada triángulo (o primero
   contra AABBs que los contienen — broad phase del módulo 17) y te quedás
   con el menor t.

   Möller-Trumbore (1997) es LA implementación estándar: no calcula el plano
   aparte ni el punto de impacto para validar: sale con u y v listos para
   interpolar (luces, UVs, colores del shader de vertex).

   == EL ALGORITMO =============================================================

   Dado el rayo (O, D normalizado) y el triángulo (p0, p1, p2):

       e1 = p1 - p0                    (primer lado)
       e2 = p2 - p0                    (segundo lado)
       h  = cross(D, e2)               (el "helper" del determinante)
       a  = dot(e1, h)                 (detección de paralelo: a == 0)
       si |a| < EPS: no toca (rayo paralelo al plano del triángulo)

       f = 1 / a
       s  = O - p0
       u  = f * dot(s, h)
       si u < 0 || u > 1: no toca (fuera por el lado e1)

       q  = cross(s, e1)
       v  = f * dot(D, q)
       si v < 0 || u + v > 1: no toca (fuera por e2 / suma)

       t = f * dot(e2, q)
       si t > EPS: toca; (u, v) son las coordenadas baricéntricas
       (sobre p0 ⊕): punto = p0 + u*e1 + v*e2
     sino: el triángulo está detrás del rayo.

   ¿POR QUÉ u <= 1 y u+v <= 1? Son las coordenadas baricéntricas:
       punto = p0 + u*(p1-p0) + v*(p2-p0)
   y el punto DENTRO del triángulo cuando u >= 0, v >= 0 y u+v <= 1.
   (u=0,v=0 → p0; u=1,v=0 → p1; u=0,v=1 → p2.)

   == BARYCÉNTRICAS: para interpolar ==========================================

   Los pesos (w0, w1, w2) con w0+w1+w2=1: w0=1-(u+v), w1=u, w2=v. Sirve para
   mezclar NORMALS/UVs/COLOROS de los 3 vértices (mod 20/21 lo usan en el
   vertex shader la interpolación la hace GLSL, acá la hacemos a pata).

   == COMO PROBAR ESTO ========================================================

   g++ -std=c++20 -Wall -Wextra leccion_13_... -o leccion13 && ./leccion13
   (Demo: triángulo visto "de frente", paralelo, con centro y esquinas.)
   ============================================================================= */

#include "include/mate3d.hpp"
#include <cmath>
#include <cstdio>
#include <iostream>

using namespace mate;

constexpr float EPS = 1e-8f;

float productoEscalar(Vec3 a, Vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
struct ImpactoTri { bool toca; float t; float u; float v; };

// El algoritmo (los ejercicios lo IMPLEMENTAN).
ImpactoTri rayoTrianguloMT(Vec3 O, Vec3 D, Vec3 p0, Vec3 p1, Vec3 p2) {
    ImpactoTri out{false, 0.0f, 0.0f, 0.0f};
    Vec3 e1 = p1 - p0;
    Vec3 e2 = p2 - p0;

    // v = cross(D, e2)
    Vec3 h = { D.y*e2.z - D.z*e2.y,
               D.z*e2.x - D.x*e2.z,
               D.x*e2.y - D.y*e2.x };
    float a = productoEscalar(e1, h);
    if (std::fabs(a) < EPS) return out;                    // paralelo / sin área

    float f = 1.0f / a;
    Vec3 s = O - p0;
    float u = f * productoEscalar(s, h);
    if (u < 0.0f || u > 1.0f) return out;                  // fuera por e1...

    // q = cross(s, e1)
    Vec3 q = { s.y*e1.z - s.z*e1.y,
               s.z*e1.x - s.x*e1.z,
               s.x*e1.y - s.y*e1.x };
    float v = f * productoEscalar(D, q);
    if (v < 0.0f || u + v > 1.0f) return out;              // fuera por e2/suma

    float t = f * productoEscalar(e2, q);
    if (t > EPS) { out.toca = true; out.t = t; out.u = u; out.v = v; }
    return out;                                            // detrás del rayo si no
}

// ==== DEMO ================================================================
int main() {
    // Triángulo en el plano z=0, "de cara" a un rayo que va en +z
    Vec3 p0{0.0f, 0.0f, 0.0f}, p1{2.0f, 0.0f, 0.0f}, p2{0.0f, 2.0f, 0.0f};
    Vec3 O{0.5f, 0.5f, -5.0f}, D{0.0f, 0.0f, 1.0f};

    ImpactoTri h = rayoTrianguloMT(O, D, p0, p1, p2);
    std::printf("(0.5,0.5) centro del triángulo: toca=%d t=%.3f u=%.3f v=%.3f\n",
                (int)h.toca, h.t, h.u, h.v);
    // u y v deberían quedar 0.25/0.25 (los pesos del punto (0.5,0.5))

    ImpactoTri h2 = rayoTrianguloMT(Vec3{1.9f, 1.9f, -5.0f}, D, p0, p1, p2);
    std::printf("(1.9,1.9) FUERA (u+v>1): toca=%d\n", (int)h2.toca);

    ImpactoTri h3 = rayoTrianguloMT(O, Vec3{1.0f, 0.0f, 0.0f}, p0, p1, p2);
    std::printf("rayo PARALELO al plano (dirección x en un tri plano xy): "
                "toca=%d\n", (int)h3.toca);

    ImpactoTri h4 = rayoTrianguloMT(Vec3{0.5f, 0.5f, 5.0f}, D, p0, p1, p2);
    std::printf("triángulo DETRÁS del rayo: toca=%d\n", (int)h4.toca);

    ImpactoTri h5 = rayoTrianguloMT(Vec3{2.0f, 0.0f, -5.0f}, D, p0, p1, p2);
    std::printf("punto (2,0) en la ESQUINA/vertice p1: toca=%d u=%.3f v=%.3f\n",
                (int)h5.toca, h5.u, h5.v);

    std::cout << "\n== BARICÉNTRICAS PARA INTERPOLAR ==\n";
    // El punto de impacto del primer test era (0.5, 0.5, 0): el mix de los
    // pesos daría las tres coordenadas: w0+w1+w2 = 1 exacto.
    if (h.toca) {
        float w0 = 1.0f - (h.u + h.v), w1 = h.u, w2 = h.v;
        std::printf("pesos: w0=%.3f w1=%.3f w2=%.3f (suman %.3f)\n",
                    w0, w1, w2, w0 + w1 + w2);
        // reconstruir el punto = p0*w0 + p1*w1 + p2*w2
        Vec3 mix = p0*w0 + p1*w1 + p2*w2;
        std::printf("punto reconstruido: (%.3f, %.3f, %.3f) — el (0.5,0.5,0)\n",
                    mix.x, mix.y, mix.z);
    }

    std::cout << "=== FIN LECCIÓN 13 ===\n";
    return 0;
}
