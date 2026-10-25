/* =============================================================================
   MÓDULO 16 - LECCIÓN 04: Timestep Variable — GetFrameTime, problemas y clamp
   =============================================================================
   TEMA 3 de TEMAS.md (1ra lección): Timestep fijo vs variable - lado variable.

   TIMESTEP VARIABLE (naïve):
   --------------------------
   float dt = GetFrameTime();  // Tiempo real del último frame
   Update(dt);                 // Usa dt directamente

   PROBLEMAS:
   1. INESTABILIDAD FÍSICA: Euler explota con dt grande; colisiones fallan (tunneling).
   2. NO DETERMINISTA: Mismos inputs -> resultados distintos según carga del sistema.
   3. SPIKES: Si el frame tarda 200ms (GC, swap, debug), dt=0.2 -> personaje vuela.
   4. REPLAY/NETWORK: Imposible reproducir exactamente sin grabar todos los dt.

   GETFRAMETIME EN RAYLIB:
   -----------------------
   - Devuelve float en segundos (ej: 0.016666 para 60 FPS).
   - Internamente usa std::chrono::high_resolution_clock (ver L06).
   - SetTargetFPS(60) intenta limitarlo, pero NO garantiza dt fijo.

   CLAMP DE DELTA GRANDE (mitigación básica):
   ------------------------------------------
   float dt = GetFrameTime();
   const float MAX_DT = 1.0f / 30.0f;  // ~33ms = 30 FPS mínimo
   if (dt > MAX_DT) dt = MAX_DT;

   // O más agresivo para física:
   const float MAX_DT_PHYSICS = 1.0f / 60.0f;  // Nunca más de 1 tick de física
   if (dt > MAX_DT_PHYSICS) dt = MAX_DT_PHYSICS;

   POR QUÉ NO BASTA EL CLAMP:
   --------------------------
   - Clamp evita explosiones, pero sigue siendo VARIABLE (16ms, 17ms, 16ms...).
   - Física sigue siendo no determinista frame a frame.
   - Solución real: TIMESTEP FIJO con acumulador (L05).

   EJEMPLO CON CLAMP:
   ------------------
   while (!WindowShouldClose()) {
       float dt = GetFrameTime();
       if (dt > 0.25f) dt = 0.25f;  // Clamp defensivo (4 FPS mínimo)

       Update(dt);  // Aún variable, pero acotado
       Render();
   }

   MEJOR PRÁCTICA: Usar timestep fijo (L05). El clamp es SOLO red de seguridad.

   ============================================================================= */

#include <iostream>
#include <algorithm>

int main() {
    std::cout << "=== LECCIÓN 04: Timestep Variable ===\n";
    std::cout << "GetFrameTime() -> dt variable (16ms, 17ms, 15ms...).\n";
    std::cout << "Problemas: inestabilidad física, no determinista, spikes.\n";
    std::cout << "Mitigación: clamp dt (ej. max 0.25s), pero NO resuelve determinismo.\n";
    std::cout << "Solución real: timestep fijo con acumulador (L05).\n";
    std::cout << "=== FIN LECCIÓN 04 ===\n";
    return 0;
}