/* =============================================================================
   MÓDULO 16 - LECCIÓN 02: Game Loop básico — estructura y while (!WindowShouldClose)
   =============================================================================
   TEMA 2 de TEMAS.md (1ra lección): Bucle principal básico.

   ESTRUCTURA DEL GAME LOOP:
   -------------------------
   El game loop es el corazón de todo juego. En raylib:

   while (!WindowShouldClose()) {
       // 1. INPUT: leer estado de teclado, ratón, gamepad
       // 2. UPDATE: simular física, IA, lógica de juego (usando delta time)
       // 3. DRAW:   emitir comandos de render (BeginDrawing...EndDrawing)
   }

   PUNTOS CLAVE:
   - WindowShouldClose() retorna true si: usuario cierra ventana, pulsa ESC,
     o se llama a SetExitKey(KEY_NULL) y se maneja manualmente.
   - El loop corre LO MÁS RÁPIDO POSIBLE a menos que limites FPS.
   - SetTargetFPS(60) inserta un sleep para ~16.67ms por frame.
   - GetFrameTime() devuelve el tiempo REAL transcurrido (delta time).

   POR QUÉ SEPARAR UPDATE Y DRAW:
   - UPDATE: lógica determinista, idealmente en timestep FIJO (ver L04-L05).
   - DRAW:   interpolación visual, puede correr a distinta tasa (vsync, FPS cap).
   - Separación permite: replay determinista, multiplayer lockstep, headless server.

   EJEMPLO MÍNIMO COMPLETO:
   ------------------------
   #include "raylib.h"

   struct Player { Vector2 pos; float speed; };

   int main() {
       InitWindow(800, 450, "Game Loop Básico");
       SetTargetFPS(60);

       Player player{{400, 225}, 300.0f};

       while (!WindowShouldClose()) {
           // INPUT + UPDATE (juntos en timestep variable simple)
           float dt = GetFrameTime();
           if (IsKeyDown(KEY_RIGHT)) player.pos.x += player.speed * dt;
           if (IsKeyDown(KEY_LEFT))  player.pos.x -= player.speed * dt;
           if (IsKeyDown(KEY_UP))    player.pos.y -= player.speed * dt;
           if (IsKeyDown(KEY_DOWN))  player.pos.y += player.speed * dt;

           // DRAW
           BeginDrawing();
               ClearBackground(RAYWHITE);
               DrawCircleV(player.pos, 20, RED);
               DrawText(TextFormat("FPS: %d | dt: %.4f", GetFPS(), dt), 10, 10, 20, DARKGRAY);
           EndDrawing();
       }

       CloseWindow();
       return 0;
   }

   PROBLEMA DEL TIMESTEP VARIABLE:
   - dt cambia cada frame (16ms, 17ms, 15ms...).
   - Física inestable: saltos distintos según FPS, túnel de colisiones a alta velocidad.
   - No determinista: misma secuencia de inputs da resultados distintos.
   - Solución: TIMESTEP FIJO (lección 04-05).

   COMPILACIÓN:
   g++ -std=c++20 -Wall -Wextra -fsyntax-only leccion_02_game_loop_basico.cpp
   (solo sintaxis; ejecución requiere raylib + CMake FetchContent)

   ============================================================================= */

#include <iostream>

int main() {
    std::cout << "=== LECCIÓN 02: Game Loop básico ===\n";
    std::cout << "Estructura: while (!WindowShouldClose) { Input -> Update -> Draw }\n";
    std::cout << "Problema: timestep variable -> física inestable, no determinista.\n";
    std::cout << "Solución en lecciones 04-05: timestep fijo con acumulador.\n";
    std::cout << "=== FIN LECCIÓN 02 ===\n";
    return 0;
}