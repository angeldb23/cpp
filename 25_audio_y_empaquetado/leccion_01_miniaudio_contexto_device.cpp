/* =============================================================================
   MÓDULO 25 - LECCIÓN 01: miniaudio — contexto, device, formato y latencia
   =============================================================================
   TEMA 1 (parte 1): miniaudio: contexto, device, formato (sample rate,
   channels), callback de mezcla y latencia — 2 lecciones (esta es la 1°).

   miniaudio es UN solo header C99 (miniaudio.h) usable desde C++ sin wrappers.
   La mezcla real la corren en un hilo propio del backend; NUNCA bloquees ese
   hilo. Todo lo que "hace audio" en miniaudio son structs transparentes: vos
   reservás la memoria (pattern config/init + uninit), la lib no usa handles
   opacos: NO copies los structs, su dirección debe ser estable mientras viven
   (por eso los objetos de audio van como miembros de la clase del motor, no
   en el stack de una función corta).

   MODELO DE TRES CAPAS
   --------------------
   1. ma_context: describe el entorno (backends disponibles). Puedes usarlo
      explícito (para elegir device o log) o dejar que ma_engine cree uno
      internamente. Config: ma_context_config_init() + ma_context_init().
   2. ma_device: un endpoint físico (playback, capture, duplex, loopback).
      Config: ma_device_config_init(ma_device_type_playback), se le setea
      playback.channels / playback.format, sampleRate, dataCallback,
      periodSizeInFrames + periods (latencia).
   3. callback: el backend te llama en SU hilo con un buffer de salida para
      llenar. La "mezcla" que lo llena es lo que venimos a diseñar en las
      próximas 4 lecciones.

   FORMATO Y LATENCIA (el trade-off que hay que entender)
   ------------------------------------------------------
   - ma_format_f32: muestras en [-1.0, +1.0], la forma que ya se mezcla en los
     motores (todo lo procesa flotante); s16 cuando el device pida int16.
   - sampleRate: 48000 Hz pro moderno; si el device ya corre a 48000 evitás
     resampling nativo (si pedís 44100 miniaudio convierte por vos, pero
     esa conversión tiene costo y un poquito de latencia).
   - periodSizeInFrames: muestras por "batida" del callback (HINT: el backend
     puede ajustar el valor que pediste — la doc dice explícito "hints, and
     are not necessarily exactly what you'll get").
   - latencia total = periodSizeInFrames * periods. Con periodSize 480
     (10 ms @ 48k) y 3 periods: ±30 ms de "buffer" antes de que escuches
     algo que escribiste. Menos periodSize = less latency pero más
     riesgo de glitch (underrun); mayor periods margen pero leer
     "más al sesgo del evento que ya pasó".
   - El callback corre en un hilo de prioridad alta del backend (WASAPI/
     ALSA/CoreAudio/PipeWire); lo prohibido ahí: locks, malloc/free de
     puntos corrientes, fopen, printf. La forma correcta de "darle datos
     nuevos" desde el hilo principal es que escriba en buffers de doble
     uso con escrituras atómicas, nunca bloquear (lección 03).

   EN ESTA MÁQUINA (Arch + Wayland, PipeWire)
   ------------------------------------------
   El backend será pipewire (miniaudio 0.11.25 lo lista como backend nativo).
   Para probar a mano LA PARTE REAL: make leccion_01 dentro del build del
   módulo (ver CMakeLists) y ejecutá el binario: enumerará dispositivos
   disponibles e imprimirá el contexto; la construcción del resto del módulo
   nos sirve para verificar compile/link sin necesitar hardware.

   DEMOS SIN MINIAUDIO (en esta sesión de construcción, compilar y correr
   con g++ SUELTO): el bloque bajo `#ifdef DEMO_MINIAUDIO` queda para el dueño
   cuando compile vía CMake (trabaja con el target library miniaudio).
   ============================================================================= */
#include "include/f25.hpp"

#include <cstdio>
#include <string>
#include <vector>

/* ----------------------------- Parte ejecutable -------------------------- */
namespace {

/* Callback REAL de las páginas de la lección: lo usa el device real si está
   disponible; en modo headless la misma firma se emula a mano. */
void dataCallbackHello(float* pOutput, unsigned frames, unsigned canales) {
    /* La salida es interleaved: L R L R L R... "frames" pares. */
    for (unsigned f = 0; f < frames; ++f) {
        for (unsigned c = 0; c < canales; ++c) {
            pOutput[(f * canales) + c] = 0.0f; /* silencio: device "corriendo" */
        }
    }
    (void)dataCallbackHello; /* silencia warning si no la usamos */
}

} // namespace

/* --------------------------- Modo con miniaudio -------------------------- */
#if DEMO_MINIAUDIO
/* Este bloque SOLO compila si el build lo activa (CMake del módulo define
   DEMO_MINIAUDIO y linkea target `miniaudio`, tag 0.11.25). Cada firma que
   usa acá está verificada contra miniaudio.h 0.11.25 en la sesión de
   construcción (búsquedas con grep, ver DECISIONES.md). */
#include "miniaudio.h"

void dataCallbackReal(ma_device* pDevice, void* pOutput, const void* pInput,
                      ma_uint32 frameCount) {
    (void)pDevice;
    (void)pInput;
    dataCallbackHello(static_cast<float*>(pOutput), frameCount, 2u);
}

int runConMiniaudio() {
    ma_result rc;
    ma_context context;

    /* el contexto queda "abriendo" y "listando" todos los backends */
    rc = ma_context_init(nullptr, 0, nullptr, &context);
    if (rc != MA_SUCCESS) {
        std::printf("context init: MA_SUCCESS==%d, error=%d\n", MA_SUCCESS, rc);
        return 1;
    }

    /* El device "por defecto": se delega al sistema (en Linux, PipeWire). */
    ma_device device;
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format = ma_format_f32;    /* muestras float en [-1,1] */
    config.playback.channels = f25::kCanales;  /* stereo */
    config.sampleRate = f25::kSampleRate;
    config.dataCallback = dataCallbackReal;
    config.periodSizeInFrames = 480;           /* hint: 10 ms @ 48k */
    config.periods = 3;                        /* ~30 ms de buffer total */

    rc = ma_device_init(&context, &config, &device);
    if (rc != MA_SUCCESS) {
        std::printf("FALLO: ma_device_init rc=%d\n", rc);
        ma_context_uninit(&context);
        return 1;
    }

    std::printf("Contexto OK. Device: %s\n", device.playback.name);
    std::printf("Backend elegido: %s\n", ma_get_backend_name(device.pContext->backend));
    /* (campos verificados en 0.11.25: playback.internalPeriods e
       internalPeriodSizeInFrames: la cantidad y el tamaño internos que
       negoció el backend; pueden cambiar respecto al hint de config). */
    std::printf("Formato real: %d canales, %d Hz, periods=%d de %d frames\n",
                (int)device.playback.channels, (int)device.sampleRate,
                (int)device.playback.internalPeriods,
                (int)device.playback.internalPeriodSizeInFrames);

    rc = ma_device_start(&device); /* arranca el callback en el hilo real */
    if (rc != MA_SUCCESS) {
        std::printf("FALLO: ma_device_start rc=%d\n", rc);
        ma_device_uninit(&device);
        ma_context_uninit(&context);
        return 1;
    }

    std::printf("Device arrancó (echo de silencio). Presioná Enter para cerrar.\n");
    std::getchar();

    /* La vida de cada "reservé" tiene su uninit ESPEJO (RAII a la C99). */
    ma_device_stop(&device);
    ma_device_uninit(&device);
    ma_context_uninit(&context);
    return 0;
}
#endif /* DEMO_MINIAUDIO */

/* ------------------------------- Modo headless ---------------------------- */
int main() {
    std::printf("M25 / L01 - contexto, device y formato de miniaudio\n");
    std::printf("=================================================\n");

    /* En esta build NO linkeamos miniaudio (g++ suelto). Imprimimos la MISMA
       idea en modo headless para que la lección sea ejecutable sin hardware:
       1) datos del device configurado; 2) el callback llenando un buffer. */
    std::printf("device config (simulada en headless):\n");
    std::printf("  type         = ma_device_type_playback\n");
    std::printf("  playback.fmt = ma_format_f32 (muestras float [-1,1])\n");
    std::printf("  playback.ch  = %u (stereo)\n", f25::kCanales);
    std::printf("  rate         = %u Hz\n", f25::kSampleRate);
    std::printf("  periodSize   = 480 frames (10 ms @ 48k)\n");
    std::printf("  periods      = 3  => latencia teórica ~30 ms\n");

    std::vector<float> buffer(480 * f25::kCanales, 0.0f);
    dataCallbackHello(buffer.data(), 480u, f25::kCanales);
    std::printf("callback sim: llené %zu frames de silencio (stereo interleaved)\n",
                buffer.size() / f25::kCanales);
    std::printf("total muestras escritas = %zu\n", buffer.size());

    std::printf("\nTablas de referencia (formatos y latencia):\n");
    struct Fmt { int id; const char* nombre; int bytesPorMuestra; };
    /* Ids reales del enum ma_format en 0.11.25 (verificado en el header). */
    const Fmt fmts[] = {
        { 2, "ma_format_s16", 2 }, { 4, "ma_format_s32", 4 },
        { 5, "ma_format_f32", 4 },
    };
    for (const Fmt& f : fmts) {
        std::printf("  %s: %d byte/muestra -> frame stereo = %d bytes\n",
                    f.nombre, f.bytesPorMuestra, f.bytesPorMuestra * 2);
    }
    const int latencias[] = { 480, 960, 1920 };
    for (int p : latencias) {
        std::printf("  periodSize=%d frames => %.1f ms por period (@48k)\n",
                    p, static_cast<double>(p) / 48.0);
    }

#if DEMO_MINIAUDIO
    return runConMiniaudio();
#else
    std::printf("\n(efecto visual: compilar vía CMake del módulo con DEMO_MINIAUDIO "
                "para ver el device real)\n");
    return 0;
#endif
}
