/* =============================================================================
   MÓDULO 19 - LECCIÓN 04: Vectores II — cross, proyección y reflexión
   =============================================================================
   TEMA 2 de TEMAS.md (2da lección): dot/cross, proyección, reflexión y
   distancia a un plano.

   == PRODUCTO VECTORIAL (cross) =============================================

   cross(a, b) = ( a.y*b.z - a.z*b.y,
                   a.z*b.x - a.x*b.z,
                   a.x*b.y - a.y*b.x )

   Resultado:
      - Un vector PERPENDICULAR a los dos operandos (la NORMAL que los
        generaría).
      - Su LARGO es el área del paralelogramo que forman a y b.
      - La dirección está fijada por right-hand rule: cross(+x, +y) = +z.

   USOS del curso:
      - Invecs: MIPS on curves: normal de un triángulo/piso:
          n = Jnormal normalizar(cross(p2 - p1, p3 - p1))
        (el orden de (p2-p1), (p3-p1) define hacia dónde apunta: si el
        polygon "winds counter-clockwise"... regla right-hand).
      - Mirar el RIGHT de una cámara: cross(dirección, up).
      - En 2D: el "cross" no eran tres componentes pero el SIGN del z
        (a.x*b.y - a.y*b.x) define si b queda b izquierda/derecha de a:
        es el "determinante 2x2" (cross 2D).

   == PROYECCIÓN: aplanar un vector sobre otro ===============================

   ¿Qué parte de v va pasa "a lo largo" de la dirección n̂? La PROYECCIÓN:

     proyectar(v, n̂) = n̂ * dot(v, n̂)          (n̂ normalizado: divide una vez)

   Error típico: olvidar ROJO normalizar n y aplicar v * dot(v, n) scalno.
   Si n NO es unitario, la versión general es:

     proyectar(v, n) = n * ( dot(v, n) / dot(n, n) )

   (dot(n, n) == largo(n) al cuadrado.) La componente PERPENDICULAR (la
   "rección"/segundoábio, rejección) es:

     perpendicular(v, n) = v - proyectar(v, n)

   También: v = proyección + perpendicular — dos partes todas de chunk.

   == REFLEXIÓN: botar contra un espejo/plano ================================

   El vector reflejado: mismo ángulo de entrada y de salida, cuidando la
   normal del plano (੫ dirección hacia dónde el vector LLEGA):

     reflejar(v, n̂) = v - 2 * dot(v, n̂) * n̂

   La normal apunta del lado POR DONDE VENÍA el rayo (como el suelo:
   n̂ = (0, 1, 0) si el vector viene de arriba hacia abajo).

   == DEMO: una véhiculo gol y bola rebotan ==================================

   La demo simula una bola que caue sobre un plano inclinado y rebota con
   la normal del plano, un "reflejo de cámara" con proyectada y más.
   ============================================================================= */

#include "include/mate3d.hpp"
#include <cstdio>
#include <cmath>
#include <iostream>

using namespace mate;

// ==== FUNCIONES DE LA LECCIÓN (los ejercicios las implementan) =============
float productoEscalar(Vec3 a, Vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
Vec3 productoVectorial(Vec3 a, Vec3 b) {
    return { a.y*b.z - a.z*b.y,
             a.z*b.x - a.x*b.z,
             a.x*b.y - a.y*b.x };
}
float largoCuadrado(Vec3 a) { return productoEscalar(a, a); }
float largo(Vec3 a) { return std::sqrt(largoCuadrado(a)); }
Vec3 normalizar(Vec3 a) {
    float l = largo(a);
    if (l <= 1e-8f) return {0.0f, 0.0f, 0.0f};
    return a / l;
}

// PROYECCIÓN (n DEBE ser normalizado; versión general en el ejercicio 02)
Vec3 proyectar(Vec3 v, Vec3 n) { return n * productoEscalar(v, n); }

// REJECCIÓN: la parte de v que NO va en la dirección de n
Vec3 perpendicular(Vec3 v, Vec3 n) { return v - proyectar(v, n); }

// REFLEXIÓN sobre un plano con normal n (debe venir normalizada)
Vec3 reflejar(Vec3 v, Vec3 n) { return v - n * (2.0f * productoEscalar(v, n)); }

// ==== DEMO ================================================================
static void mostrar(Vec3 const& v, const char* nombre) {
    std::printf("%-22s (%6.3f, %6.3f, %6.3f)\n", nombre, v.x, v.y, v.z);
}

int main() {
    std::cout << "== NORMAL DE UN TRIÁNGULO (cross) ==\n";
    // El piso es un triángulo apoyado en el eje X y en el eje Z
    Vec3 p1{ 0.0f, 0.0f, 0.0f}, p2{1.0f, 0.0f, 0.0f}, p3{1.0f, 0.0f, 1.0f};
    Vec3 lado1 = p2 - p1, lado2 = p3 - p1;
    Vec3 normalTriangulo = normalizar(productoVectorial(lado1, lado2));
    mostrar(normalTriangulo, "normal del triángulo");
    std::cout << " (= (0, 1, 0) — arriba; el ORDEN de los lados la define)\n";

    std::cout << "\n== PROYECCIÓN: pendiente ==\n";
    // Bola que rueda por una rampa de 30° en el eje X-Y: pendiente n=(0,1,0)
    // proyección sobre (0, 1, 0) de la velocidad (-1, 1, 0)
    Vec3 vel{-1.0f, 1.0f, 0.0f};
    Vec3 n{0.0f, 1.0f, 0.0f};
    mostrar(proyectar(vel, n), "la proyección (dot·n)");
    mostrar(perpendicular(vel, n), "el rechazo");
    std::cout << "vel = proyección + rechazo (APARTE PARTES UNA VEZ MÁS)\n";

    std::cout << "\n== REFLEXIÓN: bote ==\n";
    Vec3 bola = {-1.0f, -1.0f, 0.0f};  // viene hacia abajo-izquierda
    Vec3 piso = {0.0f, 1.0f, 0.0f};    // normal del piso (arriba)
    mostrar(reflejar(bola, piso), "la dirección reflejada");
    std::cout << " (1, -1) invertido horizontal: (1, 1, 0) hacia arriba-der\n";

    std::cout << "\n== PROYECCIÓN de dot > 0 ==\n";
    Vec3 v2 = {1.0f, 2.0f, 1.0f};
    Vec3 n2 = normalizar(Vec3{1.0f, 1.0f, 0.0f});
    std::cout << "área de proyección sobre n: dot = " << productoEscalar(v2, n2)
              << " — o sea punto a distancia |v|·cos(θ) sobre n̂\n";

    std::cout << "=== FIN LECCIÓN 04 ===\n";
    return 0;
}
