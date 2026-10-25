/* =============================================================================
   MÓDULO 23 - SOLUCIÓN 11: Integración ECS con eventos de física
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
        contactos_.clear();

        for (auto& e : entidades_) {
            if (!e.activa) continue;
            e.t.posAnterior = e.t.pos;
            e.t.anguloAnterior = e.t.angulo;

            if (e.b.tipo == TipoCuerpo::Static) continue;

            e.t.pos += e.b.vel * dt;
            e.t.angulo += e.b.angVel * dt;

            // El AABB acompaña la traslación (la rotación no lo recalcula).
            const Vec2 delta = e.t.pos - e.t.posAnterior;
            e.caja.minX += delta.x;
            e.caja.minY += delta.y;
            e.caja.maxX += delta.x;
            e.caja.maxY += delta.y;
        }

        std::set<Pair> ahora;
        for (size_t i = 0; i < entidades_.size(); ++i) {
            if (!entidades_[i].activa) continue;
            for (size_t j = i + 1; j < entidades_.size(); ++j) {
                if (!entidades_[j].activa) continue;
                if (!entidades_[i].caja.overlaps(entidades_[j].caja)) continue;

                const Pair p{static_cast<int>(i), static_cast<int>(j)};
                ahora.insert(p);
                if (!paresActivos_.contains(p)) {
                    const Vec2 ca{(entidades_[i].caja.minX + entidades_[i].caja.maxX) * 0.5f,
                                  (entidades_[i].caja.minY + entidades_[i].caja.maxY) * 0.5f};
                    const Vec2 cb{(entidades_[j].caja.minX + entidades_[j].caja.maxX) * 0.5f,
                                  (entidades_[j].caja.minY + entidades_[j].caja.maxY) * 0.5f};
                    Vec2 n = f2d::normalize(cb - ca);
                    if (f2d::lengthSq(n) <= f2d::kEps) n = {1.0f, 0.0f};
                    contactos_.push_back({TipoContacto::Begin, p.a, p.b, n,
                                          entidades_[i].sensor || entidades_[j].sensor});
                }
            }
        }

        for (const auto& p : paresActivos_) {
            if (ahora.contains(p)) continue;
            contactos_.push_back({TipoContacto::End, p.a, p.b, {},
                                  entidades_[p.a].sensor || entidades_[p.b].sensor});
        }

        paresActivos_ = std::move(ahora);
    }

    Transform2D paraDibujar(int id, float alpha) const {
        Transform2D r = entidades_[id].t;
        r.pos = r.posAnterior * (1.0f - alpha) + r.pos * alpha;
        r.angulo = r.anguloAnterior * (1.0f - alpha) + r.angulo * alpha;
        return r;
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
    std::cout << "Solución 11 (ECS física)...\n";
    return RUN_ALL_TESTS();
}
