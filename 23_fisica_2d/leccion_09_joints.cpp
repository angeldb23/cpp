/* =============================================================================
   MÓDULO 23 - LECCIÓN 09: Constraints y joints
   =============================================================================
   TEMA 6 de TEMAS.md: "Constraints/joints: distance, revolute, prismatic,
   weld y motor".

   Un joint (articulación) es una restricción entre dos cuerpos. En vez de
   resolver choques, el solver resuelve ecuaciones del tipo C(x) = 0:
     - DISTANCE: |pB - pA| = longitud. Una cuerda, una cadena, un resorte.
     - REVOLUTE: un punto de A coincide con un punto de B; permite rotar.
       Una rueda, una puerta con bisagra.
     - PRISMATIC: como revolute pero solo permite deslizar por un eje (sin
       rotación). Un pistón, una plataforma que sube y baja.
     - WELD: fija posición y rotación relativa. Dos partes de un mismo objeto.
     - MOTOR: no es un joint solo; es un actuador que se agrega a revolute o
       prismatic y aplica un torque/fuerza hasta cierta velocidad objetivo.

   IMPULSO DE DISTANCIA
   --------------------
   Error: C = |pB - pA| - restLength
   Dirección: n = normalize(pB - pA)
   Impulso escalar: j = -C / (invMassA + invMassB)
   A recibe +j*n y B recibe -j*n (si C > 0 están lejos, se atraen).
   Con bias de Baumgarte se usa j = -(C + beta*C/dt) / denom, pero acá
   mostramos la versión sin bias para que se vea la geometría.

   LÍMITES Y MOTORES
   -----------------
   Revolute/prismatic pueden tener lower/upper limits. Si el ángulo/desplaza-
   miento se pasa del límite, se resuelve como un contacto: impulso en un solo
   sentido. El motor aplica un impulso angular/fuerza acotado por maxTorque
   (o maxForce) y dt:
       L = clamp((ω_objetivo - ω_actual) / invInertia, -maxTorque*dt, +...)
   Sin el clamp, el motor teletransporta la velocidad y se siente antinatural.

   OJO CON LOS ANCHORS: los anchors locales se guardan en el espacio LOCAL de
   cada cuerpo y se transforman a world cada frame. Si se guardan en world, el
   joint se "rompe" apenas el cuerpo rota.
   ============================================================================= */
#include "include/fisica2d.hpp"

#include <iomanip>
#include <iostream>

using f2d::Vec2;

namespace {

struct EntradaDistancia {
    Vec2 posA;
    Vec2 posB;
    Vec2 velA;
    Vec2 velB;
    float invMassA = 1.0f;
    float invMassB = 1.0f;
    float longitudReposo = 1.0f;
};

struct SalidaDistancia {
    Vec2 deltaA;
    Vec2 deltaB;
    float error = 0.0f;
};

SalidaDistancia resolverDistancia(const EntradaDistancia& e) {
    const Vec2 d = e.posB - e.posA;
    const float largo = f2d::length(d);
    const float error = largo - e.longitudReposo;

    if (largo <= f2d::kEps) return {{}, {}, error};

    const Vec2 n = d / largo;
    const float denom = e.invMassA + e.invMassB;
    if (denom <= f2d::kEps) return {{}, {}, error};

    const float j = error / denom;
    return {n * (j * e.invMassA), -n * (j * e.invMassB), error};
}

// Impulso angular L (torque*dt) para llevar ω a ω_objetivo sin pasarse.
float motorAngular(float omegaActual, float omegaObjetivo, float maxTorque,
                   float invInertia, float dt) {
    if (invInertia <= f2d::kEps) return 0.0f;
    const float deseado = (omegaObjetivo - omegaActual) / invInertia;
    const float limite = maxTorque * dt;
    return f2d::clampf(deseado, -limite, limite);
}

float limitarPrismatico(float q, float minimo, float maximo) {
    return f2d::clampf(q, minimo, maximo);
}

} // namespace

int main() {
    std::cout << std::fixed << std::setprecision(4);

    // Cuerda estirada: A en (0,0), B en (2,0), longitud de reposo 1.
    const auto estirada = resolverDistancia({{0.0f, 0.0f}, {2.0f, 0.0f}, {}, {}, 1.0f, 1.0f, 1.0f});
    std::cout << "Cuerda estirada: error=" << estirada.error
              << " deltaA=(" << estirada.deltaA.x << ", " << estirada.deltaA.y
              << ") deltaB=(" << estirada.deltaB.x << ", " << estirada.deltaB.y << ")\n";

    // Cuerda comprimida: empuja para separar.
    const auto comprimida = resolverDistancia({{0.0f, 0.0f}, {0.5f, 0.0f}, {}, {}, 1.0f, 1.0f, 1.0f});
    std::cout << "Cuerda comprimida: error=" << comprimida.error
              << " deltaA.x=" << comprimida.deltaA.x
              << " deltaB.x=" << comprimida.deltaB.x << "\n";

    // Un cuerpo estático: solo se mueve el dinámico.
    const auto estatico = resolverDistancia({{0.0f, 0.0f}, {2.0f, 0.0f}, {}, {}, 0.0f, 1.0f, 1.0f});
    std::cout << "A estático: deltaA.x=" << estatico.deltaA.x
              << " deltaB.x=" << estatico.deltaB.x << "\n";

    // Motor de una rueda: ω va de 0 a 10, maxTorque 100, I=2 (invI=0.5).
    const float L1 = motorAngular(0.0f, 10.0f, 100.0f, 0.5f, 0.1f);
    const float L2 = motorAngular(0.0f, 10.0f, 1000.0f, 0.5f, 0.1f);
    std::cout << "Motor ω*10, τmax=100: L=" << L1
              << " (clampeado a 10) | τmax=1000: L=" << L2
              << " (sin clamp, 20)\n";

    // Límite prismático: un pistón entre -2 y 3.
    std::cout << "Prismático q=5 -> " << limitarPrismatico(5.0f, -2.0f, 3.0f)
              << " | q=-5 -> " << limitarPrismatico(-5.0f, -2.0f, 3.0f) << "\n";

    std::cout << "\nWeld se implementa resolviendo a la vez la restricción de\n"
                 "posición y la de ángulo. Revolute es un punto compartido;\n"
                 "prismatic es un punto compartido más un eje bloqueado.\n";
    return 0;
}
