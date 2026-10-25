/*
 * MÓDULO 28: Estudio de Código Real — Arquitectura de Librerías
 * LECCIÓN 07: Estudio de Dear ImGui — Immediate Mode GUI Architecture
 *
 * TEMAS:
 * - Immediate mode vs retained mode: ventajas/desventajas
 * - API core: NewFrame, Render, EndFrame, GetDrawData
 * - ID stack: PushID/PopID, ### labels, ID collision avoidance
 * - Layout: Begin/End, columns, tables, tree nodes, tabs
 * - Widgets: InputText, Slider, Drag, ColorEdit, Combo, ListBox
 * - Font atlas: cargar TTF, rangos Unicode, merge, custom rects
 * - Backend separation: platform (inputs/window) + renderer (draw lists)
 * - Docking: DockSpaceOverViewport, branch docking, multi-viewport
 * - Viewports: platform windows nativos, owner/parent, focus
 * - Styling: StyleVar stack, colors, rounding, spacing, alpha
 * - Memory: allocadores custom (ImGuiMemAlloc/ImGuiMemFree)
 * - Thread safety: no thread-safe por defecto, un contexto por thread
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>
#include <functional>

namespace modulo28_imgui {

// Tipos base mínimos de ImGui (en la librería real viven en imgui.h)
using ImWchar = unsigned short;

struct ImVec2 {
    float x = 0.0f, y = 0.0f;
};

template<typename T>
using ImVector = std::vector<T>;

// ============================================================================
// ARQUITECTURA IMMEDIATE MODE
// ============================================================================

/*
 * Immediate Mode GUI (IMGUI):
 *   - No hay objetos persistentes (botones, ventanas) entre frames
 *   - Cada frame: se reconstruye toda la UI desde cero
 *   - Estado efímero: solo existe durante la llamada a la función
 *   - Estado persistente: guardado en variables del usuario (referencias)
 * 
 * Ventajas:
 *   - Simple: no retención, no callbacks, no event loop propio
 *   - Data-driven: UI refleja estado actual directamente
 *   - Hot-reload friendly: código UI = lógica, recarga = recompilar
 *   - Bajo overhead memoria: solo draw data + estado interno mínimo
 * 
 * Desventajas:
 *   - CPU: reconstrucción cada frame (mitigado: culling, early-out)
 *   - Latencia input: 1 frame (input -> NewFrame -> UI -> Render)
 *   - Estado complejo: requiere ID stack para widgets dinámicos
 */

// ============================================================================
// CORE API Y CICLO DE FRAME
// ============================================================================

struct ImGuiContext; // opaque

struct ImDrawData {
    bool Valid = false;
    int CmdListsCount = 0;
    int TotalIdxCount = 0;
    int TotalVtxCount = 0;
    // CmdLists[] punteros a ImDrawList
};

struct ImDrawCmd {
    uint32_t ElemCount = 0;
    float ClipRect[4] = {0, 0, 0, 0};
    uintptr_t TextureId = 0;
    uintptr_t VtxOffset = 0;
    uintptr_t IdxOffset = 0;
};

struct ImDrawVert {
    float pos[2];
    float uv[2];
    uint32_t col;
};

using ImDrawIdx = uint16_t; // o uint32_t si 32-bit indices

struct ImDrawList {
    std::vector<ImDrawCmd> CmdBuffer;
    std::vector<ImDrawVert> VtxBuffer;
    std::vector<ImDrawIdx> IdxBuffer;
    
    // Primitivas
    void AddRect(const float* rect_min, const float* rect_max, uint32_t col, float rounding = 0);
    void AddText(const float* pos, uint32_t col, const char* text);
    void AddImage(uintptr_t texture_id, const float* p_min, const float* p_max, const float* uv_min, const float* uv_max, uint32_t col);
};

// API principal
namespace ImGui {
    // Context management
    ImGuiContext* CreateContext();
    void DestroyContext(ImGuiContext* ctx);
    void SetCurrentContext(ImGuiContext* ctx);
    ImGuiContext* GetCurrentContext();
    
    // Frame cycle
    void NewFrame();
    void EndFrame();
    void Render();
    ImDrawData* GetDrawData();
    
    // Main window
    bool Begin(const char* name, bool* p_open = nullptr, int flags = 0);
    void End();
    
    // ID Stack
    void PushID(const char* str_id);
    void PushID(const void* ptr_id);
    void PushID(int int_id);
    void PopID();
    uint32_t GetID(const char* str_id);
    uint32_t GetID(const void* ptr_id);
    
    // Layout
    void BeginChild(const char* str_id, const float size[2] = nullptr, bool border = false, int flags = 0);
    void EndChild();
    bool BeginTable(const char* str_id, int columns, int flags = 0, const float outer_size[2] = nullptr, float inner_width = 0.0f);
    void EndTable();
    void TableSetupColumn(const char* label, int flags = 0, float init_width = 0.0f, uint32_t user_id = 0);
    void TableHeadersRow();
    bool TableNextRow(int flags = 0, float min_height = 0.0f);
    bool TableNextColumn();
    void TableSetColumnIndex(int column_n);
    
    // Widgets básicos
    bool Button(const char* label, const float size[2] = nullptr);
    bool SmallButton(const char* label);
    bool InvisibleButton(const char* str_id, const float size[2]);
    bool Checkbox(const char* label, bool* v);
    bool RadioButton(const char* label, int* v, int v_button);
    float DragFloat(const char* label, float* v, float speed = 1.0f, float min = 0, float max = 0, const char* format = "%.3f", int flags = 0);
    float SliderFloat(const char* label, float* v, float min, float max, const char* format = "%.3f", int flags = 0);
    bool InputText(const char* label, char* buf, size_t buf_size, int flags = 0);
    bool ColorEdit3(const char* label, float col[3], int flags = 0);
    bool ColorEdit4(const char* label, float col[4], int flags = 0);
    bool Combo(const char* label, int* current_item, const char* const* items, int items_count, int popup_max_height = -1);
    bool ListBox(const char* label, int* current_item, const char* const* items, int items_count, int height_in_items = -1);
    bool TreeNode(const char* label);
    bool TreeNodeEx(const char* label, int flags);
    void TreePop();
    bool BeginTabBar(const char* str_id, int flags = 0);
    void EndTabBar();
    bool BeginTabItem(const char* label, bool* p_open = nullptr, int flags = 0);
    void EndTabItem();
    
    // Styling
    void PushStyleVar(int idx, float val);
    void PushStyleVar(int idx, const float val[2]);
    void PopStyleVar(int count = 1);
    void PushStyleColor(int idx, uint32_t col);
    void PopStyleColor(int count = 1);
    
    // Fonts
    struct ImFontAtlas* GetIOFonts();
    struct ImFont* AddFontFromFileTTF(const char* filename, float size_pixels, const void* glyph_ranges = nullptr);
    void PushFont(struct ImFont* font);
    void PopFont();
    
    // Viewports/Docking
    void DockSpaceOverViewport(uint32_t dockspace_id = 0, struct ImGuiViewport* viewport = nullptr, int flags = 0);
    bool BeginViewport(const char* name, bool* p_open = nullptr, int flags = 0);
    void EndViewport();
    
    // Debug/Metrics
    void ShowDemoWindow(bool* p_open = nullptr);
    void ShowMetricsWindow(bool* p_open = nullptr);
    void ShowStyleEditor(struct ImGuiStyle* ref = nullptr);
    
    // IO
    struct ImGuiIO& GetIO();
}

// ============================================================================
// BACKEND ARCHITECTURE
// ============================================================================

/*
 * Separación Platform + Renderer:
 * 
 * Platform Backend (ej: imgui_impl_sdl3.cpp, imgui_impl_glfw.cpp):
 *   - Crear/manejar ventana
 *   - Procesar eventos: mouse, keyboard, gamepad, touch
 *   - Setear ImGuiIO: DisplaySize, DeltaTime, MousePos, KeysDown, etc.
 *   - IME support, clipboard, cursor shapes
 *   - Multi-viewport: crear/destruir platform windows
 * 
 * Renderer Backend (ej: imgui_impl_opengl3.cpp, imgui_impl_vulkan.cpp):
 *   - Crear recursos: shaders, textures, buffers
 *   - RenderDrawData(ImDrawData*): traducir draw cmds a API gráfica
 *   - Texture management: crear/destruir/actualizar textures
 *   - Multi-viewport: render targets por viewport
 * 
 * Integración:
 *   ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
 *   ImGui_ImplOpenGL3_Init("#version 330");
 *   // Frame loop:
 *   ImGui_ImplOpenGL3_NewFrame();
 *   ImGui_ImplSDL3_NewFrame();
 *   ImGui::NewFrame();
 *   // ... UI code ...
 *   ImGui::Render();
 *   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
 *   // Multi-viewport:
 *   if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
 *       ImGui::UpdatePlatformWindows();
 *       ImGui::RenderPlatformWindowsDefault();
 *   }
 */

struct ImGuiViewport {
    uint32_t ID = 0;
    int Flags = 0;
    float Pos[2] = {0, 0};
    float Size[2] = {0, 0};
    float WorkPos[2] = {0, 0};
    float WorkSize[2] = {0, 0};
    float DpiScale = 1.0f;
    ImGuiViewport* ParentViewport = nullptr;
    void* PlatformHandle = nullptr;
    void* PlatformHandleRaw = nullptr;
    bool PlatformRequestMove = false;
    bool PlatformRequestResize = false;
    bool PlatformRequestClose = false;
};

struct ImGuiPlatformIO {
    // Platform functions
    void (*Platform_CreateWindow)(ImGuiViewport* viewport);
    void (*Platform_DestroyWindow)(ImGuiViewport* viewport);
    void (*Platform_ShowWindow)(ImGuiViewport* viewport);
    void (*Platform_SetWindowPos)(ImGuiViewport* viewport, const float pos[2]);
    void (*Platform_GetWindowPos)(ImGuiViewport* viewport, float pos[2]);
    void (*Platform_SetWindowSize)(ImGuiViewport* viewport, const float size[2]);
    void (*Platform_GetWindowSize)(ImGuiViewport* viewport, float size[2]);
    void (*Platform_SetWindowTitle)(ImGuiViewport* viewport, const char* title);
    void (*Platform_SetWindowFocus)(ImGuiViewport* viewport);
    bool (*Platform_GetWindowFocus)(ImGuiViewport* viewport);
    bool (*Platform_GetWindowMinimized)(ImGuiViewport* viewport);
    void (*Platform_SetWindowAlpha)(ImGuiViewport* viewport, float alpha);
    void (*Platform_UpdateWindow)(ImGuiViewport* viewport);
    void (*Platform_RenderWindow)(ImGuiViewport* viewport, void* render_arg);
    void (*Platform_SwapBuffers)(ImGuiViewport* viewport);
    
    // Monitors
    void (*Platform_GetMonitorWorkRect)(int monitor_id, float rect[4]);
    void (*Platform_GetMonitorDPI)(int monitor_id, float* dpi);
};

struct ImGuiRendererIO {
    void (*Renderer_RenderDrawData)(ImDrawData* draw_data);
    bool (*Renderer_CreateTexture)(uintptr_t* out_texture_id, int width, int height, const uint32_t* pixels);
    void (*Renderer_DestroyTexture)(uintptr_t texture_id);
    void (*Renderer_UpdateTexture)(uintptr_t texture_id, int x, int y, int width, int height, const uint32_t* pixels);
    bool (*Renderer_BeginFrame)();
    void (*Renderer_EndFrame)();
};

// ============================================================================
// FONT ATLAS
// ============================================================================

/*
 * ImFontAtlas: gestiona texturas de fuentes
 * - Múltiples fuentes a diferentes tamaños
 * - Rangos Unicode personalizados (GetGlyphRangesLatin, Chinese, Japanese, Korean, Cyrillic)
 * - Merge mode: combinar fuentes (fallback icons + texto)
 * - Custom rects: iconos embebidos (FontAwesome, etc.)
 * - Rasterización: stb_truetype interno, oversampling configurable
 * - Texture: una sola textura atlas (puede ser multi-página si muy grande)
 */

struct ImFontConfig {
    int FontDataSize = 0;
    void* FontData = nullptr;
    float SizePixels = 0;
    int OversampleH = 3;
    int OversampleV = 1;
    bool PixelSnapH = false;
    float GlyphOffset[2] = {0, 0};
    const ImWchar* GlyphRanges = nullptr;
    float GlyphMinAdvanceX = 0;
    float GlyphMaxAdvanceX = 0;
    bool MergeMode = false;
    uint32_t RasterizerFlags = 0;
    float RasterizerMultiply = 1.0f;
};

struct ImFontAtlas;

struct ImFontGlyph {
    uint32_t Codepoint = 0;
    float AdvanceX = 0;
    float X0 = 0, Y0 = 0, X1 = 0, Y1 = 0;
    float U0 = 0, V0 = 0, U1 = 0, V1 = 0;
};

struct ImFont {
    float FontSize = 0;
    float Scale = 1.0f;
    int DisplayOffset = 0;
    ImVector<ImWchar> IndexLookup;
    ImVector<float> IndexAdvanceX;
    float FallbackAdvanceX = 0;
    float FallbackChar = (ImWchar)'?';
    ImFontAtlas* ContainerAtlas = nullptr;
    ImFontConfig Config;
    ImVector<ImFontGlyph> Glyphs;
};

struct ImFontAtlas {
    int TexWidth = 0, TexHeight = 0;
    uint32_t* TexPixels = nullptr;
    bool TexReady = false;
    
    ImFont* AddFont(const ImFontConfig* config);
    ImFont* AddFontFromFileTTF(const char* filename, float size_pixels, const ImFontConfig* config = nullptr, const ImWchar* glyph_ranges = nullptr);
    ImFont* AddFontFromMemoryTTF(void* data, int size, float size_pixels, const ImFontConfig* config = nullptr, const ImWchar* glyph_ranges = nullptr);
    ImFont* AddFontFromMemoryCompressedTTF(const void* data, int size, float size_pixels, const ImFontConfig* config = nullptr, const ImWchar* glyph_ranges = nullptr);
    void Build();
    void GetTexDataAsRGBA32(uint8_t** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel);
    void GetTexDataAsAlpha8(uint8_t** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel);
    void SetTexID(uintptr_t id);
    uintptr_t GetTexID() const;
    
    // Custom rects para iconos
    bool AddCustomRect(unsigned short width, unsigned short height, unsigned short* out_x, unsigned short* out_y, unsigned short* out_w = nullptr, unsigned short* out_h = nullptr);
    ImVec2 GetCustomRectByIndex(int index) const;
};

// ============================================================================
// DOCKING Y MULTI-VIEWPORT
// ============================================================================

/*
 * Docking (branch docking en v1.91+):
 * - DockSpaceOverViewport: crea dockspace que ocupa viewport principal
 * - DockNode: nodo del árbol de dock (split horizontal/vertical, tabs)
 * - DockNodeFlags: None, KeepAliveOnly, NoDockingInCentralNode, PassthruCentralNode, NoSplit, NoResize, AutoHideTabBar
 * - ID-based: cada ventana/dock tiene ID único (PushID/PopID)
 * - Persistence: .ini guarda layout (docking, window pos/size, tabs)
 * 
 * Multi-Viewport:
 * - ImGuiConfigFlags_ViewportsEnable: habilita viewports secundarios
 * - Platform windows nativos (no owned por main window)
 * - Viewport flags: NoDecoration, NoTaskBarIcon, NoFocusOnAppearing, TopMost
 * - Parent/child hierarchy: viewport principal -> hijos
 * - DPI awareness: cada viewport tiene su DpiScale
 */

// ============================================================================
// MEMORY ALLOCATORS
// ============================================================================

/*
 * ImGui usa allocadores customizables:
 *   ImGuiMemAlloc(size) / ImGuiMemFree(ptr)
 *   Por defecto: malloc/free
 *   Para engines: arena allocator, pool allocator, tracking
 *   Thread-local: cada contexto puede tener su allocador
 */

typedef void* (*ImGuiMemAllocFunc)(size_t sz, void* user_data);
typedef void (*ImGuiMemFreeFunc)(void* ptr, void* user_data);

void ImGuiSetAllocator(ImGuiMemAllocFunc alloc, ImGuiMemFreeFunc free, void* user_data = nullptr);

// ============================================================================
// EJEMPLO DE USO MÍNIMO
// ============================================================================

/*
#include "imgui.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_opengl3.h"

int main() {
    // Setup SDL3 + OpenGL
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_Window* window = SDL_CreateWindow("ImGui Demo", 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // VSync
    
    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
    
    ImGui::StyleColorsDark();
    
    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    bool show_demo = true;
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT) quit = true;
        }
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        
        if (show_demo) ImGui::ShowDemoWindow(&show_demo);
        
        ImGui::Begin("Hello");
        ImGui::Text("Hello, ImGui!");
        ImGui::End();
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
        
        SDL_GL_SwapWindow(window);
    }
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    return 0;
}
*/

} // namespace modulo28_imgui