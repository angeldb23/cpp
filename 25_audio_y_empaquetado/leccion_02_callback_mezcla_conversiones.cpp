/* =============================================================================
   MÓDULO 25 - LECCIÓN 02: callback de mezcla, conversiones y reglas del hilo
   =============================================================================
   TEMA 1 (parte 2): callback de mezcla y latencia — y el "cómo muevo datos"
   entre el hilo del juego y el hilo de audio.

   REGLAS DEL HILO DE AUDIO (el callback NO es un hilo vuestro)
   ------------------------------------------------------------
   El backend llama dataCallback con prioridad alta y en tiempo real. Dentro:
   PROHIBIDO sostener locks que puedan mantenerse en el hilo del juego (std::mutex
   grande, locks de sistema de archivos); PROHIBIDO fopen/read; PROHIBIDO
   printf/cout; evitá alocar memoria en el heap por frame. Vale: aritmética
   simple, trabajar con buffers pre-reservados y leer/escribir variables
   pequeñas por valor (bools/floats atómicos) o pasar por buffers de doble
   uso (double-buffer) que el hilo principal prepara ANTES del store.

   El patrón canónico: doble buffer.
   - Hilo de juego: escribe en la página NO activa y publica el nuevo índice
     con un store atómico (así lo hace ma_resource_manager_data_stream
     internamente con sus pages[2] atómicas, verificado en el header 0.11.25).
   - Hilo de audio: lee el índice UNA vez por frame y después solo toca la página
     que ese índice elige.
   Si el hilo de audio pide más de lo que hay (starvation), el motor rellena
   con ceros (silencio) y cuenta el evento para la estadística; nunca
   crasheea ni bloquea.

   CONVERSIONES DE FORMATO (ma_data_converter)
   -------------------------------------------
   Firma verificada en 0.11.25:
     ma_data_converter_config_init(formatIn, formatOut, channelsIn,
                                   channelsOut, sampleRateIn, sampleRateOut);
     ma_data_converter_init(&config, nullptr, &conv);
   Hace: cambio de formato (s16 <-> f32), cambio de canales (mono <-> stereo)
   y resampling en un solo subsistema. En la práctica el device pide f32 x 2 y
   el motor mezcla todo en f32. Si cargás s16 crudo de disco, la conversión a
   mano es determinística:
     f32 = s16 / 32768.0f        (el -32768 s16 -> -1.0 f32 exacto)
     s16 = f32 * 32767 (redondeo) con clipping a [-32768, +32767]
   Nota: /32768 y *32767 NO son inversos exactos (32767/32768 no devuelve
   32767 tras redondear si se divide por 32768); el motor decide UNA política
   y la respet Everywhere.

   MEZCLA CANÓNICA DE VOCES EN EL CALLBACK
   ---------------------------------------
   Cada voz ya viene resampleada a (formato del engine, canales del engine);
   el trabajo del callback es solo SUMAR ponderada al buffer y clamp. No hay
   un "límite máximo de voces" intrínseco: hay presupuesto de CPU y
   saturación. Por eso la lección 04 arma un pool con prioridad: cuando
   toca "steal", se roba la voz de MENOR prioridad (y a igual prioridad, la
   más vieja), se detiene su fuente y se reutiliza su slot.
   ============================================================================= */
#include "include/f25.hpp"

#include <atomic>
#include <cmath>
#include <cstdio>
#include <vector>

/* ---------------------- Simulación ejecutable (headless) ----------------- */
namespace {

/* Reemplaza el MISMO contrato del callback real sin hardware: el "hilo de
   audio" en la demo es main corriendo frames. */
struct MotorAudioSim {
    /* Voz simple: amplitud fija (a la fuente ya le aplicaron gain). */
    float amplitudVoz1 = 0.25f;
    float amplitudVoz2 = 0.35f;

    void mezclar(float* out, unsigned frames, unsigned canales) {
        for (unsigned f = 0; f < frames; ++f) {
            float suma = amplitudVoz1 + amplitudVoz2; /* 0.60 en la demo */
            for (unsigned c = 0; c < canales; ++c) {
                out[(f * canales) + c] = suma;
            }
        }
    }

    /* dB reales: 20*log10(amplitud). Cuidado con la confusión de siempre:
       0.5 en amplitud NO es -6 dB... es -6.02 dB. Y -3 dB es x0.7079. */
    static double amplitudADb(float a) {
        return 20.0 * std::log10(static_cast<double>(a) + 1e-12);
    }
};

/* Doble página con índice atómico (el patrón del streaming interno). */
struct PaginaDoble {
    std::vector<float> paginas[2];
    std::atomic<int> activa{ 0 };

    void init(int frames, int canales) {
        paginas[0].assign(static_cast<std::size_t>(frames) * canales, 0.0f);
        paginas[1] = paginas[0];
    }

    /* El hilo de JUEGO llama esto: escribe la otra página y la publica. */
    void publicarSiguiente(int indiceNueva, float valor) {
        paginas[indiceNueva][0] = valor;
        activa.store(indiceNueva, std::memory_order_release);
    }

    /* El hilo de AUDIO llama esto: carga el índice y usa esa página. */
    int indiceActual() const { return activa.load(std::memory_order_acquire); }
};

} // namespace

/* --------------------------- Modo con miniaudio -------------------------- */
#if DEMO_MINIAUDIO
/* Este bloque solo compila si el build lo activa (CMake define DEMO_MINIAUDIO
   y linkea el target `miniaudio`, tag 0.11.25). Firmas verificadas contra
   miniaudio.h 0.11.25 con grep en la sesión de construcción (DECISIONES.md). */
#include "miniaudio.h"

struct MotorAudioSim; /* prototipo: el sim vive arriba */

void dataCallbackMezcla(ma_device* pDevice, void* pOutput, const void* pInput,
                        ma_uint32 frameCount) {
    (void)pInput;
    auto* out = static_cast<float*>(pOutput);
    auto* ctx = static_cast<MotorAudioSim*>(pDevice->pUserData);

    /* Base: mezclar las voces "vivas" al buffer del frame (esto es EXACTO a
       lo que MotorAudioSim::mezclar hace en headless). */
    ctx->mezclar(out, frameCount, 2u);
}

int demoConversiones() {
    /* Convertir s16 -> f32 con miniaudio real (mismo formato de muestras,
       mismos channels y mismo rate: NO resamplea, solo cambia el tipo). */
    ma_data_converter conv;
    ma_data_converter_config cfg =
        ma_data_converter_config_init(ma_format_s16, ma_format_f32, 2, 2,
                                      48000, 48000);
    if (ma_data_converter_init(&cfg, nullptr, &conv) != MA_SUCCESS) {
        return 1;
    }
    std::int16_t entrada[4] = { 0, 16000, -16000, 32767 };
    float salida[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    ma_uint64 inN = 4;
    ma_uint64 outN = 4;
    ma_data_converter_process_pcm_frames(&conv, entrada, &inN, salida, &outN);
    std::printf("convertidor miniaudio: %d -> %.6f\n", entrada[1], salida[1]);
    ma_data_converter_uninit(&conv, nullptr);
    return 0;
}
#endif /* DEMO_MINIAUDIO */

int main() {
    std::printf("M25 / L02 - callback de mezcla y conversiones\n");
    std::printf("=============================================\n");

    /* 1. La misma firma del callback real, corrida en el hilo principal. */
    MotorAudioSim motor;
    const unsigned frames = 480;
    std::vector<float> salida(frames * f25::kCanales, 0.0f);
    motor.mezclar(salida.data(), frames, f25::kCanales);
    std::printf("mezcla de 2 voces: frame 0 canal L = %.4f\n", salida[0]);
    std::printf("  (0.25 + 0.35 = 0.60; 20*log10(0.6) = %.2f dB)\n",
                MotorAudioSim::amplitudADb(0.60f));

    /* 2. Conversión s16 -> f32 a mano (lo mismo que miniaudio hace al
       "formato de mixing"). */
    const std::int16_t s16 = 16000;
    const float f32 = static_cast<float>(s16) / 32768.0f;
    std::printf("s16 %d -> f32 %.6f (= 16000/32768)\n", s16, f32);
    const float want = 32768.0f * f32;
    std::printf("roundtrip f32 -> s16 (x32768, redondeo): %.0f\n", want);

    /* 3. Doble página con índice atómico (patrón del streaming interno). */
    PaginaDoble pagina;
    pagina.init(static_cast<int>(frames), static_cast<int>(f25::kCanales));
    pagina.publicarSiguiente(1, 0.42f);
    const int leida = pagina.indiceActual();
    std::printf("doble página: publicada=1, callback lee página %d, valor %.2f\n",
                leida, pagina.paginas[leida][0]);
    std::printf("regla: el hilo de audio NUNCA bloquea; solo carga el índice.\n");

#if DEMO_MINIAUDIO
    demoConversiones();
#else
    std::printf("\n(Conversión real con ma_data_converter: compilar vía CMake "
                "del módulo, que define DEMO_MINIAUDIO)\n");
#endif
    return 0;
}
