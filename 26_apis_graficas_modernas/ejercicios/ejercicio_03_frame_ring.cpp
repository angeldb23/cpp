/* =============================================================================
   MÓDULO 26 - EJERCICIO 03: sincronización — ring de frames in-flight
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 03 (command buffers, fences y semáforos; el patrón
   in-flight).

   CONSIGNA
   --------
   Implementá el simulador del ring de frames (sin driver):

   `FrameRing(n)` con N slots y el método `frame()` que:
   1. mira el slot i % N;
   2. si el slot está BUSY (pasó 1 frame o menos desde su último submit:
      el GPU "termina" cada submit un frame atrás), cuenta una
      `espera_cpu` (el vkWaitForFences frenó a la CPU);
   3. registra el submit (slot.ultimo_submit = frame_ actual) y avanza
      el contador de frame, devolviendo el slot usado.

   La regla del modelo (LECCIÓN 03): busy <=> (frame_- ultimo_submit) <= 1.
   Con ring=1 casi SIEMPRE frena (solo la 1° vez no); con ring>=2, la CPU
   no frena nunca (el GPU le viene ganando un frame de margen).
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
        // TODO: mirar slot, contar espera si busy, registrar submit
        (void)frames_;
        return 0;   // stub: siempre slot 0, sin registrar
    }

    int esperas_cpu() const { return esperas_; }

private:
    std::vector<InFlightSlot> frames_;
    std::uint32_t frame_       = 0;
    int           esperas_     = 0;
};

} // namespace

/* ------------------------------ TESTS (no tocar)-------------------------- */
TEST_CASE(RingUnSoloSlotFrena) {
    FrameRing r(1);
    /* 7 frames: el 1° no espera (nunca se envió nada); los 6 siguientes
       encuentran el mismo slot busy (diff==1). */
    std::uint32_t esperado = 0;
    for (int i = 0; i < 7; ++i) esperado = r.frame();
    ASSERT_EQ(esperado, 0u);           /* ring=1: siempre el slot 0 */
    ASSERT_EQ(r.esperas_cpu(), 6);
}

TEST_CASE(RingDobleNoFrena) {
    FrameRing r(2);
    for (int i = 0; i < 10; ++i) (void)r.frame();
    ASSERT_EQ(r.esperas_cpu(), 0);     /* margen de un frame alcanza */
}

TEST_CASE(AlternanciaDeSlots) {
    FrameRing r(3);
    ASSERT_EQ(r.frame(), 0u);
    ASSERT_EQ(r.frame(), 1u);
    ASSERT_EQ(r.frame(), 2u);
    ASSERT_EQ(r.frame(), 0u);          /* recicla en orden */
    ASSERT_EQ(r.esperas_cpu(), 0);
}

TEST_CASE(GpuAtrasadoFrena) {
    /* El modelo de latencia: si el envío fue en el frame ANTERIOR y el
       ring es de 1, la espera ES el comportamiento esperado (CPU frenó,
       pero no se corrompe el buffer: por eso la regla existe). */
    FrameRing r(1);
    (void)r.frame();
    ASSERT_EQ(r.frame(), 0u);   /* frame 1: diff==1 -> espera contada */
    ASSERT_EQ(r.esperas_cpu(), 1);
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
