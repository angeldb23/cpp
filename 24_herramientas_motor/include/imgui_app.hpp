/* =============================================================================
   MÓDULO 24 - ARRANQUE MÍNIMO DE DEAR IMGUI (SDL3 + OpenGL 3.3 core)
   =============================================================================
   Este header concentra el "cableado" que NO es el tema de cada lección:
   crear ventana, contexto GL 3.3 core, contexto de ImGui, backends, loop y
   shutdown ordenado. Cada lección aporta solo su función `dibujarUI()`.

   ¿Por qué OpenGL 3.3 core? Es el techo de la Intel UHD 600 del dueño.
   ImGui no necesita nada más moderno.

   Si Dear ImGui no está disponible (compilación suelta con g++), el header
   define M24_HAS_IMGUI=0 y las lecciones corren una demo headless de la
   misma lógica. Con CMake, FetchContent trae ImGui v1.91.5-docking y la
   ventana funciona.

   Orden obligatorio del ciclo (si lo cambiás, ImGui abandona):
       ImGui_ImplOpenGL3_NewFrame()
       ImGui_ImplSDL3_NewFrame()
       ImGui::NewFrame()
       ... widgets ...
       ImGui::Render()
       ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData())
       SDL_GL_SwapWindow()

   Con multiple viewports habría que actualizar las plataformas después de
   Render(); acá no se habilitan (la lección 11 explica el costo).
   ============================================================================= */
#pragma once

#if __has_include(<imgui.h>) && __has_include(<SDL3/SDL.h>)
#define M24_HAS_IMGUI 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl3.h>

#include <cstdio>

namespace gui {

using DrawFn = void (*)();

inline int runImGuiDemo(const char* title, DrawFn draw, int width = 1100, int height = 720) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::fprintf(stderr, "SDL_Init fallo: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_Window* window = SDL_CreateWindow(title, width, height,
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow fallo: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_GLContext gl = SDL_GL_CreateContext(window);
    if (!gl) {
        std::fprintf(stderr, "SDL_GL_CreateContext fallo: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_GL_MakeCurrent(window, gl);
    SDL_GL_SetSwapInterval(1);  // vsync: la UHD 600 agradece no girar al vacío

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    ImGui::GetStyle().WindowRounding = 4.0f;

    ImGui_ImplSDL3_InitForOpenGL(window, gl);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    bool running = true;
    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            ImGui_ImplSDL3_ProcessEvent(&ev);
            if (ev.type == SDL_EVENT_QUIT) running = false;
            if (ev.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
                ev.window.windowID == SDL_GetWindowID(window)) {
                running = false;
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        draw();
        ImGui::Render();

        int w = 0, h = 0;
        SDL_GetWindowSizeInPixels(window, &w, &h);
        glViewport(0, 0, w, h);
        glClearColor(0.07f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DestroyContext(gl);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

} // namespace gui

#else
#define M24_HAS_IMGUI 0
#endif
