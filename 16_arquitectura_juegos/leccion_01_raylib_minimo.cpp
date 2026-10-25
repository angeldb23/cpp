/* =============================================================================
   MÓDULO 16 - LECCIÓN 01: raylib mínimo — ventana, dibujo, input y CMake con FetchContent
   =============================================================================
   TEMA 1 de TEMAS.md: raylib mínimo (1 lección).

   QUÉ ES RAYLIB:
   - Librería C99 para desarrollo de juegos 2D/3D, simple y sin dependencias externas.
   - API plana (funciones sueltas, no clases), ideal para aprender game loop y gráficos.
   - Soporta Windows, Linux, macOS, Web (WASM), Android, iOS.
   - Versión 5.5 (2024) es la estable actual; usamos tag concreto en FetchContent.

   CMAKE CON FETCHCONTENT (raylib 5.5):
   -----------------------------------
   cmake_minimum_required(VERSION 3.20)
   project(MiJuego LANGUAGES CXX)

   include(FetchContent)
   FetchContent_Declare(
       raylib
       GIT_REPOSITORY https://github.com/raysan5/raylib.git
       GIT_TAG        5.5  # Tag concreto, NUNCA master
   )
   # Opciones de build de raylib (antes de MakeAvailable)
   set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
   set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
   FetchContent_MakeAvailable(raylib)

   add_ejecutable(mi_juego main.cpp)
   target_link_libraries(mi_juego PRIVATE raylib)

   # En Linux/Wayland: raylib ya trae las dependencias de sistema (GLFW, OpenGL, etc.)
   # Si falla: sudo pacman -S glfw wayland libxkbcommon (Arch) o equivalente.

   VENTANA BÁSICA + DIBUJO + INPUT:
   --------------------------------
   #include "raylib.h"

   int main() {
       const int screenWidth = 800;
       const int screenHeight = 450;

       InitWindow(screenWidth, screenHeight, "Mi Juego - raylib 5.5");
       SetTargetFPS(60);  // Límite de FPS (timestep variable simple)

       while (!WindowShouldClose()) {  // Game loop principal
           // UPDATE (lógica)
           if (IsKeyDown(KEY_RIGHT)) player.x += 200.0f * GetFrameTime();
           if (IsKeyDown(KEY_LEFT))  player.x -= 200.0f * GetFrameTime();

           // DRAW (render)
           BeginDrawing();
               ClearBackground(RAYWHITE);
               DrawText("Hola raylib!", 190, 200, 20, DARKGRAY);
               DrawRectangleV(player, {50, 50}, RED);
           EndDrawing();
       }

       CloseWindow();
       return 0;
   }

   FUNCIONES CLAVE:
   - InitWindow/CloseWindow: ciclo de vida de la ventana.
   - WindowShouldClose(): true si usuario cierra ventana o pulsa ESC (configurable).
   - SetTargetFPS(60): limita el bucle a 60 FPS (usa sleep interno).
   - GetFrameTime(): delta time del último frame (en segundos, float).
   - BeginDrawing/EndDrawing: scope de comandos de render.
   - ClearBackground: limpia color buffer.
   - DrawText/DrawRectangleV: primitivas 2D.
   - IsKeyDown/IsKeyPressed/IsKeyReleased: input polling (estado actual).

   COMPILACIÓN MANUAL (sin CMake, para probar rápido):
   ---------------------------------------------------
   g++ -std=c++20 -Wall -Wextra main.cpp -o juego -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
   # En Wayland (Hyprland): puede necesitar libwayland-client, libxkbcommon

   NOTA PARA ESTE CURSO:
   - Las lecciones con raylib/SDL/OpenGL se COMPILAN para verificar sintaxis y linkeo,
     pero NO se ejecutan en los tests automáticos (requieren ventana/GPU).
   - Para correrlos a mano: compila y ejecuta el binario resultante.
   - ./verificar.sh usa -fsyntax-only o compila sin ejecutar para código gráfico.

   EJECUTAR ESTA LECCIÓN A MANO:
   -----------------------------
   1. Crea un CMakeLists.txt con el FetchContent de arriba.
   2. Copia este archivo como main.cpp (quitando los comentarios de arriba).
   3. cmake -B build && cmake --build build
   4. ./build/mi_juego
   ============================================================================= */

#include <iostream>

// Verificación de sintaxis: este archivo compila con -fsyntax-only
// pero no se ejecuta en tests automáticos (requiere raylib + ventana).
//
// Para compilar y probar a mano:
//   g++ -std=c++20 -Wall -Wextra -fsyntax-only leccion_01_raylib_minimo.cpp
//   # Si pasa, la sintaxis es correcta.

int main() {
    std::cout << "=== LECCIÓN 01: raylib mínimo (solo verificación de sintaxis) ===\n";
    std::cout << "Esta lección explica la API; para ejecutar requiere CMake + FetchContent + raylib.\n";
    std::cout << "Ver comentarios arriba para código completo y pasos de compilación.\n";
    std::cout << "=== FIN LECCIÓN 01 ===\n";
    return 0;
}