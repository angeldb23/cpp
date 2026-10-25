/* =============================================================================
   MÓDULO 24 - SOLUCIÓN 06: Matemática de gizmo
   =============================================================================
   DIFICULTAD: Alta
   APOYO: Lección 07 (rayo-plano, rayo-eje, snap, base local y picking).

   CONSIGNA
   --------
   Implementá:

   1. `rayoPlano(r, punto, normal)`: intersección rayo-plano. Devuelve el punto
      sólo si |dot(dir, normal)| >= 1e-6 y t >= 0. Si es paralelo o el plano
      está detrás, `std::nullopt`.
   2. `parametroEnEje(r, origenEje, eje)`: parámetro `s` del punto de la recta
      `origenEje + s*eje` más cercano al rayo:
          w0 = origenEje - r.origen
          a = eje·eje, b = eje·dir, c = dir·dir
          d = eje·w0, e = dir·w0
          denom = a*c - b*b
          s = (b*e - c*d) / denom
      Si |denom| < 1e-6 (paralelos), `std::nullopt`.
   3. `snap(valor, paso)`: redondea al múltiplo de `paso` más cercano. Si
      `paso <= 0`, devuelve `valor` sin tocar.
   4. `aplicarBase(local, bx, by, bz)`: `bx*l.x + by*l.y + bz*l.z`.
   5. `distanciaASegmento(p, a, b)`: distancia euclídea en 2D, con el punto
      proyectado clampeado al segmento (no a la recta infinita).
   ============================================================================= */
#include "../include/gui_util.hpp"
#include "../../include/test_framework.hpp"

#include <cmath>
#include <optional>

struct Rayo {
    gui::Vec3 origen;
    gui::Vec3 dir;
};

std::optional<gui::Vec3> rayoPlano(const Rayo& r, gui::Vec3 puntoPlano, gui::Vec3 normal) {
    const float denom = gui::dot(r.dir, normal);
    if (std::fabs(denom) < 1e-6f) return std::nullopt;
    const float t = gui::dot(puntoPlano - r.origen, normal) / denom;
    if (t < 0.0f) return std::nullopt;
    return r.origen + r.dir * t;
}

std::optional<float> parametroEnEje(const Rayo& r, gui::Vec3 origenEje, gui::Vec3 eje) {
    const gui::Vec3 w0 = origenEje - r.origen;
    const float a = gui::dot(eje, eje);
    const float b = gui::dot(eje, r.dir);
    const float c = gui::dot(r.dir, r.dir);
    const float d = gui::dot(eje, w0);
    const float e = gui::dot(r.dir, w0);
    const float denom = a * c - b * b;
    if (std::fabs(denom) < 1e-6f) return std::nullopt;
    return (b * e - c * d) / denom;
}

float snap(float valor, float paso) {
    if (paso <= 0.0f) return valor;
    return std::round(valor / paso) * paso;
}

gui::Vec3 aplicarBase(gui::Vec3 local, gui::Vec3 bx, gui::Vec3 by, gui::Vec3 bz) {
    return bx * local.x + by * local.y + bz * local.z;
}

float distanciaASegmento(gui::Vec2 p, gui::Vec2 a, gui::Vec2 b) {
    const gui::Vec2 ab = b - a;
    const float len2 = gui::dot(ab, ab);
    if (len2 < 1e-12f) {
        const gui::Vec2 d = p - a;
        return std::sqrt(gui::dot(d, d));
    }
    float t = gui::dot(p - a, ab) / len2;
    t = gui::clampf(t, 0.0f, 1.0f);
    const gui::Vec2 d = p - (a + ab * t);
    return std::sqrt(gui::dot(d, d));
}

// ============================================================================
// TESTS (no tocar)
// ============================================================================
TEST_CASE(RayoPlanoImpacta) {
    const Rayo r{{0.2f, 1.5f, 5.0f}, {0.0f, 0.0f, -1.0f}};
    const auto p = rayoPlano(r, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f});
    ASSERT_TRUE(p.has_value());
    ASSERT_NEAR(p->x, 0.2f, 1e-5f);
    ASSERT_NEAR(p->y, 1.5f, 1e-5f);
    ASSERT_NEAR(p->z, 0.0f, 1e-5f);
}

TEST_CASE(RayoPlanoParaleloOdetras) {
    const Rayo paralelo{{0.0f, 0.0f, 5.0f}, {1.0f, 0.0f, 0.0f}};
    ASSERT_FALSE(rayoPlano(paralelo, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}).has_value());

    const Rayo alejandose{{0.0f, 0.0f, 5.0f}, {0.0f, 0.0f, 1.0f}};
    ASSERT_FALSE(rayoPlano(alejandose, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}).has_value());
}

TEST_CASE(ParametroSobreEjeX) {
    const Rayo r{{0.8f, 1.5f, 5.0f}, {0.0f, 0.0f, -1.0f}};
    const auto s = parametroEnEje(r, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f});
    ASSERT_TRUE(s.has_value());
    ASSERT_NEAR(*s, 0.8f, 1e-5f);
}

TEST_CASE(ParametroEjeParaleloAlRayo) {
    const Rayo r{{0.0f, 0.0f, 5.0f}, {0.0f, 0.0f, -1.0f}};
    ASSERT_FALSE(parametroEnEje(r, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}).has_value());
}

TEST_CASE(Snap) {
    ASSERT_NEAR(snap(0.37f, 0.25f), 0.25f, 1e-6f);
    ASSERT_NEAR(snap(0.13f, 0.25f), 0.25f, 1e-6f);
    ASSERT_NEAR(snap(0.12f, 0.25f), 0.0f, 1e-6f);
    ASSERT_NEAR(snap(1.24f, 0.5f), 1.0f, 1e-6f);
    ASSERT_NEAR(snap(3.7f, 0.0f), 3.7f, 1e-6f);
}

TEST_CASE(BaseIdentidadYRotada) {
    const auto v = aplicarBase({1.0f, 2.0f, 3.0f}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1});
    ASSERT_NEAR(v.x, 1.0f, 1e-6f);
    ASSERT_NEAR(v.y, 2.0f, 1e-6f);
    ASSERT_NEAR(v.z, 3.0f, 1e-6f);

    const float c = std::cos(gui::kPi / 2.0f);
    const float s = std::sin(gui::kPi / 2.0f);
    const auto r = aplicarBase({1.0f, 0.0f, 0.0f}, {c, s, 0}, {-s, c, 0}, {0, 0, 1});
    ASSERT_NEAR(r.x, 0.0f, 1e-5f);
    ASSERT_NEAR(r.y, 1.0f, 1e-5f);
}

TEST_CASE(DistanciaASegmento) {
    ASSERT_NEAR(distanciaASegmento({50.0f, 52.0f}, {10.0f, 50.0f}, {90.0f, 50.0f}), 2.0f,
                1e-5f);
    ASSERT_NEAR(distanciaASegmento({0.0f, 0.0f}, {10.0f, 0.0f}, {20.0f, 0.0f}), 10.0f,
                1e-5f);
    ASSERT_NEAR(distanciaASegmento({15.0f, 4.0f}, {10.0f, 0.0f}, {20.0f, 0.0f}), 4.0f,
                1e-5f);
}

int main() {
    std::cout << "Solución 06 (gizmo)...\n";
    return RUN_ALL_TESTS();
}
