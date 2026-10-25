/* =============================================================================
   MÓDULO 24 - LECCIÓN 01: Dear ImGui, contexto y ciclo de frames
   =============================================================================
   TEMA 1 de TEMAS.md: "Dear ImGui: contexto, backend SDL3+OpenGL3,
   NewFrame/Render/EndFrame y ImGuiIO".

   QUÉ ES IMMEDIATE MODE
   ---------------------
   En una UI "retained" (Qt, GTK, HTML) construís un árbol de widgets una vez
   y el framework lo conserva, mide y repinta. En immediate mode NO existe ese
   árbol persistente: cada frame volvés a escribir:

       if (ImGui::Button("Guardar")) { ... }

   La función dibuja el botón Y te dice si lo clickearon en el mismo frame.
   No hay callbacks, no hay que enlazar señales ni destruir widgets. El estado
   que sí persiste vive en TU código (una variable) o en el storage interno de
   ImGui (IDs, ventanas, layout).

   Ventaja: menos ceremonia y el estado de la app siempre es la fuente de
   verdad. Costo: se re-emite toda la UI cada frame (barato para herramientas)
   y el ID de cada widget debe ser estable dentro del frame.

   EL CONTEXTO
   -----------
   `ImGui::CreateContext()` reserva el estado global (ventanas, tabla de IDs,
   estilo, font atlas). Es UNO por hilo y por ventana de nivel superior. Un
   segundo contexto en el mismo hilo requiere `SetCurrentContext`.
   `ImGui::DestroyContext()` lo libera: llamalo SIEMPRE al final.

   CICLO DE UN FRAME (backend SDL3 + OpenGL3)
   ------------------------------------------
       SDL_PollEvent -> ImGui_ImplSDL3_ProcessEvent(&e)   // input primero
       ImGui_ImplOpenGL3_NewFrame()                        // prepara texturas
   GL ImGui_ImplSDL3_NewFrame()                           // reloj, mouse,
   teclado ImGui::NewFrame()                                   // abre el frame
           ... widgets ...
       ImGui::Render()                                     // cierra y genera
   draw data ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData())
       SDL_GL_SwapWindow()

   `ImGui::EndFrame()` existe, pero `Render()` ya lo llama. Si no vas a
   dibujar (ventana minimizada), podés saltearte el frame entero; llamar a
   NewFrame sin Render desperdicia CPU y puede desincronizar el reloj.

   QUÉ HAY EN ImGuiIO
   ------------------
   `ImGui::GetIO()` devuelve la estructura de entrada/salida: `DisplaySize`
   (en puntos lógicos), `DisplayFramebufferScale` (píxeles / punto en Hi-DPI),
   `DeltaTime`, `Framerate`, posición del mouse, estado de teclas,
   `WantCaptureMouse` y `WantCaptureKeyboard` (si la UI consumió el input, el
   juego NO debe actuar), `ConfigFlags` (docking, navegación por teclado,
   viewports) y `IniFilename`.

   En Wayland con escalado fraccional, `DisplaySize` y el tamaño en píxeles del
   framebuffer NO coinciden. El backend SDL3 ya carga ese factor; si hacés
   cálculos de picking, usá el mismo espacio que ImGui (puntos) o convertí.

   ESTA LECCIÓN
   ------------
   Con ImGui disponible abre una ventana con un contador y datos del frame.
   Sin ImGui (g++ suelto) imprime la misma información en modo headless.
   =============================================================================
 */
#include "include/gui_util.hpp"
#include "include/imgui_app.hpp"

#include <cstdio>

namespace {

struct EstadoDemo {
  int clicks = 0;
  int frames = 0;
  float tiempoAcumulado = 0.0f;
};

EstadoDemo g_estado;

#if M24_HAS_IMGUI
void dibujarUI() {
  ImGuiIO &io = ImGui::GetIO();
  g_estado.frames++;

  ImGui::SetNextWindowPos(ImVec2(24.0f, 24.0f), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSize(ImVec2(560.0f, 320.0f), ImGuiCond_FirstUseEver);
  ImGui::Begin("M24 / L01 - contexto y ciclo de frames");

  ImGui::Text("Dear ImGui %s", IMGUI_VERSION);
  ImGui::Text("Frame %.3f ms  (%.1f FPS)",
              1000.0f / (io.Framerate > 0.0f ? io.Framerate : 1.0f),
              io.Framerate);
  ImGui::Text("DisplaySize: %.0f x %.0f puntos", io.DisplaySize.x,
              io.DisplaySize.y);
  ImGui::Text("FramebufferScale: %.2f x %.2f", io.DisplayFramebufferScale.x,
              io.DisplayFramebufferScale.y);
  ImGui::Text("WantCaptureMouse=%d  WantCaptureKeyboard=%d",
              io.WantCaptureMouse ? 1 : 0, io.WantCaptureKeyboard ? 1 : 0);

  ImGui::Separator();
  if (ImGui::Button("Click me")) {
    ++g_estado.clicks;
  }
  ImGui::SameLine();
  ImGui::Text("clicks = %d (frames = %d)", g_estado.clicks, g_estado.frames);

  ImGui::Spacing();
  ImGui::TextWrapped(
      "Cada frame se vuelve a emitir toda la UI. El estado que sobrevive "
      "(clicks, frames) vive en variables de la aplicación, no en widgets.");

  ImGui::End();
}
#endif

} // namespace

int main() {
  std::puts("M24 / L01 - Ciclo de frames de Dear ImGui");
  std::puts("-----------------------------------------");

#if M24_HAS_IMGUI
  std::puts(
      "ImGui disponible: abriendo ventana 1100x720 (SDL3 + GL 3.3 core).");
  std::puts(
      "Cerrá la ventana para terminar. El juego debe ignorar input cuando");
  std::puts("WantCaptureMouse/Keyboard sea true.");
  return gui::runImGuiDemo("M24 L01 - ciclo de frames", dibujarUI);
#endif

  std::puts("ImGui no disponible en esta compilación (g++ suelto).");
  std::puts("Con CMake, FetchContent trae Dear ImGui v1.91.5-docking.");
  std::puts("");
  std::puts("Ciclo que ejecutaria el backend cada frame:");
  const char *pasos[] = {
      "SDL_PollEvent + ImGui_ImplSDL3_ProcessEvent",
      "ImGui_ImplOpenGL3_NewFrame",
      "ImGui_ImplSDL3_NewFrame",
      "ImGui::NewFrame",
      "widgets (Button, InputText, Table...)",
      "ImGui::Render",
      "ImGui_ImplOpenGL3_RenderDrawData",
      "SDL_GL_SwapWindow",
  };
  int n = 1;
  for (const char *p : pasos) {
    std::printf("  %d. %s\n", n++, p);
  }
  std::puts("");
  std::puts(
      "Un ID estable + estado propio = toda la magia del immediate mode.");
  std::puts("DisplaySize esta en puntos; DisplayFramebufferScale convierte a "
            "pixeles.");
  return 0;
}
