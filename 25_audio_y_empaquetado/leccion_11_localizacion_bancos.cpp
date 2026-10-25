/* =============================================================================
   MÓDULO 25 - LECCIÓN 11: localización de audio: bancos por idioma, fallback
   y hot-swap sin reiniciar
   =============================================================================
   TEMA 7: Localización: bancos de audio por idioma, fallback y hot-swap
   sin reiniciar (1 lección).

   LOCALIZAR AUDIO = LOCALIZAR TODO
   --------------------------------
   Audio también se localiza: voces de VO (actores por idioma), SFX
   localizados a la cara (un mismo golpe puede sonar "gritero" en un
   idioma y seco en otro) y la "música del menú" localizada (temas por
   idioma, algo común en juegos con bandas regionales). El esquema
   canónico:
     audio/
       es-AR/     <- banco por idioma
       es-ES/
       en-US/     <- el banco "de trabajo" (casi siempre completo)
   Cada banco mapea audio_id -> nombre del lump. La lógica del juego solo
   llama play(AudioId::Disparo); el localizador resuelve contra el banco
   activo y su chain de fallback.

   FALLBACK (el "chain" de idiomas)
   --------------------------------
   La idea es: "sirvo el audio de es-AR; si el idioma NO tiene el audio_id,
   CAER al siguiente del chain". El chain canónico concreto:
     idioma pedido -> idioma base (casi siempre completo) -> default (en-US)
   Si el chain falla completo: tocar SILENCIO (y contarlo en debug:
   "audio faltante: X") es PREFERIBLE a error: el juego no se lanza abajo
   por una ficha faltante en un banco.

   HOT-SWAP SIN REINICIAR
   ----------------------
   Si la config de idioma cambia EN EL MENÚ, el motor:
   1. Carga el banco nuevo ASINCRÓNICO (no bloquea el frame: lección 03).
   2. CROSSFADE: los sonidos ya sonando terminan con el banco VIEJO; los
      nuevos ya arrancan con el banco nuevo (un sonido ya EN PLAY devolver
      reelección abrupta produce click; el crossfade suena "profesional").
   3. El smoothing del engine ayuda: defaultVolumeSmoothTimeInPCMFrames
      (verificado en ma_engine_config) + ma_sound_set_fade_in_milliseconds
      (lección 05) para la transición del grupo (el bus hace lo mismo).
   4. Los SFX EN LOOP (pasos, motores) cambian de banco apenas el loop
      vuelve a pedir datos: es "hot-swap barato": el swap ocurre en el
      BORDE DEL LOOP (loop boundary), nunca a mitad de un sonido.
   ============================================================================= */
#include "include/f25.hpp"

#include <cstdio>
#include <string>
#include <unordered_map>
#include <vector>

/* ------------------------ El banco (para la demo) ------------------------- */
namespace {

using MapaAudio = std::unordered_map<std::string, std::string>;

struct BancoIdioma {
    std::string idioma;            /* "es-AR", "en-US", etc. */
    MapaAudio audio;               /* audio_id -> nombre de lump */
};

class Localizador {
public:
    /* Los bancos se cargan "una vez" (el "chain" se pasa en el ctor). */
    void agregarBanco(BancoIdioma b) {
        bancos_.push_back(std::move(b));
    }

    /* El "chain" de bajo hacia arriba: idioma pedido -> base -> default. */
    void setChain(std::vector<std::string> chain) {
        chain_ = std::move(chain);
    }

    /* Resuelve el lump para un audio_id O contra el fallback. */
    std::string resolver(const std::string& id) const {
        for (const std::string& idioma : chain_) {
            const BancoIdioma* b = banco(idioma);
            if (b != nullptr) {
                const auto it = b->audio.find(id);
                if (it != b->audio.end()) {
                    return it->second;
                }
            }
        }
        return ""; /* no encontrado: silencio + contador en debug */
    }

    /* HOT-SWAP: cambia el frente del chain (idéntico swap de id). */
    void hotSwap(const std::vector<std::string>& nuevoChain) {
        chain_ = nuevoChain;
    }

    const std::string& idiomaActivo() const {
        static const std::string vacio;
        return chain_.empty() ? vacio : chain_.front();
    }

private:
    const BancoIdioma* banco(const std::string& idioma) const {
        for (const BancoIdioma& b : bancos_) {
            if (b.idioma == idioma) {
                return &b;
            }
        }
        return nullptr;
    }

    std::vector<BancoIdioma> bancos_;
    std::vector<std::string> chain_ = { "es-AR", "es-ES", "en-US" };
};

} // namespace

/* --------------------------- Modo con miniaudio -------------------------- */
#if DEMO_MINIAUDIO
#include "miniaudio.h"
/* Hot-swap real: los SOLES lumps van por asset; ninguno reloj del engine:
   el swap es un swap de PUNTERO al banco (thread-safe si se hace en el
   hilo de update del juego, nunca dentro del data callback). */
int demoHotSwapReal() {
    ma_engine engine;
    ma_engine_config cfg = ma_engine_config_init();
    cfg.defaultVolumeSmoothTimeInPCMFrames = 480; /* 10 ms de smoothing */
    if (ma_engine_init(&cfg, &engine) != MA_SUCCESS) {
        return 1;
    }
    ma_sound_group grupo;
    ma_sound_group_init(&engine, 0, nullptr, &grupo);
    ma_sound_group_start(&grupo);
    ma_sound_group_set_volume(&grupo, 1.0f);
    /* En el juego real: al cambiar idioma, fade-out del grupo (lección 05)
       mientras el banco nuevo carga async, y fade-in cuando está listo. */
    (void)engine;
    ma_engine_uninit(&engine);
    return 0;
}
#endif /* DEMO_MINIAUDIO */

int main() {
    std::printf("M25 / L11 - localización de audio\n");
    std::printf("=================================\n");

    BancoIdioma esAR;
    esAR.idioma = "es-AR";
    esAR.audio = { { "disparo", "es-ar/disparo.ogg" } };

    BancoIdioma esES;
    esES.idioma = "es-ES";
    esES.audio = { { "disparo", "es-es/disparo.ogg" },
                   { "salto", "es-es/salto.ogg" } };

    BancoIdioma enUS;
    enUS.idioma = "en-US";
    enUS.audio = { { "disparo", "en-us/disparo.ogg" },
                   { "salto", "en-us/salto.ogg" },
                   { "dolor", "en-us/dolor.ogg" } };

    Localizador loc;
    loc.agregarBanco(std::move(esAR));
    loc.agregarBanco(std::move(esES));
    loc.agregarBanco(std::move(enUS));

    /* Chain por defecto: es-AR -> es-ES -> en-US. */
    std::printf("chain activo: %s\n", loc.idiomaActivo().c_str());
    std::printf("resolver disparo -> %s (el banco es-AR lo tiene)\n",
                loc.resolver("disparo").c_str());
    std::printf("resolver salto   -> %s (es-AR NO lo tiene: fallback es-ES)\n",
                loc.resolver("salto").c_str());
    std::printf("resolver dolor   -> %s (doble fallback: en-US)\n",
                loc.resolver("dolor").c_str());
    std::printf("resolver fantasma-> '%s' (no existe: SILENCIO contado)\n",
                loc.resolver("fantasma").c_str());

    /* Hot-swap: el jugador puso pt-BR... no existe: cae franco a en-US. */
    loc.hotSwap({ "pt-BR", "en-US" });
    std::printf("hot-swap a pt-BR: disparo -> %s (pt-BR no está: en-US)\n",
                loc.resolver("disparo").c_str());
    std::printf("idioma activo: %s\n", loc.idiomaActivo().c_str());

#if DEMO_MINIAUDIO
    demoHotSwapReal();
#endif
    return 0;
}
