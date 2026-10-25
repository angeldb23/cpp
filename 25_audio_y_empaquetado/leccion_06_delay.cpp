/* =============================================================================
   MÓDULO 25 - LECCIÓN 06: efectos en cadena I — delay (echo) con feedback
   =============================================================================
   TEMA 4 (parte 1): Efectos: delay, reverb (freeverb), low-pass/high-pass,
   distorsión y chain de efectos por bus — 2 lecciones. Acá: delay puro y el
   delay_node real de miniaudio.

   DELAY LINE (el efecto más simple y más importante)
   --------------------------------------------------
   Idea: guardás los últimos D frames y los devolvés más tarde. Con feedback:
      D = sampleRate * tiempoEnSegundos   (el "gap" en muestras)
      wet  = señal delayeada
      dry  = señal actual, sin delay
      out[n]    = dry[n] + wet[n]
      delay[n]  = entrada[n] + wet[n - D] * feedback   (recursión)
   Feedback en [0, 1): 0 = un solo echo; 0.5 = los echos mueren a mitad cada
   vuelta; >= 1.0 = el delay se ROMPE (el eco crece para siempre: la
   inestabilidad clásica del "eco infinito"). Un LFO en D (modulación) da el
   chorus, pero eso no entra en el módulo (chorus/flanger quedan fuera de
   TEMAS).

   TIEMPO REAL: 125 ms de delay es "room echo"; ~250 ms un slapback notorio;
   cuanto más feedback + delay largo, más "trance delay".

   miniaudio REAL: ma_delay_node
   ------------------------------
   Existe como nodo del node graph (menos setup que el DSP a mano):
     ma_delay_node_config cfg = ma_delay_node_config_init(channels,
         sampleRate, delayInFrames, decay);   // decay = feedback [0..1)
     ma_delay_node_init(&nodeGraph, &cfg, nullptr, &delayNode);
     ma_delay_node_set_wet(&delayNode, 1.0f);  // mezcla ecos
     ma_delay_node_set_dry(&delayNode, 1.0f);  // mezcla señal directa
   El nodo vive EN el grafo del engine y se engancha con
   ma_node_attach_output_bus (lección 07 lo usa para el chain).
   Delay max = el delayInFrames que pediste (el buffer se alloca del tamaño
   pedido: pe. 48000*0.5 = 24000 frames ≈ 96 KB f32 stereo a malloc una vez).

   REPARTO DE VOLUMEN: sumar wet + dry SIN escalar suma hasta 2.0 (> 0 dB):
   los dos a 0.5 es la "regla de las mitades" del mix de echo (manifestamente
   es wet = 0.4, dry = 0.7 si se quiere pegar).
   ============================================================================= */
#include "include/f25.hpp"

#include <cmath>
#include <cstdio>
#include <vector>

/* ------------------------- DSP a mano (headless) -------------------------- */
namespace {

class DelayLine {
public:
    void init(int delayEnFrames, float feedback) {
        buffer_.assign(static_cast<std::size_t>(delayEnFrames), 0.0f);
        pos_ = 0;
        feedback_ = feedback;
    }

    /* Un frame: devuelve "wet" y alimenta la recursión. */
    float step(float entrada) {
        const float wet = buffer_[pos_];
        buffer_[pos_] = entrada + wet * feedback_;
        pos_ = (pos_ + 1) % buffer_.size();
        return wet;
    }

    float wet() const { return buffer_[(pos_ + buffer_.size() - 1) % buffer_.size()]; }

private:
    std::vector<float> buffer_;
    std::size_t pos_ = 0;
    float feedback_ = 0.0f;
};

} // namespace

/* --------------------------- Modo con miniaudio -------------------------- */
#if DEMO_MINIAUDIO
#include "miniaudio.h"

int runDelayReal() {
    /* Node graph standalone (sin engine; el buffer se llena a mano). */
    ma_node_graph grafo;
    ma_node_graph_config cfgGrafo = ma_node_graph_config_init(2);
    if (ma_node_graph_init(&cfgGrafo, nullptr, &grafo) != MA_SUCCESS) {
        return 1;
    }
    ma_delay_node delay;
    /* 48000*0.25 = 12000 frames de delay; feedback 0.45. */
    ma_delay_node_config cfg =
        ma_delay_node_config_init(2, 48000, 12000, 0.45f);
    if (ma_delay_node_init(&grafo, &cfg, nullptr, &delay) != MA_SUCCESS) {
        ma_node_graph_uninit(&grafo, nullptr);
        return 1;
    }
    ma_delay_node_set_wet(&delay, 0.4f);
    ma_delay_node_set_dry(&delay, 0.7f);
    /* El dueño engancha: ma_node_attach_output_bus(&delay, 0,
       ma_node_graph_get_endpoint(&grafo), 0); y procesa frames; acá la demo
       queda como referencia del API (el DSP pasoreal de efectos está en la
       lección 07 con el chain). */
    ma_delay_node_uninit(&delay, nullptr);
    ma_node_graph_uninit(&grafo, nullptr);
    return 0;
}
#endif /* DEMO_MINIAUDIO */

int main() {
    std::printf("M25 / L06 - delay con feedback (echo)\n");
    std::printf("=====================================\n");

    /* 1. Impulso a través de un delay de 6000 frames (125 ms @ 48k). */
    const int delayFrames = 6000;
    DelayLine dl;
    dl.init(delayFrames, 0.5f); /* feedback 0.5: cada eco a mitad */

    /* Meto un impulso (1 en t=0, 0 después) y registro la salida wet. */
    const int total = delayFrames * 3 + 16;
    std::vector<float> salida;
    for (int n = 0; n < total; ++n) {
        const float entrada = (n == 0) ? 1.0f : 0.0f;
        salida.push_back(dl.step(entrada));
    }
    /* Los echos: en 6000 (amp 1.0), 12000 (amp 0.5), 18000 (amp 0.25). */
    std::printf("eco #1 @ %d frames = %.4f (esperado 1.0)\n",
                delayFrames, salida[delayFrames]);
    std::printf("eco #2 @ %d frames = %.4f (esperado 0.5)\n",
                delayFrames * 2, salida[delayFrames * 2]);
    std::printf("eco #3 @ %d frames = %.4f (esperado 0.25)\n",
                delayFrames * 3, salida[delayFrames * 3]);
    std::printf("entre echos la salida vale %.4f (silencio entre impulsos)\n",
                salida[delayFrames - 10]);

    /* 2. En ms a mano: 125 ms @ 48000 = 6000 muestras. */
    std::printf("125 ms -> %.0f frames (@48k); feedback %.1f = -6 dB por eco\n",
                f25::msAmuestras(125, f25::kSampleRate), 0.5);

#if DEMO_MINIAUDIO
    runDelayReal();
#endif
    return 0;
}
