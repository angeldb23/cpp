/* =============================================================================
   MÓDULO 16 - LECCIÓN 05: Timestep Fijo — Acumulador, Interpolación (Alpha) y Determinismo
   =============================================================================
   TEMA 3 de TEMAS.md (2da lección): Timestep fijo con acumulador e interpolación.

   ALGORITMO CLÁSICO (Gaffer on Games "Fix Your Timestep"):
   -------------------------------------------------------
   const float FIXED_DT = 1.0f / 60.0f;  // 60 Hz física (16.666ms)
   float accumulator = 0.0f;
   State currentState, previousState;

   while (!WindowShouldClose()) {
       float frameTime = GetFrameTime();
       frameTime = fminf(frameTime, 0.25f);  // Clamp defensivo (spikes)

       accumulator += frameTime;

       // Paso de física: puede correr 0, 1, 2+ veces por frame
       while (accumulator >= FIXED_DT) {
           previousState = currentState;  // Guardar estado anterior para interp.
           Update(currentState, FIXED_DT);  // Lógica con dt FIJO
           accumulator -= FIXED_DT;
       }

       // Interpolación para render suave
       float alpha = accumulator / FIXED_DT;  // [0, 1)
       State renderState = Interpolate(previousState, currentState, alpha);
       Render(renderState);
   }

   POR QUÉ FUNCIONA:
   -----------------
   - Update SIEMPRE recibe el mismo dt (1/60). Física determinista.
   - Acumulador absorbe variabilidad de frameTime.
   - Alpha interpola visualmente entre ticks de física.
   - Determinismo: misma secuencia de inputs -> misma simulación exacta.

   INTERPOLACIÓN (LERP):
   ---------------------
   Vector2 Lerp(Vector2 a, Vector2 b, float t) {
       return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t };
   }

   // Para structs complejos:
   State Interpolate(const State& prev, const State& curr, float alpha) {
       State result;
       result.playerPos = Lerp(prev.playerPos, curr.playerPos, alpha);
       result.enemyPos  = Lerp(prev.enemyPos,  curr.enemyPos,  alpha);
       // Copiar campos que no se interpolan (timers, scores, etc.)
       result.timer = curr.timer;
       return result;
   }

   MANEJO DE ESTADO ANTERIOR:
   --------------------------
   Opción A: Copia completa (State prev = curr) - simple, costo memoria.
   Opción B: Double buffer (State[2], flip índice) - sin copias, más cache-friendly.
   Opción C: Solo interpolar posición/rotación, resto se toma de current.

   CLAMP DEL ACUMULADOR (Spiral of Death):
   ---------------------------------------
   Si Update tarda > FIXED_DT, accumulator crece indefinido -> más Updates -> más lento.
   Solución: limitar pasos máximos por frame.

   const int MAX_STEPS = 5;  // 5 * 16ms = 80ms de simulación por frame real
   int steps = 0;
   while (accumulator >= FIXED_DT && steps < MAX_STEPS) {
       Update(currentState, FIXED_DT);
       accumulator -= FIXED_DT;
       steps++;
   }
   if (steps == MAX_STEPS) accumulator = 0;  // Descarta tiempo perdido (slow motion)

   DETERMINISMO EN PRÁCTICA:
   -------------------------
   - Mismos inputs + mismo estado inicial = misma simulación exacta.
   - Permite: replay, rollback netcode, testing automatizado.
   - Requiere: RNG determinista (std::mt19937 con seed fijo), sin datos de tiempo real.

   EJEMPLO COMPLETO (pseudocódigo integrable):
   -------------------------------------------
   struct GameState {
       Vector2 playerPos, playerVel;
       Vector2 prevPlayerPos;  // Para interpolación
   };

   void Update(GameState& s, float dt) {
       s.prevPlayerPos = s.playerPos;
       s.playerPos.x += s.playerVel.x * dt;
       s.playerPos.y += s.playerVel.y * dt;
   }

   Vector2 GetRenderPos(const GameState& s, float alpha) {
       return Lerp(s.prevPlayerPos, s.playerPos, alpha);
   }

   ============================================================================= */

#include <iostream>
#include <algorithm>

struct Vec2 { float x, y; };

Vec2 Lerp(Vec2 a, Vec2 b, float t) {
    return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t };
}

struct GameState {
    Vec2 playerPos{400, 225};
    Vec2 playerVel{0, 0};
    Vec2 prevPlayerPos{400, 225};
};

void Update(GameState& s, float dt) {
    s.prevPlayerPos = s.playerPos;
    s.playerPos.x += s.playerVel.x * dt;
    s.playerPos.y += s.playerVel.y * dt;
}

int main() {
    std::cout << "=== LECCIÓN 05: Timestep Fijo con Acumulador ===\n";

    const float FIXED_DT = 1.0f / 60.0f;
    float accumulator = 0.0f;
    GameState state;
    state.playerVel = {100, 50};

    // Simular 3 frames con frameTimes variables
    float frameTimes[3] = { 0.016f, 0.017f, 0.033f };  // 60, 58, 30 FPS

    for (int frame = 0; frame < 3; ++frame) {
        float frameTime = std::min(frameTimes[frame], 0.25f);
        accumulator += frameTime;

        std::cout << "Frame " << frame << ": frameTime=" << frameTime
                  << ", accumulator=" << accumulator << "\n";

        while (accumulator >= FIXED_DT) {
            Update(state, FIXED_DT);
            accumulator -= FIXED_DT;
            std::cout << "  -> Physics tick! pos=(" << state.playerPos.x
                      << ", " << state.playerPos.y << ")\n";
        }

        float alpha = accumulator / FIXED_DT;
        Vec2 renderPos = Lerp(state.prevPlayerPos, state.playerPos, alpha);
        std::cout << "  Render pos (alpha=" << alpha << "): (" << renderPos.x
                  << ", " << renderPos.y << ")\n";
    }

    std::cout << "\nFísica determinista: misma secuencia -> mismos resultados.\n";
    std::cout << "=== FIN LECCIÓN 05 ===\n";
    return 0;
}