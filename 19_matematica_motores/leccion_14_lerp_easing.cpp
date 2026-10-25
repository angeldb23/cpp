/* =============================================================================
   MÓDULO 19 - LECCIÓN 14: Interpolación I — lerp y funciones de easing
   =============================================================================
   TEMA 8 de TEMAS.md (1ra lección): lerp (y por qué las CÁMARAS "brincan" cuando
   se interpola sin cuidado), suavizado de t (easing), y las notas que ligan los
   "paths" con los movimientos (cámara del 17, timesteps del 16).

   == LERP: mezclar DOS VALORES ==============================================

   La operación más usada de TODO el render:

       lerp(a, b, t) = a + t * (b - a)      con t en [0, 1]

   Resumen: t=0 → a; t=1 → b; t=0.5 → "el medio". Con vectores: LERP por
   componente (el punto viaja sobre la RECTA correcta).

   Los movimientos de cámara del engine del curso (módulo 17, el
   "lerp follow") dependen de esto: cada frame, con su dt:

       posCam = lerp(posCam, objetivo, k)   — el k depende del dt (abajo)

   == PERO CUIDADO (el error de novato) ======================================

   NO uses un k fijo por FRAME (0.1 por frame): si el timestep (módulo 16)
   varía, la cámara cambia de velocidad. El modelo CORRECTO (exponencial):

       pos = lerp(pos, objetivo, 1 - exp(-suavizado * dt))

   que da el mismo comportamiento incluso si dt cambia (frames perdidos).

   == EASING: ¿cómo "avanza" t? ==============================================

   lerp con t LINEAL se ve robótica. Se le da "carácter" con una función
   ease(t) que curva la progresión:

     smoothstep(t)   = t*t*(3 - 2*t)          (arranca y fin suaves, C¹)
     quintic(t)      = 6t⁵-15t⁴+10t³          (C²: el estándar de noise — L16)
     easeInOutCubic: t < 0.5? 4t³ : 1-((-2t+2)³)/2       (acelerar y frenar)
     clamp01(t)      = min(1, max(0, t))      (SIEMPRE antes de los easing)

   Aclaración: el lerp con t FUERA de [0..1] EXTRAPOLA: con t=1.5 va
   "más allá de b". Útil para overshoot (con cuidado).

   == LA DEMO ==============================================================

   La demo corre una simple "cámara que sigue" con/sin easing para VER
   cómo se siente la diferencia; imprime una tabla (los tests del curso NO abren ventanas).
   ============================================================================= */

#include "include/mate3d.hpp"
#include <cmath>
#include <cstdio>
#include <iostream>
#include <vector>

using namespace mate;

// ==== FUNCIONES DE LA LECCIÓN ==============================================
float lerpEscalar(float a, float b, float t) {
    return a + t * (b - a);
}
Vec3 lerpVec(Vec3 a, Vec3 b, float t) {
    return a + (b - a) * t;
}
float clamp01(float t) { return std::fmin(1.0f, std::fmax(0.0f, t)); }
float smoothstep(float t) {
    t = clamp01(t);
    return t * t * (3.0f - 2.0f * t);
}
float quintica(float t) {
    t = clamp01(t);
    return t*t*t*(t*(t*6.0f - 15.0f) + 10.0f);   // 6t⁵ − 15t⁴ + 10t³
}
float easeInOutCubica(float t) {
    t = clamp01(t);
    return (t < 0.5f) ? 4.0f*t*t*t : 1.0f - std::pow(-2.0f*t + 2.0f, 3)/2.0f;
}

// Cámara simple "de seguimiento" con paso exponencial, independiente de dt:
Vec3 seguirCamara(Vec3 posCam, Vec3 objetivo, float suavizado, float dt) {
    float k = 1.0f - std::exp(-suavizado * dt);
    return lerpVec(posCam, objetivo, k);
}

// ==== DEMO ================================================================
int main() {
    std::cout << "== LERP BASICO (y sus extremos) ==\n";
    std::printf("lerp(0, 10, 0)=%.1f  lerp(0, 10, 0.5)=%.1f  lerp(0, 10, 1)=%.1f\n",
                lerpEscalar(0,10,0), lerpEscalar(0,10,0.5f), lerpEscalar(0,10,1));
    Vec3 a{0,0,0}, b{10,20,30};
    Vec3 medio = lerpVec(a, b, 0.5f);
    std::printf("lerpVec((0,0,0),(10,20,30), 0.5) = (%.1f, %.1f, %.1f)\n",
                medio.x, medio.y, medio.z);

    std::cout << "\n== EASING: t vs curva (tabla) ==\n";
    std::cout << "  t    | lineal smoothstep quintica easeInOutCubica\n";
    for (int i = 0; i <= 5; ++i) {
        float t = i / 5.0f;
        std::printf(" %.2f  | %6.3f  %9.3f %7.3f %14.3f\n", t,
                    t, smoothstep(t), quintica(t), easeInOutCubica(t));
    }

    std::cout << "\n== SEGUIMIENTO: EXPONENCIAL (estable con dt) ==\n";
    // objetivo fijo. La cámara arranca lejos y "se acerca" suavizado=3:
    Vec3 posCam{50.0f, 50.0f, 0.0f};
    Vec3 objetivo{0.0f, 0.0f, 0.0f};
    float dtFijo = 1.0f/60.0f;
    for (int frame = 0; frame < 6; ++frame) {
        posCam = seguirCamara(posCam, objetivo, 3.0f, dtFijo);
        std::printf("frame %d: cámara = (%7.3f, %7.3f)\n",
                    frame, posCam.x, posCam.y);
    }
    std::cout << "(en los módulos 16/17 la cámara usa el lerp follow con\n"
                 "coeficiente 1 - exp(-k*dt): estable aunque dt varíe)\n";

    std::cout << "=== FIN LECCIÓN 14 ===\n";
    return 0;
}
