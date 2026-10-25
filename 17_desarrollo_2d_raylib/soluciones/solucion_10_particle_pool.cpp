/* =============================================================================
   MÓDULO 17 - SOLUCIÓN 10: ParticlePool — Swap-Remove y Emisión por Segundo
   =============================================================================
   Resuelve ejercicio_10_particle_pool.cpp. El swap-remove es O(1) y no tumba
   el recorrido del Update (al matar en el índice i, la última viva viene a
   replacearlo; el mismo índice i se revisa OTRA VEZ con esa nueva residente:
   por eso NO se hace ++i tras un Kill).

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_10_particle_pool.cpp -o sol && ./sol
   ============================================================================= */
#include <vector>
#include <cstddef>
#include <iostream>
#include "../../include/test_framework.hpp"

// ============================ TIPOS (no tocar) ===============================
struct Particle {
    int id = -1;
    float age = 0.0f;
    float lifetime = 1.0f;
};

class ParticlePool {
    std::vector<Particle> pool_;
    long aliveCount_ = 0;

public:
    explicit ParticlePool(size_t max) { pool_.resize(max); }

    long AliveCount() const { return aliveCount_; }
    Particle& At(long i) { return pool_[size_t(i)]; }

    // TODO 1: solución — spawn en el primer hueco (detrás de aliveCount_)
    Particle* Spawn(int id) {
        if (aliveCount_ >= long(pool_.size())) return nullptr;
        Particle& p = pool_[size_t(aliveCount_)];
        ++aliveCount_;
        p = Particle{};           // limpia el hueco reciclado
        p.id = id;
        return &p;
    }

    // TODO 2: solución — swap-remove O(1)
    void Kill(long i) {
        if (i < 0 || i >= aliveCount_) return;
        if (i != aliveCount_ - 1) {
            pool_[size_t(i)] = pool_[size_t(aliveCount_) - 1];  // la última viva al hueco
        }
        --aliveCount_;
        // El hueco del final queda fuera del rango activo: se reescribe al
        // spawnearse de nuevo.
    }

    // TODO 3: solución — el bucle revisa índices; n++ solo cuando SOBREVIVE
    void Update(float dt) {
        for (long i = 0; i < aliveCount_;) {
            Particle& p = pool_[size_t(i)];
            p.age += dt;
            if (p.age >= p.lifetime) {
                Kill(i);    // el swap trae OTRA partícula a este índice:
                // no i++ → el próximo giro la revisa (edad y vida fresca)
            } else {
                ++i;        // región revisada: seguimos con la siguiente
            }
        }
    }
};

class ParticleEmitter {
    float accumulator_ = 0.0f;
public:
    long spawned = 0;

    // TODO 4: solución — acumulador determinista (lección 08)
    void Emit(ParticlePool& pool, float dt, float rate, int idBase) {
        accumulator_ += rate * dt;
        while (accumulator_ >= 1.0f) {
            accumulator_ -= 1.0f;           // conserva la fracción
            if (pool.Spawn(idBase + int(spawned)) != nullptr) {
                ++spawned;                   // SOLO cuenta spawns exitosos
            }
            // pool lleno: no cuenta PERO sigue drenando el acumulador
        }
    }
};

// ============================ TESTS (iguales) ================================
TEST_CASE(SpawnHastaLlenarYNull) {
    ParticlePool pool(3);
    ASSERT_TRUE(pool.Spawn(10) != nullptr);
    ASSERT_TRUE(pool.Spawn(11) != nullptr);
    ASSERT_TRUE(pool.Spawn(12) != nullptr);
    ASSERT_EQ(pool.AliveCount(), 3);
    ASSERT_TRUE(pool.Spawn(13) == nullptr);
}

TEST_CASE(SwapRemoveMueveLaUltima) {
    ParticlePool pool(4);
    pool.Spawn(1); pool.Spawn(2); pool.Spawn(3);
    pool.Kill(1);
    ASSERT_EQ(pool.AliveCount(), 2);
    ASSERT_EQ(pool.At(0).id, 1);
    ASSERT_EQ(pool.At(1).id, 3);
    Particle* p = pool.Spawn(99);
    ASSERT_TRUE(p != nullptr);
    ASSERT_EQ(pool.AliveCount(), 3);
}

TEST_CASE(UpdateMataPorLifetime) {
    ParticlePool pool(2);
    Particle* a = pool.Spawn(1); a->lifetime = 0.5f;
    Particle* b = pool.Spawn(2); b->lifetime = 1.0f;
    const float dt = 0.1f;
    for (int f = 0; f < 6; ++f) pool.Update(dt);
    ASSERT_EQ(pool.AliveCount(), 1);
    ASSERT_EQ(pool.At(0).id, 2);
}

TEST_CASE(EmitRateAcumuladorDeterminista) {
    ParticlePool pool(100);
    ParticleEmitter em;
    const float rate = 60.0f;
    const float dt = 1.0f / 60.0f;
    for (int f = 0; f < 60; ++f) em.Emit(pool, dt, rate, f);
    ASSERT_EQ(em.spawned, 60);
    ASSERT_EQ(pool.AliveCount(), 60);
}

TEST_CASE(EmitRateFraccionalNoPiedeParticulas) {
    // 30*0.033 = 0.99 no llega a 1: el acumulador conserva la fracción
    // y la 59ª emisión usa 58 frames de sobra acumulados
    ParticlePool pool(250);
    ParticleEmitter em;
    const float rate = 30.0f;
    const float dt = 0.033f;
    for (int f = 0; f < 60; ++f) em.Emit(pool, dt, rate, f);
    ASSERT_EQ(em.spawned, 59);
}

int main() {
    std::cout << "Solución 10: ParticlePool\n";
    return RUN_ALL_TESTS();
}
