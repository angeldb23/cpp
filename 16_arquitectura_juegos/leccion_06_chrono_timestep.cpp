/* =============================================================================
   MÓDULO 16 - LECCIÓN 06: Paso de Tiempo — std::chrono, duration<float>, Clamp
   =============================================================================
   TEMA 4 de TEMAS.md: std::chrono::steady_clock, duration<float>, clamp delta.

   POR QUÉ std::chrono (y no GetFrameTime de raylib):
   ---------------------------------------------------
   - GetFrameTime() es conveniente pero opaco; std::chrono da control total.
   - Portabilidad: funciona sin raylib (tests headless, servidor, otras libs).
   - Precisión: steady_clock es monótono (no salta por NTP/ajuste reloj).
   - Composición: duration_cast, aritmética type-safe, literales (C++14).

   STEADY_CLOCK VS SYSTEM_CLOCK:
   -----------------------------
   - steady_clock: monótono, nunca decrece, ideal para medir intervalos.
   - system_clock: reloj de pared, puede saltar (NTP, usuario cambia hora).
   - HIGH_RESOLUTION_CLOCK: alias de steady_clock (Linux) o system_clock (Windows).
     -> USA steady_clock EXPLÍCITAMENTE para game loop.

   TIPOS BÁSICOS:
   --------------
   using Clock = std::chrono::steady_clock;
   using TimePoint = Clock::time_point;
   using Seconds = std::chrono::duration<float>;  // float seconds
   using Milliseconds = std::chrono::duration<float, std::milli>;

   TimePoint now = Clock::now();
   TimePoint later = now + Seconds(1.0f);  // 1 segundo después

   Seconds dt = later - now;  // duration<float> = 1.0f
   float dt_seconds = dt.count();  // Extraer float crudo

   LITERALES (C++14, namespace std::chrono_literals):
   --------------------------------------------------
   using namespace std::chrono_literals;
   auto dt = 16.666ms;        // std::chrono::milliseconds
   auto dt_f = 0.016666s;     // std::chrono::duration<float>
   auto hour = 1h;            // std::chrono::hours

   CONVERSIÓN SEGURA (duration_cast):
   ----------------------------------
   auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dt_f);
   // ms.count() = 16 (trunca)
   // Para redondear: std::chrono::round<ms>(dt_f)

   GAME LOOP MANUAL CON std::chrono:
   ---------------------------------
   using Clock = std::chrono::steady_clock;
   using Seconds = std::chrono::duration<float>;

   const Seconds FIXED_DT{1.0f / 60.0f};
   const Seconds MAX_FRAME{0.25f};  // Clamp 4 FPS mínimo

   TimePoint prev = Clock::now();
   Seconds accumulator{0.0f};

   while (!WindowShouldClose()) {
       TimePoint now = Clock::now();
       Seconds frameTime = now - prev;
       prev = now;

       // Clamp delta grande (spike de debug, swap, etc.)
       if (frameTime > MAX_FRAME) frameTime = MAX_FRAME;

       accumulator += frameTime;

       while (accumulator >= FIXED_DT) {
           Update(FIXED_DT.count());  // Pasa float a Update
           accumulator -= FIXED_DT;
       }

       float alpha = (accumulator / FIXED_DT).count();  // [0, 1)
       Render(alpha);
   }

   POR QUÉ duration<float> (NO double):
   ------------------------------------
   - GPU/raylib usa float (Vector2, Matrix, GetFrameTime).
   - Consistencia evita casts y warnings de narrowing.
   - Precisión suficiente: float tiene ~7 dígitos; 16ms = 0.016 (3 dígitos).
   - double solo necesario para acumular tiempo muy largo (horas/días).

   CLAMP DEFENSIVO:
   ----------------
   - frameTime > 0.25s (4 FPS) = spike anómalo (debugger, alt-tab, GC).
   - Clamp evita "spiral of death" sin need de MAX_STEPS en muchos casos.
   - MAX_STEPS (L05) es más robusto para servidores/juegos competitivos.

   EJEMPLO: Medir tiempo de una función:
   -------------------------------------
   auto start = Clock::now();
   DoWork();
   auto elapsed = Clock::now() - start;
   std::cout << "DoWork tomó " << elapsed.count() * 1000 << " ms\n";

   ============================================================================= */

#include <iostream>
#include <chrono>
#include <thread>

using Clock = std::chrono::steady_clock;
using Seconds = std::chrono::duration<float>;
using Milliseconds = std::chrono::duration<float, std::milli>;

int main() {
    std::cout << "=== LECCIÓN 06: std::chrono para Game Loop ===\n";

    // Demostración de tipos y aritmética
    auto start = Clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto end = Clock::now();

    Seconds dt_sec = end - start;
    Milliseconds dt_ms = end - start;

    std::cout << "Tiempo transcurrido: " << dt_sec.count() << " s\n";
    std::cout << "Tiempo transcurrido: " << dt_ms.count() << " ms\n";

    // Literales C++14
    using namespace std::chrono_literals;
    Seconds fixed_dt = 1.0f / 60.0f * 1s;  // 16.666ms
    std::cout << "Fixed DT (60 Hz): " << fixed_dt.count() << " s\n";

    // Clamp ejemplo
    Seconds frame_time = 500ms;  // Spike grande
    const Seconds MAX_FRAME = 250ms;
    if (frame_time > MAX_FRAME) frame_time = MAX_FRAME;
    std::cout << "Frame time tras clamp: " << frame_time.count() << " s\n";

    // Acumulador con duration<float>
    Seconds accumulator{0.0f};
    accumulator += frame_time;
    int steps = 0;
    while (accumulator >= fixed_dt && steps < 5) {
        accumulator -= fixed_dt;
        steps++;
    }
    std::cout << "Pasos de física: " << steps
              << ", accumulator restante: " << accumulator.count() << " s\n";

    float alpha = (accumulator / fixed_dt);  // ratio de durations = float [0, 1)
    std::cout << "Alpha interpolación: " << alpha << "\n";

    std::cout << "=== FIN LECCIÓN 06 ===\n";
    return 0;
}