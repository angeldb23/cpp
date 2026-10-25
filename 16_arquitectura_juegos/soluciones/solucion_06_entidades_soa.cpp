/* =============================================================================
   MÓDULO 16 - SOLUCIÓN 06: Entidades SoA — Struct of Arrays con Swap-Remove
   ============================================================================= */
#include <vector>
#include <cstddef>
#include <utility>
#include "../../include/test_framework.hpp"

struct ParticleSoA {
    std::vector<float> x, y;
    std::vector<float> vx, vy;
    std::vector<float> life;
    std::vector<bool> active;
    size_t count = 0;

    void EnsureCapacity(size_t n) {
        if (n >= x.size()) {
            size_t newCap = std::max(n + 1, x.size() * 2);
            x.resize(newCap); y.resize(newCap);
            vx.resize(newCap); vy.resize(newCap);
            life.resize(newCap);
            active.resize(newCap);
        }
    }

    size_t Add(float px, float py, float pvx, float pvy, float plife) {
        EnsureCapacity(count + 1);
        x[count] = px; y[count] = py;
        vx[count] = pvx; vy[count] = pvy;
        life[count] = plife;
        active[count] = true;
        return count++;
    }

    void RemoveAt(size_t i) {
        size_t last = count - 1;
        if (i != last) {
            x[i] = x[last]; y[i] = y[last];
            vx[i] = vx[last]; vy[i] = vy[last];
            life[i] = life[last];
            active[i] = active[last];
        }
        count--;
    }

    void Update(float dt) {
        for (size_t i = 0; i < count; ) {
            life[i] -= dt;
            if (life[i] <= 0.0f) {
                RemoveAt(i);
            } else {
                x[i] += vx[i] * dt;
                y[i] += vy[i] * dt;
                ++i;
            }
        }
    }

    std::pair<float, float> GetPos(size_t id) const {
        if (id < count && active[id]) return {x[id], y[id]};
        return {0.0f, 0.0f};
    }

    size_t Count() const { return count; }
};

// ============= TESTS =============
TEST_CASE(AddReturnsSequentialIds) {
    ParticleSoA p;
    size_t id1 = p.Add(0, 0, 1, 0, 1.0f);
    size_t id2 = p.Add(10, 10, 0, 1, 2.0f);
    ASSERT_EQ(id1, 0u);
    ASSERT_EQ(id2, 1u);
    ASSERT_EQ(p.Count(), 2u);
}

TEST_CASE(UpdateMovesParticles) {
    ParticleSoA p;
    size_t id = p.Add(0, 0, 100, 50, 10.0f);
    p.Update(0.01f);
    auto [x, y] = p.GetPos(id);
    ASSERT_NEAR(x, 1.0f, 0.01f);
    ASSERT_NEAR(y, 0.5f, 0.01f);
}

TEST_CASE(LifeDecreases) {
    ParticleSoA p;
    size_t id = p.Add(0, 0, 0, 0, 0.05f);
    p.Update(0.01f);
    ASSERT_EQ(p.Count(), 1u);
    p.Update(0.05f);
    ASSERT_EQ(p.Count(), 0u);
}

TEST_CASE(SwapRemoveMaintainsCount) {
    ParticleSoA p;
    size_t a = p.Add(0, 0, 0, 0, 0.1f);
    size_t b = p.Add(10, 10, 0, 0, 0.01f);
    size_t c = p.Add(20, 20, 0, 0, 0.1f);
    ASSERT_EQ(p.Count(), 3u);

    p.Update(0.02f);
    ASSERT_EQ(p.Count(), 2u);
}

TEST_CASE(GetPosReturnsCorrectData) {
    ParticleSoA p;
    size_t id = p.Add(123.5f, 456.7f, 0, 0, 1.0f);
    auto [x, y] = p.GetPos(id);
    ASSERT_NEAR(x, 123.5f, 0.01f);
    ASSERT_NEAR(y, 456.7f, 0.01f);
}

TEST_CASE(UpdateDoesNotCrashOnEmpty) {
    ParticleSoA p;
    p.Update(0.016f);
    ASSERT_EQ(p.Count(), 0u);
}

int main() {
    std::cout << "Solución 06 (Entidades SoA)...\n";
    return RUN_ALL_TESTS();
}