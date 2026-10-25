/* =============================================================================
   MÓDULO 18 - LECCIÓN 08: Audio nativo: SDL_AudioSpec, streams y latencia
   =============================================================================
   TEMA 5 de TEMAS.md: Audio nativo: SDL_AudioSpec, callback, conversión de
   formato y streaming (1 lección).

   CORRER A MANO (activa el audio de tu máquina; sin ventana)
     g++ -std=c++20 -Wall -Wextra leccion_08_audio_nativo.cpp \
         $(pkg-config --cflags --libs sdl3) -o leccion08 && ./leccion08
   (genera 2 segundos de bleep 440 Hz sinusoidal por el stream)

   EL MODELO SDL3 (verificado en SDL_audio.h 3.4.18):
     struct SDL_AudioSpec { SDL_AudioFormat format; int channels; int freq; }
     - freq: FRAMES por segundo (no bytes). Cada frame = samples para
       TODOS los canales de UN instante (2 canales = 2 samples = 1 frame).
     - format: SDL_AUDIO_F32 (float 32, el preferido), SDL_AUDIO_S16,
       SDL_AUDIO_S8, SDL_AUDIO_U8, ... El TAMAÑO de sample depende de format.
     - SDL_OpenAudioDeviceStream(devid, spec ingress, callback, userdata)
       -> SDL_AudioStream* que YA está playing-ready en el device: vos le
       metés datos (SDL_PutAudioStreamData(stream, buf, bytes)) y SDL LOS
       CONVIERTE (frecuencia/ formato/canales) para alimentar al device.
     - El callback (SDL_AudioStreamCallback) NO hace nada de conversión:
       corre en el THREAD del device y pide bytes cuando quedan pocos;
       dentro de él vos AGREGAS más (los streams son thread-safe por diseño).
     - SDL_SetAudioStreamFormat(stream, src_spec, dst_spec) redefine la
       conversión (cambiás canales o sample rate en caliente, sin reabrir).
   ============================================================================
   EL MODELO SDL2 (lo que reemplaza; cabecera SDL_audio.h 2.32.74):
     SDL_AudioSpec wanted{freq, format, channels, samples, callback,...}
     SDL_OpenAudioDevice(dev, 0, &wanted, &have, 0) -> ID; el callback llena
     un buffer 'stream' con 'len' bytes CADA VEZ que el device queda sin datos...
     TODO con formato del device en 'have': convertir vos TODO tu audio.
     SDL_MixAudio(dst, src, fmt, len, vol 0..SDL_MIX_MAXVOLUME=128).
   ------------------------------------------------------------------------ */

#include <cmath>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <vector>
#include <numbers>
#include <SDL3/SDL.h>

// -----------------------------------------------------------------------------
// 1. MATEMÁTICA DE FRAMES Y BYTES (la núcleo del ej 05, SIN SDL):
//    frames = bytes / bytesPorFrame ; bytesPorFrame = canales * sizeof(fmt)
// -----------------------------------------------------------------------------
struct SpecA {
    int freq = 48000;
    int canales = 2;
    int szSample = 4; // bytes por sample (F32=4, S16=2, U8=1)

    int frameBytes() const { return canales * szSample; }
    int segundosABytes(double segs) const {
        return int(double(segs) * double(freq)) * frameBytes();
    }
    double bytesASegundos(int bytes) const {
        return double(bytes) / (double(freq) * frameBytes());
    }
};

// -----------------------------------------------------------------------------
// 2. BLEEP: generar S_INT sinusal 440Hz de 2 segundos (F32, mono a stereo)
// -----------------------------------------------------------------------------
static void generarBleep(SpecA spec, std::vector<float>& out) {
    const int totalFrames = spec.segundosABytes(2.0) / spec.frameBytes();
    out.clear();
    out.resize(size_t(totalFrames) * size_t(spec.canales));
    const double perFrame = 1.0 / double(spec.freq);
    for (int f = 0; f < totalFrames; ++f) {
        const float t = float(f) * float(perFrame);
        const float muestra = std::sin(2 * std::numbers::pi_v<float> * 440.0f * t);
        for (int c = 0; c < spec.canales; ++c)
            out[size_t(f) * size_t(spec.canales) + size_t(c)] = muestra * 0.3f;
    }
}

// Callback del stream: como corre en el thread del audio, imprime POCO
static void SDLCALL rellenaBleep(void* userdata, SDL_AudioStream* stream,
                                 int adicional, int total) {
    (void)total; // total: el chunk objetivo; adicional = lo que falta AHORA
    auto* buffer = static_cast<std::vector<float>*>(userdata);
    // 440 Hz sin pico: como escuchamos el "bleep" de toda la ola, silencio...
    if (buffer->empty()) return;
    // Pocos bytes por callback: entrega incremental (el stream es thread-safe)
    const int szSample = 4, canales = 2;
    const int frameBytes = szSample * canales;
    const int maxFrame = std::min(adicional / frameBytes, int(buffer->size()));
    if (maxFrame <= 0) return;

    std::vector<float> out(size_t(maxFrame) * size_t(canales));
    std::copy_n(buffer->begin(), out.size(), out.begin());
    buffer->erase(buffer->begin(), buffer->begin() + long(out.size()));
    SDL_PutAudioStreamData(stream, out.data(), int(out.size() * sizeof(float)));
}

/* -----------------------------------------------------------------------------
   3. POR QUÉ IMPORTA LA CONVERSIÓN EN TU HARDWARE (Celeron N4020):
   - El audio de un juego ES un stream: si tu buffer se seca, tenés "glitch"
     (click/secado). El pico de latencia de SDL con los streams está en
     "bytes por callback", que podés controlarlo con tu callback manual.
   - La conversión de format (S16 -> F32, mono -> stereo, 44.1kHz a 48kHz)
     la ejecuta el stream (te ahorra tus propios conversores por formato).
   - En los tests del módulo NO reproducimos sonido: el ej 05 comprueba la
     MATEMÁTICA de frames/bytes/latencia con specs simulados. La escucha
     real es la demo de esta lección, a mano.
   - Proyecto: leer archivos OGG/WAV/FLAC (miniaudio, módulo 25) decodifica
     al stream (data del "sample audio": put al stream para su playback).
----------------------------------------------------------------------------- */

// -----------------------------------------------------------------------------
// 4. DEMO RUNTIME: 2 segundos de bleep por tu audio del sistema
// -----------------------------------------------------------------------------
static int demoAudio() {
    if (!SDL_Init(SDL_INIT_AUDIO)) { std::cout << "init " << SDL_GetError() << "\n"; return 1; }

    SpecA spec{48000, 2, 4};
    SDL_AudioSpec sdlSpec{SDL_AUDIO_F32, spec.canales, spec.freq};

    std::vector<float> datos;
    generarBleep(spec, datos);
    std::cout << "bleep: " << datos.size() << " samples ("
              << datos.size() * sizeof(float) << " bytes)\n";

    SDL_AudioStream* stream = SDL_OpenAudioDeviceStream(
        SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &sdlSpec, rellenaBleep, &datos);
    if (stream == nullptr) { std::cout << "stream " << SDL_GetError() << "\n"; SDL_Quit(); return 1; }

    SDL_ResumeAudioStreamDevice(stream); // stream empieza paused
    SDL_Delay(2500);                     // 2.5 s de bleep y chau
    SDL_DestroyAudioStream(stream);      // destruye stream, no device
    SDL_Quit();
    return 0;
}

/* =============================================================================
   QUÉ QUEDA TESTEABLE (ej 05, sin SDL y sin oír nada):
   - SpecA::frameBytes / segundosABytes / bytesASegundos (del spec ASÍ)
   - conversiones (44.1k->48k) con SPEC SIMULADO (ej 05: Rasample)
   - el modelo "callback pide N bytes, tenés menos M": clamp de lo pedido.
   ============================================================================= */
int main() { return demoAudio(); }
