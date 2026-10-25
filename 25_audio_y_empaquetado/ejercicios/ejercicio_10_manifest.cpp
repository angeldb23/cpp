/* =============================================================================
   MÓDULO 25 - EJERCICIO 10: parser del manifest del pipeline de assets
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 10 (pipeline: import -> cook -> pack; el manifest es el
   inventario que consume el cooker).

   CONSIGNA
   --------
   Completá `Manifest::parse()`. Formato de línea a soportar:

     - Líneas vacías o que arrancan con '#'   -> se ignoran.
     - "[seccion]"                            -> cambia el rol ACTUAL:
              [audio]     -> Rol::Audio
              [texturas]  -> Rol::Textura
              [modelo]    -> Rol::Modelo
              cualquier otra -> Rol::Datos
     - "fuente @out destino"                  -> una EntradaManifest:
              e.fuente   = texto ANTES del '@' (recortado, sin espacios)
              e.rol      = el rol de la sección vigente
              e.destino  = texto después de "@out" (recortado)
              (las claves van pegadas: "@out": después del '@' viene
               "out " y lo que queda es el destino.)
     - Una línea que NO tenga '@' (y no sea comentario/sección) -> error:
       parse() devuelve false.

   IMPORTANTE: el rol se toma de la sección vigente AL MOMENTO de parsear
   la línea (el parser guarda rolActual_ como estado).
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <string>
#include <vector>

namespace {

enum class Rol { Textura, Audio, Modelo, Datos };

struct EntradaManifest {
    std::string fuente;   /* src real en el repo */
    std::string destino;  /* output del cooker */
    Rol rol = Rol::Datos;
};

[[maybe_unused]] std::string recortar(const std::string& t) {
    std::size_t a = 0;
    std::size_t b = t.size();
    while (a < b && (t[a] == ' ' || t[a] == '\t')) { ++a; }
    while (b > a && (t[b - 1] == ' ' || t[b - 1] == '\t')) { --b; }
    return t.substr(a, b - a);
}

class Manifest {
public:
    bool parse(const std::vector<std::string>& lines) {
        // TODO
        (void)lines;
        return true;
    }

    const std::vector<EntradaManifest>& entradas() const { return entradas_; }

private:
    Rol rolActual_ = Rol::Datos;
    std::vector<EntradaManifest> entradas_;
};

} // namespace

/* ------------------------------ TESTS (no tocar)-------------------------- */
TEST_CASE(ParseBasico) {
    const std::vector<std::string> texto = {
        "# manifest del juego",
        "[texturas]",
        "s_hud.png           @out atlas/hud.bin",
        "[audio]",
        "s_explosion.wav     @out sfx/explosion.ogg",
        "s_pat_hat.wav       @out music/drums.ogg",
        "[datos]",
        "mapa.tiled          @out bin/mapa_tilemap.json",
    };
    Manifest m;
    ASSERT_TRUE(m.parse(texto));
    ASSERT_EQ(m.entradas().size(), static_cast<std::size_t>(4));
    ASSERT_TRUE(m.entradas()[0].fuente == "s_hud.png");
    ASSERT_TRUE(m.entradas()[0].destino == "atlas/hud.bin");
    ASSERT_TRUE(m.entradas()[1].fuente == "s_explosion.wav");
    ASSERT_TRUE(m.entradas()[1].destino == "sfx/explosion.ogg");
}

TEST_CASE(RolesPorSeccion) {
    const std::vector<std::string> texto = {
        "[texturas]",
        "s_hud.png @out atlas/hud.bin",
        "[audio]",
        "s_explosion.wav @out sfx/explosion.ogg",
        "[modelo]",
        "nave.blend @out modelos/nave.bin",
        "[datos]",
        "mapa.tiled @out bin/mapa_tilemap.json",
    };
    Manifest m;
    ASSERT_TRUE(m.parse(texto));
    ASSERT_EQ(m.entradas().size(), static_cast<std::size_t>(4));
    ASSERT_TRUE(m.entradas()[0].rol == Rol::Textura);
    ASSERT_TRUE(m.entradas()[1].rol == Rol::Audio);
    ASSERT_TRUE(m.entradas()[2].rol == Rol::Modelo);
    ASSERT_TRUE(m.entradas()[3].rol == Rol::Datos);
}

TEST_CASE(LineaInvalida) {
    /* Sin '@' el cooker no sabe qué hacer: parse() devuelve false. */
    Manifest m1;
    ASSERT_FALSE(m1.parse({ "s_hud.png conout atlas/hud.bin" }));
    /* Y una línea de comentario SU apagada al principio no rompe: */
    Manifest m2;
    ASSERT_TRUE(m2.parse({ "# solo un comentario", "" }));
    ASSERT_EQ(m2.entradas().size(), static_cast<std::size_t>(0));
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
