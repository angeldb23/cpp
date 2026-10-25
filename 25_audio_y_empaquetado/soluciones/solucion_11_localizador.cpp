/* =============================================================================
   MÓDULO 25 - SOLUCIÓN 11: localización de audio (bancos + fallback +
   hot-swap)
   =============================================================================
   La lógica entera cabe en 20 líneas: la “dificultad” del tema 7 real no
   está acá sino en el crossfade del hot-swap (ma_sound_group_set_volume +
   fade en ms, lección 05) y en que NO REVIENTE cuando falta un audio_id:
   resolver devuelve "" y el juego toca silencio contado en debug, en vez de
   tirar el frame o crashear por un asset faltante del banco.
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <string>
#include <utility>
#include <unordered_map>
#include <vector>

namespace {

using MapaAudio = std::unordered_map<std::string, std::string>;

struct BancoIdioma {
    std::string idioma;            /* "es-AR", "en-US", etc. */
    MapaAudio audio;               /* audio_id -> nombre de lump */
};

class Localizador {
public:
    void agregarBanco(BancoIdioma b) {
        bancos_.push_back(std::move(b));
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

    /* HOT-SWAP: cambia el frente del chain (swap de la variable). */
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
            if (b.idioma == idioma) { return &b; }
        }
        return nullptr;
    }

    std::vector<BancoIdioma> bancos_;
    std::vector<std::string> chain_ = { "es-AR", "es-ES", "en-US" };
};

} // namespace

/* ------------------------------ TESTS (no tocar)-------------------------- */
TEST_CASE(ResolucionDirecta) {
    Localizador loc;
    BancoIdioma esAR;
    esAR.idioma = "es-AR";
    esAR.audio = { { "disparo", "es-ar/disparo.ogg" } };
    BancoIdioma enUS;
    enUS.idioma = "en-US";
    enUS.audio = { { "disparo", "en-us/disparo.ogg" } };
    loc.agregarBanco(std::move(esAR));
    loc.agregarBanco(std::move(enUS));
    /* el chain default arranca por es-AR: el banco lo tiene. */
    ASSERT_TRUE(loc.resolver("disparo") == "es-ar/disparo.ogg");
    ASSERT_TRUE(loc.idiomaActivo() == "es-AR");
}

TEST_CASE(FallbackChain) {
    Localizador loc;
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
    loc.agregarBanco(std::move(esAR));
    loc.agregarBanco(std::move(esES));
    loc.agregarBanco(std::move(enUS));
    /* salto NO está en es-AR: cae al segundo del chain (es-ES). */
    ASSERT_TRUE(loc.resolver("salto") == "es-es/salto.ogg");
    /* dolor solo está en el default (en-US): doble fallback. */
    ASSERT_TRUE(loc.resolver("dolor") == "en-us/dolor.ogg");
    /* fantasma no existe NINGUN lado: silencio (""), no error. */
    ASSERT_TRUE(loc.resolver("fantasma") == "");
    /* disparo NO asciende: gana el PRIMERO del chain que lo tenga. */
    ASSERT_TRUE(loc.resolver("disparo") == "es-ar/disparo.ogg");
}

TEST_CASE(HotSwap) {
    Localizador loc;
    BancoIdioma enUS;
    enUS.idioma = "en-US";
    enUS.audio = { { "disparo", "en-us/disparo.ogg" } };
    loc.agregarBanco(std::move(enUS));
    /* El jugador eligió pt-BR (no hay banco): el chain nuevo hace caer
       TODO a en-US (el "default casi siempre completo"). */
    loc.hotSwap({ "pt-BR", "en-US" });
    ASSERT_TRUE(loc.resolver("disparo") == "en-us/disparo.ogg");
    ASSERT_TRUE(loc.idiomaActivo() == "pt-BR");
    /* Y se puede volver (el swap es reversible: es una variable). */
    loc.hotSwap({ "es-AR", "en-US" });
    ASSERT_TRUE(loc.idiomaActivo() == "es-AR");
    ASSERT_TRUE(loc.resolver("disparo") == "en-us/disparo.ogg");
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
