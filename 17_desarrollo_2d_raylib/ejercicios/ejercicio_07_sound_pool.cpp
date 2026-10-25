/* =============================================================================
   MÓDULO 17 - EJERCICIO 07: SoundPool — Vozes del Pool con Robo del Más Viejo
   =============================================================================
   DIFICULTAD: Intermedio
   APOYO: Lección 05 (Audio: pooling de sonidos)

   CONSIGNA:
   Implementa SoundPool con N voces (4 en tests):

   - Acquire(): devuelve la primera voz LIBRE (índice menor primero) y la
     marca en uso. Si NO hay libres: roba la voz 0 (la "más vieja" en la
     cola de uso real: aproximación circular) y la marca como "robada"
     (stealCount se incrementa — así el test puede verificar el robo).
   - Release(index): libera la voz. Release de algo ya libre: no-op.
   - CountInUse(): cuántas voces hay marcadas en uso.

   El "steal" es la estrategia del juego real: SFX jump en el frame 5 le
   robó la voz al jump del frame 0 (sonaba hace 90ms, mejor interrumpirlo
   que silenciar el nuevo).

   TESTS (no tocar): acquire secuencial, robo al llenar, release/reuse,
   release doble sin romper el conteo.

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_07_sound_pool.cpp -o ej && ./ej
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
    int stealCount = 0;    // cuántos robos hizo (para verificar el modo robo)

    explicit SoundPool(size_t n) : voices_(n) {}

    std::size_t Size() const { return voices_.size(); }

    // TODO 1: obtiene una voz libre (la menor primera) o roba la 0
    int Acquire() {
        // Pistas:
        // 1) recorro voices_ en orden: la primera !inUse => marco y devuelvo índice
        // 2) si no hay: marco voz 0 en uso (robo), ++stealCount, devuelvo 0
        // 3) pool de tamaño 0: devolver -1
        return -1;   // placeholder
    }

    // TODO 2: libera una voz (índice); no-op si estaba ya libre
    void Release(int index) {
    }

    // TODO 3: cuántas voces hay en uso
    int CountInUse() const {
        return 0;   // placeholder
    }
};

// ============================ TESTS (no tocar) ==============================
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
    int voice = pool.Acquire();     // lleno: ROBO la 0
    ASSERT_EQ(voice, 0);
    ASSERT_EQ(pool.stealCount, 1);
    ASSERT_EQ(pool.CountInUse(), 4);   // sigue lleno (todo sigue en uso)
}

TEST_CASE(ReleaseYReutilizacion) {
    SoundPool pool(4);
    for (int i = 0; i < 4; ++i) pool.Acquire();
    pool.Release(2);
    ASSERT_EQ(pool.CountInUse(), 3);
    int voice = pool.Acquire();      // libre: 2 (NO robo)
    ASSERT_EQ(voice, 2);
    ASSERT_EQ(pool.stealCount, 0);   // sin robo: la voz estaba libre
}

TEST_CASE(ReleaseDobleNoRompeConteo) {
    SoundPool pool(4);
    pool.Acquire();
    pool.Acquire();
    pool.Release(0);
    pool.Release(0);   // doble release: no-op (no reset sin ser relevante)
    ASSERT_EQ(pool.CountInUse(), 1);
    // Recicla igual: 0 libre desde el primer release
    ASSERT_EQ(pool.Acquire(), 0);
}

TEST_CASE(PoolVacioDevuelveMenosUno) {
    SoundPool pool(0);
    ASSERT_EQ(pool.Acquire(), -1);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 07 (SoundPool)...\n";
    return RUN_ALL_TESTS();
}
