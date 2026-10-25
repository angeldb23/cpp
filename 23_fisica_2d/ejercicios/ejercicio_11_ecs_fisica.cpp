/* =============================================================================
   MÓDULO 23 - EJERCICIO 11: Integración ECS con eventos de física
   =============================================================================
   DIFICULTAD: Alto
   APOYO: Lecciones 12 (sistema de física + sync de transform, interpolación)
          y 13 (eventos begin/end, sensores, cola diferida).

   CONSIGNA:
     - `paso(dt)`:
         1. Guardar pos/ángulo anteriores de CADA entidad activa.
         2. Static no se mueve. Kinematic y Dynamic integran:
              pos += vel*dt; ángulo += angVel*dt.
            La caja AABB de una entidad que se mueve se traslada el mismo
            delta que su posición (no se recalcula por rotación).
         3. Detectar solapes de AABB entre entidades activas.
         4. Emitir un Contacto::Begin solo la PRIMERA vez que aparece el par
            (comparando con `paresActivos_`), y Contacto::End cuando el par
            desaparece. Un par es (min(id), max(id)).
         5. Reemplazar `paresActivos_` por los pares de este paso.
       `contactos_` se limpia al principio de cada paso.
     - `paraDibujar(id, alpha)`: interpolar pos y ángulo entre anterior y
       actual (solo para render; no tocar la posición lógica).
     - `desactivar(id)`: la entidad ni se mueve ni genera contactos.
     El flag `sensor` del contacto es true si cualquiera de las dos entidades
   es sensor.
   ============================================================================= */
#include "../include/fisica2d.hpp"
#include "../../include/test_framework.hpp"

#include <algorithm>
#include <set>
#include <vector>

using f2d::AABB;
using f2d::Pair;
using f2d::Vec2;

enum class TipoCuerpo { Static, Kinematic, Dynamic };

struct Transform2D {
    Vec2 pos;
    Vec2 posAnterior;
    float angulo = 0.0f;
    float anguloAnterior = 0.0f;
};

struct RigidBody2D {
    Vec2 vel;
    float angVel = 0.0f;
    float invMass = 1.0f;
    TipoCuerpo tipo = TipoCuerpo::Dynamic;
};

enum class TipoContacto { Begin, End };

struct Contacto {
    TipoContacto tipo = TipoContacto::Begin;
    int a = -1;
    int b = -1;
    Vec2 normal;
    bool sensor = false;
};

class EscenaFisica {
public:
    int agregar(Transform2D t, RigidBody2D b, AABB caja, bool sensor = false) {
        const int id = static_cast<int>(entidades_.size());
        t.posAnterior = t.pos;
        t.anguloAnterior = t.angulo;
        entidades_.push_back({t, b, caja, sensor, true});
        return id;
    }

    void paso(float dt) {
        // TODO
        (void)dt;
    }

    Transform2D paraDibujar(int id, float alpha) const {
        // TODO
        (void)id;
        (void)alpha;
        return {};
    }

    const Transform2D& transform(int id) const { return entidades_[id].t; }
    const std::vector<Contacto>& contactos() const { return contactos_; }
    void desactivar(int id) { entidades_[id].activa = false; }

private:
    struct Entidad {
        Transform2D t;
        RigidBody2D b;
        AABB caja;
        bool sensor = false;
        bool activa = true;
    };

    std::vector<Entidad> entidades_;
    std::vector<Contacto> contactos_;
    std::set<Pair> paresActivos_;
};

// ============================================================================
// TESTS (no tocar)
// ============================================================================
namespace {

Transform2D t(Vec2 pos, float angulo = 0.0f) {
    return {pos, pos, angulo, angulo};
}

RigidBody2D cuerpo(Vec2 vel, TipoCuerpo tipo = TipoCuerpo::Dynamic,
                   float angVel = 0.0f) {
    return {vel, angVel, tipo == TipoCuerpo::Dynamic ? 1.0f : 0.0f, tipo};
}

} // namespace

TEST_CASE(DinamicoIntegraYGuardaAnterior) {
    EscenaFisica escena;
    const int id = escena.agregar(t({0.0f, 0.0f}), cuerpo({3.0f, 0.0f}),
                                  {-0.5f, -0.5f, 0.5f, 0.5f});
    escena.paso(1.0f / 60.0f);
    ASSERT_NEAR(escena.transform(id).pos.x, 0.05f, 1e-5f);
    ASSERT_NEAR(escena.transform(id).posAnterior.x, 0.0f, 1e-6f);
}

TEST_CASE(EstaticoYcinematico) {
    EscenaFisica escena;
    const int estatico = escena.agregar(t({1.0f, 1.0f}),
                                        cuerpo({5.0f, 0.0f}, TipoCuerpo::Static),
                                        {0.0f, 0.0f, 1.0f, 1.0f});
    const int cinematico = escena.agregar(t({0.0f, 0.0f}),
                                          cuerpo({2.0f, 0.0f}, TipoCuerpo::Kinematic),
                                          {0.0f, 0.0f, 1.0f, 1.0f});
    escena.paso(0.5f);
    ASSERT_NEAR(escena.transform(estatico).pos.x, 1.0f, 1e-6f);
    ASSERT_NEAR(escena.transform(cinematico).pos.x, 1.0f, 1e-6f);
}

TEST_CASE(InterpolacionSoloParaDibujo) {
    EscenaFisica escena;
    const int id = escena.agregar(t({0.0f, 0.0f}), cuerpo({10.0f, 0.0f}, TipoCuerpo::Dynamic, 2.0f),
                                  {0.0f, 0.0f, 1.0f, 1.0f});
    escena.paso(1.0f);
    const Transform2D dibujo = escena.paraDibujar(id, 0.5f);
    ASSERT_NEAR(dibujo.pos.x, 5.0f, 1e-5f);
    ASSERT_NEAR(dibujo.angulo, 1.0f, 1e-5f);
    // La lógica sigue en el valor actual, no en el interpolado.
    ASSERT_NEAR(escena.transform(id).pos.x, 10.0f, 1e-6f);
    ASSERT_NEAR(escena.transform(id).angulo, 2.0f, 1e-6f);
}

TEST_CASE(BeginContactSoloUnaVez) {
    EscenaFisica escena;
    escena.agregar(t({0.0f, 0.0f}), cuerpo({}), {0.0f, 0.0f, 1.0f, 1.0f});
    escena.agregar(t({0.5f, 0.0f}), cuerpo({}), {0.5f, 0.0f, 1.5f, 1.0f});

    escena.paso(1.0f / 60.0f);
    ASSERT_EQ(escena.contactos().size(), size_t{1});
    ASSERT_TRUE(escena.contactos()[0].tipo == TipoContacto::Begin);
    ASSERT_EQ(escena.contactos()[0].a, 0);
    ASSERT_EQ(escena.contactos()[0].b, 1);
    ASSERT_NEAR(escena.contactos()[0].normal.x, 1.0f, 1e-6f);

    escena.paso(1.0f / 60.0f);
    ASSERT_EQ(escena.contactos().size(), size_t{0});
}

TEST_CASE(EndContactAlSepararse) {
    EscenaFisica escena;
    escena.agregar(t({0.0f, 0.0f}), cuerpo({-2.0f, 0.0f}), {0.0f, 0.0f, 1.0f, 1.0f});
    escena.agregar(t({0.5f, 0.0f}), cuerpo({2.0f, 0.0f}), {0.5f, 0.0f, 1.5f, 1.0f});

    escena.paso(0.1f);
    ASSERT_EQ(escena.contactos().size(), size_t{1});
    ASSERT_TRUE(escena.contactos()[0].tipo == TipoContacto::Begin);

    escena.paso(0.1f);
    ASSERT_EQ(escena.contactos().size(), size_t{1});
    ASSERT_TRUE(escena.contactos()[0].tipo == TipoContacto::End);
}

TEST_CASE(SensorSePropagaAlEvento) {
    EscenaFisica escena;
    escena.agregar(t({0.0f, 0.0f}), cuerpo({}), {0.0f, 0.0f, 1.0f, 1.0f});
    escena.agregar(t({0.5f, 0.0f}), cuerpo({}), {0.5f, 0.0f, 1.5f, 1.5f}, true);
    escena.paso(1.0f / 60.0f);
    ASSERT_EQ(escena.contactos().size(), size_t{1});
    ASSERT_TRUE(escena.contactos()[0].sensor);
}

TEST_CASE(EntidadDesactivadaNoSeMueveNiChoca) {
    EscenaFisica escena;
    escena.agregar(t({0.0f, 0.0f}), cuerpo({}), {0.0f, 0.0f, 1.0f, 1.0f});
    const int b = escena.agregar(t({0.5f, 0.0f}), cuerpo({5.0f, 0.0f}),
                                 {0.5f, 0.0f, 1.5f, 1.0f});
    escena.desactivar(b);
    escena.paso(1.0f);
    ASSERT_NEAR(escena.transform(b).pos.x, 0.5f, 1e-6f);
    ASSERT_EQ(escena.contactos().size(), size_t{0});
}

int main() {
    std::cout << "Ejercicio 11 (ECS física)...\n";
    return RUN_ALL_TESTS();
}
