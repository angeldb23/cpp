/* =============================================================================
   MÓDULO 23 - LECCIÓN 10: Box2D 2.4.x — mundo, cuerpos, shapes y step
   =============================================================================
   TEMA 7 de TEMAS.md: "Box2D 2.4.x: b2World, b2Body, b2Shape,
   b2ContactListener, filtrado por categoría/máscara (API C++ de 2.4)".

   DEPENDENCIA Y VERSIÓN
   ---------------------
   El módulo trae Box2D por FetchContent con el tag `v2.4.2` (el tag real del
   repo erincatto/box2d; el `project()` interno todavía declara 2.4.1, es
   esperable). Se configuran `BOX2D_BUILD_UNIT_TESTS=OFF` y
   `BOX2D_BUILD_TESTBED=OFF` para no compilar el testbed con GLFW/ImGui, que
   en la máquina del dueño no hace falta.
   La API de esta lección se verificó contra los headers del tag v2.4.2:
   `b2World.h`, `b2Body.h`, `b2_fixture.h`, `b2_polygon_shape.h`.

   UNIDADES
   --------
   Box2D trabaja en METROS y segundos. Un personaje mide ~1.8 m, una caja
   ~0.5 m. Si usás píxeles (800 px de pantalla), la simulación se vuelve
   numéricamente inestable. Se convierte al dibujar: pixeles = metros * PPM.

   EL MUNDO
   --------
       b2Vec2 gravedad(0, -9.8f);
       b2World mundo(gravedad);
       mundo.Step(1/60.f, 8, 3);   // dt fijo, velocityIter, positionIter
   `Step` debe llamarse con un dt FIJO (1/60). Si le pasás el frame time
   variable, perdés determinismo y estabilidad. El acumulador de la lección 01
   es el que alimenta a `Step`.

   CUERPOS Y SHAPES
   ----------------
   - `b2BodyDef` describe el cuerpo: tipo (`b2_staticBody`, `b2_kinematicBody`,
     `b2_dynamicBody`), posición, ángulo, damping, `fixedRotation`, etc.
   - `b2Body` se crea con `mundo.CreateBody(&def)`. El mundo es dueño: se
     destruye con `mundo.DestroyBody(body)` o al destruir el mundo.
   - `b2PolygonShape::SetAsBox(hx, hy)` crea una caja centrada en el origen
     del cuerpo con SEMI-extents (la mitad del ancho/alto).
   - `b2CircleShape` tiene `m_p` (centro local) y `m_radius`.
   - Un `b2Fixture` une un shape a un cuerpo y define densidad, fricción,
     restitución y filtros. Un cuerpo puede tener varios fixtures.

   DENSIDAD, MASA E INERCIA
   -------------------------
   La masa no se setea a mano: Box2D la calcula de `densidad * área` de los
   fixtures. `body->GetMass()` y `body->GetInertia()` quedan disponibles
   después de crear los fixtures. Para un cuerpo estático, la masa es 0.

   Este archivo corre headless: no abre ventana.
   ============================================================================= */
#include <box2d/box2d.h>

#include <iomanip>
#include <iostream>

namespace {

b2Body* crearCaja(b2World& mundo, b2BodyType tipo, float x, float y,
                  float hx, float hy, float densidad = 1.0f) {
    b2BodyDef def;
    def.type = tipo;
    def.position.Set(x, y);
    b2Body* cuerpo = mundo.CreateBody(&def);

    b2PolygonShape shape;
    shape.SetAsBox(hx, hy);

    b2FixtureDef fixture;
    fixture.shape = &shape;
    fixture.density = densidad;
    fixture.friction = 0.4f;
    fixture.restitution = 0.1f;
    cuerpo->CreateFixture(&fixture);
    return cuerpo;
}

} // namespace

int main() {
    std::cout << std::fixed << std::setprecision(3);

    b2World mundo(b2Vec2(0.0f, -9.8f));

    // Piso estático: centro en y=-0.5, semi-extents 10 x 0.5 -> tope en y=0.
    b2Body* piso = crearCaja(mundo, b2_staticBody, 0.0f, -0.5f, 10.0f, 0.5f);

    // Caja dinámica que cae desde 5 m.
    b2Body* caja = crearCaja(mundo, b2_dynamicBody, 0.0f, 5.0f, 0.5f, 0.5f);

    // Plataforma cinemática: se mueve sola, los choques no la mueven.
    b2Body* plataforma = crearCaja(mundo, b2_kinematicBody, 3.0f, 2.0f, 1.0f, 0.1f);
    plataforma->SetLinearVelocity(b2Vec2(-0.5f, 0.0f));

    std::cout << "Masa de la caja: " << caja->GetMass()
              << " kg | inercia: " << caja->GetInertia() << " kg·m²\n";
    std::cout << "Masa del piso estático: " << piso->GetMass() << " kg\n\n";

    constexpr float dt = 1.0f / 60.0f;
    for (int paso = 0; paso <= 180; ++paso) {
        if (paso % 30 == 0) {
            const b2Vec2 p = caja->GetPosition();
            const b2Vec2 v = caja->GetLinearVelocity();
            std::cout << "paso " << std::setw(3) << paso
                      << " | caja y=" << p.y
                      << " vy=" << v.y
                      << " | plataforma x=" << plataforma->GetPosition().x << "\n";
        }
        mundo.Step(dt, 8, 3);  // 8 iteraciones de velocidad, 3 de posición
    }

    std::cout << "\nLa caja terminó en y=" << caja->GetPosition().y
              << " (el piso tiene el tope en y=0 y la caja mide 1 m de alto:\n"
              << "el centro de una caja apoyada queda en y≈0.5).\n";
    std::cout << "La plataforma cinemática se movió sin que la caja la frene.\n"
                 "`mundo.Step` con dt fijo es el corazón del determinismo.\n";
    return 0;
}
