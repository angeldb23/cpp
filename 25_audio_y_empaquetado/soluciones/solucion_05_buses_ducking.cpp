/* =============================================================================
   MÓDULO 25 - SOLUCIÓN 05: buses (grupos) y ducking con follower
   =============================================================================
   El follower "sin overshoot" es la clave: mover `min(paso, distancia al
   objetivo)` — si el paso pasara al objetivo, se queda EXACTO en el
   objetivo. El ducking DOWNS con pasoAtaque (fast duck) y vuelve con
   pasoRelease. En el motor real esto lo resuelve el smoothing del engine
   (defaultVolumeSmoothTimeInPCMFrames) o los fades por milliseconds, y el
   duckOn/duckOff es solo cambiar el objetivo del grupo.
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <cstdlib>
#include <string>
#include <unordered_map>

class GrafoBus {
public:
    void crear(const std::string& nombre, float volumen) {
        buses_[nombre] = clamp01(volumen);
    }

    void setMaster(float v) { master_ = clamp01(v); }

    void setVolumen(const std::string& nombre, float v) {
        auto it = buses_.find(nombre);
        if (it != buses_.end()) {
            it->second = clamp01(v);
        }
    }

    bool existe(const std::string& nombre) const {
        return buses_.find(nombre) != buses_.end();
    }

    float efectivo(const std::string& nombre) const {
        auto it = buses_.find(nombre);
        if (it == buses_.end()) {
            return 0.0f;
        }
        return master_ * it->second;
    }

private:
    static float clamp01(float v) {
        if (v < 0.0f) { return 0.0f; }
        if (v > 1.0f) { return 1.0f; }
        return v;
    }
    std::unordered_map<std::string, float> buses_;
    float master_ = 1.0f;
};

class FollowerDucking {
public:
    FollowerDucking(float inicial, float pasoAtaque, float pasoRelease)
        : vol_(inicial), pasoAtaque_(pasoAtaque), pasoRelease_(pasoRelease) {}

    void setObjetivo(float v) { objetivo_ = v; }

    float paso() {
        const float delta = objetivo_ - vol_;
        if (delta == 0.0f) {
            return vol_; /* ya está */
        }
        const float paso = delta < 0.0f ? pasoAtaque_ : pasoRelease_;
        /* Avanzar `paso` en la dirección del objetivo SIN pasarse:
           magnitud = min(paso, |delta|), y signo según el delta. */
        float mag = delta < 0.0f ? -delta : delta;
        if (mag > paso) {
            mag = paso;
        }
        vol_ += (delta < 0.0f ? -mag : mag);
        if (vol_ < 0.0f) { vol_ = 0.0f; }
        if (vol_ > 1.0f) { vol_ = 1.0f; }
        return vol_;
    }

    float volumen() const { return vol_; }

private:
    float vol_ = 1.0f;
    float objetivo_ = 1.0f;
    float pasoAtaque_ = 0.1f;
    float pasoRelease_ = 0.05f;
};

/* ------------------------------ TESTS (no tocar)-------------------------- */
TEST_CASE(VolumenesJerarquicos) {
    GrafoBus g;
    g.crear("music", 0.5f);
    g.crear("sfx", 1.0f);
    g.setMaster(0.8f);
    ASSERT_TRUE(g.existe("music"));
    ASSERT_TRUE(g.existe("sfx"));
    ASSERT_FALSE(g.existe("nope"));
    ASSERT_NEAR(g.efectivo("music"), 0.4f, 0.000001f);
    ASSERT_NEAR(g.efectivo("sfx"), 0.8f, 0.000001f);
    ASSERT_NEAR(g.efectivo("nope"), 0.0f, 0.000001f);
    g.setMaster(1.5f);
    ASSERT_NEAR(g.efectivo("sfx"), 1.0f, 0.000001f);
    g.setMaster(-0.2f);
    ASSERT_NEAR(g.efectivo("sfx"), 0.0f, 0.000001f);
    g.setVolumen("sfx", 2.0f);
    ASSERT_TRUE(g.efectivo("sfx") == 0.0f); /* master 0 x bus clamp 1 */
}

TEST_CASE(AtaqueAlObjetivo) {
    FollowerDucking f(0.5f, 0.1f, 0.05f);
    f.setObjetivo(0.15f);
    float v = f.paso();
    ASSERT_NEAR(v, 0.4f, 0.000001f);
    v = f.paso(); ASSERT_NEAR(v, 0.3f, 0.000001f);
    v = f.paso(); ASSERT_NEAR(v, 0.2f, 0.000001f);
    v = f.paso(); ASSERT_NEAR(v, 0.15f, 0.000001f); /* no se pasó */
    for (int i = 0; i < 5; ++i) {
        v = f.paso();
        ASSERT_NEAR(v, 0.15f, 0.000001f);
    }
}

TEST_CASE(ReleaseMasLento) {
    FollowerDucking f(0.15f, 0.1f, 0.05f);
    f.setObjetivo(0.5f);
    float v = 0.0f;
    for (int i = 0; i < 7; ++i) {
        v = f.paso(); /* +0.05 por frame */
    }
    ASSERT_NEAR(v, 0.5f, 0.000001f);
    v = f.paso(); /* ya está: sin overshoot */
    ASSERT_NEAR(v, 0.5f, 0.000001f);
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
