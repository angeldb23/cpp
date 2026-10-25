/* =============================================================================
   MÓDULO 26 - SOLUCIÓN 03: sincronización — ring de frames in-flight
   =============================================================================
   La solución de ejercicio_03_frame_ring.cpp. La regla del modelo es
   simple y ES la que justifica el patrón: un slot recién enviado no se
   puede reciclar (el GPU lo está leyendo); con N>=2 slots en vuelo la
   CPU tiene un frame de margen y nunca bloquea. Todo motor Vulkan
   moderno usa 2 o 3 in-flight frames; el costo es duplicar la familia
   de recursos per-frame (command pools, semáforos, uniform buffers).
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <cstdint>
#include <vector>

namespace {

struct InFlightSlot {
    std::uint32_t slot_id       = 0;
    int           ultimo_submit = -1000000; /* "nunca submit" */
};

class FrameRing {
public:
    explicit FrameRing(std::size_t n) : frames_(n) {
        for (std::size_t i = 0; i < n; ++i) frames_[i].slot_id = std::uint32_t(i);
    }

    std::uint32_t frame() {
        auto& f = frames_[frame_ % frames_.size()];

        /* vkWaitForFences humanizado: si el GPU todavía no terminó el
           submit previo de ESTE slot (modelo: termina 1 frame después),
           la CPU se frena UNA vez y sigue. */
        const int diff = static_cast<int>(frame_) - f.ultimo_submit;
        if (diff <= 1) ++esperas_;

        const std::uint32_t usado = frame_ % std::uint32_t(frames_.size());
        f.ultimo_submit = static_cast<int>(frame_);
        ++frame_;
        return usado;
    }

    int esperas_cpu() const { return esperas_; }

private:
    std::vector<InFlightSlot> frames_;
    std::uint32_t frame_ = 0;
    int           esperas_ = 0;
};

} // namespace

/* ------------------------------ TESTS (no tocar)-------------------------- */
TEST_CASE(RingUnSoloSlotFrena) {
    FrameRing r(1);
    std::uint32_t esperado = 0;
    for (int i = 0; i < 7; ++i) esperado = r.frame();
    ASSERT_EQ(esperado, 0u);
    ASSERT_EQ(r.esperas_cpu(), 6);
}

TEST_CASE(RingDobleNoFrena) {
    FrameRing r(2);
    for (int i = 0; i < 10; ++i) (void)r.frame();
    ASSERT_EQ(r.esperas_cpu(), 0);
}

TEST_CASE(AlternanciaDeSlots) {
    FrameRing r(3);
    ASSERT_EQ(r.frame(), 0u);
    ASSERT_EQ(r.frame(), 1u);
    ASSERT_EQ(r.frame(), 2u);
    ASSERT_EQ(r.frame(), 0u);
    ASSERT_EQ(r.esperas_cpu(), 0);
}

TEST_CASE(GpuAtrasadoFrena) {
    FrameRing r(1);
    (void)r.frame();
    ASSERT_EQ(r.frame(), 0u);
    ASSERT_EQ(r.esperas_cpu(), 1);
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
