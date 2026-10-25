/*
 * MÓDULO 28: Estudio de Código Real — Arquitectura de Librerías
 * LECCIÓN 04: Estudio de miniaudio — Single-Header, C99/C++ Backend Abstraction
 *
 * TEMAS:
 * - miniaudio: single-header library (miniaudio.h), ~30K LOC
 * - C99 compatible, usable from C++ sin wrapper
 * - Backend abstraction: WASAPI, CoreAudio, ALSA, PulseAudio, JACK, OpenSL ES, WebAudio
 * - Device management: playback, capture, duplex, loopback
 * - Data conversion: sample format, channel mapping, sample rate conversion
 * - Engine API: high-level (ma_engine) para reproducción simple
 * - Node graph: ma_node_graph para routing complejo
 * - Threading: lock-free ring buffers, job system interno
 * - Streaming vs decoding: ma_decoder, ma_data_source
 * - Resource management: resource manager con pooling
 * - Build: header-only, opcional miniaudio.c para unity build
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

namespace modulo28_miniaudio {

// Tipos base de miniaudio (en el header real vienen de ma.h / miniaudio.h)
using ma_uint32 = std::uint32_t;
using ma_int32 = std::int32_t;
using ma_uint64 = std::uint64_t;
using ma_bool32 = std::uint32_t;

// ============================================================================
// ARQUITECTURA HEADER-ONLY C99
// ============================================================================

/*
 * miniaudio es single-header: todo en miniaudio.h
 * Para C++: extern "C" { #include "miniaudio.h" }
 * 
 * Ventajas single-header C99:
 *   - Zero dependencies, easy integration
 *   - ABI stable: C linkage, opaque pointers
 *   - Compilable como C y C++
 *   - Unity build opcional: #define MINIAUDIO_IMPLEMENTATION en un .c
 * 
 * Estructura típica:
 *   - Tipos opacos: ma_device, ma_context, ma_engine, ma_node_graph
 *   - Callbacks C-style: function pointers + void* user_data
 *   - Config structs: ma_device_config, ma_engine_config
 *   - Result codes: MA_SUCCESS, MA_ERROR, MA_INVALID_ARGS, etc.
 */

// ============================================================================
// BACKEND ABSTRACTION
// ============================================================================

/*
 * Backends soportados (orden de prioridad por plataforma):
 *   Windows: WASAPI (shared/exclusive) -> DirectSound -> WinMM
 *   macOS/iOS: CoreAudio
 *   Linux: ALSA -> PulseAudio -> PipeWire -> JACK -> OSS
 *   Android: OpenSL ES -> AAudio
 *   Emscripten: WebAudio
 *   Fallback: null backend (silencio)
 * 
 * Selección automática en ma_context_init()
 * Forzar backend: ma_context_config.pBackend
 * 
 * Cada backend implementa:
 *   - Device enumeration
 *   - Format negotiation
 *   - Buffer management
 *   - Start/stop callbacks
 */

typedef enum {
    MA_BACKEND_WASAPI,
    MA_BACKEND_DSOUND,
    MA_BACKEND_WINMM,
    MA_BACKEND_COREAUDIO,
    MA_BACKEND_ALSA,
    MA_BACKEND_PULSEAUDIO,
    MA_BACKEND_JACK,
    MA_BACKEND_OPENSL,
    MA_BACKEND_AAUDIO,
    MA_BACKEND_WEBAUDIO,
    MA_BACKEND_NULL,
    MA_BACKEND_COUNT
} ma_backend_type;

// ============================================================================
// DEVICE API (LOW-LEVEL)
// ============================================================================

/*
 * ma_device: API de bajo nivel para control total
 * 
 * Flujo típico:
 *   1. ma_context_init() - inicializar backend
 *   2. ma_device_config_init() - configurar
 *   3. ma_device_init() - crear device
 *   4. ma_device_start() - empezar callback
 *   5. Callback: ma_device_callback (separate thread)
 *   6. ma_device_stop/uninit/context_uninit
 * 
 * Callback signature:
 *   void callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
 * 
 * Threading: callback runs on audio thread (high priority, real-time constraints)
 *   - NO locking, NO allocation, NO syscalls
 *   - Lock-free communication con main thread via ring buffer
 */

typedef struct ma_device ma_device;
typedef struct ma_context ma_context;

typedef void (*ma_device_callback_proc)(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

// Enums de formato/canales (en miniaudio.h están antes de las configs)
typedef enum {
    MA_FORMAT_U8,
    MA_FORMAT_S16,
    MA_FORMAT_S24,
    MA_FORMAT_S32,
    MA_FORMAT_F32
} ma_format;

typedef enum {
    MA_CHANNEL_NONE = 0,
    MA_CHANNEL_MONO = 1,
    MA_CHANNEL_FRONT_LEFT = 2,
    MA_CHANNEL_FRONT_RIGHT = 3,
    MA_CHANNEL_FRONT_CENTER = 4,
    MA_CHANNEL_LFE = 5,
    MA_CHANNEL_BACK_LEFT = 6,
    MA_CHANNEL_BACK_RIGHT = 7,
    // ... más canales
} ma_channel;

constexpr ma_uint32 MA_MAX_CHANNELS = 8;

typedef struct {
    ma_device_callback_proc callback;
    void* pUserData;
    ma_uint32 periodSizeInFrames;
    ma_uint32 periodSizeInMilliseconds;
    ma_uint32 periods;
    ma_format format;
    ma_uint32 channels;
    ma_uint32 sampleRate;
    ma_channel channelMap[MA_MAX_CHANNELS];
    ma_bool32 noPreSilencedOutputBuffer;
    ma_bool32 noClip;
} ma_device_config;

// ============================================================================
// ENGINE API (HIGH-LEVEL)
// ============================================================================

/*
 * ma_engine: API de alto nivel para casos simples
 * 
 * Características:
 *   - Node graph interno automático
 *   - Resource manager para audio assets
 *   - Spatial audio (3D positioning)
 *   - Groups/buses para mixing
 *   - Fade in/out, pitch, volume
 *   - Streaming automático para archivos grandes
 * 
 * Uso básico:
 *   ma_engine_config config = ma_engine_config_init();
 *   ma_engine engine;
 *   ma_engine_init(&config, &engine);
 *   ma_engine_play_sound(&engine, "explosion.wav", NULL);
 *   ma_engine_uninit(&engine);
 * 
 * Resource Manager:
 *   - Pool de decoders (ma_decoder)
 *   - Reference counting
 *   - Streaming vs fully decoded (threshold configurable)
 *   - Hot reload support
 */

typedef struct ma_engine ma_engine;
typedef struct ma_engine_config ma_engine_config;

typedef struct {
    const char* pFilePath;
    ma_engine* pEngine;
    ma_uint32 flags; // MA_SOUND_FLAG_STREAM, MA_SOUND_FLAG_SPATIAL, etc.
} ma_sound_config;

typedef struct ma_sound ma_sound;

// ============================================================================
// DATA CONVERSION PIPELINE
// ============================================================================

/*
 * Pipeline interno de conversión (formato dispositivo -> formato source):
 * 
 * Source (file/decoder) -> Sample Rate Conversion -> Channel Mapping -> Format Conversion -> Device
 * 
 * ma_data_converter: maneja todo el pipeline
 *   - Linear interpolation para SRC (rápido, calidad decente)
 *   - Channel mapping: remix matrix (upmix/downmix)
 *   - Format conversion: f32/s16/s24/s32/u8 <-> cualquier
 *   - Dithering opcional para reducción de bits
 * 
 * ma_channel_map: array de ma_channel (FL, FR, FC, LFE, BL, BR, etc.)
 * ma_channel_converter: matrix-based remixing
 */

// ============================================================================
// NODE GRAPH (ROUTING AVANZADO)
// ============================================================================

/*
 * ma_node_graph: grafo de nodos de procesamiento
 * 
 * Nodos built-in:
 *   - ma_splitter_node: 1 input -> N outputs
 *   - ma_linear_resampler_node: sample rate conversion
 *   - ma_channel_router_node: channel mapping
 *   - ma_gainer_node: volume/pan
 *   - ma_fader_node: fade in/out
 *   - ma_panner_node: spatial positioning
 *   - ma_delay_node: delay line
 *   - ma_biquad_node: IIR filters
 *   - Custom nodes: ma_node_vtable
 * 
 * Topología: directed acyclic graph (DAG)
 * Procesamiento: topological order, pull-based (output asks input)
 * Thread-safe: lock-free para audio thread, mutex para cambios de topología
 */

typedef struct ma_node_graph ma_node_graph;
typedef struct ma_node ma_node;
typedef struct ma_node_vtable ma_node_vtable;

// ============================================================================
// THREADING Y REAL-TIME
// ============================================================================

/*
 * Restricciones real-time en audio callback:
 *   - Latency budget: típicamente 5-10ms (480 frames @ 48kHz)
 *   - NO malloc/free
 *   - NO mutex lock (except try_lock con fallback)
 *   - NO syscalls (file I/O, network)
 *   - NO unbounded loops
 * 
 * miniaudio solutions:
 *   - Ring buffers (SPSC lock-free) para comunicación main <-> audio
 *   - Job system interno para decoding/processing pesado
 *   - Pre-allocated buffers (pool allocators)
 *   - Double/triple buffering para device buffers
 * 
 * ma_async_notification: notificación lock-free main -> audio
 * ma_job_queue: cola de trabajos para decoding
 */

// ============================================================================
// STREAMING VS DECODING
// ============================================================================

/*
 * ma_decoder: decodifica archivos (WAV, MP3, FLAC, OGG, etc. via stb_vorbis, dr_flac, dr_mp3)
 * 
 * Modos:
 *   - Full decode: ma_decoder_init_file -> ma_decoder_read_pcm_frames (todo en memoria)
 *   - Streaming: ma_data_source + ma_decoder_init_file -> read on demand
 * 
 * Streaming threshold: ma_engine_config.streamingThresholdInFrames (default ~1M frames = ~20s @ 48kHz)
 * 
 * ma_data_source_vtable: interfaz polimórfica para sources
 *   - read_pcm_frames
 *   - seek_to_frame
 *   - get_data_format
 *   - get_cursor
 *   - get_length
 * 
 * Implementaciones: decoder, noise generator, tone generator, wave form generator
 */

// ============================================================================
// EJEMPLO DE USO MÍNIMO (C++)
// ============================================================================

/*
extern "C" {
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
}

void playback_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    // Generar tono 440Hz
    static float phase = 0.0f;
    float* output = (float*)pOutput;
    for (ma_uint32 i = 0; i < frameCount * pDevice->playback.channels; ++i) {
        output[i] = sinf(phase);
        phase += 2.0f * 3.14159f * 440.0f / pDevice->sampleRate;
        if (phase >= 2.0f * 3.14159f) phase -= 2.0f * 3.14159f;
    }
}

int main() {
    ma_context context;
    ma_context_init(NULL, 0, NULL, &context);
    
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format = ma_format_f32;
    config.playback.channels = 2;
    config.sampleRate = 48000;
    config.dataCallback = playback_callback;
    
    ma_device device;
    ma_device_init(&context, &config, &device);
    ma_device_start(&device);
    
    // Run for 5 seconds
    ma_sleep(5000);
    
    ma_device_stop(&device);
    ma_device_uninit(&device);
    ma_context_uninit(&context);
    return 0;
}
*/

} // namespace modulo28_miniaudio