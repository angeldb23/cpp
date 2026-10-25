/*
 * MÓDULO 28: Estudio de Código Real — Arquitectura de Librerías
 * EJERCICIO 03: miniaudio — Ring Buffer SPSC, Data Conversion, Job Queue
 *
 * OBJETIVO: Implementar componentes core de miniaudio: ring buffer lock-free,
 * conversión de formato y job queue para decoding.
 */

#include <cstddef>
#include <cstdint>
#include <atomic>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <cstring>
#include <cmath>
#include "../../include/test_framework.hpp"

// ============================================================================
// TIPOS BASE
// ============================================================================

using ma_format = uint32_t;
constexpr ma_format MA_FORMAT_U8 = 0;
constexpr ma_format MA_FORMAT_S16 = 1;
constexpr ma_format MA_FORMAT_S24 = 2;
constexpr ma_format MA_FORMAT_S32 = 3;
constexpr ma_format MA_FORMAT_F32 = 4;

using ma_uint32 = uint32_t;
using ma_int32 = int32_t;
using ma_uint64 = uint64_t;
using ma_bool32 = uint32_t;

constexpr ma_bool32 MA_TRUE = 1;
constexpr ma_bool32 MA_FALSE = 0;

// ============================================================================
// 1. RING BUFFER SPSC (SINGLE PRODUCER SINGLE CONSUMER) LOCK-FREE
// ============================================================================

// TODO: Implementar ma_ring_buffer
struct ma_ring_buffer {
    std::vector<uint8_t> buffer;
    std::atomic<size_t> read_pos{0};
    std::atomic<size_t> write_pos{0};
    size_t capacity;
    
    // TODO: Constructor
    ma_ring_buffer() = default;
    explicit ma_ring_buffer(size_t cap) : buffer(cap), capacity(cap) {}
    
    // TODO: acquire_write - reserva espacio para escritura, devuelve puntero y tamaño disponible
    // Retorna {ptr, size} o {nullptr, 0} si no hay espacio
    struct WriteAcquire {
        uint8_t* ptr;
        size_t size;
    };
    
    WriteAcquire acquire_write(size_t requested) {
        (void)requested;
        return {nullptr, 0};
    }
    
    // TODO: commit_write - confirma bytes escritos
    void commit_write(size_t written) {
        (void)written;
    }
    
    // TODO: acquire_read - reserva espacio para lectura
    struct ReadAcquire {
        const uint8_t* ptr;
        size_t size;
    };
    
    ReadAcquire acquire_read(size_t requested) {
        (void)requested;
        return {nullptr, 0};
    }
    
    // TODO: commit_read - confirma bytes leídos
    void commit_read(size_t read) {
        (void)read;
    }
    
    // TODO: available_read - bytes disponibles para leer
    size_t available_read() const {
        return 0;
    }
    
    // TODO: available_write - espacio disponible para escribir
    size_t available_write() const {
        return 0;
    }
};

// ============================================================================
// 2. FORMAT CONVERSION
// ============================================================================

// TODO: ma_convert_sample - convierte un sample entre formatos
inline float ma_convert_sample_to_f32(void* sample, ma_format format) {
    (void)sample; (void)format;
    return 0.0f;
}

// TODO: ma_convert_sample_from_f32 - convierte de f32 a formato destino
inline void ma_convert_sample_from_f32(float sample, void* dst, ma_format format) {
    (void)sample; (void)dst; (void)format;
}

// TODO: ma_get_format_size - tamaño en bytes por sample
inline size_t ma_get_format_size(ma_format format) {
    (void)format;
    return 0;
}

// TODO: ma_convert_frames - convierte buffer de frames entre formatos
inline void ma_convert_frames(const void* src, ma_format src_fmt, 
                              void* dst, ma_format dst_fmt, 
                              ma_uint32 frame_count, ma_uint32 channels) {
    (void)src; (void)src_fmt; (void)dst; (void)dst_fmt;
    (void)frame_count; (void)channels;
}

// ============================================================================
// 3. CHANNEL MAPPING / REMIXING
// ============================================================================

// TODO: ma_channel enum
enum ma_channel {
    MA_CHANNEL_NONE = 0,
    MA_CHANNEL_MONO = 1,
    MA_CHANNEL_FRONT_LEFT = 2,
    MA_CHANNEL_FRONT_RIGHT = 3,
    MA_CHANNEL_FRONT_CENTER = 4,
    MA_CHANNEL_LFE = 5,
    MA_CHANNEL_BACK_LEFT = 6,
    MA_CHANNEL_BACK_RIGHT = 7,
    MA_CHANNEL_SIDE_LEFT = 8,
    MA_CHANNEL_SIDE_RIGHT = 9
};

// TODO: ma_remix_matrix - matriz de mezcla [out_channels][in_channels]
struct ma_remix_matrix {
    std::vector<float> matrix; // row-major: [out_ch][in_ch]
    ma_uint32 in_channels;
    ma_uint32 out_channels;
    
    ma_remix_matrix(ma_uint32 out_ch, ma_uint32 in_ch) 
        : matrix(out_ch * in_ch, 0.0f), in_channels(in_ch), out_channels(out_ch) {}
    
    float& at(ma_uint32 out_ch, ma_uint32 in_ch) {
        return matrix[out_ch * in_channels + in_ch];
    }
    
    const float& at(ma_uint32 out_ch, ma_uint32 in_ch) const {
        return matrix[out_ch * in_channels + in_ch];
    }
};

// TODO: ma_build_stereo_to_mono_matrix
inline ma_remix_matrix ma_build_stereo_to_mono_matrix() {
    return ma_remix_matrix(1, 2);
}

// TODO: ma_build_mono_to_stereo_matrix
inline ma_remix_matrix ma_build_mono_to_stereo_matrix() {
    return ma_remix_matrix(2, 1);
}

// TODO: ma_apply_remix - aplica matriz de mezcla a buffer f32 interleaved
inline void ma_apply_remix(const float* in, float* out, ma_uint32 frame_count, const ma_remix_matrix& matrix) {
    (void)in; (void)out; (void)frame_count; (void)matrix;
}

// ============================================================================
// 4. JOB QUEUE PARA DECODING ASÍNCRONO
// ============================================================================

// TODO: ma_job_queue - thread pool para trabajos de I/O y decoding
struct ma_job_queue {
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> jobs;
    std::mutex mutex;
    std::condition_variable cv;
    std::atomic<bool> shutdown{false};
    std::atomic<size_t> pending{0};
    
    ma_job_queue(size_t num_threads = 0) {
        if (num_threads == 0) num_threads = std::thread::hardware_concurrency();
        if (num_threads == 0) num_threads = 2;
        
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this] { worker_loop(); });
        }
    }
    
    ~ma_job_queue() {
        shutdown.store(true, std::memory_order_release);
        cv.notify_all();
        for (auto& t : workers) if (t.joinable()) t.join();
    }
    
    // TODO: post_job - encola trabajo
    template<typename F>
    void post_job(F&& f) {
        (void)f;
    }
    
    // TODO: wait_all - espera a que todos los trabajos terminen
    void wait_all() {
    }
    
    // TODO: worker_loop
    void worker_loop() {
    }
    
    size_t pending_jobs() const { return pending.load(std::memory_order_acquire); }
};

// ============================================================================
// TESTS (NO MODIFICAR)
// ============================================================================

TEST_CASE(TestRingBufferBasic) {
    ma_ring_buffer rb(1024);
    ASSERT_EQ(rb.available_write(), 1023); // -1 for full/empty distinction
    ASSERT_EQ(rb.available_read(), 0);
    
    auto wa = rb.acquire_write(100);
    ASSERT_TRUE(wa.ptr != nullptr);
    ASSERT_EQ(wa.size, 100);
    std::memset(wa.ptr, 0xAA, 100);
    rb.commit_write(100);
    
    ASSERT_EQ(rb.available_read(), 100);
    ASSERT_EQ(rb.available_write(), 923);
    
    auto ra = rb.acquire_read(50);
    ASSERT_TRUE(ra.ptr != nullptr);
    ASSERT_EQ(ra.size, 50);
    ASSERT_EQ(ra.ptr[0], 0xAA);
    rb.commit_read(50);
    
    ASSERT_EQ(rb.available_read(), 50);
    ASSERT_EQ(rb.available_write(), 973);
}

TEST_CASE(TestRingBufferWrapAround) {
    ma_ring_buffer rb(256);
    
    // Write near end
    auto wa = rb.acquire_write(200);
    ASSERT_EQ(wa.size, 200);
    rb.commit_write(200);
    
    // Read some
    auto ra = rb.acquire_read(150);
    ASSERT_EQ(ra.size, 150);
    rb.commit_read(150);
    
    // Write again (should wrap)
    wa = rb.acquire_write(100);
    ASSERT_TRUE(wa.ptr != nullptr);
    rb.commit_write(100);
    
    // Read all
    ra = rb.acquire_read(150);
    ASSERT_TRUE(ra.ptr != nullptr);
    rb.commit_read(150);
    
    ASSERT_EQ(rb.available_read(), 0);
}

TEST_CASE(TestFormatConversion) {
    // U8 -> F32
    uint8_t u8_val = 255;
    float f32 = ma_convert_sample_to_f32(&u8_val, MA_FORMAT_U8);
    ASSERT_TRUE(std::abs(f32 - 1.0f) < 0.01f);
    
    u8_val = 0;
    f32 = ma_convert_sample_to_f32(&u8_val, MA_FORMAT_U8);
    ASSERT_TRUE(std::abs(f32 - (-1.0f)) < 0.01f);
    
    u8_val = 128;
    f32 = ma_convert_sample_to_f32(&u8_val, MA_FORMAT_U8);
    ASSERT_TRUE(std::abs(f32 - 0.0f) < 0.01f);
    
    // S16 -> F32
    int16_t s16_val = 32767;
    f32 = ma_convert_sample_to_f32(&s16_val, MA_FORMAT_S16);
    ASSERT_TRUE(std::abs(f32 - 1.0f) < 0.001f);
    
    s16_val = -32768;
    f32 = ma_convert_sample_to_f32(&s16_val, MA_FORMAT_S16);
    ASSERT_TRUE(std::abs(f32 - (-1.0f)) < 0.001f);
    
    // F32 -> S16
    float f = 0.5f;
    int16_t out_s16;
    ma_convert_sample_from_f32(f, &out_s16, MA_FORMAT_S16);
    ASSERT_EQ(out_s16, 16383); // 0.5 * 32767 ≈ 16383
    
    f = -0.5f;
    ma_convert_sample_from_f32(f, &out_s16, MA_FORMAT_S16);
    ASSERT_EQ(out_s16, -16383); // -0.5 * 32767 = -16383.5 -> -16383 (truncation toward zero)
}

TEST_CASE(TestFormatConversionBuffer) {
    // Convert stereo S16 -> F32
    int16_t s16_data[4] = {32767, -32768, 0, 16384}; // 2 frames stereo
    float f32_data[4] = {};
    
    ma_convert_frames(s16_data, MA_FORMAT_S16, f32_data, MA_FORMAT_F32, 2, 2);
    
    ASSERT_TRUE(std::abs(f32_data[0] - 1.0f) < 0.001f);
    ASSERT_TRUE(std::abs(f32_data[1] - (-1.0f)) < 0.001f);
    ASSERT_TRUE(std::abs(f32_data[2] - 0.0f) < 0.001f);
    ASSERT_TRUE(std::abs(f32_data[3] - 0.5f) < 0.001f);
}

TEST_CASE(TestChannelRemix) {
    // Stereo to mono
    ma_remix_matrix stereo_to_mono = ma_build_stereo_to_mono_matrix();
    ASSERT_EQ(stereo_to_mono.in_channels, 2);
    ASSERT_EQ(stereo_to_mono.out_channels, 1);
    ASSERT_EQ(stereo_to_mono.at(0, 0), 0.5f);
    ASSERT_EQ(stereo_to_mono.at(0, 1), 0.5f);
    
    // Mono to stereo
    ma_remix_matrix mono_to_stereo = ma_build_mono_to_stereo_matrix();
    ASSERT_EQ(mono_to_stereo.in_channels, 1);
    ASSERT_EQ(mono_to_stereo.out_channels, 2);
    ASSERT_EQ(mono_to_stereo.at(0, 0), 1.0f);
    ASSERT_EQ(mono_to_stereo.at(1, 0), 1.0f);
}

TEST_CASE(TestApplyRemix) {
    ma_remix_matrix stereo_to_mono = ma_build_stereo_to_mono_matrix();
    
    float stereo[4] = {1.0f, -1.0f, 0.5f, -0.5f}; // 2 frames
    float mono[2] = {};
    
    ma_apply_remix(stereo, mono, 2, stereo_to_mono);
    
    // Frame 0: (1.0 + -1.0) * 0.5 = 0
    // Frame 1: (0.5 + -0.5) * 0.5 = 0
    ASSERT_EQ(mono[0], 0.0f);
    ASSERT_EQ(mono[1], 0.0f);
    
    // Test with different values
    stereo[0] = 1.0f; stereo[1] = 1.0f; // Both channels 1.0
    stereo[2] = -1.0f; stereo[3] = 1.0f;
    ma_apply_remix(stereo, mono, 2, stereo_to_mono);
    ASSERT_EQ(mono[0], 1.0f);  // (1+1)*0.5 = 1
    ASSERT_EQ(mono[1], 0.0f);  // (-1+1)*0.5 = 0
}

TEST_CASE(TestJobQueue) {
    ma_job_queue jq(2);
    std::atomic<int> counter{0};
    
    for (int i = 0; i < 100; ++i) {
        jq.post_job([&counter]() { counter.fetch_add(1); });
    }
    
    jq.wait_all();
    ASSERT_EQ(counter.load(), 100);
}

TEST_CASE(TestJobQueueConcurrent) {
    ma_job_queue jq(4);
    std::atomic<int> counter{0};
    const int num_threads = 4;
    const int per_thread = 1000;
    
    std::vector<std::thread> threads;
    for (int t = 0; t < num_threads; ++t) {
        threads.emplace_back([&jq, &counter, per_thread]() {
            for (int i = 0; i < per_thread; ++i) {
                jq.post_job([&counter]() { counter.fetch_add(1); });
            }
        });
    }
    
    for (auto& t : threads) t.join();
    jq.wait_all();
    
    ASSERT_EQ(counter.load(), num_threads * per_thread);
}

TEST_CASE(TestS24Conversion) {
    // S24: 3 bytes, test max positive
    uint8_t s24_max[3] = {0xFF, 0xFF, 0x7F}; // 0x7FFFFF = 8388607
    float f = ma_convert_sample_to_f32(s24_max, MA_FORMAT_S24);
    ASSERT_TRUE(std::abs(f - 1.0f) < 0.0001f);
    
    // S24: max negative
    uint8_t s24_min[3] = {0x00, 0x00, 0x80}; // 0x800000 = -8388608 (sign extended)
    f = ma_convert_sample_to_f32(s24_min, MA_FORMAT_S24);
    ASSERT_TRUE(std::abs(f - (-1.0f)) < 0.0001f);
    
    // F32 -> S24
    float f32 = 0.5f;
    uint8_t out_s24[3];
    ma_convert_sample_from_f32(f32, out_s24, MA_FORMAT_S24);
    
    int32_t reconstructed = (out_s24[2] << 16) | (out_s24[1] << 8) | out_s24[0];
    if (reconstructed & 0x800000) reconstructed |= ~0xFFFFFF;
    float back = reconstructed / 8388608.0f;
    ASSERT_TRUE(std::abs(back - 0.5f) < 0.001f);
}

int main() { return RUN_ALL_TESTS(); }