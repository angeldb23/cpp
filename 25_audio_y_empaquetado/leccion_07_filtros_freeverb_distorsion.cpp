/* =============================================================================
   MÓDULO 25 - LECCIÓN 07: efectos II — filtros biquad, freeverb, distorsión
   y chain de efectos por bus
   =============================================================================
   TEMA 4 (parte 2).

   BIQUAD (el bloque de construcción de TODOS los filtros)
   --------------------------------------------------------
   Un biquad (fórmula "Direct Form I" canónica, la misma de ma_biquad):
   y[n] = (b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]) / a0
   Los coeficientes vienen del RBJ Audio EQ Cookbook (Robert Bristow-Johnson,
   documento público y estándar de facto). Para un low-pass @ fs:
   ... w0 = 2*pi*f0/fs ; alpha = sin(w0)/(2*Q)
       b0 = (1-cos(w0))/2 ; b1 = 1-cos(w0) ; b2 = (1-cos(w0))/2
       a0 = 1+alpha ; a1 = -2*cos(w0) ; a2 = 1-alpha.
   En miniaudio el config le admite coeficientes a mano:
     ma_biquad_config_init(format, channels, b0, b1, b2, a0, a1, a2);
   (firma verificada en el header 0.11.25) o los "empaquetados":
     ma_lpf_config_init(format, channels, sampleRate, cutoff, order)
     ma_hpf_config_init(format, channels, sampleRate, cutoff, order)
   NOTA: 3 dB puff: en f0 la ganancia del LPF RBJ es EXACTAMENTE -3.01 dB
   (por eso Q = 1/sqrt(2) es "Butterworth/Q0.707", el filtrado "sin resonancia").

   REVERB = FREEVERB (versión coupled delay lines)
   ----------------------------------------------
   Reverb = MUCHOS delays apilados: la cola del room es la suma de las colas
   de todos los combs. Freeverb (de Jezar, public domain)
   usa POR CANAL: 8 comb filters (delay + feedback) + 4 allpass filters en
   serie, y aplica "stereo spread" para desfasar los combs entre L y R. Los
   tunings clásicos del algoritmo (1116, 1188, 1277, 1356, 1422, 1491, 1557,
   1617 muestras a 44.1k) están ESCRITOS en verblib.h, el archivo que trae
   el ma_reverb_node de miniaudio 0.11.25 (extras/nodes/ma_reverb_node/;
   "based on Freeverb, a public domain reverb written by Jezar at
   Dreampoint", verificado EN el repo del tag)
   En el C++ del curso igual lo escribís a mano (comb + allpass), porque la
   forma de entender un reverb esnotar que reverb = "muchos delays juntos"
   y no magia.

   LA DISTORSIÓN (waveshaper del ejercicio)
   ------------------------------------------
   NO existe un nodo de distorsión en miniaudio 0.11.25 (grep ma_ds:
   0 resultados; el antiguo ma_ds de 0.10 fue eliminado). Se computa a mano:
   función de transferencia por muestra. La más útil del juego:
   drive tanh: y = tanh(x * drive) / tanh(drive)
   -> drive 1 = signal casi limpia; drive 8 = soft clip notorio.
   (raw clip a [-1,1] = "hard clip" digital: suena "chato".)

   CHAIN POR BUS (el node graph real del engine)
   ---------------------------------------------
   Los nodos del engine se enganchan con buses de node: cada nodo tiene
   input buses y output buses (verificado: ma_node_get_input_bus_count,
   ma_node_attach_output_bus(pNode, outBus, pOtherNode, inBus)). Cadena:
       sfx bus --> biquad(N) --> reverb_node --> endpoint (out)
   El reverb node (extras) está disponible en el CMake del módulo:
   FetchContent genera target `miniaudio_reverb_node` (extras/nodes/...
   con verblib.h, "Freeverb's reverb algorithm"). Conéctalé y sonás YA.
   ============================================================================= */
#include "include/f25.hpp"

#include <cmath>
#include <cstdio>
#include <vector>

/* ------------------------------ DSP a mano -------------------------------- */
namespace {

/* Biquad Direct Form I con coeficientes NORMALIZADOS por a0 (a0 = 1). */
class Biquad {
public:
    void setCoef(double b0, double b1, double b2, double a1, double a2) {
        b0_ = b0; b1_ = b1; b2_ = b2; a1_ = a1; a2_ = a2;
    }

    double step(double x) {
        const double y = b0_ * x + b1_ * x1_ + b2_ * x2_ - a1_ * y1_ - a2_ * y2_;
        x2_ = x1_; x1_ = x;
        y2_ = y1_; y1_ = y;
        return y;
    }

private:
    double b0_ = 1.0, b1_ = 0.0, b2_ = 0.0, a1_ = 0.0, a2_ = 0.0;
    double x1_ = 0.0, x2_ = 0.0, y1_ = 0.0, y2_ = 0.0;
};

/* Coeficientes RBJ LPF: fs=48000, f0=1000 Hz, Q=0.7071 (Butterworth).
   Código del "cookbook" sin modificar... se comprueban en el ejercicio. */
void configLPF(Biquad& bq, double fs, double f0, double Q) {
    const double w0 = 2.0 * f25::kPi * f0 / fs;
    const double alpha = std::sin(w0) / (2.0 * Q);
    const double cosw = std::cos(w0);
    const double a0 = 1.0 + alpha;
    const double b0 = ((1.0 - cosw) / 2.0) / a0;
    const double b1 = (1.0 - cosw) / a0;
    const double b2 = ((1.0 - cosw) / 2.0) / a0;
    const double a1 = (-2.0 * cosw) / a0;
    const double a2 = (1.0 - alpha) / a0;
    bq.setCoef(b0, b1, b2, a1, a2);
}

/* Comb (+ allpass) = reverb "freeverb" ULTRA reducido para entenderlo. */
class Comb {
public:
    void init(int delayFrames, float feedback) {
        buffer_.assign(static_cast<std::size_t>(delayFrames), 0.0f);
        feedback_ = feedback;
        pos_ = 0;
    }
    float step(float x) {
        const float salida = buffer_[pos_];
        buffer_[pos_] = x + salida * feedback_;
        pos_ = (pos_ + 1) % buffer_.size();
        return salida;
    }

private:
    std::vector<float> buffer_;
    std::size_t pos_ = 0;
    float feedback_ = 0.0f;
};

class AllPass {
public:
    void init(int delayFrames) {
        buffer_.assign(static_cast<std::size_t>(delayFrames), 0.0f);
        pos_ = 0;
    }
    /* Freeverb allpass: gain fijo 0.5 es el valor clásico del algoritmo. */
    float step(float x) {
        const float bufout = buffer_[pos_];
        const float salida = bufout - x;
        buffer_[pos_] = x + bufout * 0.5f;
        pos_ = (pos_ + 1) % buffer_.size();
        return salida;
    }

private:
    std::vector<float> buffer_;
    std::size_t pos_ = 0;
};

/* Distorsión tanh con normalizado (drive >= 1). */
inline float driveTanh(float x, float drive) {
    const float g = std::tanh(x * drive) / std::tanh(drive);
    return g;
}

/* Tone de ejemplo. */
inline double tone(int n, double freq) {
    return std::sin(2.0 * f25::kPi * freq * static_cast<double>(n)
                    / static_cast<double>(f25::kSampleRate));
}

} // namespace

/* --------------------------- Modo con miniaudio -------------------------- */
#if DEMO_MINIAUDIO
#include "miniaudio.h"

int demoFiltrosReal() {
    /* LPF de orden 2 (1 biquad) con la API empaquetada. */
    ma_lpf lpf;
    ma_lpf_config cfg = ma_lpf_config_init(ma_format_f32, 2, 48000,
                                           1000.0, 2 /* biquads en cadena */);
    if (ma_lpf_init(&cfg, nullptr, &lpf) != MA_SUCCESS) {
        return 1;
    }
    float in[8] = { 1, 0, 0, 0, 0, 0, 0, 0 };
    float out[8] = { 0 };
    ma_lpf_process_pcm_frames(&lpf, out, in, 8);
    std::printf("LPF mini (respuesta al impulso): %.6f, %.6f, %.6f\n",
                (double)out[0], (double)out[1], (double)out[2]);
    ma_lpf_uninit(&lpf, nullptr);
    return 0;
}
#endif /* DEMO_MINIAUDIO */

int main() {
    std::printf("M25 / L07 - filtros, freeverb y distorsión\n");
    std::printf("==========================================\n");

    /* 1. LPF @ 1kHz: deja pasar el tono de 100 Hz y absorbe el de 8000 Hz. */
    Biquad bq;
    configLPF(bq, f25::kSampleRate, 1000.0, 0.7071);
    /* Amplitud pico (estimar filtrado un segundo) de tonos de 0.1 s. */
    auto picoTrayecto = [&](double freq) {
        double pico = 0.0;
        const int n = 4800; /* 100 ms */
        for (int i = 0; i < n; ++i) {
            const double y = bq.step(tone(i, freq));
            pico = std::max(pico, std::fabs(y));
        }
        return pico;
    };
    const double pico1000 = picoTrayecto(1000.0);
    const double pico100 = picoTrayecto(100.0);
    const double pico8000 = picoTrayecto(8000.0);
    std::printf("LPF 1kHz: 100 Hz pasa con pico %.3f; @f0 pico %.3f (-3 dB);\n"
                "          8 kHz queda en pico %.4f\n",
                pico100, pico1000, pico8000);

    /* 2. Freeverb-like: 4 combs y 3 allpass (la versión de la Lección
       simplificada del algoritmo real, que tiene 8+4 por canal). */
    const int taps[4] = { 1116, 1277, 1422, 1557 }; /* tunings freeverb (44.1k) */
    Comb combs[4];
    for (int i = 0; i < 4; ++i) {
        combs[i].init(taps[i], 0.8f); /* room size chico: cola aún audible */
    }
    const int apTaps[3] = { 347, 113, 37 }; /* posterior al combs */
    AllPass aps[3];
    for (int i = 0; i < 3; ++i) {
        aps[i].init(apTaps[i]);
    }

    std::vector<float> cola;
    const int medioSegundo = static_cast<int>(f25::kSampleRate / 2);
    for (int n = 0; n < medioSegundo; ++n) { /* 0.5 s */
        const float x = (n == 0) ? 1.0f : 0.0f;
        float acc = 0.0f;
        for (int c = 0; c < 4; ++c) {
            acc += combs[c].step(x * 0.35f);
        }
        acc *= 0.25f; /* equal-comb */
        float y = acc;
        for (int a = 0; a < 3; ++a) {
            y = aps[a].step(y);
        }
        cola.push_back(y);
    }
    /* La "cola" del reverberator no se muere de golpe: mid samples at tail */
    double suela = 0.0;
    for (int i = static_cast<int>(cola.size()) - 128; i < static_cast<int>(cola.size()); ++i) {
        suela = std::max(suela, static_cast<double>(std::fabs(cola[static_cast<std::size_t>(i)])));
    }
    std::printf("reverb: la cola a los 0.5 s todavía vale del orden %.6f "
                "(no muere de golpe)\n", suela);

    /* 3. Distorsión: el techo se "redondea" con drive. */
    const float entrada = 0.9f;
    std::printf("distorsión tanh: entrada %.2f, drive 1 -> %.6f; drive 8 -> %.6f\n",
                entrada, driveTanh(entrada, 1.0f), driveTanh(entrada, 8.0f));

#if DEMO_MINIAUDIO
    demoFiltrosReal();
#endif
    return 0;
}
