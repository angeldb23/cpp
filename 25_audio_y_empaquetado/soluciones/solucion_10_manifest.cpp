/* =============================================================================
   MÓDULO 25 - SOLUCIÓN 10: parser del manifest del pipeline de assets
   =============================================================================
   Un pass lineal: comentarios # y secciones [x] cambian el estado
   (rolActual_); las líneas "src @out dst" se trocean en el '@'. Es un parser
   "parseado a mano" DELIBERADO: sin librerías externas para que el cooker
   sea testeable con strings puros (la versión JSON del manifest sería el
   paso siguiente del cooker real, pero el pipeline que importa es la
   semántica: rol -> tipo de cook, destino en el pack, hash del src para
   detectar "stale").
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

std::string recortar(const std::string& t) {
    std::size_t a = 0;
    std::size_t b = t.size();
    while (a < b && (t[a] == ' ' || t[a] == '\t')) { ++a; }
    while (b > a && (t[b - 1] == ' ' || t[b - 1] == '\t')) { --b; }
    return t.substr(a, b - a);
}

class Manifest {
public:
    bool parse(const std::vector<std::string>& lines) {
        for (const std::string& raw : lines) {
            if (raw.empty() || raw[0] == '#') {
                continue;
            }
            if (raw.size() > 2 && raw[0] == '[') {
                rolActual_ = rolDeSeccion(raw);
                continue;
            }
            const std::size_t sep = raw.find('@');
            if (sep == std::string::npos) {
                return false; /* línea sin destino: error del cooker */
            }
            EntradaManifest e;
            e.fuente = recortar(raw.substr(0, sep));
            e.rol = rolActual_;
            std::string resto = recortar(raw.substr(sep + 1));
            if (resto.rfind("out", 0) == 0) {
                e.destino = recortar(resto.substr(3));
            }
            entradas_.push_back(e);
        }
        return true;
    }

    const std::vector<EntradaManifest>& entradas() const { return entradas_; }

private:
    static Rol rolDeSeccion(const std::string& s) {
        if (s.find("audio") != std::string::npos) { return Rol::Audio; }
        if (s.find("textura") != std::string::npos) { return Rol::Textura; }
        if (s.find("modelo") != std::string::npos) { return Rol::Modelo; }
        return Rol::Datos;
    }

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
