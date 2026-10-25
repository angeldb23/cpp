/* =============================================================================
   MÓDULO 23 - LECCIÓN 08: Cuerpos: tipo, masa, inercia y damping
   =============================================================================
   TEMA 5 de TEMAS.md: "Cuerpos: estático, cinemático, dinámico; masa,
   inercia, centro de masa y damping".

   TIPOS DE CUERPO
   ---------------
   - STATIC (estático): masa infinita, no se mueve nunca. Paredes, piso.
     invMass = 0 e invInertia = 0.
   - KINEMATIC (cinemático): se mueve con velocidad propia, pero las fuerzas
     y los choques NO lo mueven. Plataformas móviles, puertas automáticas.
     invMass = 0, pero position += velocity * dt.
   - DYNAMIC (dinámico): lo mueven las fuerzas y los impulsos.
     invMass = 1/masa, invInertia = 1/inercia.

   Por qué se guarda la INVERSA: el solver de impulsos divide por la masa todo
   el tiempo. Guardar `invMass = 0` para los estáticos evita el `if (esStatic)`
   en cada operación: multiplicar por 0 ya los deja quietos. Es el mismo truco
   que usa Box2D (`b2Body::GetInverseMass`).

   MASA E INERCIA
   --------------
   - Caja de ancho w y alto h: I = m*(w² + h²)/12 (alrededor del centro).
   - Círculo de radio r: I = 0.5*m*r².
   - La inercia es el "análogo rotacional de la masa": cuánto cuesta cambiar
     ω. Un cuerpo con mucha inercia gira poco ante el mismo torque.
   Para formas compuestas se usa el teorema de ejes paralelos:
       I = I_cm + m*d²
   El centro de masa (COM) es el promedio pesado de las partes. En un motor
   simple se asume que el origen del cuerpo es el COM; si no, hay que guardar
   el offset y rotar alrededor de él.

   INTEGRACIÓN (semi-implícita de Euler)
   -------------------------------------
       v += (F/m) * dt          <- primero la velocidad
       x += v * dt              <- después la posición con la v NUEVA
   Es estable para juegos y es lo que usa Box2D. Euler explícito (usar la v
   vieja para x) se va de rango con resortes.

   DAMPING
   -------
   Sin damping, un cuerpo en el espacio se mueve para siempre. El damping
   lineal/angular simula aire/rozamiento:
       v *= 1 / (1 + damping * dt)
   Es una aproximación barata de la exponencial e^(-damping*dt) y es estable
   para cualquier dt (no puede dar negativo).
   ============================================================================= */
#include "include/fisica2d.hpp"

#include <iomanip>
#include <iostream>

using f2d::Vec2;

namespace {

enum class TipoCuerpo { Static, Kinematic, Dynamic };

struct DefinicionCuerpo {
    TipoCuerpo tipo = TipoCuerpo::Dynamic;
    float masa = 1.0f;
    float inercia = 1.0f;
    Vec2 velocidad;
    float velocidadAngular = 0.0f;
    float dampingLineal = 0.0f;
    float dampingAngular = 0.0f;
};

struct Cuerpo {
    TipoCuerpo tipo = TipoCuerpo::Dynamic;
    Vec2 pos;
    Vec2 vel;
    float angulo = 0.0f;
    float angVel = 0.0f;
    float invMass = 0.0f;
    float invInertia = 0.0f;
    float dampingLineal = 0.0f;
    float dampingAngular = 0.0f;
};

float inerciaCaja(float masa, float ancho, float alto) {
    return masa * (ancho * ancho + alto * alto) / 12.0f;
}

float inerciaCirculo(float masa, float radio) {
    return 0.5f * masa * radio * radio;
}

Cuerpo crearCuerpo(const DefinicionCuerpo& def) {
    Cuerpo c;
    c.tipo = def.tipo;
    c.vel = def.velocidad;
    c.angVel = def.velocidadAngular;
    c.dampingLineal = def.dampingLineal;
    c.dampingAngular = def.dampingAngular;

    if (def.tipo == TipoCuerpo::Dynamic) {
        if (def.masa > 0.0f) c.invMass = 1.0f / def.masa;
        if (def.inercia > 0.0f) c.invInertia = 1.0f / def.inercia;
    }
    // Static y Kinematic: invMass e invInertia quedan en 0.
    return c;
}

void integrar(Cuerpo& c, Vec2 fuerza, float torque, float dt) {
    if (c.tipo == TipoCuerpo::Static) return;

    if (c.tipo == TipoCuerpo::Dynamic) {
        c.vel += fuerza * (c.invMass * dt);
        c.angVel += torque * c.invInertia * dt;
    }

    // Damping estable para cualquier dt.
    c.vel *= 1.0f / (1.0f + c.dampingLineal * dt);
    c.angVel *= 1.0f / (1.0f + c.dampingAngular * dt);

    c.pos += c.vel * dt;
    c.angulo += c.angVel * dt;
}

} // namespace

int main() {
    std::cout << std::fixed << std::setprecision(4);

    const float Icaja = inerciaCaja(2.0f, 4.0f, 2.0f);
    const float Icirculo = inerciaCirculo(2.0f, 3.0f);
    std::cout << "Inercia caja 2x(4x2): " << Icaja
              << " kg·m² | círculo m=2 r=3: " << Icirculo << " kg·m²\n\n";

    Cuerpo caja = crearCuerpo({TipoCuerpo::Dynamic, 2.0f, Icaja, {0.0f, 0.0f}, 0.0f, 0.0f, 0.0f});
    Cuerpo estatico = crearCuerpo({TipoCuerpo::Static, 0.0f, 0.0f, {0.0f, 0.0f}, 0.0f, 0.0f, 0.0f});
    Cuerpo cinematico = crearCuerpo({TipoCuerpo::Kinematic, 0.0f, 0.0f, {2.0f, 0.0f}, 0.0f, 0.0f, 0.0f});

    std::cout << "invMass: dinámico=" << caja.invMass
              << " estático=" << estatico.invMass
              << " cinemático=" << cinematico.invMass << "\n";

    // Gravedad sobre la caja: F = m*g = 2 * (-9.8) = -19.6 N.
    constexpr float dt = 0.1f;
    for (int i = 0; i < 5; ++i) {
        integrar(caja, {0.0f, -19.6f}, 0.0f, dt);
    }
    std::cout << "Caja tras 0.5 s de gravedad: y=" << caja.pos.y
              << " vy=" << caja.vel.y << "\n";

    // Un estático ignora fuerza y velocidad; un cinemático se mueve pero
    // ignora la fuerza.
    integrar(estatico, {1000.0f, 0.0f}, 0.0f, dt);
    integrar(cinematico, {1000.0f, 0.0f}, 0.0f, dt);
    std::cout << "Estático x=" << estatico.pos.x
              << " | cinemático x=" << cinematico.pos.x
              << " (la fuerza no lo mueve; sí su velocidad)\n";

    // Damping: con 1.0 y dt=1, la velocidad se reduce a la mitad.
    Cuerpo conDamping = crearCuerpo({TipoCuerpo::Dynamic, 1.0f, 1.0f, {10.0f, 0.0f}, 0.0f, 1.0f, 0.0f});
    integrar(conDamping, {0.0f, 0.0f}, 0.0f, 1.0f);
    std::cout << "Damping lineal 1.0 tras 1 s: vx=" << conDamping.vel.x
              << " (aprox e^-1 = 0.3679)\n";

    // Torque: un impulso angular cambia ω según invInertia.
    Cuerpo girador = crearCuerpo({TipoCuerpo::Dynamic, 2.0f, Icaja, {0.0f, 0.0f}, 0.0f, 0.0f, 0.0f});
    integrar(girador, {0.0f, 0.0f}, 10.0f, 0.1f);
    std::cout << "Torque 10 N·m durante 0.1 s -> ω=" << girador.angVel
              << " rad/s (Δω = τ*dt/I)\n";
    return 0;
}
