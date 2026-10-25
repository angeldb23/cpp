/* =============================================================================
   MÓDULO 18 - EJERCICIO 06: AssetResolver con base path inyectada (y pack
   de archivos simulado)
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 09 (SDL_GetBasePath/SDL_GetPrefPath/relativa), Módulo 12
          (std::filesystem), Módulo 03 (string/string_view)

   CONSIGNA:
   La lección 09 muestra cómo SDL te da la carpeta base y la de guarda-
   dos. Completá el RESOLVER lógico (sin SDL: la basePath es un string
   inyectado, como pseudo-SDL_GetBasePath) y el PACK simulado:
     1. normalizarBase(base): SIEMPRE termina con '/'; vacía es "" (CWD).
        (lo hace el resolver ya en constructor; vos completás el método)
     2. resolverAsset(rel): "" -> inválido; ABSOLUTA (arranca con '/') ->
        inválida (seguridad: un save corrupto no lee /etc/passwd); contiene
        ".." -> inválida; relativa válida -> base + rel.
     3. registrarPack(nombre, offset, long): el "pack" con índice
        (nombre -> {offset, long}) que la L09 describe: assets.pak.
     4. abrirDesdePack(nombre) -> std::optional<EntradaPack>: si el nombre
        está en el índice (registrado ANTES), devuelve la entrada; si no,
        nullopt. Los offsets del idx NO se consultan: el pack es simulado.
     5. regla del pack: registrar el MISMO nombre 2 veces es rechazo
        (registrarPack devuelve false y la entrada original queda intacta).

   ESTRUCTURA (el modelo, no tocar):
     struct EntradaPack { long offset; unsigned long long longi; }
   La clase AssetResolver (abajo) es la que completás:
     - AssetResolver(std::string base)    (normalizá la base al construir)
     - registrarPack (devuelve bool: true si se registró por 1ª vez)
     - resolverAsset(...) -> std::string
     - abrirDesdePack(...) -> std::optional<EntradaPack>
     - escribir ruta relativa con std::filesystem: la ruta de un asset es
       self-contained (no tocás el disco: devolvés strings; los tests no
       hacen operaciones de filesystem reales).

   TESTS (no tocar). COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra ejercicio_06_asset_resolver.cpp -o ej && ./ej
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

// ============================ TU CÓDIGO AQUÍ ================================
class AssetResolver {
public:
    explicit AssetResolver(std::string base) {
        // TODO 1: normalizar la base (termina en '/'; vacía queda vacía)
        (void)base;
    }

    // TODO 2..3: resolvers con guard (vacía/absoluta/".." rechazadas)
    std::string resolverAsset(const std::string& rel) {
        (void)rel;
        return {};
    }

    // TODO 4: registrarPack (true si es el primer registro del nombre) +
    // el índice interno del pack + abrirDesdePack
    bool registrarPack(const std::string& nombre, long offset, unsigned long long longi) {
        (void)nombre; (void)offset; (void)longi;
        return false;
    }

    std::optional<EntradaPack> abrirDesdePack(const std::string& nombre) {
        (void)nombre;
        return std::nullopt;
    }

    int cantidadRegistrada() const { return 0; }

private:
    std::string base_;
    // TODO: el mapa del pack va acá
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
