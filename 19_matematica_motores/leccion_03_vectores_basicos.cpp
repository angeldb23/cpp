/* =============================================================================
   MÓDULO 19 - LECCIÓN 03: Vectores I — largo, normalizar, distancia y ángulo
   =============================================================================
   TEMA 2 de TEMAS.md (1ra lección): operaciones, normalización, dot.

   En 3D un vector respresenta dos cosas distintas (¡mantener la diferencia
   mental limpia es LA costumbre base del módulo!):
     - PUNTO: "estoy en el lugar (x, y, z)".
     - DIRECCIÓN: "el vector va de un punto a otro" (además: velocidades,
       normales de superficies, colores...).

   == PRODUCTO ESCALAR (dot) ==================================================

   dot(a, b) = a.x*b.x + a.y*b.y + a.z*b.z

   Tres lecturas del MISMO número:
     1) Cuánto estira "a" en la dirección de "b": proyección MESMACA sin
         normalizar (si |b| = 1, es la cantidad estricta).
     2) Forma veloz de saber ÁNGULO: dot(â, b̂) = cos(ángulo entre a y b).
        (positivo=mismo lado, 0=perpendiculares, negativo=opuestos)
     3) Marca de ORTNOMALIDAD: dot = 0 ==> son perpendiculares. Acá nace
        el "mirar normal e périodica" del módulo: un vector normal de una
        superficie es perpendicular a las direcciones que apuntan en el plano.

   == LARGO (módulo, magnitud) ===============================================

   largo(a) = sqrt(dot(a, a))      -> la raíz sale DESPUÉS del dot.
   largoCuadrado(a) = dot(a, a)    -> no pierde precisión ni cuesta sqrt.
                                            (comparar largos: ¡nada de sqrt!)
   normalizar(a) = a / largo(a)    -> el vector UNITARIO (largo 1).

   Cuidado con normalizar el vector CERO: en el curso es un error del
   programa (assert), pero funcionES seguras devuelven (0,0,0):

     Vec3 normalizar(Vec3 a) {
         float largo = largo(a);
         if (largo <= 1e-8f) return {0,0,0};  // evitamos dividir por 0
         return a / largo;
     }

   == DISTANCIA Y ÁNGULO =====================================================

   distancia(p, q) = largo(q - p)         (mira el VECTOR, no los puntos)
   ángulo entre a y b (radianes):
     dot(anormal, bnormailzar) = cos(theta)  ->  theta = acos(cosθ)

   ¡Cuidado con acos! Si por error numérico me pasa 1.0000001, acos da NaN.
   SIEMPRE limito: cosTheta = clamp(cosTheta, -1.0f, 1.0f) antes del acos.

   == COMO USAR ==============================================================

   La demo construye sobre las funciones VACÍAS del include/mate3d.hpp
   (como las de los ejercicios). La zonas de trabajo:
     g++ -std=c++20 -Wall -Wextra -I. leccion_03_... -o leccion03 && ./leccion03
    (Nota académica: -I. es para FROM ROOT of the module; dentro del ejercicio
     ya va el compilador con los includes relativos.)
   ============================================================================= */

#include "include/mate3d.hpp"
#include <cassert>
#include <cstdio>
#include <cmath>
#include <iostream>

using namespace mate;

// ==== FUNCIONES DE LA LECCIÓN (a futuro: los ejercicios las realizan) ======
float productoEscalar(Vec3 a, Vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

float largoCuadrado(Vec3 a) { return productoEscalar(a, a); }

float largo(Vec3 a) { return std::sqrt(largoCuadrado(a)); }

Vec3 normalizar(Vec3 a) {
    float l = largo(a);
    if (l <= 1e-8f) return {0.0f, 0.0f, 0.0f};
    return a / l;
}

float distancia(Vec3 p, Vec3 q) {
    return largo(q - p);  // q - p es el VECTOR desde a p hasta a q
}

// Ángulo entre a y b, en radianes. Estas MEJOR con acos clamp (NaN cero).
float anguloEntre(Vec3 a, Vec3 b) {
    float cosTheta = productoEscalar(normalizar(a), normalizar(b));
    cosTheta = std::max(-1.0f, std::min(1.0f, cosTheta));
    return std::acos(cosTheta);
}

// ==== DEMO ================================================================
static void mostrar(Vec3 const& v, const char* nombre) {
    std::printf("%-6s (%6.3f, %6.3f, %6.3f)\n", nombre, v.x, v.y, v.z);
}

int main() {
    std::cout << "== LARGO Y NORMALIZAR ==\n";
    Vec3 a{3.0f, 4.0f, 0.0f};
    mostrar(a, "a");
    std::cout << "largoCuadrado(a) = " << largoCuadrado(a) << "  (= 25 exactly)\n";
    std::cout << "largo(a)         = " << largo(a) << "\n";
    Vec3 an = normalizar(a);
    mostrar(an, "a normalizado");
    std::cout << "largo de a_norm = " << largo(an) << "  ( Unitario!)\n";

    std::cout << "\n== DISTANCIA ==\n";
    Vec3 p1{0.0f, 0.0f, 0.0f}, p2{1.0f, 2.0f, 2.0f};
    std::cout << "distancia(p1, p2) = " << distancia(p1, p2) << "  (= 3)\n";

    std::cout << "\n== DOT Y ANGULO ==\n";
    std::cout << "dot(diag, diag) = " << productoEscalar(a, a) << " (= largo²)\n";
    std::cout << "dot(x, y)  = " << productoEscalar(V3X, V3Y) << "  (⊥:  0)\n";
    std::cout << "dot(x, -x) = " << productoEscalar(V3X, -V3X) << " (opuestos: -1)\n";
    std::cout << "ángulo(a, rotZ(90)·a)  (radianes)= " << anguloEntre(a, Vec3{-a.y, a.x, 0.0f})
              << " (" << aGrados(anguloEntre(a, Vec3{-a.y, a.x, 0.0f})) << " grados)\n";

    std::cout << "\n== OBJETIVO: vector cero ==\n";
    mostrar(normalizar(Vec3{}), "normalizar({0,0,0}) = (0,0,0), seguro");
    assert(casiIgual(largo(normalizar(Vec3{})), 0.0f));

    std::cout << "=== FIN LECCIÓN 03 ===\n";
    return 0;
}
