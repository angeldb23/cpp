/* =============================================================================
   MÓDULO 17 - SOLUCIÓN 07: SoundPool — Vozes del Pool con Robo del Más Viejo
   =============================================================================
   Resuelve ejercicio_07_sound_pool.cpp. El robo de la voz 0 es una
   aproximación circular a "la más vieja"; la variante fina en raylib 5.5 es
   reutilizar primero la voz cuyo IsSoundPlaying() sea false (voz terminada,
   no hay que interrumpir nada) antes de robar una activa.

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra solucion_07_sound_pool.cpp -o sol && ./sol
   ============================================================================= */
#include <vector>
#include <cstddef>
#include <iostream>
#include "../../include/test_framework.hpp"

// ============================ ESTRUCTURA (no tocar) ==========================
struct Voice {
    bool inUse = false;
};

class SoundPool {
    std::vector<Voice> voices_;
public:
    int stealCount = 0;

    explicit SoundPool(size_t n) : voices_(n) {}

    std::size_t Size() const { return voices_.size(); }

    // TODO 1: solución
    int Acquire() {
        if (voices_.empty()) return -1;
        // (1) la primera voz libre: orden menor primero
        for (std::size_t i = 0; i < voices_.size(); ++i) {
            if (!voices_[i].inUse) {
                voices_[i].inUse = true;
                return int(i);
            }
        }
        // (2) lleno: ROBO la voz 0 y cuenta el robo (no hay free, pero la 0
        // era la más "vieja" de la cola de uso circular)
        voices_[0].inUse = true;
        ++stealCount;
        return 0;
    }

    // TODO 2: solución: el guard de inUse hace idempotente al Release
    void Release(int index) {
        if (index < 0 || index >= int(voices_.size())) return;
        voices_[std::size_t(index)].inUse = false;
    }

    // TODO 3: solución
    int CountInUse() const {
        int n = 0;
        for (const auto& v : voices_) if (v.inUse) ++n;
        return n;
    }
};

// ============================ TESTS (iguales) ================================
TEST_CASE(AcquireSecuencialDevuelveMenorIndice) {
    SoundPool pool(4);
    ASSERT_EQ(pool.Acquire(), 0);
    ASSERT_EQ(pool.Acquire(), 1);
    ASSERT_EQ(pool.Acquire(), 2);
    ASSERT_EQ(pool.Acquire(), 3);
    ASSERT_EQ(pool.CountInUse(), 4);
}

TEST_CASE(RoboAlLlenarMueveVozCero) {
    SoundPool pool(4);
    for (int i = 0; i < 4; ++i) pool.Acquire();
    ASSERT_EQ(pool.stealCount, 0);
    int voice = pool.Acquire();
    ASSERT_EQ(voice, 0);
    ASSERT_EQ(pool.stealCount, 1);
    ASSERT_EQ(pool.CountInUse(), 4);
}

TEST_CASE(ReleaseYReutilizacion) {
    SoundPool pool(4);
    for (int i = 0; i < 4; ++i) pool.Acquire();
    pool.Release(2);
    ASSERT_EQ(pool.CountInUse(), 3);
    int voice = pool.Acquire();
    ASSERT_EQ(voice, 2);
    ASSERT_EQ(pool.stealCount, 0);
}

TEST_CASE(ReleaseDobleNoRompeConteo) {
    SoundPool pool(4);
    pool.Acquire();
    pool.Acquire();
    pool.Release(0);
    pool.Release(0);
    ASSERT_EQ(pool.CountInUse(), 1);
    ASSERT_EQ(pool.Acquire(), 0);
}

TEST_CASE(PoolVacioDevuelveMenosUno) {
    SoundPool pool(0);
    ASSERT_EQ(pool.Acquire(), -1);
}

int main() {
    std::cout << "Solución 07: SoundPool\n";
    return RUN_ALL_TESTS();
}
