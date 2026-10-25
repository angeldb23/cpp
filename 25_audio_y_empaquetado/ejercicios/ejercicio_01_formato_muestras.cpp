/* =============================================================================
   MÓDULO 25 - EJERCICIO 01: matemática de formatos de muestras
   =============================================================================
   DIFICULTAD: Fácil
   APOYO: Lecciones 01 y 02 (formatos, f32 <-> s16, frames y duración).

   CONSIGNA
   --------
   Completá `AudioFmt`:
   1. `framesDe(bytes, bytesPorFrame)`: cantidad de frames en un buffer.
   2. `bytesDe(frames, bytesPorFrame)`: tamaño del buffer en bytes.
   3. `s16AF32(x)`: muestra int16 -> float en [-1, +1). La conversión
      EXACTA es: x / 32768.0f (el -32768 queda -1 exacto, el 32767 queda
      0.99997).
   4. `f32AS16(x)`: float -> int16. Recortar el float a [-1, +1] ANTES
      (con -1 exacto = -32768; +1 se recorta a 32767) y devolver
      static_cast<int16>(x * 32768.0f) del valor recortado.
   5. `duracionSegundos(frames, sampleRate)`: frames / sampleRate (float).
   6. `dBDe(amplitud)`: 20 * log10(amplitud) con amplitud > 0.
      Si amplitud <= 0, devolver -100.0f (silencio/absurdo guard).
   ============================================================================= */
#include "../include/f25.hpp"
#include "../../include/test_framework.hpp"

#include <cstdint>

class AudioFmt {
public:
    static std::size_t framesDe(std::size_t bytes, std::size_t bytesPorFrame) {
        // TODO
        (void)bytes;
        (void)bytesPorFrame;
        return 0;
    }

    static std::size_t bytesDe(std::size_t frames, std::size_t bytesPorFrame) {
        // TODO
        (void)frames;
        (void)bytesPorFrame;
        return 0;
    }

    static float s16AF32(std::int16_t x) {
        // TODO
        (void)x;
        return 0.0f;
    }

    static std::int16_t f32AS16(float x) {
        // TODO
        (void)x;
        return 0;
    }

    static float duracionSegundos(std::size_t frames, std::uint32_t sampleRate) {
        // TODO
        (void)frames;
        (void)sampleRate;
        return 0.0f;
    }

    static float dBDe(float amplitud) {
        // TODO
        (void)amplitud;
        return 0.0f;
    }
};

/* ------------------------------ TESTS (no tocar)-------------------------- */
TEST_CASE(FrameMath) {
    ASSERT_TRUE(AudioFmt::framesDe(960, 4) == 240);
    ASSERT_TRUE(AudioFmt::framesDe(960, 8) == 120);
    ASSERT_TRUE(AudioFmt::bytesDe(12345, 4) == 49380);
    ASSERT_TRUE(AudioFmt::framesDe(0, 4) == 0);
}

TEST_CASE(S16aF32) {
    ASSERT_NEAR(AudioFmt::s16AF32(32767), 0.99997f, 0.0001f);
    ASSERT_NEAR(AudioFmt::s16AF32(-32768), -1.0f, 0.000001f);
    ASSERT_NEAR(AudioFmt::s16AF32(0), 0.0f, 0.000001f);
    ASSERT_NEAR(AudioFmt::s16AF32(-16000), -0.488281f, 0.0001f);
}

TEST_CASE(F32aS16) {
    ASSERT_TRUE(AudioFmt::f32AS16(1.0f) == 32767);   /* +1 se recorta */
    ASSERT_TRUE(AudioFmt::f32AS16(-1.0f) == -32768); /* -1 exacto    */
    ASSERT_TRUE(AudioFmt::f32AS16(-1.5f) == -32768); /* fuera de rango: clamp */
    ASSERT_TRUE(AudioFmt::f32AS16(1.5f) == 32767);
    ASSERT_TRUE(AudioFmt::f32AS16(0.5f) == 16384);
    ASSERT_TRUE(AudioFmt::f32AS16(0.0f) == 0);
}

TEST_CASE(DurationYdB) {
    ASSERT_NEAR(AudioFmt::duracionSegundos(48000, 48000), 1.0f, 0.0001f);
    ASSERT_NEAR(AudioFmt::duracionSegundos(1200, 48000), 0.025f, 0.0001f);
    ASSERT_NEAR(AudioFmt::dBDe(0.5f), -6.02f, 0.01f);
    ASSERT_NEAR(AudioFmt::dBDe(0.0f), -100.0f, 0.01f);
    ASSERT_NEAR(AudioFmt::dBDe(-0.5f), -100.0f, 0.01f);
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
