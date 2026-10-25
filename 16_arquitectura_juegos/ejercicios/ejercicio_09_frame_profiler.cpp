/* =============================================================================
   MÓDULO 16 - EJERCICIO 09: Frame Profiler — Histograma, Percentiles, Spike Detection
   =============================================================================
   DIFICULTAD: Avanzado
   APOYO: Lección 13 (Profiling básico)

   CONSIGNA:
   Implementa FrameProfiler con ventana deslizante (rolling window):
   - AddFrame(dtMs): agrega sample, sobrescribe el más viejo (circular buffer)
   - Average(), Min(), Max(): estadísticas básicas
   - Percentile(p): p-ésimo percentil (p50, p95, p99) usando nth_element
   - SpikeDetector: threshold + count

   ESTRUCTURA:
   - static constexpr MAX_SAMPLES = 600 (10 seg @ 60 FPS)
   - array<float, MAX_SAMPLES> + head + count
   - Percentile: copiar samples válidos a vector, nth_element, retornar valor

   TESTS:
   - AddFrame llena buffer circular
   - Average/Min/Max correctos
   - Percentile(50) = mediana, Percentile(99) ≈ máximo
   - SpikeDetector cuenta frames > threshold

   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_09_frame_profiler.cpp -o ej09 && ./ej09
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
    // TODO 1: AddFrame - circular buffer
    // TU CÓDIGO AQUÍ:
    void AddFrame(float dtMs) {
        // Pista: samples_[head_] = dtMs; head_ = (head_ + 1) % MAX_SAMPLES; count_ = min(count_+1, MAX_SAMPLES);
    }

    // TODO 2: Average
    // TU CÓDIGO AQUÍ:
    float Average() const {
        // Pista: if (count_ == 0) return 0; sum / count_
        return 0.0f;
    }

    // TODO 3: Min
    // TU CÓDIGO AQUÍ:
    float Min() const {
        // Pista: if (count_ == 0) return 0; loop min
        return 0.0f;
    }

    // TODO 4: Max
    // TU CÓDIGO AQUÍ:
    float Max() const {
        // Pista: if (count_ == 0) return 0; loop max
        return 0.0f;
    }

    // TODO 5: Percentile using nth_element
    // TU CÓDIGO AQUÍ:
    float Percentile(float p) const {
        // Pista: if (count_ == 0) return 0; copy samples to vector; nth_element at idx = count_ * p / 100
        return 0.0f;
    }

    size_t Count() const { return count_; }
};

struct SpikeDetector {
    float thresholdMs_;
    int count_ = 0;

    // TODO 6: Check
    // TU CÓDIGO AQUÍ:
    void Check(float frameMs) {
        // Pista: if (frameMs > thresholdMs_) ++count_;
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
    for (int i = 0; i < 700; ++i) p.AddFrame(float(i));  // > MAX_SAMPLES
    ASSERT_EQ(p.Count(), FrameProfiler::MaxSamples());
    // Los más viejos (0-99) deberían haberse sobrescrito
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
    for (int i = 1; i <= 100; ++i) p.AddFrame(float(i));  // 1..100
    ASSERT_NEAR(p.Percentile(50.0f), 50.0f, 1.0f);  // Mediana ≈ 50
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
    SpikeDetector sd{20.0f};  // threshold 20ms
    sd.Check(16.0f);  // normal
    sd.Check(25.0f);  // spike
    sd.Check(18.0f);  // normal
    sd.Check(30.0f);  // spike
    ASSERT_EQ(sd.Count(), 2);
}

TEST_CASE(SpikeDetectorNoFalsePositives) {
    SpikeDetector sd{16.67f};  // 60 FPS budget
    for (int i = 0; i < 100; ++i) sd.Check(16.0f);  // todos normales
    ASSERT_EQ(sd.Count(), 0);
}

int main() {
    std::cout << "Ejecutando pruebas para Ejercicio 09 (Frame Profiler)...\n";
    return RUN_ALL_TESTS();
}