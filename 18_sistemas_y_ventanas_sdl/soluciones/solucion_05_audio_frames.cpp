/* =============================================================================
   MÓDULO 18 - SOLUCIÓN 05: Matemática de audio: frames, bytes y callbacks
   =============================================================================
   MISMO ENUNCIADO que ejercicios/ejercicio_05_audio_frames.cpp.
   SOLUCIÓN COMENTADA: qué hace cada pieza y por qué.

   PUNTOS CLAVE:
   - bytesPorFrecuencia: la cuenta es frames = int(freq * segundos) con
     TRUNCADO (no redondeo): si no completa el frame entero, no cuenta
     (lección 08: el stream procesa frames, no "mitades").
   - callbackPasa: los bytes al stream SIEMPRE en múltiplos de frameBytes
     (un medio frame no puede mezclarse: si no llega al frame entero, no
     se entrega — el stream interno de SDL ya lo hace así).
   - resampleoFrecuencia: framesDestino = framesOrigen * (dest.freq/orig.freq)
     con truncado; la duración en REAL TIME queda idéntica, y el número
     base es la RATIO de frecuencias (cuentas con doubles, trunc al final).
   ============================================================================= */
#include <algorithm>
#include "../../include/test_framework.hpp"

// ============================ MODELO (no tocar) ==============================
struct Spec {
    int freq        = 48000; // frames/segundo
    int canales     = 2;     // 1 mono, 2 stereo (SDL_AudioSpec)
    int bytesSample = 4;     // F32 = 4 bytes (SDL_AUDIO_F32)
};

// ============================ SOLUCIÓN ======================================
// 1. bytes de un frame (canales * bytesSample)
static int bytesFrame(const Spec& s) {
    return s.canales * s.bytesSample;
}

// resuelto 2: duración -> bytes truncando frames fraccionarios
static int bytesPorFrecuencia(const Spec& s, float segundos) {
    // frames = int(freq * segundos): trunc (no round: no cuenta fracciones)
    const int frames = int(float(s.freq) * segundos);
    return frames * bytesFrame(s);
}

// resuelto 3: bytes -> segundos (double para no perder en frame único)
static double frecuenciaPorBytes(const Spec& s, int bytes) {
    const double bytesPorSegundo = double(s.freq) * double(bytesFrame(s));
    return double(bytes) / bytesPorSegundo;
}

// resuelto 4: bytes pasados al callback: min(disponibles, necesarios)
static int callbackPasa(int disponibles, int necesarios, int bytesFrame_) {
    if (bytesFrame_ <= 0) return 0;                 // defensivo: frame "0"?
    const int querido = std::min(disponibles, necesarios);
    return (querido / bytesFrame_) * bytesFrame_;    // MÚLTIPLO de frame entero
}

// resuelto 5: frames destino tras el resampleo (duración constante)
static int resampleoFrecuencia(const Spec& origen, const Spec& destino, int framesOrigen) {
    // destino = origen * (dest.freq / orig.freq)
    // con doubles para no perder en intermedios; trunc al final
    const double ratio = double(destino.freq) / double(origen.freq);
    return int(double(framesOrigen) * ratio);
}

// ============================ TESTS (no tocar) ==============================
TEST_CASE(BytesDeFrameYDuraciones) {
    Spec estereo{48000, 2, 4};
    ASSERT_EQ(bytesFrame(estereo), 8);            // 2 canales * 4 bytes
    ASSERT_EQ(bytesPorFrecuencia(estereo, 1.0f), 48000 * 8);
    ASSERT_EQ(bytesPorFrecuencia(estereo, 0.5f), 48000 / 2 * 8);
    // truncado: 0.0001 s a mono S16 -> 4.8 frames -> 4 frames -> 8 bytes
    Spec monoS16{48000, 1, 2};
    ASSERT_EQ(bytesPorFrecuencia(monoS16, 0.0001f), 8);
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
    ASSERT_EQ(callbackPasa(32, 512, bf), 32);
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
