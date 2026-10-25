/* =============================================================================
   MÓDULO 25 - EJERCICIO 05: buses (grupos) y ducking con follower
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 05 (ma_sound_group jerárquico, master x bus, ducking con
   fade/attack y release).

   CONSIGNA
   --------
   Completá `GrafoBus` (master + buses colgados) y `FollowerDucking`:

   `GrafoBus`:
   1. `setMaster(v)` y `setVolumen(nombre, v)`: guardan el valor [0,1]
      CLAMPEADO (setMaster(1.5) queda 1.0; setMaster(-0.2) queda 0.0).
   2. `efectivo(nombre)`: master.volumen * bus.volumen. Bus inválido: 0.0.
   3. `existe(nombre)`: master siempre "existe"; los buses luego de crear
      con `crear(nombre, volumen)`.

   `FollowerDucking` (el gain follower del sidechain, SIN miniaudio para
   poder testearlo; en el motor es ma_sound_set_fade_in_milliseconds):
    1. `setObjetivo(v)`: el volumen de música a ir A (con el valor exacto
      que te pasan: el ratio del ducking típico es 0.3 x vol del grupo).
   2. El constructor `FollowerDucking(volInicial, pasoAtaque, pasoRelease)`
      fija los DOS pasos por frame: pasoAtaque (BAJAR el volumen: fast
      duck) y pasoRelease (VOLVER A SUBIR: caída lenta y "con
      respiración"). La dirección se decide por frame: si el objetivo está
      ABAJO del volumen actual, el incremento es pasoAtaque; si está
      ARRIBA, es pasoRelease.
   3. `paso()`: avanza UN frame: se mueve hacia el objetivo exactamente
      `paso` (ataque o release según dir) pero SIN PASARSE (y el volumen
      queda siempre en [0,1]). Devuelve el volumen actual tras el paso.
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <string>
#include <unordered_map>

class GrafoBus {
public:
    void crear(const std::string& nombre, float volumen) {
        // TODO
        (void)nombre; (void)volumen;
    }

    void setMaster(float v) {
        // TODO
        (void)v;
    }

    void setVolumen(const std::string& nombre, float v) {
        // TODO
        (void)nombre; (void)v;
    }

    bool existe(const std::string& nombre) const {
        // TODO
        (void)nombre;
        return false;
    }

    float efectivo(const std::string& nombre) const {
        // TODO
        (void)nombre;
        return 0.0f;
    }
};

class FollowerDucking {
public:
    FollowerDucking(float inicial, float pasoAtaque, float pasoRelease)
        : vol_(inicial), pasoAtaque_(pasoAtaque), pasoRelease_(pasoRelease) {}

    void setObjetivo(float v) {
        // TODO
        (void)v;
    }

    float paso() {
        // TODO
        return 0.0f;
    }

    float volumen() const {
        // TODO
        return 0.0f;
    }

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
    /* clamp del master y de los buses */
    g.setMaster(1.5f);
    ASSERT_NEAR(g.efectivo("sfx"), 1.0f, 0.000001f);
    g.setMaster(-0.2f);
    ASSERT_NEAR(g.efectivo("sfx"), 0.0f, 0.000001f);
    g.setVolumen("sfx", 2.0f);
    ASSERT_TRUE(g.efectivo("sfx") == 0.0f); /* master 0 x bus clamp 1 */
}

TEST_CASE(AtaqueAlObjetivo) {
    /* Ducking: bajar la música del 0.5 al 0.15 (30% de 0.5): ataque con
       paso 0.1 => 4 frames exactos. */
    FollowerDucking f(0.5f, 0.1f, 0.05f);
    f.setObjetivo(0.15f);
    float v = f.paso();
    ASSERT_NEAR(v, 0.4f, 0.000001f);
    v = f.paso(); ASSERT_NEAR(v, 0.3f, 0.000001f);
    v = f.paso(); ASSERT_NEAR(v, 0.2f, 0.000001f);
    v = f.paso(); ASSERT_NEAR(v, 0.15f, 0.000001f); /* no se pasó */
    /* sigue exacto (sin overshoot) */
    for (int i = 0; i < 5; ++i) {
        v = f.paso();
        ASSERT_NEAR(v, 0.15f, 0.000001f);
    }
}

TEST_CASE(ReleaseMasLento) {
    /* Volver de 0.15 a 0.5 con release 0.05: 7 frames exactos. */
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
