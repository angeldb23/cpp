/* =============================================================================
   MÓDULO 17 - EJERCICIO 10: ParticlePool — Swap-Remove y Emisión por Segundo
   =============================================================================
   DIFICULTAD: Avanzado
   APOYO: Lección 08 (Partículas: pool, emitter, lifetime)

   CONSIGNA:
   1) ParticlePool::Spawn(): crea en el primer índice LIBRE tras "aliveCount";
      si el pool está lleno devuelve nullptr (NO crece en runtime).
   2) ParticlePool::Kill(i): swap-remove: copia la ÚLTIMA viva al hueco i y
      reduce aliveCount. Los índices留 del final quedan reciclables.
   3) ParticlePool::Update(dt): avanza age; la partícula con age >= lifetime
      se elimina: el swap-remove debe convivir con el recorrido (no saltar
      vivas ni caer del array, ver lección 08).
   4) ParticleEmitter::Emit(dt, rate, pool): acumulador: rate*dt se suma;
      por cada hit de 1 del acumulador, spawn con lifetime. Sin fracción
      perdida ("rate 60 con dt 1/60 debe generar EXACTAMENTE 60 en 60 frames").
      spawned cuenta UNICAMENTE spawns exitosos (pool lleno no suma).

   TESTS (no tocar): pool lleno, swap-remove desplaza índices, la emisión
   acumulada determinista y la muerte por lifetime.

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_10_particle_pool.cpp -o ej && ./ej
   ============================================================================= */
#include <vector>
#include <cstddef>
#include <iostream>
#include "../../include/test_framework.hpp"

// ============================ TIPOS (no tocar) ===============================
struct Particle {
    int id = -1;         // para rastrear en tests (índice de spawn)
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

    // TODO 1: spawn en el primer hueco, nullptr si lleno
    Particle* Spawn(int id) {
        (void)id;
        // IMPORTANTE: el placeholder NO puede devolver nullptr directamente:
        // los tests desreferencian el puntero (a->lifetime) y crashearian.
        // Devuelve una partícula dummy "siempre viva": los tests fallan por
        // aserción (aliveCount que no crece), no por segfault.
        static Particle dummy;
        return &dummy;   // TODO: implementar el spawn real
    }

    // TODO 2: swap-remove (la última viva pasa al hueco i)
    void Kill(long i) {
        (void)i;
    }

    // TODO 3: integración + aging + muerte (sin crash del bucle al matar)
    void Update(float dt) {
        (void)dt;
    }
};

class ParticleEmitter {
    float accumulator_ = 0.0f;
public:
    long spawned = 0;

    // TODO 4: emisión por segundo con acumulador determinista
    void Emit(ParticlePool& pool, float dt, float rate, int idBase) {
        (void)pool; (void)dt; (void)rate; (void)idBase;
    }
};

// ============================ TESTS (no tocar) ==============================
TEST_CASE(SpawnHastaLlenarYNull) {
    ParticlePool pool(3);
    ASSERT_TRUE(pool.Spawn(10) != nullptr);
    ASSERT_TRUE(pool.Spawn(11) != nullptr);
    ASSERT_TRUE(pool.Spawn(12) != nullptr);
    ASSERT_EQ(pool.AliveCount(), 3);
    ASSERT_TRUE(pool.Spawn(13) == nullptr);   // lleno: NO crece
}

TEST_CASE(SwapRemoveMueveLaUltima) {
    ParticlePool pool(4);
    pool.Spawn(1); pool.Spawn(2); pool.Spawn(3);
    pool.Kill(1);                     // mata (2): la última (3) viene AL HUECO 1
    ASSERT_EQ(pool.AliveCount(), 2);
    ASSERT_EQ(pool.At(0).id, 1);
    ASSERT_EQ(pool.At(1).id, 3);      // swap-remove real (la 2 desapareció)
    // el índice 2 está libre para el próximo spawn
    Particle* p = pool.Spawn(99);
    ASSERT_TRUE(p != nullptr);
    ASSERT_EQ(pool.AliveCount(), 3);
}

TEST_CASE(UpdateMataPorLifetime) {
    ParticlePool pool(2);
    Particle* a = pool.Spawn(1); a->lifetime = 0.5f;
    Particle* b = pool.Spawn(2); b->lifetime = 1.0f;
    // 6 frames de 0.1: a cumple 0.5 en frame 5; b sigue
    const float dt = 0.1f;
    for (int f = 0; f < 6; ++f) pool.Update(dt);
    ASSERT_EQ(pool.AliveCount(), 1);
    ASSERT_EQ(pool.At(0).id, 2);      // swap-remove: b quedó en el hueco 0
}

TEST_CASE(EmitRateAcumuladorDeterminista) {
    ParticlePool pool(100);
    ParticleEmitter em;
    const float rate = 60.0f;         // 60/s
    const float dt = 1.0f / 60.0f;    // timestep fijo
    for (int f = 0; f < 60; ++f) em.Emit(pool, dt, rate, f);
    ASSERT_EQ(em.spawned, 60);        // 60 frames => EXACTAMENTE 60 spawns
    ASSERT_EQ(pool.AliveCount(), 60);
    // vida 1.0 default: sigue vivas al final del segundo
}

TEST_CASE(EmitRateFraccionalNoPiedeParticulas) {
    // rate 30 con frame de 33 ms: 30*0.033 = 0.99 no llega a 1. El acumulador
    // GUARDA la fracción y las emisiones salen cada 2 frames con 1 extra...:
    // en 60 frames el acumulador suma 59.4 => EXACTAMENTE 59 spawns.
    ParticlePool pool(250);
    ParticleEmitter em;
    const float rate = 30.0f;
    const float dt = 0.033f;
    for (int f = 0; f < 60; ++f) em.Emit(pool, dt, rate, f);
    ASSERT_EQ(em.spawned, 59);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 10 (ParticlePool)...\n";
    return RUN_ALL_TESTS();
}
