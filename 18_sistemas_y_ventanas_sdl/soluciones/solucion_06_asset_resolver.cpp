/* =============================================================================
   MÓDULO 18 - SOLUCIÓN 06: AssetResolver con base path inyectada y pack
   =============================================================================
   MISMO ENUNCIADO que ejercicios/ejercicio_06_asset_resolver.cpp.
   SOLUCIÓN COMENTADA: qué hace cada pieza y por qué.

   PUNTOS CLAVE:
   - La base se NORMALIZA en el constructor: separación de preocupación
     entre base y "rel" (lección 09: SDL_GetBasePath SIEMPRE trae '/' final,
     pero la inyección falsa para tests no garantiza nada).
   - El índice del pack es un std::map<std::string, EntradaPack>: la
     REGLA "primera entrada gana" sale gratis con emplace (no sobreescribe
     si la clave ya existe: lección 11, tema de inserción fina).
   - Rechazo de rutas peligrosas: ABSOLUTA ('/' inicial) y '..' (salida
     de base): en un save corrupto evita leer archivos del sistema.
   ============================================================================= */
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include "../../include/test_framework.hpp"

// ============================ MODELO (no tocar) ==============================
struct EntradaPack {
    long                  offset = 0;     // offset en el pack (ficticio)
    unsigned long long    longi  = 0;     // longitud en bytes
};

// ============================ SOLUCIÓN ======================================
class AssetResolver {
public:
    explicit AssetResolver(std::string base) {
        // la base SIEMPRE termina con '/'; el vacío queda vacío (CWD)
        if (!base.empty() && base.back() != '/') base += '/';
        base_ = std::move(base);
    }

    std::string resolverAsset(const std::string& rel) {
        if (rel.empty()) return {};               // vacía: inválida
        if (!rel.empty() && rel.front() == '/') return {}; // absoluta: rechazo
        if (rel.find("..") != std::string::npos) return {}; // subida: rechazo
        return base_ + rel;   // base ya normalizada: append plano
    }

    bool registrarPack(const std::string& nombre, long offset, unsigned long long longi) {
        // primer registro gana: NO pisa la entrada ya existente (lección 11:
        // insert de map no sobreescribe si la clave existe)
        auto [it, nuevo] = pack_.emplace(nombre, EntradaPack{offset, longi});
        (void)it;
        return nuevo;
    }

    std::optional<EntradaPack> abrirDesdePack(const std::string& nombre) {
        auto it = pack_.find(nombre);
        if (it == pack_.end()) return std::nullopt;
        return it->second;
    }

    int cantidadRegistrada() const { return int(pack_.size()); }

private:
    std::string base_;
    std::map<std::string, EntradaPack> pack_;
};

// ============================ TESTS (no tocar) ==============================
TEST_CASE(BaseNormalizadaConSlash) {
    AssetResolver r("/home/perro/juego");
    ASSERT_EQ(r.resolverAsset("img/fondo.png"),
              std::string("/home/perro/juego/img/fondo.png")); // la base va con '/' final
    AssetResolver r2("/home/perro/juego/");
    ASSERT_EQ(r2.resolverAsset("img/fondo.png"),
              std::string("/home/perro/juego/img/fondo.png")); // ya con slash
    AssetResolver r3("");
    ASSERT_EQ(r3.resolverAsset("assets/a.bin"), std::string("assets/a.bin")); // CWD relativo
}

TEST_CASE(RechazoDeRutasPeligrosas) {
    AssetResolver r("/base/");
    ASSERT_EQ(r.resolverAsset(""), std::string(""));         // vacío: inválido
    ASSERT_EQ(r.resolverAsset("/etc/passwd"), std::string("")); // absoluta
    ASSERT_EQ(r.resolverAsset("img/../secrets"), std::string("")); // subida
    ASSERT_EQ(r.resolverAsset("img/ok.png"), std::string("/base/img/ok.png"));
}

TEST_CASE(RegistranteDePackCumplido) {
    AssetResolver r("/base/");
    ASSERT_TRUE(r.registrarPack("img/fondo.png", 0, 1024));
    ASSERT_FALSE(r.registrarPack("img/fondo.png", 9999, 88));  // duplicado: rechazado
    ASSERT_EQ(r.cantidadRegistrada(), 1);                      // no 2
    auto e = r.abrirDesdePack("img/fondo.png");
    ASSERT_TRUE(e.has_value());
    ASSERT_EQ(e->offset, 0);
    ASSERT_EQ(e->longi, 1024ULL);                              // la PRIMERA entrada
    ASSERT_FALSE(r.abrirDesdePack("img/faltante.png").has_value());
}

TEST_CASE(PackNoInterfiereConResolverLibre) {
    AssetResolver r("/base/");
    (void)r.registrarPack("img/fondo.png", 0, 1024);
    // el resolver STRING sigue resolviendo rutas que no están en el pack:
    ASSERT_EQ(r.resolverAsset("sfx/fuego.wav"), std::string("/base/sfx/fuego.wav"));
    ASSERT_EQ(r.cantidadRegistrada(), 1);
}

int main() { return RUN_ALL_TESTS(); }
