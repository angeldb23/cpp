/* =============================================================================
   MÓDULO 25 - LECCIÓN 10: pipeline de assets: import, proceso offline,
   manifest y el "cooker"
   =============================================================================
   TEMA 6: Pipeline de assets: import (aseprite, tiled, blender),
   procesamiento offline (texture compression, audio encode) y manifest
   (1 lección).

   EL PIPELINE (por qué "no se guardan los archivos finales a mano")
   -----------------------------------------------------------------
   El asset del editor NO es el asset del juego. La cadena real:
    Fuente en editor -> PASO 1: import -> PASO 2: cook -> pack.bin
   - IMPORT: los formatos del editor son "documentos de trabajo" (frágiles,
     llenos de metadatos de edición): .aseprite (Aseprite), .tiled (Tiled),
     .blend (Blender), .psd (Photoshop). El juego NUNCA los lee.
   - COOK: convertir a lo que la GPU y el mixer quieren EN CADA PLATAFORMA:
       textura png     -> BC1/BC3/BC7 (S3TC/BCn) o KTX2/basisu: un binario
                          GPU-direct, sin decoder en runtime.
       sprite .aseprite -> atlas + JSON de frames (módulo 17).
       sonido .wav 96k  -> .ogg 44.1k (10x más chico, mismo sueno percibido).
       El cook es reproducible: MISMO fuente + misma versión de tooling =
       MISMO binario de salida (build determinista).
   - PACK: la lección 08 toma los binarios finales y escribe el .pak con su
     TOC. El juego NUNCA descomprime el pack entero: lee lumps por offset.

   MANIFEST (el inventario determinista del cooker)
   -------------------------------------------------
   Cada asset tiene UNA entrada; el cooker la consume línea por línea:
     [texturas]
     s_hud.png        @out atlas/hud.bin
     [audio]
     s_explosion.wav  @out sfx/explosion.ogg
   Secciones por rol: el rol decide el paso de cook (TEXTURA -> BCn;
   AUDIO -> OGG; DATOS -> copiar tal cual está). Con el FPS del CRC del src
   (lección 09) el cooker marca "stale" solo lo que cambió: re-cocinar de
   nuevo todo el proyecto en cada build sería carísimo.
   ============================================================================= */
#include "include/f25.hpp"

#include <cstdint>
#include <cstdio>
#include <string>
#include <unordered_map>
#include <vector>

/* ----------------------------- Manifest parser ---------------------------- */
namespace {

enum class Rol { Textura, Audio, Modelo, Datos };

struct EntradaManifest {
    std::string fuente;   /* src real en el repo */
    std::string destino;  /* output del cooker */
    Rol rol = Rol::Datos;
};

/* Parser "key candido -> key/value" del manifest (sin JSON para poder
   testear; JSON sería el siguiente paso del ejercicio). */
class Manifest {
public:
    bool parse(const std::vector<std::string>& lines) {
        for (const std::string& raw : lines) {
            if (pasa(raw)) {
                continue;
            }
            if (esSeccionYVale(raw)) {
                rolActual_ = rolDeSeccion(raw);
                continue;
            }
            const std::size_t sep = raw.find('@');
            if (sep == std::string::npos) {
                return false;
            }
            std::string fuente = recortar(raw.substr(0, sep));
            std::string resto = recortar(raw.substr(sep + 1));
            EntradaManifest e;
            e.fuente = fuente;
            e.rol = rolActual_;
            parseResto(resto, e);
            entradas_.push_back(e);
        }
        return true;
    }

    const std::vector<EntradaManifest>& entradas() const { return entradas_; }

private:
    static bool pasa(const std::string& s) {
        return s.empty() || s[0] == '#';
    }
    static std::string recortar(const std::string& t) {
        std::size_t a = 0;
        std::size_t b = t.size();
        while (a < b && (t[a] == ' ' || t[a] == '\t')) { ++a; }
        while (b > a && (t[b - 1] == ' ' || t[b - 1] == '\t')) { --b; }
        return t.substr(a, b - a);
    }
    static bool esSeccionYVale(const std::string& s) {
        return s.size() > 2 && s[0] == '[';
    }
    static Rol rolDeSeccion(const std::string& s) {
        if (s.find("audio") != std::string::npos) {
            return Rol::Audio;
        }
        if (s.find("textura") != std::string::npos) {
            return Rol::Textura;
        }
        if (s.find("modelo") != std::string::npos) {
            return Rol::Modelo;
        }
        return Rol::Datos;
    }
    void parseResto(std::string s, EntradaManifest& e) {
        /* "out atlas/hud.bin": destino directo (sin dos puntos). */
        if (s.rfind("out", 0) == 0) {
            e.destino = recortar(s.substr(3));
        }
    }

    Rol rolActual_ = Rol::Datos;
    std::vector<EntradaManifest> entradas_;
};

} // namespace

int main() {
    std::printf("M25 / L10 - pipeline de assets y manifest\n");
    std::printf("=========================================\n");

    const std::vector<std::string> texto = {
        "# manifest del juego (comentarios con #)",
        "[texturas]",
        "s_hud.png           @out atlas/hud.bin",
        "[audio]",
        "s_explosion.wav     @out sfx/explosion.ogg",
        "s_pat_hat.wav       @out music/drums.ogg",
        "[datos]",
        "mapa.tiled          @out bin/mapa_tilemap.json",
    };
    Manifest m;
    if (!m.parse(texto)) {
        std::printf("manifest inválido: errores de una línea\n");
        return 1;
    }
    const char* rols[] = { "Textura", "Audio", "Modelo", "Datos" };
    for (const EntradaManifest& e : m.entradas()) {
        std::printf("  src %-18s rol %-8s out %s\n", e.fuente.c_str(),
                    rols[static_cast<int>(e.rol)], e.destino.c_str());
    }

    /* El "cooker plan": QUÉ hace por rol (los detalles del pipeline). */
    std::printf("plan por rol:\n");
    std::printf("  Textura -> comprimir a BC3/BC7 (binario GPU-direct)\n");
    std::printf("  Audio   -> encode OGG (Vorbis ~160 kbps mono/stereo)\n");
    std::printf("  Datos   -> copiar el source (JSON/Tiled ya es final)\n");
    std::printf("  todo    -> CRC32 por lump (lección 09), pack final\n");

    /* La ley de "stale": si el CRC del src cambió, el cooker vuelve a run. */
    std::printf("stale: re-cocina lo que cambió (hash del src al manifest)\n");
    return 0;
}
