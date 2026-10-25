/* =============================================================================
   MÓDULO 25 - EJERCICIO 11: localización de audio (bancos + fallback +
   hot-swap)
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 11 (bancos por idioma, chain de fallback, hot-swap).

   CONSIGNA
   --------
   Completá `Localizador`:

   1. `agregarBanco(b)`: guarda el banco en bancos_ (push_back + move).
   2. `banco(idioma)`: devuelve PUNTERO al banco con ese idioma, o nullptr
      si no existe (busqueda lineal está bien: son 3-4 bancos).
   3. `resolver(id)`: recorre el chain_ EN ORDEN; el primer banco que tenga
      el audio_id gana (devolvé su lump). Si NADIE lo tiene: "" (silencio,
      contado en debug: la lección muestra por qué NO es un error duro).
   4. `hotSwap(nuevoChain)`: reemplaza el chain_ COMPLETO (el swap es de
      una variable: el costo del hot-swap real está en el crossfade del
      grupo de audio, no acá).

   `idiomaActivo()` ya está: es el PRIMER idioma del chain ( chain_.front() ).
   Y el chain por defecto es { "es-AR", "es-ES", "en-US" } (ya en el ctor).
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
        // TODO
        (void)b;
    }

    /* Resuelve el lump para un audio_id O contra el fallback. */
    std::string resolver(const std::string& id) const {
        // TODO
        (void)id;
        return "";
    }

    /* HOT-SWAP: cambia el frente del chain. */
    void hotSwap(const std::vector<std::string>& nuevoChain) {
        // TODO
        (void)nuevoChain;
    }

    const std::string& idiomaActivo() const {
        static const std::string vacio;
        return chain_.empty() ? vacio : chain_.front();
    }

private:
    const BancoIdioma* banco(const std::string& idioma) const {
        // TODO
        (void)idioma;
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
