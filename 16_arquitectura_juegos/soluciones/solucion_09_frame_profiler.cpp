/* =============================================================================
   MÓDULO 16 - SOLUCIÓN 09: Frame Profiler — Histograma, Percentiles, Spike Detection
   ============================================================================= */
#include <array>
#include <vector>
#include <algorithm>
#include <cmath>
#include "../../include/test_framework.hpp"

class FrameProfiler {
    static constexpr size_t MAX_SAMPLES = 600;
    std::array<float, MAX_SAMPLES> samples_{};
    size_t head_ = 0;
    size_t count_ = 0;

public:
    static constexpr size_t MaxSamples() { return MAX_SAMPLES; }
    void AddFrame(float dtMs) {
        samples_[head_] = dtMs;
        head_ = (head_ + 1) % MAX_SAMPLES;
        count_ = std::min(count_ + 1, MAX_SAMPLES);
    }

    float Average() const {
        if (count_ == 0) return 0.0f;
        float sum = 0.0f;
        for (size_t i = 0; i < count_; ++i) sum += samples_[i];
        return sum / count_;
    }

    float Min() const {
        if (count_ == 0) return 0.0f;
        float m = samples_[0];
        for (size_t i = 1; i < count_; ++i) m = std::min(m, samples_[i]);
        return m;
    }

    float Max() const {
        if (count_ == 0) return 0.0f;
        float m = samples_[0];
        for (size_t i = 1; i < count_; ++i) m = std::max(m, samples_[i]);
        return m;
    }

    float Percentile(float p) const {
        if (count_ == 0) return 0.0f;
        std::vector<float> sorted(samples_.begin(), samples_.begin() + count_);
        size_t idx = std::min<size_t>(count_ - 1, size_t(count_ * p / 100.0f));
        std::nth_element(sorted.begin(), sorted.begin() + idx, sorted.end());
        return sorted[idx];
    }

    size_t Count() const { return count_; }
};

struct SpikeDetector {
    float thresholdMs_;
    int count_ = 0;

    void Check(float frameMs) {
        if (frameMs > thresholdMs_) ++count_;
    }

    int Count() const { return count_; }
};

// ============= TESTS =============
TEST_CASE(AddFrameFillsBuffer) {
    FrameProfiler p;
    for (int i = 0; i < 10; ++i) p.AddFrame(16.0f + i * 0.1f);
    ASSERT_EQ(p.Count(), 10u);
}

TEST_CASE(CircularBufferOverwrites) {
    FrameProfiler p;
    for (int i = 0; i < 700; ++i) p.AddFrame(float(i));
    ASSERT_EQ(p.Count(), FrameProfiler::MaxSamples());
}

TEST_CASE(AverageCorrect) {
    FrameProfiler p;
    p.AddFrame(10.0f);
    p.AddFrame(20.0f);
    p.AddFrame(30.0f);
    ASSERT_NEAR(p.Average(), 20.0f, 0.001f);
}

TEST_CASE(MinMaxCorrect) {
    FrameProfiler p;
    p.AddFrame(5.0f);
    p.AddFrame(15.0f);
    p.AddFrame(25.0f);
    ASSERT_NEAR(p.Min(), 5.0f, 0.001f);
    ASSERT_NEAR(p.Max(), 25.0f, 0.001f);
}

TEST_CASE(PercentileMedian) {
    FrameProfiler p;
    for (int i = 1; i <= 100; ++i) p.AddFrame(float(i));
    ASSERT_NEAR(p.Percentile(50.0f), 50.0f, 1.0f);
}

TEST_CASE(Percentile95NearMax) {
    FrameProfiler p;
    for (int i = 1; i <= 100; ++i) p.AddFrame(float(i));
    float p95 = p.Percentile(95.0f);
    ASSERT_TRUE(p95 >= 90.0f);
    ASSERT_TRUE(p95 <= 100.0f);
}

TEST_CASE(Percentile99NearMax) {
    FrameProfiler p;
    for (int i = 1; i <= 100; ++i) p.AddFrame(float(i));
    float p99 = p.Percentile(99.0f);
    ASSERT_TRUE(p99 >= 95.0f);
    ASSERT_TRUE(p99 <= 100.0f);
}

TEST_CASE(SpikeDetectorCounts) {
    SpikeDetector sd{20.0f};
    sd.Check(16.0f);
    sd.Check(25.0f);
    sd.Check(18.0f);
    sd.Check(30.0f);
    ASSERT_EQ(sd.Count(), 2);
}

TEST_CASE(SpikeDetectorNoFalsePositives) {
    SpikeDetector sd{16.67f};
    for (int i = 0; i < 100; ++i) sd.Check(16.0f);
    ASSERT_EQ(sd.Count(), 0);
}

int main() {
    std::cout << "Solución 09 (Frame Profiler)...\n";
    return RUN_ALL_TESTS();
}