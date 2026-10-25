/* =============================================================================
   MÓDULO 25 - LECCIÓN 04: engine de alto nivel, voces, pool y prioridad
   =============================================================================
   TEMA 2 (parte 2): Sonido vs música: pool de voces y prioridad.

   LA API HIGH-LEVEL: ma_engine
   ----------------------------
   ma_engine es miniaudio "de fábrica": un grafo de nodos (node graph) + un
   device por defecto + decoders ya conectados. Firma verificada:
     ma_engine_config cfg = ma_engine_config_init();
     cfg.channels = 2; cfg.sampleRate = 48000;  (dejar en 0 = device nativo)
     cfg.periodSizeInFrames / periodSizeInMilliseconds
     ma_engine_init(&cfg, &engine);
     ma_engine_play_sound(&engine, "paso.wav", nullptr);   // fire-and-forget
     ma_sound_init_from_file(&engine, path, flags, grupo, fence, &sound);
     ma_sound_start(&sound); ma_sound_is_playing(&sound);
     ma_sound_set_pitch, ma_sound_set_volume, ma_sound_set_looping
     ma_engine_uninit(&engine);
   `ma_engine_play_sound` crea un ma_sound efímero cada vez: es la forma
   rápida, pero cada play reserva y destruye un objeto sound (y compite por
   recursos). Para el motor de un juego querés un POOL: N ma_sound
   pre-inicializados que se reutilizan.

   ¿QUÉ ES UNA "VOZ"?
   ------------------
   Una voz = un slot de mezcla activo que consume (fracción de) CPU y suma al
   buffer final. Hardware analógico "8 voces del SID": el límite hoy no es el
   hardware sino el presupuesto de tiempo: mezclar 32 voces es barato; 2000
   se muere en CPU antes de saturar el DAC. Cada voz cuesta
   (su decodificación/resampleo) + una suma por muestra.

   VOICE STEALING (la política real de los motores)
   ------------------------------------------------
   Cuando el pool está lleno y llega un play con prioridad P:
   1. La voz robable debe tener prioridad < P (si ninguna, el play se
      RECHAZA: no hay robo de voces más importantes).
   2. A igual prioridad robo la MÁS VIEJA (FIFO): la que ya sonó más tiempo.
   3. Al robar: stop de la fuente del pool-slot + arranco el nuevo contenido
      en ese slot (la "fuente" cambia pero el slot sobrevive).
   4. Un corte abrupto ("gate" a full) también produce CLICK: la muestra que
      cae de golpe es un escalón. Si el "steal" es necesario, un micro-fade
      (30-80 ms) de la voz saliente lo disimula (lección 05: fade con
      ma_sound_set_fade_in_milliseconds).
   Ejemplo real: "la explosión tumba el paso": la explosión entra con
   prioridad de categoría mayor y roba una voz de SFX común
   (categorías tipicas: UI < SFX < música < diálogo crítico).

   LA MODELIZACIÓN (la parte que pende del ejercicio)
   --------------------------------------------------
   El pool es un vector de slots; cada slot guarda (id del audio, prioridad,
   tiempo de inicio en frames). Cuando llega play(id, prioridad):
   - ¿slot libre? primera posición libre.
   - Si no: menor prioridad; empate: mayor tiempo de inicio.
   - Si no hay nada robable: RECHAZO explícito (y contarlo en debug: "10 de
     1000", la señal de que el pool es chico para tu juego).
   ============================================================================= */
#include "include/f25.hpp"

#include <cstdio>
#include <vector>

/* ------------------------- El pool (para la demo) ------------------------ */
namespace {

struct Voz {
    int id = -1;            /* -1 = slot libre */
    int prioridad = 0;      /* mayor = más importante */
    long inicioFrames = 0;  /* "reloj" del audio (frames desde init) */
};

class PoolVoces {
public:
    explicit PoolVoces(int slots) : slots_(slots) {}

    /* Devuelve el índice de slot usado, o -1 si rechaza. */
    int play(int id, int prioridad, long ahora) {
        /* 1) primer slot libre. */
        for (std::size_t i = 0; i < slots_.size(); ++i) {
            if (slots_[i].id == -1) {
                slots_[i] = Voz{ id, prioridad, ahora };
                return static_cast<int>(i);
            }
        }
        /* 2) steal: índice de la peor voz robable. */
        int mejorSteal = -1;
        for (std::size_t i = 0; i < slots_.size(); ++i) {
            const Voz& v = slots_[i];
            if (v.prioridad >= prioridad) {
                continue; /* no robo iguales ni superiores */
            }
            if (mejorSteal == -1) {
                mejorSteal = static_cast<int>(i);
                continue;
            }
            const Voz& r = slots_[mejorSteal];
            /* menor prioridad primero; empate: la más vieja */
            const bool gana = (v.prioridad < r.prioridad)
                           || (v.prioridad == r.prioridad
                               && v.inicioFrames < r.inicioFrames);
            if (gana) {
                mejorSteal = static_cast<int>(i);
            }
        }
        if (mejorSteal == -1) {
            return -1; /* rechazo explícito: contalo en debug */
        }
        slots_[mejorSteal] = Voz{ id, prioridad, ahora };
        return mejorSteal;
    }

    void stop(int slot) {
        if (slot >= 0 && slot < static_cast<int>(slots_.size())) {
            slots_[slot].id = -1;
        }
    }

    int activas() const {
        int n = 0;
        for (const Voz& v : slots_) {
            if (v.id != -1) { ++n; }
        }
        return n;
    }

    int idDe(int slot) const {
        return (slot >= 0 && slot < static_cast<int>(slots_.size()))
                   ? slots_[slot].id : -1;
    }

private:
    std::vector<Voz> slots_;
};

} // namespace

/* --------------------------- Modo con miniaudio -------------------------- */
#if DEMO_MINIAUDIO
#include "miniaudio.h"

int demoEngineReal() {
    ma_engine engine;
    ma_engine_config cfg = ma_engine_config_init();
    cfg.channels = 2;
    cfg.sampleRate = 48000;
    cfg.periodSizeInMilliseconds = 20; /* ~ 960 frames */

    if (ma_engine_init(&cfg, &engine) != MA_SUCCESS) {
        return 1;
    }
    std::printf("engine real OK: %u Hz x %u canales; endpoint listo.\n",
                ma_engine_get_sample_rate(&engine),
                ma_engine_get_channels(&engine));
    /* El dueño puede probar a mano con un WAV:  ma_engine_play_sound(
       &engine, "paso.wav", nullptr); y apreciar el pool FALSE de play
       efímero (esto se discute en la lección como "lento para juegos"). */
    ma_engine_uninit(&engine);
    return 0;
}
#endif /* DEMO_MINIAUDIO */

int main() {
    std::printf("M25 / L04 - engine, pool de voces y prioridad\n");
    std::printf("============================================\n");

    PoolVoces pool(4);
    long reloj = 0;

    /* Play normal: 3 voces con prioridades distintas; quedan slots libres. */
    const int s0 = pool.play(101, 5, reloj++); /* paso (SFX común) */
    const int s1 = pool.play(102, 3, reloj++);
    const int s2 = pool.play(103, 8, reloj++); /* explosión: alta */
    std::printf("plays: slots %d %d %d; activas=%d\n", s0, s1, s2, pool.activas());

    /* El pool se llena: la cuarta entra al último slot libre. */
    const int s3 = pool.play(104, 4, reloj++);
    std::printf("cuarta voz: slot %d; activas=%d (pool lleno)\n", s3, pool.activas());

    /* Pool LLENO. Un play con prioridad 5: roba la voz de menor prioridad
       (la 102 de prioridad 3). */
    const int s4 = pool.play(105, 5, reloj++);
    std::printf("robo (prioridad 5 sobre 3): slot %d -> id %d (era 102)\n",
                s4, pool.idDe(s4));

    /* Pool lleno con prioridad mínima: RECHAZO explícito, sin robo. */
    const int s5 = pool.play(106, 1, reloj++);
    std::printf("play prioridad 1: recibí slot %d (rechazado si es %d)\n",
                s5, -1);

    /* Stop de una voz y reutilización. */
    pool.stop(s0);
    const int s6 = pool.play(107, 2, reloj++);
    std::printf("tras stop, la 107 reusa el slot %d\n", s6);

#if DEMO_MINIAUDIO
    demoEngineReal();
#endif
    return 0;
}
