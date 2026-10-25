/* =============================================================================
   MÓDULO 19 - LECCIÓN 16: Aleatoriedad determinista y noise (value noise)
   =============================================================================
   TEMA 9 de TEMAS.md (1 lección): std::mt19937, distribuciones, noise
   (value/Perlin) y semillas por frame.

   == DETERMINISMO: LA REGLA DE ORO DEL CURSO ================================

   La "misma semilla, misma secuencia" es lo que permite:
     - DESREPRODUCTAR UN BUG exacto (el dueño corre y ve lo mismo).
     - Procedural REPRODUCIBLE: el mapa Nº 712 es SIEMPRE el mapa 712 (solo
       envías la semilla, no los datos).
     - NET SYNC determinista para replays (el módulo 23 retoma esto).

   std::mt19937: generador rápido, 32 bits, período 2^19937-1. Por sí solo
   produce uint32 uniformes. El ALGORITMO está especificado por el
   estándar C++, así que la misma semilla da la misma secuencia en GCC,
   Clang y MSVC por igual.

   Cuidado: std::uniform_real_distribution NO está detallada por el
   estándar (MSVC y libstdc++ dan secuencias distintas). Regla del curso:
       - para determinismo entre plataformas: usar mt19937 + derivas a
         mano (no std::distributions).
       - float [0..1) por la formula ux = (uint32 >> 8) * (1.0f / 2^24):
         determinista UNIVERSAL (el mapa de bits del Uint32 es estándar).

   == VALUE NOISE 2D (El padre de Perlin) ====================================

   La idea:
       1) cada esquina entera (i, j) se PASA a un hash → un float [0..1):
              hashInt(i, j, semilla)  (con un hash de enteros A MANO)
       2) interpolar bilineal entre las 4 esquinas del celular:
              f00 = hash(i, j); f10 = hash(i+1, j);
              f01 = hash(i, j+1);  f11 = hash(i+1, j+1)
              top = lerp(f00, f10, fade(dx)); bottom = lerp(f01, f11, fade(dx))
              noise = lerp(top, bottom, fade(dy))
       3) fade(t) = 6t⁵ - 15t⁴ + 10t³  (LA CURVA QUINTICA de la L14: suave C²
          en las esquinas: sin "cuasi." en las res líneas del grid).

   Es determinista, barato y continuo. En 3D son 8 esquinas bilinear con
   trilerp (los módulos 22, 26 lo usan para variedad escénica). Perlin añade
   gradiente a cada esquina (Gradient noise: se lo ve en fijos posteriores;
   Para el curso basta el value noise).

   == SEMILLAS: ÚNICA, POR OBJETO, POR FRAME =================================

   - SEMILLA ÚNICA (run): reproducible toda la partida (replay).
   - SEMILLA POR ENTIDAD: spawnSeed = hash(semillaLv, idEntidad) y cada
     entidad "tira sus dados" sin pelear el GENERADOR GLOBAL (no intercalas).
   - SEMILLA POR FRAME: NO hay que volver SEMILLAR mt19937 cada frame (si
     no se REPARTE el estado: lento MAL). La práctica: un rng por MAGNITUD
     con su propia semilla derivada (hash de parentId) o avanzar estado y
     guardar.

   == DEMO ====================================================================

   La demo imprime:
     - la misma semilla dos veces da EXACTAMENTE los mismos números;
     - semilla distinta da otra secuencia;
     - tabla de noise 2D en una cuadrícula (chequeo visual con la mirada).
   ============================================================================= */

#include "include/mate3d.hpp"
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <random>
#include <vector>
#include <iostream>

using namespace mate;

// ==== RNG (u32 y float determinista cross-plataforma) ======================
std::mt19937& rngDe(uint32_t semilla) {
    // cada semilla es un GENERADOR PROPIO (sin pelear el global)
    static thread_local std::mt19937 actual;   // (demo: semilla única simple)
    actual.seed(semilla);
    return actual;
}

uint32_t siguienteU32(std::mt19937& rng) { return rng(); }   // [0, 2^32)

float u32AFloat(uint32_t u) {
    // [0, 1) con 24 bits de mantisa: determinista en cualquier plataforma
    return (float)(u >> 8) * (1.0f / 16777216.0f);   // 2^24
}

// ==== HASH DE ENTEROS (para el noise) ======================================
// Wang-ish: una pasada de xorshift/corrimientos baratos y decentes.
uint32_t hashEnteros(int x, int y, uint32_t semilla) {
    uint32_t h = (uint32_t)x * 374761393u + (uint32_t)y * 668265263u + semilla;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

// ==== VALUE NOISE 2D =======================================================
float valueNoise2D(float x, float y, uint32_t semilla) {
    // esquinas enteras del "celular" que contiene (x, y)
    int xi = (int)std::floor(x), yi = (int)std::floor(y);
    float dx = x - (float)xi, dy = y - (float)yi;
    float u = 6.0f*dx*dx*dx*dx*dx - 15.0f*dx*dx*dx*dx + 10.0f*dx*dx*dx;
    float v = 6.0f*dy*dy*dy*dy*dy - 15.0f*dy*dy*dy*dy + 10.0f*dy*dy*dy;
    auto hash = [&](int fx, int fy) -> float {
        return (float)(hashEnteros(fx, fy, semilla) >> 8) / 16777216.0f;
    };
    float f00 = hash(xi,     yi);     float f10 = hash(xi + 1, yi);
    float f01 = hash(xi, yi + 1);     float f11 = hash(xi + 1, yi + 1);
    float top  = f00 + (f10 - f00) * u;      // lerp horizontal (y fija)
    float bottom = f01 + (f11 - f01) * u;
    return top + (bottom - top) * v;          // lerp vertical (vertical)
}

// ==== DEMO ================================================================
int main() {
    std::cout << "== DETERMINISMO: misma semilla, misma secuencia ==\n";
    std::mt19937 r1 = rngDe(777u); std::mt19937 r2 = rngDe(777u);
    std::printf("semilla 777:  r1: %u %u %u\n", siguienteU32(r1), siguienteU32(r1), siguienteU32(r1));
    std::printf("semilla 777:  r2: %u %u %u  (identica)\n", siguienteU32(r2), siguienteU32(r2), siguienteU32(r2));

    std::mt19937 r3 = rngDe(2048u);
    std::printf("semilla 2048: r3: %u %u %u  (otra secuencia)\n",
                siguienteU32(r3), siguienteU32(r3), siguienteU32(r3));

    std::cout << "\n== FLOAT [0..1) determinista ==\n";
    std::mt19937 r4 = rngDe(42u);
    for (int i = 0; i < 4; ++i)
        std::printf("%.6f  ", u32AFloat(siguienteU32(r4)));
    std::cout << "(la forma u32 >> 8 * 1/2^24 NO depende del compilador)\n";

    std::cout << "\n== VALUE NOISE 2D (tabla 8x8, semilla 7) ==\n";
    for (int gy = 0; gy < 8; ++gy) {
        for (int gx = 0; gx < 8; ++gx) {
            float n = valueNoise2D((float)gx, (float)gy, 7u);
            std::putchar(" .:-=+*#%@"[std::min(9, (int)(n * 10.0f))]);
        }
        std::putchar('\n');
    }
    std::cout << " (foto 8x8: manchas suaves que varían; esquinas = nº hash)\n";

    std::cout << "== SEMILLAS DERIVADAS (por entidad sin pelear el estado global) ==\n";
    std::printf("spawnSeed(ent 1)         = %u\n", hashEnteros(1, 0, 2024u));
    std::printf("spawnSeed(ent 2)         = %u\n", hashEnteros(2, 0, 2024u));
    std::printf("spawnSeed(ent 1, nivel B)= %u\n", hashEnteros(1, 1, 2024u));

    std::cout << "=== FIN LECCIÓN 16 ===\n";
    return 0;
}
