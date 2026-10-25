/* =============================================================================
   MÓDULO 16 - LECCIÓN 03: Separación Update / Render — arquitectura limpia
   =============================================================================
   TEMA 2 de TEMAS.md (2da lección): Separación de actualización y render.

   POR QUÉ SEPARAR:
   ----------------
   1. DETERMINISMO: Update con timestep fijo = misma simulación siempre.
   2. FLEXIBILIDAD: Render puede correr a 144Hz, 60Hz, 30Hz o headless.
   3. INTERPOLACIÓN: Suaviza render entre ticks de física (alpha blending).
   4. TESTING: Update se puede testear sin GPU/ventana (lógica pura).
   5. PARALELISMO: Update y Render en hilos distintos (avanzado).

   ARQUITECTURA RECOMENDADA:
   -------------------------
   // Estado del juego (datos puros, sin dependencias de raylib)
   struct GameState {
       Vector2 playerPos;
       Vector2 playerVel;
       float   timer;
       // ... entidades, partículas, etc.
   };

   // Sistema de actualización (lógica pura, testable)
   void Update(GameState& state, float fixedDt) {
       // Física, IA, colisiones, timers...
       state.playerPos.x += state.playerVel.x * fixedDt;
       state.playerPos.y += state.playerVel.y * fixedDt;
       state.timer += fixedDt;
   }

   // Sistema de render (solo lectura de estado + comandos GPU)
   void Render(const GameState& state, float alpha) {
       // alpha = interpolación [0,1] entre tick anterior y actual
       Vector2 renderPos = Vector2Lerp(state.prevPos, state.playerPos, alpha);
       DrawCircleV(renderPos, 20, RED);
   }

   // Game Loop con timestep fijo + interpolación (ver L04-L05)
   float accumulator = 0.0f;
   const float fixedDt = 1.0f / 60.0f;  // 60 Hz física

   while (!WindowShouldClose()) {
       float frameDt = GetFrameTime();
       frameDt = fminf(frameDt, 0.25f);  // clamp delta grande (spike)

       accumulator += frameDt;
       while (accumulator >= fixedDt) {
           Update(state, fixedDt);  // Puede correr MÚLTIPLES veces por frame
           accumulator -= fixedDt;
       }

       float alpha = accumulator / fixedDt;  // [0, 1) para interpolación
       Render(state, alpha);                 // Corre 1 vez por frame
   }

   INTERPOLACIÓN (ALPHA):
   ----------------------
   - alpha = 0.0 -> posición del tick anterior
   - alpha = 1.0 -> posición del tick actual (teórico, nunca se alcanza)
   - Lerp lineal: renderPos = prev + (curr - prev) * alpha
   - Suaviza movimiento aunque render corra a distinta tasa que física.

   VENTAJAS PARA TESTING:
   ----------------------
   - Update(GameState&, float) es una función pura: entrada -> salida.
   - Se puede testear con Catch2/GoogleTest SIN raylib, SIN ventana.
   - Ej: test que "saltar + 60 ticks = altura X" siempre pasa igual.

   NOTA: Esta lección es conceptual. El código completo integra en L04-L05.

   ============================================================================= */

#include <iostream>

int main() {
    std::cout << "=== LECCIÓN 03: Separación Update/Render ===\n";
    std::cout << "Update: lógica pura, timestep fijo, testable sin GPU.\n";
    std::cout << "Render: solo lectura, interpolación con alpha, variable rate.\n";
    std::cout << "Game loop: acumulador + while (accumulator >= fixedDt) Update().\n";
    std::cout << "=== FIN LECCIÓN 03 ===\n";
    return 0;
}