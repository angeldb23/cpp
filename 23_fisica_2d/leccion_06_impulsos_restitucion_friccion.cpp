/* =============================================================================
   MÓDULO 23 - LECCIÓN 06: Resolución por impulsos, restitución y fricción
   =============================================================================
   TEMA 4 de TEMAS.md: "Resolución: impulso, restitución, fricción, posición
   correction (Baumgarte) y warm-starting" (primera de dos lecciones).

   Un impulso J cambia la velocidad instantáneamente:
       Δv = J / m          (o Δv = invMass * J)
       Δω = (r × J) / I    (r = punto de contacto - centro de masa)

   VELOCIDAD RELATIVA EN EL CONTACTO
   ---------------------------------
       vA_punto = vA + ωA × rA
       vB_punto = vB + ωB × rB
       vRel = vB_punto - vA_punto
   Si dot(vRel, normal) < 0, los cuerpos se están acercando. La normal va de
   A hacia B por convención.

   IMPULSO NORMAL
   --------------
       j = -(1 + e) * dot(vRel, normal) / (invMassA + invMassB
                                        + invIA*(rA×n)² + invIB*(rB×n)²)
       A recibe -j*n; B recibe +j*n.
   `e` es la restitución: 0 = choque perfectamente inelástico (se pegan),
   1 = elástico (rebote perfecto). En la práctica e < 1 porque se pierde
   energía.

   UMBRAL DE RESTITUCIÓN
   ---------------------
   Si la velocidad de aproximación es muy chica (un cuerpo apoyado en el
   piso), aplicar e=0.8 hace que vibre eternamente. Box2D usa un umbral: por
   debajo de cierta velocidad, e se fuerza a 0. Eso "duerme" los contactos.

   FRICCIÓN
   --------
   Después del impulso normal se calcula la tangente:
       t = normalizar(vRel - normal * dot(vRel, normal))
       jt = -dot(vRel, t) / denomTangente
       jt = clamp(jt, -μ*j, +μ*j)     <- cono de fricción de Coulomb
   Sin el clamp, la fricción podría empujar más de lo que la normal aprieta.
   ============================================================================= */
#include "include/fisica2d.hpp"

#include <iomanip>
#include <iostream>

using f2d::Vec2;

namespace {

struct Cuerpo {
    Vec2 pos;
    Vec2 vel;
    float angulo = 0.0f;
    float angVel = 0.0f;
    float invMass = 1.0f;
    float invInertia = 0.0f;
};

struct Manifold {
    Vec2 normal;       // de A hacia B
    Vec2 punto;        // punto de contacto en world space
    float penetracion = 0.0f;
};

struct Material {
    float restitucion = 0.0f;
    float friccion = 0.5f;
    float umbralRestitucion = 1.0f;
};

Vec2 velocidadEnPunto(const Cuerpo& c, const Vec2& r) {
    return c.vel + f2d::perp(r) * c.angVel;
}

void aplicarImpulso(Cuerpo& c, const Vec2& impulso, const Vec2& r) {
    c.vel += impulso * c.invMass;
    c.angVel += c.invInertia * f2d::cross(r, impulso);
}

float denomParaEje(const Cuerpo& a, const Cuerpo& b, const Vec2& ra,
                   const Vec2& rb, const Vec2& eje) {
    const float ca = f2d::cross(ra, eje);
    const float cb = f2d::cross(rb, eje);
    return a.invMass + b.invMass + a.invInertia * ca * ca + b.invInertia * cb * cb;
}

void resolver(Cuerpo& a, Cuerpo& b, const Manifold& m, const Material& mat) {
    const Vec2 ra = m.punto - a.pos;
    const Vec2 rb = m.punto - b.pos;
    const Vec2 n = f2d::normalize(m.normal);
    if (f2d::lengthSq(n) < f2d::kEps) return;

    Vec2 vRel = velocidadEnPunto(b, rb) - velocidadEnPunto(a, ra);
    const float vRelN = f2d::dot(vRel, n);
    if (vRelN > 0.0f) return;  // ya se están separando

    const float e = (std::fabs(vRelN) < mat.umbralRestitucion) ? 0.0f : mat.restitucion;
    const float denom = denomParaEje(a, b, ra, rb, n);
    if (denom <= f2d::kEps) return;

    const float j = -(1.0f + e) * vRelN / denom;
    const Vec2 impulsoN = n * j;
    aplicarImpulso(a, -impulsoN, ra);
    aplicarImpulso(b, impulsoN, rb);

    // Fricción: recalcular la velocidad relativa con las velocidades nuevas.
    vRel = velocidadEnPunto(b, rb) - velocidadEnPunto(a, ra);
    Vec2 tangente = vRel - n * f2d::dot(vRel, n);
    if (f2d::lengthSq(tangente) <= f2d::kEps) return;
    tangente = f2d::normalize(tangente);

    const float denomT = denomParaEje(a, b, ra, rb, tangente);
    if (denomT <= f2d::kEps) return;
    float jt = -f2d::dot(vRel, tangente) / denomT;
    const float limite = mat.friccion * j;
    jt = f2d::clampf(jt, -limite, limite);
    const Vec2 impulsoT = tangente * jt;
    aplicarImpulso(a, -impulsoT, ra);
    aplicarImpulso(b, impulsoT, rb);
}

void imprimir(const char* nombre, const Cuerpo& a, const Cuerpo& b) {
    std::cout << nombre << ":\n"
              << "  A vel=(" << a.vel.x << ", " << a.vel.y
              << ") ω=" << a.angVel << "\n"
              << "  B vel=(" << b.vel.x << ", " << b.vel.y
              << ") ω=" << b.angVel << "\n";
}

} // namespace

int main() {
    std::cout << std::fixed << std::setprecision(4);

    // 1) Choque frontal perfectamente inelástico: misma masa, se llevan la
    //    mitad de la velocidad cada uno.
    Cuerpo a1{{0.0f, 0.0f}, {1.0f, 0.0f}};
    Cuerpo b1{{1.0f, 0.0f}, {0.0f, 0.0f}};
    resolver(a1, b1, {{1.0f, 0.0f}, {0.5f, 0.0f}, 0.1f}, {0.0f, 0.6f, 1.0f});
    imprimir("Inelástico (e=0)", a1, b1);

    // 2) Elástico: A se detiene y B sale con la velocidad de A.
    Cuerpo a2{{0.0f, 0.0f}, {1.0f, 0.0f}};
    Cuerpo b2{{1.0f, 0.0f}, {0.0f, 0.0f}};
    resolver(a2, b2, {{1.0f, 0.0f}, {0.5f, 0.0f}, 0.1f}, {1.0f, 0.0f, 0.0f});
    imprimir("Elástico (e=1)", a2, b2);

    // 3) Fricción: A se mueve en diagonal contra B. La normal es vertical,
    //    así que la componente horizontal es tangencial y la fricción la frena.
    Cuerpo a3{{0.0f, 1.0f}, {1.0f, -1.0f}};
    Cuerpo b3{{0.0f, 0.0f}, {0.0f, 0.0f}};
    resolver(a3, b3, {{0.0f, -1.0f}, {0.0f, 0.0f}, 0.1f}, {0.0f, 0.5f, 1.0f});
    imprimir("Fricción μ=0.5", a3, b3);

    // 4) Contacto descentrado: el impulso genera giro (r × J != 0).
    Cuerpo a4{{0.0f, 0.0f}, {1.0f, 0.0f}, 0.0f, 0.0f, 1.0f, 1.0f};
    Cuerpo b4{{1.0f, 0.0f}, {0.0f, 0.0f}, 0.0f, 0.0f, 1.0f, 1.0f};
    resolver(a4, b4, {{1.0f, 0.0f}, {0.5f, 0.5f}, 0.1f}, {0.0f, 0.0f, 1.0f});
    imprimir("Contacto descentrado (con inercia)", a4, b4);

    std::cout << "\nSin el clamp de Coulomb, la fricción podría inyectar energía.\n"
                 "Sin el umbral de restitución, una caja apoyada vibraría siempre.\n";
    return 0;
}
