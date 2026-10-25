/* =============================================================================
   MÓDULO 25 - LECCIÓN 03: streaming vs decoding completo (sonido vs música)
   =============================================================================
   TEMA 2 (parte 1): Sonido vs música: streaming (OGG/Vorbis/FLAC/MP3) vs
   decoding completo, pool de voces y prioridad — 2 lecciones (esta es la 1°).

   DOS FORMAS DE TENER UN SONIDO EN MEMORIA
   ----------------------------------------
   1) Decoding COMPLETO: al cargar (o al primer play), decodificás TODO el
      archivo a PCM y lo tenés como muestras listas:
      Pros: play instantáneo, seek O(1), podés tocar con loops sin jank, y
      "muchas copias" del mismo audio comparten UN PCM en memoria.
      Contras: memoria. Números exactos:
        duración(s) * sampleRate * canales * bytesPorMuestra
        30 s * 48000 * 2 * 4 (f32/stereo) = 11.520.000 B ≈ 11 MB
      Para SFX (guardo, pasos, botones: < 2 s) es lo correcto.
   2) STREAMING: leés el archivo comprimido (o el pack) y decodificás EN
      PEDACITOS mientras suena. Contras: seek caro, no podés re-loop sin
      re-leer, una "copia" = un decoder vivo. Pros: memoria casi constante:
      2 páginas de ~0.5 s c/u ≈ 48000*0.5*2*4 = 192.000 B ≈ 0.19 MB, aunque la
      música dure media hora.

   miniaudio (0.11.25) soporta los cuatro: WAV (dr_wav), FLAC (dr_flac),
   MP3 (dr_mp3) y Vorbis/OGG vía stb_vorbis integrado (se puede desactivar con
   MINIAUDIO_DISABLE_VORBIS; el push-mode de stb_vorbis tiene la limitación de
   que ma_decoder_get_length_in_pcm_frames devuelve 0 para Vorbis en decoders
   de memoria-push, bug/documentado en el header; en archivos usa pull-mode y sí
   conoce la duración).

   API DE DECODER (firmas verificadas con grep en el header)
   ---------------------------------------------------------
     ma_decoder_config cfg = ma_decoder_config_init(ma_format_f32, 2, 48000);
     ma_decoder_init_file("musica.ogg", &cfg, &dec);       // archivo
     ma_decoder_init_memory(data, dataSize, &cfg, &dec);   // pack/lump
     ma_decoder_read_pcm_frames(&dec, frames, N, &leidos); // 0 = EOF
     ma_decoder_uninit(&dec, NULL);
   La config le PIDE a miniaudio que te entregue el PCM ya en (formato,
   canales, rate) de tu engine: el resampleo/convertido lo hace la lib.

   PARTE HIGH-LEVEL: el resource manager
   -------------------------------------
   ma_engine suele usarse combinado con ma_resource_manager que mantiene
   streams y buffers decodificados; con flag MA_SOUND_FLAG_ASYNC el load
   es asincrónico (jobThreadCount de la config). El struct interno
   ma_resource_manager_data_stream usa EXACTAMENTE el patrón de la lección 02:
   MA_ATOMIC pageFrameCount[2], pPageData y seekCounter (leído en el header).
   Para streaming desde un PACK: lección 09 (custom VFS).

   ¿CUÁNDO CADA UNO? (regla de project-rest con números)
   ----------------------------------------------------
   - SFX frecuentes -> decoded (todo a PCM una vez; la copia es "una referencia").
   - Música y VO largas -> streamed.
   - Audio dentro de un pack -> SIEMPRE streamed con offset+read (lección 09);
     decodificar "todo el pack" es el error clásico de memoria.
   ============================================================================= */
#include "include/f25.hpp"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <vector>

/* --------------------------- Modo con miniaudio -------------------------- */
#if DEMO_MINIAUDIO
#include "miniaudio.h"

int demoDecodingReal() {
    /* Mini WAV generado en memoria (un tono de 1 s a 48000 Hz, PCM s16 mono)
       para que la demo funcione sin assets del dueño. Formato WAV canónico:
       "RIFF" + sizes + "WAVE" + "fmt " + "data". */
    const int n = 48000;
    std::vector<std::uint8_t> wav;
    const std::uint32_t dataBytes = n * 2;
    const std::uint32_t riffBytes = 36 + dataBytes;
    auto push32 = [&wav](std::uint32_t v) {
        wav.push_back(std::uint8_t(v)); wav.push_back(std::uint8_t(v >> 8));
        wav.push_back(std::uint8_t(v >> 16)); wav.push_back(std::uint8_t(v >> 24));
    };
    auto push16 = [&wav](std::uint16_t v) {
        wav.push_back(std::uint8_t(v)); wav.push_back(std::uint8_t(v >> 8));
    };
    auto pushTag = [&wav](const char* t) {
        for (int i = 0; i < 4; ++i) { wav.push_back(static_cast<std::uint8_t>(t[i])); }
    };
    pushTag("RIFF"); push32(riffBytes); pushTag("WAVE"); pushTag("fmt ");
    push32(16); push16(1); push16(1); push32(f25::kSampleRate);
    push32(f25::kSampleRate * 2); push16(2); push16(16); pushTag("data");
    push32(dataBytes);
    for (int i = 0; i < n; ++i) {
        const double s = std::sin(2.0 * f25::kPi * 440.0 * (double)i / f25::kSampleRate);
        push16(static_cast<std::uint16_t>(static_cast<std::int16_t>(s * 16000.0)));
    }

    /* Decodificar TODO con la API real, pidiendo f32/stereo/48k de salida. */
    ma_decoder dec;
    ma_decoder_config cfg = ma_decoder_config_init(ma_format_f32, 2, 48000);
    if (ma_decoder_init_memory(wav.data(), wav.size(), &cfg, &dec) != MA_SUCCESS) {
        return 1;
    }
    std::vector<float> pcm(480 * 2);
    ma_uint64 leidos = 0;
    ma_decoder_read_pcm_frames(&dec, pcm.data(), 480, &leidos);
    std::printf("decodificación real: %llu frames leídos (pedidos 480), "
                "frame 0 = %.4f\n", (unsigned long long)leidos, pcm[0]);    ma_decoder_uninit(&dec);
    return 0;
}
#endif /* DEMO_MINIAUDIO */

/* ------------------------------- Modo headless ----------------------------- */
namespace {

/* Simulación del "stream scheduler": lee páginas de un "archivo" vectorial. */
struct Streamer {
    const std::vector<float>& fuente;
    std::size_t cursor = 0;
    int paginasServidas = 0;

    /* Una página de 480 frames: la "decodificación" aquí es un copiar. */
    bool leerPagina(std::vector<float>& destino, std::size_t frames) {
        if (cursor >= fuente.size()) {
            /* EOF: en streaming real el motor hace loop o pasa al próximo. */
            return false;
        }
        const std::size_t n = frames < (fuente.size() - cursor)
                                  ? frames : (fuente.size() - cursor);
        destino.assign(fuente.begin() + static_cast<long>(cursor),
                       fuente.begin() + static_cast<long>(cursor + n));
        cursor += n;
        ++paginasServidas;
        return true;
    }
};

} // namespace

int main() {
    std::printf("M25 / L03 - streaming vs decoding completo\n");
    std::printf("===========================================\n");

    /* 1. La matemática de memoria que justifica el streaming. */
    const double duracionS = 30.0;
    const double bytesDecoded = duracionS * f25::kSampleRate * f25::kCanales * 4;
    std::printf("30 s de música decodificada a f32/stereo/48k = %.0f bytes "
                "(~%.1f MB)\n", bytesDecoded, bytesDecoded / (1024.0 * 1024.0));
    const double bytesPagina = 0.5 * f25::kSampleRate * f25::kCanales * 4;
    std::printf("streaming con 2 páginas de 0.5 s = %.0f bytes (~%.0f KB)\n",
                bytesPagina * 2, bytesPagina * 2 / 1024.0);
    std::printf("30 s de OGG comprimido ~ 160 kbps = %.2f MB\n",
                160e3 / 8.0 * duracionS / (1024.0 * 1024.0));

    /* 2. Stream scheduler simulado: páginas hasta el EOF. */
    std::vector<float> archivo(48000 * 2 / 4); /* 0.25 s simulado */
    for (std::size_t i = 0; i < archivo.size(); ++i) {
        archivo[i] = static_cast<float>(i) / static_cast<float>(archivo.size());
    }
    Streamer stream{ archivo };
    std::vector<float> pagina;
    int paginas = 0;
    while (stream.leerPagina(pagina, 480)) {
        ++paginas;
    }
    std::printf("streamer: %d páginas de 480 frames servidas; "
                "la última quedó de %zu frames\n", paginas, pagina.size());
    std::printf("EOF -> la próxima lectura devuelve false (el loop del "
                "motor decide: stop, loop o próxima pista).\n");

    /* 3. Decoding completo simulado: "todo a PCM de una vez". */
    std::vector<float> pcmCompleto = archivo; /* copia */
    std::printf("decoding completo: %zu frames en RAM desde el primer ms\n",
                pcmCompleto.size());
    std::printf("seek a 1.0 s = índice %zu (O(1) vs streaming re-decode)\n",
                static_cast<std::size_t>(1.0 * f25::kSampleRate));

#if DEMO_MINIAUDIO
    demoDecodingReal();
#else
    std::printf("\n(decoder REAL con ma_decoder_init_memory + OGG: compilar "
                "vía CMake, que define DEMO_MINIAUDIO)\n");
#endif
    return 0;
}
