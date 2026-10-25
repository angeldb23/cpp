/* =============================================================================
   MÓDULO 24 - EJERCICIO 07: Curve editor con keyframes
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 08 (curve editor e interpolación).

   CONSIGNA
   --------
   Completá `Curve`:

   - `addKey(k)`: inserta ordenado por tiempo. Si ya hay una key con el mismo
     tiempo (±1e-5), la REEMPLAZA (devuelve true en ambos casos).
   - `removeKey(t)`: borra la key en ese tiempo; true si existía.
   - `evaluate(t)`:
       * sin keys -> 0;
       * t <= primera -> valor de la primera;
       * t >= última  -> valor de la última;
       * entre dos keys, usa el modo de la key IZQUIERDA:
           Step   -> valor izquierdo,
           Linear -> lerp,
           Smooth -> lerp con smoothstep(u) = u*u*(3-2u).
   - `keysInRange(t0, t1)` devuelve las keys con `t0 <= time <= t1`.
   ============================================================================= */
#include "../include/gui_util.hpp"
#include "../../include/test_framework.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

enum class Interp { Step, Linear, Smooth };

struct Key {
    float tiempo = 0.0f;
    float valor = 0.0f;
    Interp interp = Interp::Linear;
};

class Curve {
public:
    bool addKey(Key k) {
        // TODO
        (void)k;
        return false;
    }

    bool removeKey(float tiempo) {
        // TODO
        (void)tiempo;
        return false;
    }

    float evaluate(float t) const {
        // TODO
        (void)t;
        return 0.0f;
    }

    std::vector<Key> keysInRange(float t0, float t1) const {
        // TODO
        (void)t0;
        (void)t1;
        return {};
    }

    std::size_t size() const { return keys_.size(); }
    const std::vector<Key>& keys() const { return keys_; }

private:
    std::vector<Key> keys_;
};

// ============================================================================
// TESTS (no tocar)
// ============================================================================
TEST_CASE(CurvaVacia) {
    Curve c;
    ASSERT_EQ(c.size(), 0u);
    ASSERT_NEAR(c.evaluate(1.0f), 0.0f, 1e-6f);
    ASSERT_TRUE(c.keysInRange(0.0f, 10.0f).empty());
}

TEST_CASE(InsertaOrdenado) {
    Curve c;
    ASSERT_TRUE(c.addKey({2.0f, 20.0f, Interp::Linear}));
    ASSERT_TRUE(c.addKey({0.0f, 0.0f, Interp::Linear}));
    ASSERT_TRUE(c.addKey({1.0f, 10.0f, Interp::Linear}));
    ASSERT_EQ(c.size(), 3u);
    ASSERT_NEAR(c.keys()[0].tiempo, 0.0f, 1e-6f);
    ASSERT_NEAR(c.keys()[1].tiempo, 1.0f, 1e-6f);
    ASSERT_NEAR(c.keys()[2].tiempo, 2.0f, 1e-6f);
}

TEST_CASE(ReemplazaMismoTiempo) {
    Curve c;
    c.addKey({1.0f, 5.0f, Interp::Linear});
    ASSERT_TRUE(c.addKey({1.0f, 9.0f, Interp::Step}));
    ASSERT_EQ(c.size(), 1u);
    ASSERT_NEAR(c.evaluate(1.0f), 9.0f, 1e-6f);
}

TEST_CASE(ClampeaFueraDeRango) {
    Curve c;
    c.addKey({1.0f, 10.0f, Interp::Linear});
    c.addKey({3.0f, 30.0f, Interp::Linear});
    ASSERT_NEAR(c.evaluate(-5.0f), 10.0f, 1e-6f);
    ASSERT_NEAR(c.evaluate(100.0f), 30.0f, 1e-6f);
}

TEST_CASE(InterpolacionStepLinealSmooth) {
    Curve step;
    step.addKey({0.0f, 0.0f, Interp::Step});
    step.addKey({1.0f, 10.0f, Interp::Step});
    ASSERT_NEAR(step.evaluate(0.75f), 0.0f, 1e-6f);

    Curve lin;
    lin.addKey({0.0f, 0.0f, Interp::Linear});
    lin.addKey({1.0f, 10.0f, Interp::Linear});
    ASSERT_NEAR(lin.evaluate(0.25f), 2.5f, 1e-6f);

    Curve smooth;
    smooth.addKey({0.0f, 0.0f, Interp::Smooth});
    smooth.addKey({1.0f, 10.0f, Interp::Smooth});
    ASSERT_NEAR(smooth.evaluate(0.5f), 5.0f, 1e-6f);   // smoothstep(0.5)=0.5
    ASSERT_NEAR(smooth.evaluate(0.25f), 1.5625f, 1e-6f);
}

TEST_CASE(UsaElModoDeLaKeyIzquierda) {
    Curve c;
    c.addKey({0.0f, 0.0f, Interp::Smooth});
    c.addKey({1.0f, 10.0f, Interp::Step});
    c.addKey({2.0f, 20.0f, Interp::Linear});
    ASSERT_NEAR(c.evaluate(0.5f), 5.0f, 1e-6f);   // tramo 0-1: smooth
    ASSERT_NEAR(c.evaluate(1.5f), 10.0f, 1e-6f);  // tramo 1-2: step de la key 1
}

TEST_CASE(RemoveKey) {
    Curve c;
    c.addKey({1.0f, 1.0f, Interp::Linear});
    ASSERT_TRUE(c.removeKey(1.0f));
    ASSERT_FALSE(c.removeKey(1.0f));
    ASSERT_EQ(c.size(), 0u);
}

TEST_CASE(KeysInRangeInclusivo) {
    Curve c;
    c.addKey({0.0f, 0.0f, Interp::Linear});
    c.addKey({1.0f, 1.0f, Interp::Linear});
    c.addKey({2.0f, 2.0f, Interp::Linear});
    const auto k = c.keysInRange(1.0f, 2.0f);
    ASSERT_EQ(k.size(), 2u);
    ASSERT_NEAR(k[0].tiempo, 1.0f, 1e-6f);
    ASSERT_NEAR(k[1].tiempo, 2.0f, 1e-6f);
}

int main() {
    std::cout << "Ejercicio 07 (curve editor)...\n";
    return RUN_ALL_TESTS();
}
