/* =============================================================================
   MÓDULO 26 - LECCIÓN 03: Vulkan parte 2 — command buffers, render pass,
   pipeline y sincronización (semáforos y fences)
   =============================================================================
   TEMA 2 (parte 2):
   "render pass, pipeline, command buffers y synchronization (semaphores/
   fences)".

   EL DÍA A DÍA DE UN FRAME MODERNO
   --------------------------------
   1. CPU: VkCommandPool (por frame, por thread) -> vkResetCommandBuffer ->
      vkBeginCommandBuffer -> vkCmdBeginRenderPass (con el framebuffer de
      la imagen del swapchain que adquiriste) -> vkCmdBindPipeline ->
      vkCmdDraw -> vkCmdEndRenderPass -> vkEndCommandBuffer.
   2. Submit: vkQueueSubmit con
        - waitSemaphore   : "imageAvailable" (la señaliza vkAcquireKHR
                            cuando la imagen te la dejan tocar)
        - signalSemaphore : "renderFinished" (lo espera el present)
        - fence           : señal que la CPU puede esperar/poll para
                            saber que este frame terminó y su command
                            buffer está libre para reciclar.
   3. Present: vkQueuePresentKHR (esperando "renderFinished").

   SEMÁFORO vs FENCE (la confusión #1 de Vulkan)
   ---------------------------------------------
   * SEMÁFORO: sincroniza DENTRO del GPU. Una cola señala, otra espera.
     No se puede poll desde CPU. Cadena típica: imageAvailable ->
     render -> renderFinished -> present.
   * FENCE: sincroniza GPU con CPU. vkWaitForFences / vkGetFenceStatus
     es lo que el host consulta para saber si puede reciclar un slot.

   IN-FLIGHT FRAMES (el patrón que todo motor copió)
   --------------------------------------------------
   Nunca sobrescribás un command buffer que el GPU puede estar leyendo.
   La solución estándar: N frames "en vuelo" (2 o 3). El frame i usa SU
   command pool, SUS semáforos y SU fence, y el slot real es i % N. Así
   la CPU corre un frame adelante del GPU y casi nunca espera: solo se
   frena en vkWaitForFences si el GPU quedó muy atrás.

   EN ESTA LECCIÓN: simulador headless del ring de frames, con un modelo
   de tiempo simple: el GPU termina cada submit ~1 frame después de que
   la CPU lo envió (con vsync de 1/formación). Un slot está BUSY cuando
   pasaron 1 frame o menos desde su último submit. Con ring=1 SIEMPRE
   está busy en el siguiente record; con ring>=2 ya NO, y por eso el
   in-flight doble es el estándar de la industria.
   ============================================================================= */
#include "include/f26.hpp"

#include <cstdio>
#include <vector>

namespace {

struct InFlightSlot {
    std::uint32_t slot_id       = 0;
    int           ultimo_submit = -1000000; /* nunca submit (slot libre) */
};

class FrameRing {
public:
    explicit FrameRing(std::size_t n) : frames_(n) {
        for (std::size_t i = 0; i < n; ++i) frames_[i].slot_id = std::uint32_t(i);
    }

    /* frame(): acquire -> record -> submit -> present, con la espera de
       fence si el slot está busy. Devuelve el slot usado (i % n). */
    std::uint32_t frame() {
        auto& f = frames_[frame_ % frames_.size()];

        /* vkWaitForFences entre frame_now y el último submit del slot:
           con nuestro modelo, quedan 2 posibles "pendientes": el mismo
           frame (diff=0, imposible en ring real) o el anterior (diff=1). */
        const int diff = static_cast<int>(frame_) - f.ultimo_submit;
        if (diff <= 1) ++esperas_cpu_;   /* CPU frenada por el fence */

        const std::uint32_t usado = frame_ % std::uint32_t(frames_.size());
        f.ultimo_submit = static_cast<int>(frame_);
        ++frame_;
        return usado;
    }

    int esperas_cpu() const { return esperas_cpu_; }

private:
    std::vector<InFlightSlot> frames_;
    std::uint32_t frame_       = 0;
    int           esperas_cpu_ = 0;
};

} // namespace

int main() {
    /* 10 frames medidos con ring=2 (in-flight estándar) y ring=1 (para
       ver qué pasa si SOLO tenés un slot). */
    FrameRing ring2(2), ring1(1), ring3(3);
    for (int i = 0; i < 10; ++i) {
        (void)ring1.frame();
        (void)ring2.frame();
        (void)ring3.frame();
    }
    std::printf("10 frames | ring=1 esperas=%d | ring=2 esperas=%d | ring=3 esperas=%d\n",
                ring1.esperas_cpu(), ring2.esperas_cpu(), ring3.esperas_cpu());

    /* Lectura: con ring=1 la CPU se frena en 9 de 10 frames (cada frame
       nuevo espera que el GPU termine el anterior). Con ring=2 (o 3)
       la CPU casi nunca espera: la memoria extra de duplicar command
       pools/semáforos/uniforms per-frame es el trade-off estándar. */
    return 0;
}

/* Reflexión final: las barreras DE MEMORIA dentro de un command buffer
   (vkCmdPipelineBarrier) cubren el caso más frecuente: corriste una
   pasada GRAPHICS y querés LEER su salida como textura en la siguiente.
   En GL eso era implícito (el driver infería); acá PAGÁS declarando
   srcStage/dstStage/srcAccess/dstAccess. La LECCIÓN 09 generaliza esto
   con un render graph que calcula las barreras SOLO: el costo fijo de
   organizarlo a mano es parte del precio de la API explícita. */
