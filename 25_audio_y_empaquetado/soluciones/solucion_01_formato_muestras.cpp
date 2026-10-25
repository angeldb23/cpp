/* =============================================================================
   MÓDULO 25 - SOLUCIÓN 01: matemática de formatos de muestras
   =============================================================================
   Lo que hacía falta entender:
   - Un "frame" = UNA muestra POR CANAL. bytesPorFrame lo encapsula la clase.
   - s16 -> f32 divide por 32768 (así -32768 mapea EXACTO a -1).
   - f32 -> s16 primero CLAMP a [-1, +1] y después cast (el cast de un float
     fuera de rango a int16 sería UB: el clamp es hardware-friendly y está
     "invisible" al dueño: es la razón de la lección).
   - dB de amplitud: 20*log10; bajo 0 no hay log válido: -100 dB es
     "silencio en pantalla" (el motor NO debe consultar NaN).
   ============================================================================= */
#include "../include/f25.hpp"
#include "../../include/test_framework.hpp"

#include <cmath>
#include <cstdint>

class AudioFmt {
public:
    static std::size_t framesDe(std::size_t bytes, std::size_t bytesPorFrame) {
        return bytesPorFrame == 0 ? 0 : bytes / bytesPorFrame;
    }

    static std::size_t bytesDe(std::size_t frames, std::size_t bytesPorFrame) {
        return frames * bytesPorFrame;
    }

    static float s16AF32(std::int16_t x) {
        return static_cast<float>(x) / 32768.0f;
    }

    static std::int16_t f32AS16(float x) {
        /* Clamp al rango REPRESENTABLE antes del cast: el techo es
           32767/32768 (porque +1.0 * 32768 = 32768, que NO entra en
           int16 y en C++20 el cast desbordado es modular: volvería
           como -32768). */
        float c = x;
        if (c > 32767.0f / 32768.0f) { c = 32767.0f / 32768.0f; }
        if (c < -1.0f) { c = -1.0f; }
        return static_cast<std::int16_t>(c * 32768.0f);
    }

    static float duracionSegundos(std::size_t frames, std::uint32_t sampleRate) {
        if (sampleRate == 0) {
            return 0.0f;
        }
        return static_cast<float>(frames) / static_cast<float>(sampleRate);
    }

    static float dBDe(float amplitud) {
        if (amplitud <= 0.0f) {
            return -100.0f;
        }
        return 20.0f * std::log10(amplitud);
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
