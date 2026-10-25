/* =============================================================================
   MÓDULO 23 - LECCIÓN 07: Baumgarte, slop y warm starting
   =============================================================================
   TEMA 4 de TEMAS.md (segunda lección): corrección de posición y warm-starting.

   EL PROBLEMA DEL IMPULSO PURO
   ----------------------------
   El impulso de la lección 06 cambia velocidades, pero si dos cuerpos ya
   están penetrados, la posición sigue metida. Al frame siguiente vuelven a
   penetrarse y el solver tiene que empujar de nuevo: la pila "se hunde" y
   vibra. Hacen falta dos cosas:
     1. Corrección de posición (Baumgarte) o "split impulse".
     2. Acumular impulsos entre frames (warm starting) para que el solver
        arranque cerca de la solución.

   BAUMGARTE
   ---------
   Se agrega un sesgo (bias) a la velocidad objetivo proporcional a la
   penetración:
       bias = (percent / dt) * max(penetración - slop, 0)
   - `percent` (~0.2) corrige solo una fracción por paso: corregir el 100%
     de golpe se ve como un "pop" y puede inyectar energía.
   - `slop` (~0.01 m) es la penetración tolerada: evita corregir ruido
     numérico todo el tiempo. Los cuerpos pueden quedar un poquito metidos.
   - Se aplica como impulso de posición, no cambiando la velocidad real (si
     no, la energía aparece de la nada). Box2D usa "position solver" separado;
     otros motores usan split impulse.

   Corrección directa de posición (más simple de entender):
       corrección = max(penetración - slop, 0) * percent
       A se mueve -normal * corrección * invMassA / (invMassA + invMassB)
       B se mueve +normal * corrección * invMassB / (invMassA + invMassB)
   Un cuerpo estático (invMass = 0) no se mueve: todo lo absorbe el dinámico.

   WARM STARTING
   -------------
   El solver de impulsos acumula `j` por contacto. En el frame siguiente, en
   vez de arrancar en 0, se aplica el impulso acumulado del frame anterior y
   se resuelve el delta. Eso acelera la convergencia de pilas y cuerdas.
   El acumulado se clampea a [0, max] (o [-max, max] para fricción) para no
   explotar si el contacto cambia de dirección.
   ============================================================================= */
#include "include/fisica2d.hpp"

#include <iomanip>
#include <iostream>
#include <vector>

using f2d::Vec2;

namespace {

struct Cuerpo {
    Vec2 pos;
    float invMass = 1.0f;
};

struct Correccion {
    Vec2 a;
    Vec2 b;
};

Correccion corregirPosicion(const Vec2& normal, float penetracion,
                            float invMassA, float invMassB,
                            float percent = 0.2f, float slop = 0.01f) {
    const float suma = invMassA + invMassB;
    if (suma <= f2d::kEps || penetracion <= slop) return {};
    const float magnitud = (penetracion - slop) * percent / suma;
    const Vec2 n = f2d::normalize(normal);
    return {-n * (magnitud * invMassA), n * (magnitud * invMassB)};
}

float acumularImpulso(float anterior, float nuevo, float maximo) {
    return f2d::clampf(anterior + nuevo, 0.0f, maximo);
}

float acumularFriccion(float anterior, float nuevo, float maximo) {
    return f2d::clampf(anterior + nuevo, -maximo, maximo);
}

} // namespace

int main() {
    std::cout << std::fixed << std::setprecision(4);

    // Dos cuerpos dinámicos penetrados 10 cm. Cada iteración corrige el 20%
    // del exceso sobre el slop: converge geométricamente.
    Vec2 posA{0.0f, 0.0f};
    Vec2 posB{1.9f, 0.0f};  // cajas de 1 m de ancho: penetración 0.1
    float penetracion = 0.1f;

    std::cout << "Baumgarte con percent=0.2, slop=0.01:\n";
    for (int i = 0; i < 6; ++i) {
        const auto c = corregirPosicion({1.0f, 0.0f}, penetracion, 1.0f, 1.0f);
        posA += c.a;
        posB += c.b;
        penetracion = std::max(0.0f, penetracion - (c.b.x - c.a.x));
        std::cout << "  iter " << i + 1 << ": A.x=" << posA.x
                  << " B.x=" << posB.x << " penetración=" << penetracion << "\n";
    }

    // Con un cuerpo estático, el dinámico absorbe toda la corrección.
    const auto soloDinamico = corregirPosicion({0.0f, 1.0f}, 0.05f, 0.0f, 1.0f);
    std::cout << "\nA estático, B dinámico: corrección B.y="
              << soloDinamico.b.y << " (A no se mueve: "
              << soloDinamico.a.y << ")\n";

    // Warm starting: el impulso acumulado se suma y se clampea.
    std::cout << "\nWarm starting (acumulado + nuevo, clamp [0, max]):\n";
    std::cout << "  anterior=2, nuevo=5, max=6  -> " << acumularImpulso(2.0f, 5.0f, 6.0f) << "\n";
    std::cout << "  anterior=2, nuevo=-5, max=6 -> " << acumularImpulso(2.0f, -5.0f, 6.0f) << "\n";
    std::cout << "  fricción anterior=1, nuevo=-9, max=3 -> "
              << acumularFriccion(1.0f, -9.0f, 3.0f) << "\n";

    std::cout << "\nSi corregís el 100% de la penetración en un solo paso, una pila\n"
                 "de cajas explota. El slop + percent + warm start es lo que la\n"
                 "mantiene estable.\n";
    return 0;
}
