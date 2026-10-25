/* =============================================================================
   MÓDULO 18 - LECCIÓN 09: Filesystem y assets: SDL_GetBasePath/SDL_GetPrefPath
   =============================================================================
   TEMA 6 de TEMAS.md: Filesystem y assets: SDL_GetBasePath, SDL_GetPrefPath,
   carga relativa y empaquetado (1 lección).

   VERIFICADO EN CABECERAS:
     SDL3 (SDL_filesystem.h, 3.4.18):
       const char* SDL_GetBasePath(void);  // CACHEADO: propiedad de SDL, NO
                                           // se libera (cambio vs SDL2!)
       char* SDL_GetPrefPath(const char* org, const char* app); // este SÍ
                                           // se devuelve con SDL_free
     SDL2 (2.32.74):
       char* SDL_GetBasePath(void);        // se devuelve con SDL_free
       char* SDL_GetPrefPath(org, app);    // igual
   (Podés confirmarlo con grep: extern...SDL_GetBasePath en ambos headers.)

   QUÉ DEVUELVEN (semántica de plataforma):
   - BasePath: donde VIVE el ejecutable (o el sandbox en móviles). Ideal
     para ASSETS de solo lectura. En SDL3 termina SIEMPRE con '/'.
   - PrefPath: ~/.local/share/org/ap (Linux), %APPDATA%\org\ap (Windows),
     ~/.apps/org/ap (macOS), el lugar para partidas guardadas y configs.
     Lo CREA si no existe (mkdir -p). Uso tu std::filesystem del módulo 12.

   LA TRAMPA "MI .EXE Y MIS ASSETS NO ESTÁN DONDE PENSÁS":
   - Correr ./juego desde otra carpeta: el CWD cambia, la BasePath NO.
   - Steam/Linux: el ejecutable suele estar en bin/ y los assets en ../share.
     Con BasePath construís rutas RELATIVAS a tu binario: nunca al CWD.
   - Wayland/Flatpak: BasePath puede estar en un sandbox distinto (los
     pref paths son lo que SDL te devuelve para eso: sabe resolverlos).

   EMPAQUETADO CASERO (intro al tema; la versión seria es el módulo 25):
     assets.pak = tabla (nombre,offset,longitud) + bytes crudos concatenados.
     Cargás el pak UNA vez, guardás el índice de offset/len, y "abrir" un
     asset es fseek+read con esos rangos: un pack con archivos para tests
     SIN filesystem real (ej 06: resolver rutas lógicas de un pack ficticio).
   ============================================================================= */

#include <filesystem>
#include <iostream>
#include <SDL3/SDL.h>
namespace fs = std::filesystem;

// -----------------------------------------------------------------------------
// 1. RESOLVER UNA RUTA DE ASSET (la lógica del ej 06, en versión	SDL)
// -----------------------------------------------------------------------------
struct AssetResolver {
    std::string basePath;  // inyecto el resultado de SDL_GetBasePath (o fake)

    // Regla: SIEMPRE base + relativa; una ruta ABSOLUTA se rechaza (seguridad:
    // un save corrupto que pide "/etc/passwd" no accede a un asset serio).
    fs::path resolver(const std::string& rel) const {
        if (rel.empty()) return {};
        if (rel.starts_with('/') || rel.find("..") != std::string::npos) return {}; // rechazo
        std::string base = basePath;
        if (!base.empty() && !base.ends_with('/')) base += '/';  // normalizo
        return fs::path(base + rel);
    }
};

void mostrarResolvers(const std::string& base) {
    std::cout << "=== LECCIÓN 09: filesystem ===\n";
    AssetResolver r{base};
    std::cout << "resolves 'img/fondo.png' -> " << r.resolver("img/fondo.png").string() << "\n"
              << "resolves '/tmp/mal.png'   -> '" << r.resolver("/tmp/mal.png").string()
              << "' (vacío: absoluta rechazada)\n"
              << "resolves '../secret.dat'  -> '" << r.resolver("../secret.dat").string()
              << "' (vacío: salir de la base rechazado)\n";
}

// -----------------------------------------------------------------------------
// 2. DEMO RUNTIME: imprime tu BasePath/PrefPath reales (sin ventana)
// -----------------------------------------------------------------------------
static int demoFilesystem() {
    if (!SDL_Init(SDL_INIT_VIDEO)) { std::cout << "init " << SDL_GetError() << "\n"; return 1; }
    // (según la cabecera, es thread-safe y sin requisito de init; lo dejamos
    //  con SDL_Init por coherencia con el resto del módulo)
    const char* base = SDL_GetBasePath();
    std::cout << "SDL_GetBasePath() = " << (base ? base : "(null)") << "\n";
    mostrarResolvers(base ? base : "");

    char* pref = SDL_GetPrefPath("miOrg", "MiJuego");
    if (pref) {
        std::cout << "SDL_GetPrefPath    = " << pref << "\n";
        // guardar una config: igual que std::ofstream del módulo 12
        fs::create_directories(fs::path(pref) / "guardado");
        std::cout << "carpeta guardado: " << (fs::path(pref) / "guardado").string() << "\n";
        SDL_free(pref);  // SOLO el pref: el base NO se libera en SDL3!
    }

    std::cout << "\nCWD actual: " << fs::current_path().string() << "\n"
              << "(usá siempre BasePath para assets: relativo A TU BINARIO, no al CWD)\n";
    SDL_Quit();
    return 0;
}

/* -----------------------------------------------------------------------------
   3. RESUMEN DE BUENAS PRÁCTICAS (checklist del curso)
     [ ] assets de solo lectura: BasePath (o pak embebido) -> nunca CWD
     [ ] saves/configs: PrefPath (la SO sabe bien dónde van tus datos)
     [ ] siempre path completo con std::filesystem (del módulo 12): SDL te
         da la CADENA del directorio base, tú construyes el árbol con path.
     [ ] en Linux, case-sensitive: Fondo.png != fondo.png
     [ ] Carpetas con espacios/barras: usa fs::path/ "" y NUNCA concatena
         con + string (los separadores de Windows romperían en Linux).
----------------------------------------------------------------------------- */

/* =============================================================================
   CÓMO PROBARLA A MANO: solo texto, sin ventanas:
     ./leccion09
   Imprime la BasePath real de TU binario y la PrefPath de org/app, crea la
   carpeta de guardado, y muestra las 3 reglas del resolver (relativa OK,
   absoluta rechazada, ".." rechazado). Es la misma lógica que el ej 06
   testea con inyección de basePath fake (sin SDL en los tests).
   ============================================================================= */
int main() { return demoFilesystem(); }
