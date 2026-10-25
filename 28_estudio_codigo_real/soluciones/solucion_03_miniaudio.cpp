/*
 * MÓDULO 28: Estudio de Código Real — Arquitectura de Librerías
 * SOLUCIÓN 03: miniaudio — Ring Buffer SPSC, Data Conversion, Job Queue
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

struct ma_ring_buffer {
    std::vector<uint8_t> buffer;
    std::atomic<size_t> read_pos{0};
    std::atomic<size_t> write_pos{0};
    size_t capacity;
    
    ma_ring_buffer() = default;
    explicit ma_ring_buffer(size_t cap) : buffer(cap), capacity(cap) {}
    
    struct WriteAcquire {
        uint8_t* ptr;
        size_t size;
    };
    
    WriteAcquire acquire_write(size_t requested) {
        size_t r = read_pos.load(std::memory_order_acquire);
        size_t w = write_pos.load(std::memory_order_relaxed);
        size_t available = (r > w) ? (r - w - 1) : (capacity - w + r - 1);
        if (available < requested) return {nullptr, 0};
        
        uint8_t* ptr = &buffer[w];
        size_t contiguous = capacity - w;
        size_t size = std::min(requested, contiguous);
        return {ptr, size};
    }
    
    void commit_write(size_t written) {
        size_t w = write_pos.load(std::memory_order_relaxed);
        write_pos.store((w + written) % capacity, std::memory_order_release);
    }
    
    struct ReadAcquire {
        const uint8_t* ptr;
        size_t size;
    };
    
    ReadAcquire acquire_read(size_t requested) {
        size_t r = read_pos.load(std::memory_order_relaxed);
        size_t w = write_pos.load(std::memory_order_acquire);
        size_t available = (w >= r) ? (w - r) : (capacity - r + w);
        if (available < requested) return {nullptr, 0};
        
        const uint8_t* ptr = &buffer[r];
        size_t contiguous = capacity - r;
        size_t size = std::min(requested, contiguous);
        return {ptr, size};
    }
    
    void commit_read(size_t read) {
        size_t r = read_pos.load(std::memory_order_relaxed);
        read_pos.store((r + read) % capacity, std::memory_order_release);
    }
    
    size_t available_read() const {
        size_t r = read_pos.load(std::memory_order_acquire);
        size_t w = write_pos.load(std::memory_order_acquire);
        return (w >= r) ? (w - r) : (capacity - r + w);
    }
    
    size_t available_write() const {
        size_t r = read_pos.load(std::memory_order_acquire);
        size_t w = write_pos.load(std::memory_order_acquire);
        return (r > w) ? (r - w - 1) : (capacity - w + r - 1);
    }
};

// ============================================================================
// 2. FORMAT CONVERSION
// ============================================================================

inline float ma_convert_sample_to_f32(void* sample, ma_format format) {
    switch (format) {
        case MA_FORMAT_U8: {
            uint8_t v = *static_cast<uint8_t*>(sample);
            return (v - 128) / 128.0f;
        }
        case MA_FORMAT_S16: {
            int16_t v = *static_cast<int16_t*>(sample);
            return v / 32768.0f;
        }
        case MA_FORMAT_S24: {
            uint8_t* bytes = static_cast<uint8_t*>(sample);
            int32_t v = (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];
            if (v & 0x800000) v |= ~0xFFFFFF;
            return v / 8388608.0f;
        }
        case MA_FORMAT_S32: {
            int32_t v = *static_cast<int32_t*>(sample);
            return v / 2147483648.0f;
        }
        case MA_FORMAT_F32: {
            return *static_cast<float*>(sample);
        }
    }
    return 0.0f;
}

inline void ma_convert_sample_from_f32(float sample, void* dst, ma_format format) {
    sample = std::max(-1.0f, std::min(1.0f, sample));
    switch (format) {
        case MA_FORMAT_U8: {
            *static_cast<uint8_t*>(dst) = static_cast<uint8_t>(sample * 127.0f + 128.0f);
            break;
        }
        case MA_FORMAT_S16: {
            *static_cast<int16_t*>(dst) = static_cast<int16_t>(sample * 32767.0f);
            break;
        }
        case MA_FORMAT_S24: {
            int32_t v = static_cast<int32_t>(sample * 8388607.0f);
            uint8_t* bytes = static_cast<uint8_t*>(dst);
            bytes[0] = v & 0xFF;
            bytes[1] = (v >> 8) & 0xFF;
            bytes[2] = (v >> 16) & 0xFF;
            break;
        }
        case MA_FORMAT_S32: {
            *static_cast<int32_t*>(dst) = static_cast<int32_t>(sample * 2147483647.0f);
            break;
        }
        case MA_FORMAT_F32: {
            *static_cast<float*>(dst) = sample;
            break;
        }
    }
}

inline size_t ma_get_format_size(ma_format format) {
    switch (format) {
        case MA_FORMAT_U8: return 1;
        case MA_FORMAT_S16: return 2;
        case MA_FORMAT_S24: return 3;
        case MA_FORMAT_S32: return 4;
        case MA_FORMAT_F32: return 4;
    }
    return 0;
}

inline void ma_convert_frames(const void* src, ma_format src_fmt, 
                              void* dst, ma_format dst_fmt, 
                              ma_uint32 frame_count, ma_uint32 channels) {
    size_t src_stride = ma_get_format_size(src_fmt) * channels;
    size_t dst_stride = ma_get_format_size(dst_fmt) * channels;
    
    const uint8_t* s = static_cast<const uint8_t*>(src);
    uint8_t* d = static_cast<uint8_t*>(dst);
    
    for (ma_uint32 i = 0; i < frame_count; ++i) {
        for (ma_uint32 ch = 0; ch < channels; ++ch) {
            float f = ma_convert_sample_to_f32(const_cast<uint8_t*>(s + ch * ma_get_format_size(src_fmt)), src_fmt);
            ma_convert_sample_from_f32(f, d + ch * ma_get_format_size(dst_fmt), dst_fmt);
        }
        s += src_stride;
        d += dst_stride;
    }
}

// ============================================================================
// 3. CHANNEL MAPPING / REMIXING
// ============================================================================

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

struct ma_remix_matrix {
    std::vector<float> matrix;
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

inline ma_remix_matrix ma_build_stereo_to_mono_matrix() {
    ma_remix_matrix m(1, 2);
    m.at(0, 0) = 0.5f;
    m.at(0, 1) = 0.5f;
    return m;
}

inline ma_remix_matrix ma_build_mono_to_stereo_matrix() {
    ma_remix_matrix m(2, 1);
    m.at(0, 0) = 1.0f;
    m.at(1, 0) = 1.0f;
    return m;
}

inline void ma_apply_remix(const float* in, float* out, ma_uint32 frame_count, const ma_remix_matrix& matrix) {
    for (ma_uint32 frame = 0; frame < frame_count; ++frame) {
        for (ma_uint32 out_ch = 0; out_ch < matrix.out_channels; ++out_ch) {
            float sum = 0.0f;
            for (ma_uint32 in_ch = 0; in_ch < matrix.in_channels; ++in_ch) {
                sum += in[frame * matrix.in_channels + in_ch] * matrix.at(out_ch, in_ch);
            }
            out[frame * matrix.out_channels + out_ch] = sum;
        }
    }
}

// ============================================================================
// 4. JOB QUEUE PARA DECODING ASÍNCRONO
// ============================================================================

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
    
    template<typename F>
    void post_job(F&& f) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            jobs.push(std::forward<F>(f));
            pending.fetch_add(1, std::memory_order_relaxed);
        }
        cv.notify_one();
    }
    
    void wait_all() {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this] { return pending.load(std::memory_order_acquire) == 0 && jobs.empty(); });
    }
    
    void worker_loop() {
        while (true) {
            std::function<void()> job;
            {
                std::unique_lock<std::mutex> lock(mutex);
                cv.wait(lock, [this] { return !jobs.empty() || shutdown.load(std::memory_order_acquire); });
                if (shutdown.load(std::memory_order_acquire) && jobs.empty()) return;
                
                job = std::move(jobs.front());
                jobs.pop();
            }
            
            job();
            {
                // El decremento se hace con el mutex tomado: si wait_all ya
                // evaluó el predicado y está por dormirse, no puede perder el
                // notify (clásico lost wakeup de condition_variable).
                std::lock_guard<std::mutex> lock(mutex);
                pending.fetch_sub(1, std::memory_order_release);
            }
            cv.notify_all();
        }
    }
    
    size_t pending_jobs() const { return pending.load(std::memory_order_acquire); }
};

// ============================================================================
// TESTS (NO MODIFICAR)
// ============================================================================

TEST_CASE(TestRingBufferBasic) {
    ma_ring_buffer rb(1024);
    ASSERT_EQ(rb.available_write(), 1023);
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
    
    auto wa = rb.acquire_write(200);
    ASSERT_EQ(wa.size, 200);
    rb.commit_write(200);
    
    auto ra = rb.acquire_read(150);
    ASSERT_EQ(ra.size, 150);
    rb.commit_read(150);
    
    wa = rb.acquire_write(100);
    ASSERT_TRUE(wa.ptr != nullptr);
    rb.commit_write(100);
    
    ra = rb.acquire_read(150);
    ASSERT_TRUE(ra.ptr != nullptr);
    rb.commit_read(150);
    
    ASSERT_EQ(rb.available_read(), 0);
}

TEST_CASE(TestFormatConversion) {
    uint8_t u8_val = 255;
    float f32 = ma_convert_sample_to_f32(&u8_val, MA_FORMAT_U8);
    ASSERT_TRUE(std::abs(f32 - 1.0f) < 0.01f);
    
    u8_val = 0;
    f32 = ma_convert_sample_to_f32(&u8_val, MA_FORMAT_U8);
    ASSERT_TRUE(std::abs(f32 - (-1.0f)) < 0.01f);
    
    u8_val = 128;
    f32 = ma_convert_sample_to_f32(&u8_val, MA_FORMAT_U8);
    ASSERT_TRUE(std::abs(f32 - 0.0f) < 0.01f);
    
    int16_t s16_val = 32767;
    f32 = ma_convert_sample_to_f32(&s16_val, MA_FORMAT_S16);
    ASSERT_TRUE(std::abs(f32 - 1.0f) < 0.001f);
    
    s16_val = -32768;
    f32 = ma_convert_sample_to_f32(&s16_val, MA_FORMAT_S16);
    ASSERT_TRUE(std::abs(f32 - (-1.0f)) < 0.001f);
    
    float f = 0.5f;
    int16_t out_s16;
    ma_convert_sample_from_f32(f, &out_s16, MA_FORMAT_S16);
    ASSERT_EQ(out_s16, 16383);
    
    f = -0.5f;
    ma_convert_sample_from_f32(f, &out_s16, MA_FORMAT_S16);
    ASSERT_EQ(out_s16, -16383);
}

TEST_CASE(TestFormatConversionBuffer) {
    int16_t s16_data[4] = {32767, -32768, 0, 16384};
    float f32_data[4];
    
    ma_convert_frames(s16_data, MA_FORMAT_S16, f32_data, MA_FORMAT_F32, 2, 2);
    
    ASSERT_TRUE(std::abs(f32_data[0] - 1.0f) < 0.001f);
    ASSERT_TRUE(std::abs(f32_data[1] - (-1.0f)) < 0.001f);
    ASSERT_TRUE(std::abs(f32_data[2] - 0.0f) < 0.001f);
    ASSERT_TRUE(std::abs(f32_data[3] - 0.5f) < 0.001f);
}

TEST_CASE(TestChannelRemix) {
    ma_remix_matrix stereo_to_mono = ma_build_stereo_to_mono_matrix();
    ASSERT_EQ(stereo_to_mono.in_channels, 2);
    ASSERT_EQ(stereo_to_mono.out_channels, 1);
    ASSERT_EQ(stereo_to_mono.at(0, 0), 0.5f);
    ASSERT_EQ(stereo_to_mono.at(0, 1), 0.5f);
    
    ma_remix_matrix mono_to_stereo = ma_build_mono_to_stereo_matrix();
    ASSERT_EQ(mono_to_stereo.in_channels, 1);
    ASSERT_EQ(mono_to_stereo.out_channels, 2);
    ASSERT_EQ(mono_to_stereo.at(0, 0), 1.0f);
    ASSERT_EQ(mono_to_stereo.at(1, 0), 1.0f);
}

TEST_CASE(TestApplyRemix) {
    ma_remix_matrix stereo_to_mono = ma_build_stereo_to_mono_matrix();
    
    float stereo[4] = {1.0f, -1.0f, 0.5f, -0.5f};
    float mono[2];
    
    ma_apply_remix(stereo, mono, 2, stereo_to_mono);
    
    ASSERT_EQ(mono[0], 0.0f);
    ASSERT_EQ(mono[1], 0.0f);
    
    stereo[0] = 1.0f; stereo[1] = 1.0f;
    stereo[2] = -1.0f; stereo[3] = 1.0f;
    ma_apply_remix(stereo, mono, 2, stereo_to_mono);
    ASSERT_EQ(mono[0], 1.0f);
    ASSERT_EQ(mono[1], 0.0f);
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
    uint8_t s24_max[3] = {0xFF, 0xFF, 0x7F};
    float f = ma_convert_sample_to_f32(s24_max, MA_FORMAT_S24);
    ASSERT_TRUE(std::abs(f - 1.0f) < 0.0001f);
    
    uint8_t s24_min[3] = {0x00, 0x00, 0x80};
    f = ma_convert_sample_to_f32(s24_min, MA_FORMAT_S24);
    ASSERT_TRUE(std::abs(f - (-1.0f)) < 0.0001f);
    
    float f32 = 0.5f;
    uint8_t out_s24[3];
    ma_convert_sample_from_f32(f32, out_s24, MA_FORMAT_S24);
    
    int32_t reconstructed = (out_s24[2] << 16) | (out_s24[1] << 8) | out_s24[0];
    if (reconstructed & 0x800000) reconstructed |= ~0xFFFFFF;
    float back = reconstructed / 8388608.0f;
    ASSERT_TRUE(std::abs(back - 0.5f) < 0.001f);
}

int main() { return RUN_ALL_TESTS(); }