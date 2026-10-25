/* =============================================================================
   MÓDULO 23 - LECCIÓN 11: Box2D — contactos, sensores y filtros
   =============================================================================
   TEMA 7 de TEMAS.md (segunda lección): b2ContactListener y filtrado por
   categoría/máscara.

   CONTACT LISTENER
   ----------------
   Box2D no te avisa de cada choque con un callback de "daño": te da contactos
   físicos. Para escuchar:
       class MiListener : public b2ContactListener {
           void BeginContact(b2Contact* c) override;
           void EndContact(b2Contact* c) override;
           void PreSolve(b2Contact* c, const b2Manifold* viejo) override;
           void PostSolve(b2Contact* c, const b2ContactImpulse* imp) override;
       };
       mundo.SetContactListener(&listener);
   - `BeginContact` se llama cuando los fixtures empiezan a tocarse.
   - `EndContact`, cuando dejan de tocarse.
   - `PreSolve` permite desactivar el contacto (`c->SetEnabled(false)`) sin
     borrar los cuerpos: ideal para plataformas de un solo sentido.
   - `PostSolve` recibe los impulsos aplicados; sirve para calcular daño.
   El listener NO debe destruir cuerpos dentro del callback: Box2D está
   iterando el mundo. Se encola la destrucción para después de `Step`.

   WORLD MANIFOLD
   --------------
       b2WorldManifold wm;
       c->GetWorldManifold(&wm);
   `wm.normal` va de A a B, `wm.points[i]` son los puntos de contacto en world
   space. `c->GetManifold()` da los puntos en espacio local y el tipo de
   manifold (circles, faceA, faceB).

   FILTROS
   -------
   Cada fixture tiene un `b2Filter`:
       categoryBits: qué soy (un bit, p. ej. 0x0002 = jugador)
       maskBits:     con qué categorías quiero chocar
       groupIndex:   >0 siempre choca con el mismo grupo,
                     <0 nunca choca con el mismo grupo,
                      0 usa category/mask.
   La regla es simétrica: A y B chocan si
       (catA & maskB) != 0 && (catB & maskA) != 0
   (salvo que groupIndex no sea 0). Box2D cachea los pares; después de
   cambiar el filtro hay que llamar a `fixture->SetFilterData(f)` y el próximo
   `Step` reevalúa.
   Un SENSOR (`fixture->SetSensor(true)`) genera Begin/EndContact pero no
   respuesta física: perfecto para triggers, pickups y zonas de daño.
   ============================================================================= */
#include <box2d/box2d.h>

#include <iomanip>
#include <iostream>

namespace {

enum Categoria : uint16_t {
    CAT_PARED = 0x0001,
    CAT_JUGADOR = 0x0002,
    CAT_ENEMIGO = 0x0004,
};

class Escucha : public b2ContactListener {
public:
    int begins = 0;
    int ends = 0;
    b2Vec2 ultimaNormal{0.0f, 0.0f};

    void BeginContact(b2Contact* contacto) override {
        ++begins;
        b2WorldManifold wm;
        contacto->GetWorldManifold(&wm);
        ultimaNormal = wm.normal;
    }

    void EndContact(b2Contact*) override { ++ends; }
};

b2Body* crearCirculo(b2World& mundo, b2BodyType tipo, b2Vec2 pos, float radio,
                     b2Filter filtro, bool sensor = false) {
    b2BodyDef def;
    def.type = tipo;
    def.position = pos;
    b2Body* cuerpo = mundo.CreateBody(&def);

    b2CircleShape shape;
    shape.m_radius = radio;

    b2FixtureDef fd;
    fd.shape = &shape;
    fd.density = 1.0f;
    fd.friction = 0.3f;
    fd.restitution = 0.2f;
    fd.filter = filtro;
    fd.isSensor = sensor;
    cuerpo->CreateFixture(&fd);
    return cuerpo;
}

} // namespace

int main() {
    std::cout << std::fixed << std::setprecision(3);

    b2World mundo(b2Vec2(0.0f, 0.0f));  // sin gravedad: solo contactos
    Escucha escucha;
    mundo.SetContactListener(&escucha);

    // A: jugador. B: enemigo. Sus máscaras NO se incluyen mutuamente.
    // El filtro es SIMÉTRICO: A quiere chocar con B y B con A. El jugador ya
    // acepta enemigos; el enemigo todavía no acepta jugadores.
    b2Filter fJugador;
    fJugador.categoryBits = CAT_JUGADOR;
    fJugador.maskBits = CAT_PARED | CAT_ENEMIGO;

    b2Filter fEnemigo;
    fEnemigo.categoryBits = CAT_ENEMIGO;
    fEnemigo.maskBits = CAT_PARED;  // por ahora solo paredes

    b2Body* jugador = crearCirculo(mundo, b2_dynamicBody, {0.0f, 0.0f}, 0.5f, fJugador);
    b2Body* enemigo = crearCirculo(mundo, b2_dynamicBody, {0.5f, 0.0f}, 0.5f, fEnemigo);

    mundo.Step(1.0f / 60.0f, 8, 3);
    std::cout << "Jugador y enemigo superpuestos pero con máscaras distintas:\n"
              << "  BeginContact = " << escucha.begins << " (esperado 0)\n";

    // Cambiamos la máscara del enemigo en runtime para que sí choque.
    b2Filter fEnemigoNuevo = fEnemigo;
    fEnemigoNuevo.maskBits = CAT_PARED | CAT_JUGADOR;
    enemigo->GetFixtureList()->SetFilterData(fEnemigoNuevo);

    // El primer Step crea el contacto en la broad-phase; el BeginContact se
    // dispara cuando el contacto se actualiza en el Step siguiente.
    mundo.Step(1.0f / 60.0f, 8, 3);
    mundo.Step(1.0f / 60.0f, 8, 3);
    std::cout << "Después de SetFilterData (2 Steps) -> BeginContact = "
              << escucha.begins << " (esperado 1), normal=("
              << escucha.ultimaNormal.x << ", " << escucha.ultimaNormal.y << ")\n";
    (void)jugador;

    // Un sensor también dispara BeginContact pero no empuja. Mundo aparte
    // para que el conteo no se mezcle con los rebotes del par anterior.
    b2World mundoSensor(b2Vec2(0.0f, 0.0f));
    Escucha escuchaSensor;
    mundoSensor.SetContactListener(&escuchaSensor);

    b2Filter fSensor;
    fSensor.categoryBits = CAT_PARED;
    fSensor.maskBits = 0xFFFF;
    crearCirculo(mundoSensor, b2_staticBody, {3.0f, 0.0f}, 1.0f, fSensor, true);
    // Dinámico (Box2D exige al menos un dinámico para generar contactos);
    // como el sensor no responde, conserva su velocidad.
    b2Body* visita = crearCirculo(mundoSensor, b2_dynamicBody, {1.0f, 0.0f}, 0.2f, fSensor);
    visita->SetLinearVelocity(b2Vec2(2.0f, 0.0f));

    for (int i = 0; i < 60; ++i) mundoSensor.Step(1.0f / 60.0f, 8, 3);
    std::cout << "Sensor: BeginContact = " << escuchaSensor.begins
              << " (el sensor no frena al cuerpo)\n";
    std::cout << "El cuerpo visitante terminó en x=" << visita->GetPosition().x
              << ": atravesó el trigger.\n";

    std::cout << "\nEn un juego, BeginContact encola eventos ('jugador pisó\n"
                 "trigger') y el sistema de juego los procesa DESPUÉS de Step.\n";
    return 0;
}
