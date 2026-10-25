/* =============================================================================
   MÓDULO 25 - LECCIÓN 05: buses (grupos), volumen jerárquico, pan 2D/3D y
   ducking
   =============================================================================
   TEMA 3: Mezcla: volumen master/grupo, pitch, pan 2D/3D, ducking y
   sidechain (1 lección).

   BUSES = ma_sound_group (el "mixer" jerárquico del engine)
   ----------------------------------------------------------
   Un bus es un ma_sound_group colgado de otro ma_sound_group. La cadena
   típica (todos son nodos del mismo node graph, verificado:
   ma_sound_group_init + ma_sound_group_start + set_volume/get_volume):

     master (el engine mismo: ma_engine_set_volume)
       |-- music   (ma_sound_group; parent = nullptr = endpoint del engine)
       |-- sfx     (...)
       |     |-- ui (sub-bus de SFX)
       |-- ambience

   Un ma_sound se inicializa GRUPO: si su grupo se baja a 0, el sonido
   "desaparece de la mezcla" sin tocar su propio volumen (ideal para mute por
   bus con settings del menú). El gain efectivo es el producto de la cadena:
     vol_efectivo = master * grupo * subgrupo * propio
   y si lo seteás en cualquier bus, TODOS los sons debajo se actualizan en el
   próximo mix (con smoothing: cfg.defaultVolumeSmoothTimeInPCMFrames).

   PITCH
   -----
   `ma_sound_set_pitch` re-samplea la voz en el mix (pitch 0.5 = mitad de
   frecuencia, suena una octava abajo y dura el doble). Es otro USO+RESERVA
   de CPU (resampler por voz): si no lo usás, MA_SOUND_FLAG_NO_PITCH
   (verificado en el header) desactiva el resampler especializado y ahorra.

   PAN 2D y 3D (y el dato que TEMAS pidió verificar)
   -------------------------------------------------
   - 2D: ma_sound_set_pan en [-1, +1] con dos modos verificados:
     ma_pan_mode_balance (el default: tipo "fader de mesa": el canal
     contrario NO se mezcla) y ma_pan_mode_pan (pan "verdadero": lo que
     viene del centro se mezcla también al canal contrario).
     ma_sound_set_pan_mode(&sound, ma_pan_mode_pan).
   - 3D: ma_sound_set_position + MA listener del engine:
     ma_attenuation_model_none / inverse (default) / linear / exponential,
     ma_sound_set_rolloff, ma_sound_set_min_gain / set_max_gain,
     ma_sound_set_cone (directividad), ma_sound_set_velocity (doppler) y
     ma_sound_set_directional_attenuation_factor.
     Todo esto hace things on the engine in the 3D pipeline: "roughly on
     feature parity with OpenAL" (header 0.11.25, sección spatialization).
   - HRTF: NO EXISTE en miniaudio 0.11.25. La doc del header lo dice
     explícito: "HRTF and environmental occlusion are not currently
     supported, but planned for the future." -> si tu juego necesita binaural,
     tendrás que implementarlo (por ejemplo con IRs de-head-related transfer
     function cargadas en ma_convolver — fuera del alcance del curso).
     TEMAS.md decía "a verificar": verificado -> NO hay HRTF; hay pan
     balance/pan y atenuación por distancia + doppler.

   DUCKING y SIDECHAIN (la técnica de "televisión y radio")
   ---------------------------------------------------------
   Sidechain = el "gate" que BAJA el volumen de un canal cuando otro hace
   "señal" (parlante en radio: la música cae cuando hablás). En un juego:
   la música baja cuando suena diálogo ("dialogue ducking"). No hay un nodo
   "sidechain" en miniaudio: la técnica se hace a mano/gestión:
    Sidechain = el "gate" que BAJA el volumen de un canal cuando otro hace
   señal (parlante en radio: la música cae cuando hablás). En un juego:
   la música baja cuando suena diálogo ("dialogue ducking"). No hay un nodo
   "sidechain" en miniaudio: la técnica es gestión de volumen:
     - objetivo = volMusic * 0.3 mientras el diálogo suene;
     - transición con ma_sound_set_fade_in_milliseconds(grupoMusic,
       fadeBegin, fadeEnd, ms) (verificado; volumenBeg = -1 = "el actual"),
       o con smoothing del engine (cfg.defaultVolumeSmoothTimeInPCMFrames).
     - release: al terminar el diálogo, fade a 1.0 relativo en ~600-800 ms
       (más lento que el ataque: la "respiración" del humano detrás de la
       música).
   El ejercicio 05 implementa el DER (gain follower) sin miniaudio para poder
   testearlo: un pass atenuado por frame con target y release añadidos.
   ============================================================================= */
#include "include/f25.hpp"

#include <cmath>
#include <cstdio>
#include <vector>

/* ------------------- Simulación del grafo (headless) ---------------------- */
namespace {

struct Bus {
    std::vector<Bus*> hijos;
    float volumen = 1.0f;    /* lineal [0, 1] (0 = mute) */
};

/* El "engine" sim: master + tree de buses. */
struct MotorSim {
    Bus master;
    Bus music;
    Bus sfx;
    Bus ambience;

    MotorSim() {
        master.hijos = { &music, &sfx, &ambience };
        music.volumen = 0.7f;
        sfx.volumen = 1.0f;
        ambience.volumen = 0.5f;
    }

    /* Volúmenes compuestos en cascada (master x grupo). */
    float volumenDe(const Bus& b) const { return master.volumen * b.volumen; }

    /* Ducking: el grupo music baja al 30% mientras dura el diálogo. */
    void duckOn()  { music.volumen = 0.30f * 0.7f; } /* 0.21 del "máximo"; */
    void duckOff() { music.volumen = 0.70f; }
};

} // namespace

/* --------------------------- Modo con miniaudio -------------------------- */
#if DEMO_MINIAUDIO
#include "miniaudio.h"

int demoBusesReal() {
    ma_engine engine;
    ma_engine_config cfg = ma_engine_config_init();
    cfg.channels = 2;
    cfg.sampleRate = 48000;
    cfg.defaultVolumeSmoothTimeInPCMFrames = 480; /* 10 ms de smoothing */
    if (ma_engine_init(&cfg, &engine) != MA_SUCCESS) {
        return 1;
    }

    /* Buses: music y sfx colgados del endpoint del engine (parent = nullptr). */
    ma_sound_group music;
    ma_sound_group sfx;
    ma_sound_group_init(&engine, 0, nullptr, &music);
    ma_sound_group_init(&engine, 0, nullptr, &sfx);
    ma_sound_group_set_volume(&music, 0.7f);
    ma_sound_group_set_volume(&sfx, 1.0f);
    ma_sound_group_set_pan_mode(&music, ma_pan_mode_pan);
    (void)engine;
    ma_engine_uninit(&engine);
    return 0;
}
#endif /* DEMO_MINIAUDIO */

int main() {
    std::printf("M25 / L05 - buses, pan y ducking\n");
    std::printf("================================\n");

    MotorSim motor;
    std::printf("volumen music = master(%.2f) x bus(%.2f) = %.4f\n",
                motor.master.volumen, motor.music.volumen, motor.volumenDe(motor.music));
    std::printf("volumen ambience = %.4f\n", motor.volumenDe(motor.ambience));

    /* Ducking directo (sin estados intermedios: la versión "fade" está en
       el ejercicio 05 con el gain follower). */
    motor.duckOn();
    std::printf("ducking ON : music = %.4f (30%% de 0.7)\n", motor.volumenDe(motor.music));
    motor.duckOff();
    std::printf("ducking OFF: music = %.4f\n", motor.volumenDe(motor.music));

    /* Pan panorámico balance vs pan (la diferencia conceptual importantísima):
       -0.5 en balance = solo soft-left, la caja RIGHT queda intacta;
       en pan "true" la señal llena con una mezcla del otro canal. */
    std::printf("pan -1..+1: use ma_pan_mode_balance (default, 'MIXER') or\n");
    std::printf("            ma_pan_mode_pan (hard center-bleed, cinemático)\n");

    /* El dato verificado: HRTF no existe en 0.11.25 (doc del header). */
    std::printf("3D: atenuación inverse/linear/exponential + Set Position, cones y\n");
    std::printf("    doppler SÍ; HRTF NO (doc: 'planned for the future').\n");

    /* ducking con fade: la técnica "radio" adaptada a juego. */
    std::printf("ducking suave: ma_sound_set_fade_in_milliseconds(group, -1, 0.21, 300);\n");
    std::printf("release:      set_fade(..., -1, 0.70, 700); (release más lento)\n");

#if DEMO_MINIAUDIO
    demoBusesReal();
#endif
    return 0;
}
