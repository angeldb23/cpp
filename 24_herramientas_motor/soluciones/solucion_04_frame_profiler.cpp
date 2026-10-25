/* =============================================================================
   MÓDULO 24 - SOLUCIÓN 04: Profiler de frames y scopes
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 05 (frame graph, percentiles y spikes).

   CONSIGNA
   --------
   Completá `FrameProfiler`:

   1. `add(m)` guarda la muestra; si supera `capacidad`, descarta la más vieja.
   2. `averageFrameMs()` = promedio de `frameMs`.
   3. `percentileFrameMs(p)` = nearest-rank: ordená una copia y tomá el
      índice `ceil(p/100 * n) - 1`, con p clampeado a [0,100] y mínimo 1.
      Con n=0 devolvé 0.
   4. `spikeCount(umbral)` cuenta frames con `frameMs > umbral` (estricto).
   5. `maxFrameMs()` = el mayor (0 si no hay muestras).
   6. `beginScope(nombre, ahoraMs)` guarda el inicio; `endScope(nombre,
      ahoraMs)` ACUMULA la duración en el mapa. `scopeMs(nombre)` devuelve lo
      acumulado (0 si no existe). `scopes()` devuelve pares ordenados por
      nombre.
   ============================================================================= */
#include "../include/gui_util.hpp"
#include "../../include/test_framework.hpp"

#include <algorithm>
#include <cmath>
#include <deque>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

struct FrameSample {
    double frameMs = 0.0;
    double cpuMs = 0.0;
    double gpuMs = 0.0;
};

class FrameProfiler {
public:
    explicit FrameProfiler(std::size_t capacidad) : capacidad_(capacidad) {}

    void add(FrameSample m) {
        muestras_.push_back(m);
        while (muestras_.size() > capacidad_) muestras_.pop_front();
    }

    double averageFrameMs() const {
        if (muestras_.empty()) return 0.0;
        double suma = 0.0;
        for (const auto& m : muestras_) suma += m.frameMs;
        return suma / static_cast<double>(muestras_.size());
    }

    double percentileFrameMs(double p) const {
        if (muestras_.empty()) return 0.0;
        std::vector<double> v;
        v.reserve(muestras_.size());
        for (const auto& m : muestras_) v.push_back(m.frameMs);
        std::sort(v.begin(), v.end());
        p = gui::clampf(static_cast<float>(p), 0.0f, 100.0f);
        std::size_t idx = static_cast<std::size_t>(std::ceil(p / 100.0 * v.size()));
        if (idx == 0) idx = 1;
        return v[idx - 1];
    }

    std::size_t spikeCount(double umbralMs) const {
        std::size_t n = 0;
        for (const auto& m : muestras_) if (m.frameMs > umbralMs) ++n;
        return n;
    }

    double maxFrameMs() const {
        double m = 0.0;
        for (const auto& x : muestras_) m = std::max(m, x.frameMs);
        return m;
    }

    std::size_t count() const { return muestras_.size(); }

    void beginScope(std::string nombre, double ahoraMs) {
        inicios_[std::move(nombre)] = ahoraMs;
    }

    void endScope(std::string nombre, double ahoraMs) {
        auto it = inicios_.find(nombre);
        if (it == inicios_.end()) return;
        acumulado_[nombre] += ahoraMs - it->second;
        inicios_.erase(it);
    }

    double scopeMs(std::string_view nombre) const {
        auto it = acumulado_.find(std::string(nombre));
        return it == acumulado_.end() ? 0.0 : it->second;
    }

    std::vector<std::pair<std::string, double>> scopes() const {
        return {acumulado_.begin(), acumulado_.end()};
    }

private:
    std::size_t capacidad_ = 0;
    std::deque<FrameSample> muestras_;
    std::map<std::string, double> inicios_;
    std::map<std::string, double> acumulado_;
};

// ============================================================================
// TESTS (no tocar)
// ============================================================================
TEST_CASE(Promedio) {
    FrameProfiler p(10);
    for (double ms : {10.0, 20.0, 30.0, 40.0, 50.0}) p.add({ms, ms * 0.5, ms * 0.8});
    ASSERT_NEAR(p.averageFrameMs(), 30.0, 1e-9);
}

TEST_CASE(PercentilNearestRank) {
    FrameProfiler p(100);
    for (int i = 1; i <= 20; ++i) p.add({static_cast<double>(i), 0.0, 0.0});
    ASSERT_NEAR(p.percentileFrameMs(95.0), 19.0, 1e-9);
    ASSERT_NEAR(p.percentileFrameMs(100.0), 20.0, 1e-9);
    ASSERT_NEAR(p.percentileFrameMs(0.0), 1.0, 1e-9);
    ASSERT_NEAR(p.percentileFrameMs(50.0), 10.0, 1e-9);
}

TEST_CASE(SpikesEstrictosYMaximo) {
    FrameProfiler p(10);
    p.add({10.0, 0, 0});
    p.add({25.0, 0, 0});
    p.add({20.0, 0, 0});
    p.add({40.0, 0, 0});
    ASSERT_EQ(p.spikeCount(20.0), 2u);
    ASSERT_NEAR(p.maxFrameMs(), 40.0, 1e-9);
}

TEST_CASE(RingBuffer) {
    FrameProfiler p(3);
    p.add({1.0, 0, 0});
    p.add({2.0, 0, 0});
    p.add({3.0, 0, 0});
    p.add({4.0, 0, 0});
    ASSERT_EQ(p.count(), 3u);
    ASSERT_NEAR(p.averageFrameMs(), 3.0, 1e-9);
}

TEST_CASE(ScopesAcumulan) {
    FrameProfiler p(10);
    p.beginScope("fisica", 1.0);
    p.endScope("fisica", 4.0);
    p.beginScope("fisica", 10.0);
    p.endScope("fisica", 11.0);
    p.beginScope("render", 0.0);
    p.endScope("render", 2.5);
    ASSERT_NEAR(p.scopeMs("fisica"), 4.0, 1e-9);
    ASSERT_NEAR(p.scopeMs("render"), 2.5, 1e-9);
    ASSERT_NEAR(p.scopeMs("audio"), 0.0, 1e-9);
    const auto s = p.scopes();
    ASSERT_EQ(s.size(), 2u);
    ASSERT_EQ(s[0].first, std::string("fisica"));
    ASSERT_EQ(s[1].first, std::string("render"));
}

TEST_CASE(VacioNoRompe) {
    FrameProfiler p(5);
    ASSERT_EQ(p.count(), 0u);
    ASSERT_NEAR(p.averageFrameMs(), 0.0, 1e-9);
    ASSERT_NEAR(p.percentileFrameMs(95.0), 0.0, 1e-9);
    ASSERT_NEAR(p.maxFrameMs(), 0.0, 1e-9);
}

int main() {
    std::cout << "Solución 04 (frame profiler)...\n";
    return RUN_ALL_TESTS();
}
