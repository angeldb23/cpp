/* =============================================================================
   MÓDULO 24 - SOLUCIÓN 05: Memory tracker por categoría
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 06 (memory tracker, peak, churn, historia).

   CONSIGNA
   --------
   Completá `MemoryTracker`:

   - `alloc(tag, bytes)`: suma a la categoría, actualiza `pico[tag]`, incrementa
     el contador de operaciones y agrega una muestra del TOTAL a la historia
     (ring buffer de capacidad fija).
   - `free(tag, bytes)`: resta, pero CLAMPEA en 0 si se libera de más. También
     incrementa operaciones y muestrea.
   - `actual(tag)`, `pico(tag)`, `total()` (suma de todas las categorías),
     `operaciones()` (alloc+free), `historia()`.
   - El pico NO baja cuando se libera.
   ============================================================================= */
#include "../include/gui_util.hpp"
#include "../../include/test_framework.hpp"

#include <array>
#include <cstddef>
#include <deque>

enum class Tag : std::size_t { General, Textura, Malla, Audio, Script, Count };
constexpr std::size_t kTags = static_cast<std::size_t>(Tag::Count);

class MemoryTracker {
public:
    explicit MemoryTracker(std::size_t capacidadHistoria) : capacidad_(capacidadHistoria) {}

    void alloc(Tag t, std::size_t bytes) {
        auto& c = actual_[static_cast<std::size_t>(t)];
        c += bytes;
        pico_[static_cast<std::size_t>(t)] =
            std::max(pico_[static_cast<std::size_t>(t)], c);
        ++allocs_;
        muestrear();
    }

    void free(Tag t, std::size_t bytes) {
        auto& c = actual_[static_cast<std::size_t>(t)];
        c = (bytes > c) ? 0 : (c - bytes);
        ++frees_;
        muestrear();
    }

    std::size_t actual(Tag t) const { return actual_[static_cast<std::size_t>(t)]; }
    std::size_t pico(Tag t) const { return pico_[static_cast<std::size_t>(t)]; }

    std::size_t total() const {
        std::size_t s = 0;
        for (auto v : actual_) s += v;
        return s;
    }

    std::size_t operaciones() const { return allocs_ + frees_; }
    const std::deque<std::size_t>& historia() const { return historia_; }

private:
    void muestrear() {
        historia_.push_back(total());
        while (historia_.size() > capacidad_) historia_.pop_front();
    }

    std::array<std::size_t, kTags> actual_{};
    std::array<std::size_t, kTags> pico_{};
    std::deque<std::size_t> historia_;
    std::size_t capacidad_ = 0;
    std::size_t allocs_ = 0;
    std::size_t frees_ = 0;
};

// ============================================================================
// TESTS (no tocar)
// ============================================================================
TEST_CASE(AllocYFree) {
    MemoryTracker m(16);
    m.alloc(Tag::Textura, 1000);
    m.alloc(Tag::Textura, 500);
    ASSERT_EQ(m.actual(Tag::Textura), 1500u);
    m.free(Tag::Textura, 700);
    ASSERT_EQ(m.actual(Tag::Textura), 800u);
}

TEST_CASE(FreeDeMasClampeaEnCero) {
    MemoryTracker m(16);
    m.alloc(Tag::Audio, 100);
    m.free(Tag::Audio, 500);
    ASSERT_EQ(m.actual(Tag::Audio), 0u);
}

TEST_CASE(PicoNoBaja) {
    MemoryTracker m(16);
    m.alloc(Tag::Malla, 4096);
    m.alloc(Tag::Malla, 2048);
    m.free(Tag::Malla, 6000);
    ASSERT_EQ(m.actual(Tag::Malla), 144u);
    ASSERT_EQ(m.pico(Tag::Malla), 6144u);
}

TEST_CASE(TotalSumaCategorias) {
    MemoryTracker m(16);
    m.alloc(Tag::General, 10);
    m.alloc(Tag::Textura, 20);
    m.alloc(Tag::Script, 30);
    ASSERT_EQ(m.total(), 60u);
    m.free(Tag::Textura, 5);
    ASSERT_EQ(m.total(), 55u);
}

TEST_CASE(HistoriaRingBuffer) {
    MemoryTracker m(3);
    m.alloc(Tag::General, 10);   // 10
    m.alloc(Tag::General, 10);   // 20
    m.alloc(Tag::General, 10);   // 30
    m.alloc(Tag::General, 10);   // 40
    ASSERT_EQ(m.historia().size(), 3u);
    ASSERT_EQ(m.historia().front(), 20u);
    ASSERT_EQ(m.historia().back(), 40u);
}

TEST_CASE(OperacionesIncluyenAllocYFree) {
    MemoryTracker m(8);
    m.alloc(Tag::General, 1);
    m.alloc(Tag::General, 1);
    m.free(Tag::General, 1);
    ASSERT_EQ(m.operaciones(), 3u);
}

int main() {
    std::cout << "Solución 05 (memory tracker)...\n";
    return RUN_ALL_TESTS();
}
