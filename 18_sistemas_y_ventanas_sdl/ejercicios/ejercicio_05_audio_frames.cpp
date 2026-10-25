/* =============================================================================
   MÓDULO 18 - EJERCICIO 05: Matemática de audio: frames, bytes y callbacks
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 08 (audio nativo: SDL_AudioSpec/streams/latencia)

   CONSIGNA:
   En SDL el audio está medido en FRAMES (un frame = un sample por canal).
   Completá la librería de audio lógica (SIN SDL, TODO determinista):

     struct Spec { int freq; int canales; int bytesSample; }
       - freq: frames por segundo (lección 08)
       - bytesSample: bytes por sample (F32=4, S16=2, U8=1)

     1. int bytesFrame(spec): canales * bytesSample
     2. int bytesPorFrecuencia(spec, segundos): duración -> bytes EXACTOS
        (truncado de frames fraccionarios: lo que no completa un frame
        se descartan — mismo criterio SDL)
     3. double frecuenciaPorBytes(spec, bytes): bytes -> segundos
     4. int callbackPasa(bytes_disponibles, bytes_necesarios): el callback
        del stream pide N bytes por vuelta; devuelvé CUANTOS LE PASAMOS:
        min(disponibles, necesarios), PERO SIEMPRE múltiplo de bytesFrame
        (medio frame no se pasa: descartar el restante si es < 1 frame).
     5. int resampleoFrecuencia(specFuente, specDestino, framesFuente):
        cuántos frames produce al pasar de una freq a otra (truncado),
        manteniendo la misma DURACIÓN en tiempo real.
        Ejemplo: 44100 Hz a 48000 Hz, 44100 frames -> 48000 frames.
        (en la vida real lo hace SDL_AudioStream, la matemática es esta)

   TESTS (no tocar). COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra ejercicio_05_audio_frames.cpp -o ej && ./ej
   ============================================================================= */
#include <algorithm>
#include "../../include/test_framework.hpp"

// ============================ MODELO (no tocar) ==============================
struct Spec {
    int freq        = 48000; // frames/segundo
    int canales     = 2;     // 1 mono, 2 stereo (SDL_AudioSpec)
    int bytesSample = 4;     // F32 = 4 bytes (SDL_AUDIO_F32)
};

// ============================ TU CÓDIGO AQUÍ ================================
// TODO 1: bytes de UN frame
static int bytesFrame(const Spec& s) {
    (void)s;
    return 0;
}

// TODO 2: duración -> bytes (truncando frames fraccionarios)
static int bytesPorFrecuencia(const Spec& s, float segundos) {
    (void)s; (void)segundos;
    return 0;
}

// TODO 3: bytes -> segundos (double, con precisión de frame)
static double frecuenciaPorBytes(const Spec& s, int bytes) {
    (void)s; (void)bytes;
    return 0.0;
}

// TODO 4: cuántos bytes le pasamos al callback (múltiplo de frame)
static int callbackPasa(int disponibles, int necesarios, int bytesFrame_) {
    (void)disponibles; (void)necesarios; (void)bytesFrame_;
    return 0;
}

// TODO 5: frames después de resamplear (mismo tiempo, truncado)
static int resampleoFrecuencia(const Spec& origen, const Spec& destino, int framesOrigen) {
    (void)origen; (void)destino; (void)framesOrigen;
    return 0;
}

// ============================ TESTS (no tocar) ==============================
TEST_CASE(BytesDeFrameYDuraciones) {
    Spec estereo{48000, 2, 4};
    ASSERT_EQ(bytesFrame(estereo), 8);            // 2 canales * 4 bytes
    ASSERT_EQ(bytesPorFrecuencia(estereo, 1.0f), 48000 * 8);
    ASSERT_EQ(bytesPorFrecuencia(estereo, 0.5f), 48000 / 2 * 8);
    // truncado: 0.0001 s a mono S16 -> 4.8 frames -> 4 frames -> 8 bytes
    Spec monoS16{48000, 1, 2};
    ASSERT_EQ(bytesPorFrecuencia(monoS16, 0.0001f), 8);   // 4 frames x 2 bytes
    ASSERT_EQ(bytesPorFrecuencia(monoS16, 0.00001f), 0);  // 0.48 frames: ni 1
}

TEST_CASE(BytesASegundosPreciso) {
    Spec estereo{48000, 2, 4};
    ASSERT_NEAR(frecuenciaPorBytes(estereo, 0), 0.0, 1e-9);
    ASSERT_NEAR(frecuenciaPorBytes(estereo, 48000 * 8), 1.0, 1e-6);
    // un frame sola: 1/48000 s
    ASSERT_NEAR(frecuenciaPorBytes(estereo, 8), 1.0 / 48000.0, 1e-9);
}

TEST_CASE(CallbackConClampYAlineadoDeFrame) {
    Spec s{48000, 2, 4};
    const int bf = bytesFrame(s);   // 8
    ASSERT_EQ(callbackPasa(1000, 512, bf), 512);  // min(1000,512) es múltiplo de 8
    ASSERT_EQ(callbackPasa(32, 512, bf), 32);     // disponibles: 4 frames = 32
    ASSERT_EQ(callbackPasa(8, 512, bf), 8);
    ASSERT_EQ(callbackPasa(3, 512, bf), 0);       // ni 1 frame: 0 bytes
}

TEST_CASE(ResampleoMantieneDuracion) {
    Spec forty44{44100, 2, 4}, forty48{48000, 2, 4};
    // 1 s de audio: 44100 frames -> 48000 frames al pasar de 44.1 a 48 kHz
    ASSERT_EQ(resampleoFrecuencia(forty44, forty48, 44100), 48000);
    ASSERT_EQ(resampleoFrecuencia(forty48, forty44, 48000), 44100);
    ASSERT_EQ(resampleoFrecuencia(forty48, forty48, 1000), 1000);
    // truncado: 3200 frames de 44.1k a 48k = 3482.1 -> 3482
    ASSERT_EQ(resampleoFrecuencia(forty44, forty48, 3200), 3482);
}

int main() { return RUN_ALL_TESTS(); }
