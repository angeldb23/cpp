/* =============================================================================
   MÓDULO 23 - LECCIÓN 12: Integración de física con un ECS
   =============================================================================
   TEMA 8 de TEMAS.md: "Integración ECS: componente RigidBody2D, sistema de
   física (step + sync transform), callbacks a eventos de juego" (primera de
   dos lecciones).

   SEPARAR DATOS DE SISTEMAS
   -------------------------
   En un ECS, la física no es una clase que hereda de "Entity". Es un SISTEMA
   que lee y escribe componentes:

       Transform2D { Vec2 pos; float angle; }
       RigidBody2D { Vec2 vel; float angVel; float invMass; BodyKind kind; }
       Collider2D  { Vec2 halfExtent; bool sensor; }

   El sistema de física:
       1. Corre en timestep fijo (lección 01): no en cada frame de render.
       2. Integra velocidad -> posición para los cuerpos dinámicos/cinemáticos.
       3. Resuelve contactos (Box2D o solver propio).
       4. Sincroniza `Transform2D` con el resultado.
       5. Encola eventos de contacto para que el gameplay los lea después.

   POR QUÉ NO GUARDAR LA POSICIÓN EN DOS LUGARES
   ---------------------------------------------
   Si `RigidBody2D` y `Transform2D` guardan posición, tarde o temprano se
   desincronizan. La regla: una sola fuente de verdad. Podés elegir:
     - El body es dueño de la posición y el transform es una vista (Box2D).
     - El transform es dueño y el body solo guarda velocidades.
   Acá usamos la segunda: el body integra y al final copia a Transform2D. El
   render lee Transform2D y el gameplay también; nadie más toca la posición.

   INTERPOLACIÓN
   -------------
   Para dibujar suave entre pasos fijos se guarda `posAnterior` en el
   Transform y se interpola con alpha:
       dibujo = anterior*(1-alpha) + actual*alpha
   La física y el gameplay usan `actual`; solo el render interpola. Si
   interpolás la lógica, las colisiones quedan un paso atrás.

   EVENTOS DEFERRED
   ----------------
   El sistema de física no debe llamar `destruirEntidad()` en medio de la
   iteración. Encola `ContactEvent{ a, b, normal, sensor }` y el sistema de
   gameplay los procesa al final del frame. Es el mismo patrón que el
   `b2ContactListener` de la lección 11.
   ============================================================================= */
#include "include/fisica2d.hpp"

#include <iomanip>
#include <iostream>
#include <vector>

using f2d::Vec2;

namespace {

enum class TipoCuerpo { Static, Kinematic, Dynamic };

struct Transform2D {
    Vec2 pos;
    Vec2 posAnterior;
    float angulo = 0.0f;
    float anguloAnterior = 0.0f;
};

struct RigidBody2D {
    int entidad = -1;
    Vec2 vel;
    float angVel = 0.0f;
    float invMass = 1.0f;
    TipoCuerpo tipo = TipoCuerpo::Dynamic;
};

struct Contacto {
    int a = -1;
    int b = -1;
    Vec2 normal;
};

class EscenaFisica {
public:
    int agregar(Transform2D t, RigidBody2D b) {
        const int id = static_cast<int>(transforms_.size());
        t.posAnterior = t.pos;
        t.anguloAnterior = t.angulo;
        b.entidad = id;
        transforms_.push_back(t);
        bodies_.push_back(b);
        activos_.push_back(true);
        return id;
    }

    void paso(float dt) {
        contactos_.clear();
        for (size_t i = 0; i < bodies_.size(); ++i) {
            if (!activos_[i]) continue;
            auto& b = bodies_[i];
            auto& t = transforms_[i];

            t.posAnterior = t.pos;
            t.anguloAnterior = t.angulo;

            if (b.tipo == TipoCuerpo::Static) continue;

            // Integración semi-implícita simple. Las fuerzas se aplicarían
            // antes de este paso con b.vel += fuerza * invMass * dt.
            t.pos += b.vel * dt;
            t.angulo += b.angVel * dt;
        }
    }

    // El render interpola; la lógica NO.
    Transform2D paraDibujar(int id, float alpha) const {
        Transform2D r = transforms_[id];
        r.pos = r.posAnterior * (1.0f - alpha) + r.pos * alpha;
        r.angulo = r.anguloAnterior * (1.0f - alpha) + r.angulo * alpha;
        return r;
    }

    const Transform2D& transform(int id) const { return transforms_[id]; }
    const std::vector<Contacto>& contactos() const { return contactos_; }
    void desactivar(int id) { activos_[id] = false; }

private:
    std::vector<Transform2D> transforms_;
    std::vector<RigidBody2D> bodies_;
    std::vector<bool> activos_;
    std::vector<Contacto> contactos_;
};

} // namespace

int main() {
    std::cout << std::fixed << std::setprecision(3);

    EscenaFisica escena;

    const int jugador = escena.agregar({{0.0f, 0.0f}, {}, 0.0f, 0.0f},
                                       {{}, {3.0f, 0.0f}, 0.0f, 1.0f, TipoCuerpo::Dynamic});
    const int plataforma = escena.agregar({{0.0f, -1.0f}, {}, 0.0f, 0.0f},
                                          {{}, {0.5f, 0.0f}, 0.0f, 0.0f, TipoCuerpo::Kinematic});
    const int pared = escena.agregar({{10.0f, 0.0f}, {}, 0.0f, 0.0f},
                                     {{}, {}, 0.0f, 0.0f, TipoCuerpo::Static});

    std::cout << "Entidades: jugador=" << jugador << " plataforma=" << plataforma
              << " pared=" << pared << "\n\n";

    constexpr float dt = 1.0f / 60.0f;
    constexpr float alpha = 0.5f;
    for (int frame = 0; frame < 4; ++frame) {
        escena.paso(dt);
        const auto logica = escena.transform(jugador);
        const auto dibujo = escena.paraDibujar(jugador, alpha);
        std::cout << "frame " << frame
                  << " | jugador lógico x=" << logica.pos.x
                  << " | dibujado x=" << dibujo.pos.x
                  << " | plataforma x=" << escena.transform(plataforma).pos.x << "\n";
    }

    std::cout << "\nEl jugador avanzó " << escena.transform(jugador).pos.x
              << " m en " << 4 * dt << " s. La pared estática no se movió: x="
              << escena.transform(pared).pos.x << "\n";
    std::cout << "El dibujo va medio paso atrás (alpha=0.5): así se ve suave\n"
                 "sin que la lógica dependa de los FPS.\n";
    return 0;
}
