/* =============================================================================
   MÓDULO 16 - LECCIÓN 13: Profiling Básico — Frame Time, FPS, Percentiles, Spikes
   =============================================================================
   TEMA 9 de TEMAS.md: Profiling básico en el bucle.

   MÉTRICAS FUNDAMENTALES:
   -----------------------
   - Frame Time: tiempo total de un frame (Update + Render + Swap + Sleep).
   - FPS (Frames Per Second): 1 / frameTime (promedio sobre ventana de tiempo).
   - Frame Budget: 16.67ms para 60 FPS, 33.33ms para 30 FPS, 8.33ms para 120 FPS.
   - Percentiles (p50, p95, p99): distribución de frame times, no solo promedio.

   POR QUÉ NO SOLO FPS PROMEDIO:
   -----------------------------
   - 60 FPS promedio puede esconder: 59 frames a 16ms + 1 frame a 200ms (spike).
   - El spike se percibe como "lag" aunque el promedio sea bueno.
   - Percentiles revelan cola de distribución (p99 = peor 1%).

   HISTOGRAMA DE FRAME TIMES (rolling window):
   -------------------------------------------
   class FrameProfiler {
       static constexpr size_t MAX_SAMPLES = 600;  // 10 segundos a 60 FPS
       std::array<float, MAX_SAMPLES> samples_{};
       size_t head_ = 0, count_ = 0;

       void AddFrame(float dt) {
           samples_[head_] = dt;
           head_ = (head_ + 1) % MAX_SAMPLES;
           count_ = std::min(count_ + 1, MAX_SAMPLES);
       }

       float Percentile(float p) const {  // p en [0, 100]
           if (count_ == 0) return 0;
           std::vector<float> sorted(samples_.begin(), samples_.begin() + count_);
           std::nth_element(sorted.begin(),
               sorted.begin() + size_t(count_ * p / 100.0f), sorted.end());
           return sorted[size_t(count_ * p / 100.0f)];
       }

       float Average() const {
           if (count_ == 0) return 0;
           float sum = 0;
           for (size_t i = 0; i < count_; ++i) sum += samples_[i];
           return sum / count_;
       }

       float Min() const { return 0; }  // implementado abajo
       float Max() const { return 0; }  // implementado abajo
   };

   DETECCIÓN DE SPIKES:
   --------------------
   - Spike = frame time > UMBRAL (ej: 2x budget = 33ms para 60 FPS).
   - Contar spikes por segundo, loggear si > N por minuto.
   - Capturar stack trace en spike (ver Módulo 24 - profilador avanzado).

   SPIKE DETECTOR SIMPLE:
   ----------------------
   struct SpikeDetector {
       float thresholdMs_;
       int spikeCount_ = 0;
       float lastSpikeTime_ = 0;

       void Check(float frameTimeMs, float currentTime) {
           if (frameTimeMs > thresholdMs_) {
               spikeCount_++;
               std::cout << "[SPIKE] " << frameTimeMs << "ms at t=" << currentTime << "\n";
               // Opcional: guardar mini-dump, stack trace, screenshot
           }
       }
   };

   HUD DE PROFILING EN PANTALLA:
   -----------------------------
   void DrawProfilerHUD(const FrameProfiler& prof) {
       char buf[256];
       snprintf(buf, sizeof(buf),
           "FPS: %.1f (avg %.2fms)\n"
           "p50: %.2fms  p95: %.2fms  p99: %.2fms\n"
           "Min: %.2fms  Max: %.2fms  Spikes: %d",
           1000.0f / prof.Average(), prof.Average(),
           prof.Percentile(50), prof.Percentile(95), prof.Percentile(99),
           prof.Min(), prof.Max(), spikeDetector.spikeCount_);
       DrawText(buf, 10, 10, 18, GREEN);
   }

   SCOPED TIMERS (para medir secciones):
   -------------------------------------
   class ScopedTimer {
       const char* name_;
       std::chrono::steady_clock::time_point start_;
       float* outMs_;

   public:
       ScopedTimer(const char* name, float* outMs)
           : name_(name), start_(std::chrono::steady_clock::now()), outMs_(outMs) {}
       ~ScopedTimer() {
           auto end = std::chrono::steady_clock::now();
           *outMs_ = std::chrono::duration<float, std::milli>(end - start_).count();
       }
   };

   // Uso:
   float updateMs = 0, renderMs = 0;
   {
       ScopedTimer t("Update", &updateMs);
       Update(dt);
   }
   {
       ScopedTimer t("Render", &renderMs);
       Render(alpha);
   }
   // updateMs, renderMs disponibles para HUD/log

   INTEGRACIÓN CON RAYLIB:
   -----------------------
   - GetFPS(): FPS actual (promedio últimos 60 frames aprox).
   - GetFrameTime(): dt del último frame.
   - No hay GetFrameTimeHistory(); implementar FrameProfiler propio.

   PERF (Linux) PARA ANÁLISIS PROFUNDO:
   ------------------------------------
   perf record -g ./mi_juego
   perf report
   # Muestra % tiempo en cada función, call graph, cache misses.

   VALGRIND/CALLGRIND:
   -------------------
   valgrind --tool=callgrind ./mi_juego
   kcachegrind callgrind.out.xxx
   # Visualización gráfica de coste por función, cache simulation.

   ============================================================================= */

#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include <array>

class FrameProfiler {
    static constexpr size_t MAX_SAMPLES = 600;
    std::array<float, MAX_SAMPLES> samples_{};
    size_t head_ = 0, count_ = 0;

public:
    void AddFrame(float dtMs) {
        samples_[head_] = dtMs;
        head_ = (head_ + 1) % MAX_SAMPLES;
        count_ = std::min(count_ + 1, MAX_SAMPLES);
    }

    float Average() const {
        if (count_ == 0) return 0;
        float sum = 0;
        for (size_t i = 0; i < count_; ++i) sum += samples_[i];
        return sum / count_;
    }

    float Min() const {
        if (count_ == 0) return 0;
        float m = samples_[0];
        for (size_t i = 1; i < count_; ++i) m = std::min(m, samples_[i]);
        return m;
    }

    float Max() const {
        if (count_ == 0) return 0;
        float m = samples_[0];
        for (size_t i = 1; i < count_; ++i) m = std::max(m, samples_[i]);
        return m;
    }

    float Percentile(float p) const {
        if (count_ == 0) return 0;
        std::vector<float> sorted(samples_.begin(), samples_.begin() + count_);
        size_t idx = std::min<size_t>(count_ - 1, size_t(count_ * p / 100.0f));
        std::nth_element(sorted.begin(), sorted.begin() + idx, sorted.end());
        return sorted[idx];
    }

    size_t Count() const { return count_; }
};

struct ScopedTimer {
    const char* name_;
    std::chrono::steady_clock::time_point start_;
    float* outMs_;
    ScopedTimer(const char* name, float* out) : name_(name), start_(std::chrono::steady_clock::now()), outMs_(out) {}
    ~ScopedTimer() {
        auto end = std::chrono::steady_clock::now();
        *outMs_ = std::chrono::duration<float, std::milli>(end - start_).count();
    }
};

struct SpikeDetector {
    float thresholdMs_;
    int count_ = 0;
    void Check(float frameMs) {
        if (frameMs > thresholdMs_) {
            ++count_;
            std::cout << "  [SPIKE] Frame: " << frameMs << "ms (threshold: " << thresholdMs_ << "ms)\n";
        }
    }
};

void SimulateFrame(float baseMs, float variance, bool spike) {
    float dt = baseMs + (rand() % 100) / 100.0f * variance;
    if (spike) dt += 50.0f;  // Spike artificial
    std::this_thread::sleep_for(std::chrono::microseconds(int(dt * 1000)));
}

int main() {
    std::cout << "=== LECCIÓN 13: Profiling Básico ===\n";

    FrameProfiler profiler;
    SpikeDetector spikes{33.33f};  // 2x budget @ 60 FPS (16.67ms)

    float updateMs = 0, renderMs = 0;
    srand(42);

    std::cout << "\nSimulando 300 frames (5 seg @ 60 FPS) con spikes ocasionales...\n";
    for (int frame = 0; frame < 300; ++frame) {
        auto frameStart = std::chrono::steady_clock::now();

        { ScopedTimer t("Update", &updateMs); SimulateFrame(8, 2, frame % 50 == 0); }
        { ScopedTimer t("Render", &renderMs); SimulateFrame(5, 1, false); }

        auto frameEnd = std::chrono::steady_clock::now();
        float frameMs = std::chrono::duration<float, std::milli>(frameEnd - frameStart).count();

        profiler.AddFrame(frameMs);
        spikes.Check(frameMs);
    }

    std::cout << "\n--- RESULTADOS ---\n";
    std::cout << "Frames capturados: " << profiler.Count() << "\n";
    std::cout << "Promedio: " << profiler.Average() << " ms (" << 1000.0f / profiler.Average() << " FPS)\n";
    std::cout << "Min: " << profiler.Min() << " ms\n";
    std::cout << "Max: " << profiler.Max() << " ms\n";
    std::cout << "p50 (mediana): " << profiler.Percentile(50) << " ms\n";
    std::cout << "p95: " << profiler.Percentile(95) << " ms\n";
    std::cout << "p99: " << profiler.Percentile(99) << " ms\n";
    std::cout << "Spikes detectados (>33ms): " << spikes.count_ << "\n";
    std::cout << "Update avg: " << updateMs << " ms (ultimo frame)\n";
    std::cout << "Render avg: " << renderMs << " ms (ultimo frame)\n";

    std::cout << "\nPercentiles > promedio revelan cola de distribución (spikes).\n";
    std::cout << "=== FIN LECCIÓN 13 ===\n";
    return 0;
}