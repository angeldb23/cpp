/* =============================================================================
   MÓDULO 18 - EJERCICIO 09: Auditor de migración SDL2 -> SDL3 (string-based)
   =============================================================================
   DIFICULTAD: Básica-Media
   APOYO: Lección 01 (SDL3 vs SDL2: cambios reales) — mismo estilo string-
          based que el ejercicio 08 de los módulos 15 y 16.

   CONSIGNA:
   Migrar una base de código de SDL2 a SDL3 es un buscar-reemplazar
   PELIGROSO: no todo es renombrado 1:1 (cambia el tipo de retorno, hay
   APIs removidas...). Completá el AUDITOR que decide si un "cambio" es
   una migración válida, y TU TABLA con cambios reales de la lección 01:

   MODELO (no tocar):
     struct CambioVerificado {
         bool firmaIntABool;   // la firma pasó de int (-1 = error) a bool
         bool rename;          // la función/flag cambió de nombre
         std::string antes;    // nombre en SDL2 (o estado previo)
         std::string despues;  // nombre en SDL3 (o estado nuevo)
         std::string tipo;     // "renombre" | "bool" | otra cosa
     };

   REGLAS DE VALIDACIÓN (esMigracionValida -> true SOLO si TODAS):
     1. el tipo es "renombre" o "bool" (otra cosa: rechazo).
     2. antes y despues NO vacíos y AMBOS empiezan con "SDL_" (es API
        de SDL, no texto suelto).
     3. si tipo == "renombre": rename verdad y el nombre CAMBIA
        (antes != despues).
     4. si tipo == "bool": firmaIntABool verdad (el nombre puede ser
        igual: la misma función con firma nueva).

   TU PARTE (lógica + TU tabla):
     1. esMigracionValida (reglas de arriba, lógica pura).
     2. std::vector<CambioVerificado> tablaMigracion(): tu tabla con
        AL MENOS 4 cambios reales de la L01, cumpliendo:
          - >= 2 de tipo "renombre"
          - >= 1 de tipo "bool"
          - entre los "renombre", al menos 1 de FLAG (constante
            renombrada: SDL_WINDOW_ALLOW_HIGHDPI ->
            SDL_WINDOW_HIGH_PIXEL_DENSITY, mismos bits 0x2000)
        Los NOMBRES correctos están en la lección 01 (verificados
        contra las cabeceras SDL2 2.32.74 y SDL3 3.4.18): no inventes.
     3. cuentaTipos(tabla, tipo): cuántos hay de ese tipo.

   TESTS (no tocar). COMPILACIÓN:
     g++ -std=c++20 -Wall -Wextra ejercicio_09_auditor_migracion.cpp -o ej && ./ej
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

// ============================ TU CÓDIGO AQUÍ ================================
// TODO 1: la validación (las 4 reglas del enunciado)
static bool esMigracionValida(const CambioVerificado& c) {
    (void)c;
    return false;
}

// TODO 2: TU tabla con >= 4 cambios reales de la L01 (verificados)
static std::vector<CambioVerificado> tablaMigracion() {
    return {};
}

// TODO 3: conteo por tipo
static int cuentaTipos(const std::vector<CambioVerificado>& tabla, const std::string& tipo) {
    (void)tabla; (void)tipo;
    return 0;
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
