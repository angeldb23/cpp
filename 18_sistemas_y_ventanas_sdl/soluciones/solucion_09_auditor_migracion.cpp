/* =============================================================================
   MÓDULO 18 - SOLUCIÓN 09: Auditor de migración SDL2 -> SDL3 (string-based)
   =============================================================================
   MISMO ENUNCIADO que ejercicios/ejercicio_09_auditor_migracion.cpp.
   SOLUCIÓN COMENTADA: qué hace cada pieza y por qué.

   PUNTOS CLAVE:
   - La validación es tabla-de-reglas, no if/else en cascada: cada regla
     DESCARTA (early return) y la última positiva es el return en sí.
   - La TABLA usa los nombres VERIFICADOS en la lección 01 (cabeceras
     SDL2 2.32.74 y SDL3 3.4.18): nada inventado. En particular:
       "renombre": SDL_RenderCopy -> SDL_RenderTexture
                   SDL_RenderCopyEx -> SDL_RenderTextureRotated
                   SDL_WINDOW_ALLOW_HIGHDPI -> SDL_WINDOW_HIGH_PIXEL_DENSITY (flag)
                   SDL_CreateCond/SDL_CondWait -> SDL_CreateCondition/SDL_WaitConditionTimeout
       "bool":     SDL_RenderSetVSync: misma función, la firma pasa de
                   int (-1 = error) a bool (false = error).
   - Dato fino: para el "bool" de SDL_RenderSetVSync, rename es FALSE
     (misma función en ambas versiones; solo cambió el tipo de retorno)
     — el auditor lo valida por el campo tipo, no por el rename.
   ============================================================================= */
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

// ============================ MODELO (no tocar) ==============================
struct CambioVerificado {
    bool firmaIntABool = false;     // firma pasó de int (-1 error) a bool
    bool rename = false;            // la función/flag cambió de nombre
    std::string antes;              // nombre SDL2
    std::string despues;            // nombre SDL3
    std::string tipo;               // "renombre" | "bool" | otra
};

// ============================ SOLUCIÓN ======================================
// reglas del enunciado, una por línea de descarte
static bool empiezaConSDL(const std::string& s) {
    return s.rfind("SDL_", 0) == 0;    // "empieza con" sin copiar strings
}

static bool esMigracionValida(const CambioVerificado& c) {
    // 1. el tipo es "renombre" o "bool" (otra cosa: rechazo)
    const bool esRenombre = (c.tipo == "renombre");
    const bool esBool     = (c.tipo == "bool");
    if (!esRenombre && !esBool) return false;
    // 2. antes y despues NO vacíos y AMBOS empiezan con "SDL_"
    if (c.antes.empty() || c.despues.empty()) return false;
    if (!empiezaConSDL(c.antes) || !empiezaConSDL(c.despues)) return false;
    // 3. si tipo == "renombre": rename y el nombre CAMBIA
    if (esRenombre) {
        if (!c.rename) return false;
        if (c.antes == c.despues) return false;
    }
    // 4. si tipo == "bool": la firma cambió int -> bool
    if (esBool && !c.firmaIntABool) return false;
    return true;
}

// tabla con CAMBIOS REALES verificados en las cabeceras de L01
static std::vector<CambioVerificado> tablaMigracion() {
    return {
        // funciones renombradas:
        {true,  true,  "SDL_RenderCopy",        "SDL_RenderTexture",            "renombre"},
        {true,  true,  "SDL_RenderCopyEx",      "SDL_RenderTextureRotated",     "renombre"},
        // flag renombrado (mismos bits 0x2000, Uint32 -> Uint64):
        {false, true,  "SDL_WINDOW_ALLOW_HIGHDPI",   "SDL_WINDOW_HIGH_PIXEL_DENSITY", "renombre"},
        // condiciones renombradas + mudaron de SDL_thread.h a SDL_mutex.h:
        {false, true,  "SDL_CreateCond",        "SDL_CreateCondition",          "renombre"},
        // misma función, firma de int a bool:
        {true,  false, "SDL_RenderSetVSync",    "SDL_RenderSetVSync",           "bool"},
        // init cambia de firma (int -> bool), nombre igual:
        {true,  false, "SDL_Init",              "SDL_Init",                     "bool"},
    };
}

// conteo por tipo (straight iterate)
static int cuentaTipos(const std::vector<CambioVerificado>& tabla, const std::string& tipo) {
    int n = 0;
    for (const auto& c : tabla) if (c.tipo == tipo) ++n;
    return n;
}

// ============================ TESTS (no tocar) ==============================
TEST_CASE(ValidaConReglasEstrictas) {
    ASSERT_FALSE(esMigracionValida(CambioVerificado{}));  // todo vacío

    CambioVerificado renombre{true, true, "SDL_RenderCopyEx", "SDL_RenderTextureRotated", "renombre"};
    ASSERT_TRUE(esMigracionValida(renombre));

    CambioVerificado tipoMalo{true, true, "SDL_RenderCopy", "SDL_RenderTexture", "otra"};
    ASSERT_FALSE(esMigracionValida(tipoMalo));          // tipo no permitido

    CambioVerificado vacio{true, true, "", "SDL_x", "renombre"};
    ASSERT_FALSE(esMigracionValida(vacio));             // antes vacío

    CambioVerificado noSDL{true, false, "SDL2_Sudo", "Otras SDLs", "renombre"};
    ASSERT_FALSE(esMigracionValida(noSDL));             // despues no es SDL_

    CambioVerificado sinNombrar{true, false, "SDL_RenderSetVSync", "SDL_RenderSetVSync", "bool"};
    ASSERT_TRUE(esMigracionValida(sinNombrar));         // bool: mismo nombre OK
}

TEST_CASE(TablaCumpleLosPisosDelEnunciado) {
    auto tabla = tablaMigracion();
    ASSERT_TRUE(tabla.size() >= 4);
    ASSERT_TRUE(cuentaTipos(tabla, "renombre") >= 2);
    ASSERT_TRUE(cuentaTipos(tabla, "bool") >= 1);
    for (const auto& c : tabla) ASSERT_TRUE(esMigracionValida(c));
}

TEST_CASE(TodasLasEntradasSonAPIVerificados) {
    // las entradas de la tabla deben ser SDL_... en ambos lados
    auto tabla = tablaMigracion();
    ASSERT_TRUE(tabla.size() >= 4);
    for (const auto& c : tabla) {
        ASSERT_TRUE(c.antes.find("SDL_") != std::string::npos);
        ASSERT_TRUE(c.despues.find("SDL_") != std::string::npos);
    }
}

TEST_CASE(ConteoTiposVacioYUnitario) {
    std::vector<CambioVerificado> tabla;
    ASSERT_EQ(cuentaTipos(tabla, "renombre"), 0);
    tabla.push_back(CambioVerificado{true, true, "SDL_a", "SDL_b", "renombre"});
    ASSERT_EQ(cuentaTipos(tabla, "renombre"), 1);
    ASSERT_EQ(cuentaTipos(tabla, "bool"), 0);
}

int main() { return RUN_ALL_TESTS(); }
