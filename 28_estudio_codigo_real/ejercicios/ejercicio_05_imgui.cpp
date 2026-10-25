/*
 * MÓDULO 28: Estudio de Código Real — Arquitectura de Librerías
 * EJERCICIO 05: ImGui — ID Stack, Layout, Draw Lists, Docking
 *
 * OBJETIVO: Implementar componentes core de ImGui: ID generation, layout system,
 * draw list recording, font atlas, y docking logic.
 */

#include <cstddef>
#include <cstdint>
#include <cfloat>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <cstring>
#include "../../include/test_framework.hpp"

// ============================================================================
// TIPOS BASE
// ============================================================================

using ImGuiID = uint32_t;
constexpr ImGuiID IMGUI_ID_NONE = 0;

// Hash function para IDs (simplified FNV-1a)
inline ImGuiID ImHashStr(const char* str, ImGuiID seed = 0) {
    ImGuiID hash = seed ^ 0x811c9dc5;
    while (*str) {
        hash ^= static_cast<uint8_t>(*str++);
        hash *= 0x01000193;
    }
    return hash;
}

inline ImGuiID ImHashData(const void* data, size_t size, ImGuiID seed = 0) {
    ImGuiID hash = seed ^ 0x811c9dc5;
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    for (size_t i = 0; i < size; ++i) {
        hash ^= bytes[i];
        hash *= 0x01000193;
    }
    return hash;
}

// ============================================================================
// 1. ID STACK MANAGEMENT
// ============================================================================

struct ImGuiIDStack {
    std::vector<ImGuiID> stack;
    
    // TODO: PushID(str) - push string ID
    void PushID(const char* str_id) {}
    
    // TODO: PushID(ptr) - push pointer ID
    void PushID(const void* ptr_id) {}
    
    // TODO: PushID(int) - push integer ID
    void PushID(int int_id) {}
    
    // TODO: PopID - pop one level
    void PopID() {}
    
    // TODO: GetCurrentID - retorna ID actual (hash combinado)
    ImGuiID GetCurrentID() const { return 0; }
    
    // TODO: GetIDWithSeed - ID con seed adicional (para widgets múltiples mismo label)
    ImGuiID GetIDWithSeed(const char* label, ImGuiID seed) const { return 0; }
};

// ============================================================================
// 2. LAYOUT SYSTEM (simplificado)
// ============================================================================

struct ImVec2 {
    float x, y;
    constexpr ImVec2() : x(0), y(0) {}
    constexpr ImVec2(float x_, float y_) : x(x_), y(y_) {}
    ImVec2 operator+(const ImVec2& v) const { return {x + v.x, y + v.y}; }
    ImVec2 operator-(const ImVec2& v) const { return {x - v.x, y - v.y}; }
    ImVec2 operator*(float s) const { return {x * s, y * s}; }
};

struct ImRect {
    ImVec2 Min, Max;
    constexpr ImRect() : Min(0,0), Max(0,0) {}
    constexpr ImRect(const ImVec2& min, const ImVec2& max) : Min(min), Max(max) {}
    ImVec2 GetSize() const { return Max - Min; }
    bool Contains(const ImVec2& p) const { return p.x >= Min.x && p.x < Max.x && p.y >= Min.y && p.y < Max.y; }
};

struct ImGuiWindow {
    std::string Name;
    ImGuiID ID;
    ImRect Rect;
    bool Active = false;
    float CursorPosX = 0, CursorPosY = 0;
    float ItemWidth = 0;
    std::vector<ImGuiID> IDStack;
};

// TODO: ImGuiLayout - manejo de layout simple
struct ImGuiLayout {
    ImGuiWindow* CurrentWindow = nullptr;
    float X = 0, Y = 0;
    float LineHeight = 19;
    float ItemSpacingX = 8, ItemSpacingY = 4;
    
    // TODO: BeginWindow
    bool BeginWindow(const char* name, ImVec2 size = {0,0}) { return false; }
    
    // TODO: EndWindow
    void EndWindow() {}
    
    // TODO: ItemSize - reserva espacio para widget
    void ItemSize(const ImVec2& size) {}
    
    // TODO: ItemAdd - registra widget, retorna true si visible
    bool ItemAdd(const ImRect& bb, ImGuiID id) { return false; }
    
    // TODO: GetCursorPos
    ImVec2 GetCursorPos() const { return {X, Y}; }
    
    // TODO: SetCursorPos
    void SetCursorPos(const ImVec2& pos) { X = pos.x; Y = pos.y; }
    
    // TODO: SameLine
    void SameLine(float offset_x = 0, float spacing_x = -1) {}
};

// ============================================================================
// 3. DRAW LIST (simplificado)
// ============================================================================

struct ImDrawVert {
    ImVec2 pos;
    ImVec2 uv;
    uint32_t col;
};

struct ImVec4 {
    float x, y, z, w;
    constexpr ImVec4() : x(0),y(0),z(0),w(0) {}
    constexpr ImVec4(float x_,float y_,float z_,float w_) : x(x_),y(y_),z(z_),w(w_) {}
};

struct ImDrawCmd {
    uint32_t ElemCount = 0;
    ImVec4 clip_rect = {0,0,0,0};
    uintptr_t texture_id = 0;
};

using ImDrawIdx = uint16_t;

struct ImDrawList {
    std::vector<ImDrawVert> VtxBuffer;
    std::vector<ImDrawIdx> IdxBuffer;
    std::vector<ImDrawCmd> CmdBuffer;
    
    ImVec4 ClipRect = {-FLT_MAX, -FLT_MAX, FLT_MAX, FLT_MAX};
    
    // TODO: AddRect - rectángulo outline
    void AddRect(const ImVec2& p_min, const ImVec2& p_max, uint32_t col, float rounding = 0) {}
    
    // TODO: AddRectFilled - rectángulo relleno
    void AddRectFilled(const ImVec2& p_min, const ImVec2& p_max, uint32_t col, float rounding = 0) {}
    
    // TODO: AddText
    void AddText(const ImVec2& pos, uint32_t col, const char* text) {}
    
    // TODO: AddLine
    void AddLine(const ImVec2& p1, const ImVec2& p2, uint32_t col, float thickness = 1.0f) {}
    
    // TODO: PrimReserve - reserva espacio en buffers
    void PrimReserve(int idx_count, int vtx_count) {}
    
    // TODO: PrimRect - helper para rectángulos (usa indices)
    void PrimRect(const ImVec2& a, const ImVec2& b, uint32_t col) {}
    
    // TODO: PrimRectUV - rectángulo con UV (para texturas)
    void PrimRectUV(const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, uint32_t col) {}
    
    // TODO: _ResetForNewFrame
    void _ResetForNewFrame() {}
};

// ============================================================================
// 4. FONT ATLAS (simplificado)
// ============================================================================

struct ImFontGlyph {
    uint32_t Codepoint = 0;
    float AdvanceX = 0;
    float X0 = 0, Y0 = 0, X1 = 0, Y1 = 0;
    float U0 = 0, V0 = 0, U1 = 0, V1 = 0;
};

struct ImFont {
    float FontSize = 0;
    float Scale = 1.0f;
    std::vector<ImFontGlyph> Glyphs;
    std::vector<float> IndexAdvanceX;
    std::vector<int> IndexLookup; // codepoint -> glyph index
    float FallbackAdvanceX = 0;
    uint32_t FallbackChar = '?';
    
    // TODO: FindGlyph - busca glifo por codepoint
    const ImFontGlyph* FindGlyph(uint32_t codepoint) const { return nullptr; }
    
    // TODO: GetCharAdvance - advance X para character
    float GetCharAdvance(uint32_t codepoint) const { return 0; }
    
    // TODO: CalcTextSizeA - calcula tamaño de texto
    ImVec2 CalcTextSizeA(float size, float max_width, float wrap_width, const char* text) const { return {0,0}; }
    
    // TODO: RenderChar - añade vértices para carácter al draw list
    void RenderChar(ImDrawList* draw_list, float size, const ImVec2& pos, uint32_t col, uint32_t codepoint) const {}
};

struct ImFontAtlas {
    int TexWidth = 512, TexHeight = 512;
    std::vector<uint8_t> TexPixels; // RGBA8
    std::vector<ImFont*> Fonts;
    ImFont* DefaultFont = nullptr;
    
    // TODO: AddFontDefault - añade fuente por defecto (embedded)
    ImFont* AddFontDefault() { return nullptr; }
    
    // TODO: Build - construye atlas, genera textura
    void Build() {}
    
    // TODO: GetTexDataAsRGBA32 - obtiene puntero a textura
    void GetTexDataAsRGBA32(uint8_t** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel) {}
};

// ============================================================================
// 5. DOCKING (simplificado)
// ============================================================================

enum class ImGuiDockNodeFlags : int {
    None = 0,
    KeepAliveOnly = 1 << 0,
    NoDockingInCentralNode = 1 << 1,
    PassthruCentralNode = 1 << 2,
    NoSplit = 1 << 3,
    NoResize = 1 << 4,
    AutoHideTabBar = 1 << 5,
};

struct ImGuiDockNode {
    ImGuiID ID = 0;
    ImGuiDockNodeFlags Flags = ImGuiDockNodeFlags::None;
    ImGuiID ParentNode = 0;
    ImGuiID ChildNodes[2] = {0, 0};
    ImGuiID TabBar = 0;
    int SplitAxis = -1; // 0=horizontal, 1=vertical
    float SizeRef = 0;
    
    // TODO: IsSplitNode
    bool IsSplitNode() const { return false; }
    
    // TODO: IsLeafNode
    bool IsLeafNode() const { return false; }
};

struct ImGuiDockContext {
    std::unordered_map<ImGuiID, ImGuiDockNode> Nodes;
    ImGuiID NextNodeID = 1;
    
    // TODO: AddNode
    ImGuiID AddNode(ImGuiDockNodeFlags flags = ImGuiDockNodeFlags::None) { return 0; }
    
    // TODO: RemoveNode
    void RemoveNode(ImGuiID node_id) {}
    
    // TODO: SplitNode - divide nodo en dos
    void SplitNode(ImGuiID node_id, int split_axis, float size_ratio, ImGuiID new_node_id) {}
    
    // TODO: DockWindow - añade ventana a nodo
    void DockWindow(ImGuiID node_id, ImGuiID window_id) {}
};

// ============================================================================
// TESTS (NO MODIFICAR)
// ============================================================================

TEST_CASE(TestIDHash) {
    ASSERT_EQ(ImHashStr("Hello"), ImHashStr("Hello"));
    ASSERT_NE(ImHashStr("Hello"), ImHashStr("World"));
    ASSERT_EQ(ImHashStr("Test", 123), ImHashStr("Test", 123));
    ASSERT_NE(ImHashStr("Test", 123), ImHashStr("Test", 456));
}

TEST_CASE(TestIDStack) {
    ImGuiIDStack ids;
    ASSERT_EQ(ids.GetCurrentID(), 0);
    
    ids.PushID("window");
    ImGuiID window_id = ids.GetCurrentID();
    ASSERT_NE(window_id, 0);
    
    ids.PushID("button");
    ImGuiID button_id = ids.GetCurrentID();
    ASSERT_NE(button_id, window_id);
    
    ids.PopID();
    ASSERT_EQ(ids.GetCurrentID(), window_id);
    
    ids.PopID();
    ASSERT_EQ(ids.GetCurrentID(), 0);
}

TEST_CASE(TestIDWithSeed) {
    ImGuiIDStack ids;
    ids.PushID("window");
    
    ImGuiID id1 = ids.GetIDWithSeed("Button", 1);
    ImGuiID id2 = ids.GetIDWithSeed("Button", 2);
    ImGuiID id3 = ids.GetIDWithSeed("Button", 1);
    
    ASSERT_NE(id1, id2);
    ASSERT_EQ(id1, id3);
}

TEST_CASE(TestLayout) {
    ImGuiLayout layout;
    ImGuiWindow window;
    window.Name = "Test";
    window.ID = ImHashStr("Test");
    window.Rect = ImRect({0,0}, {400,300});
    layout.CurrentWindow = &window;
    
    ASSERT_TRUE(layout.BeginWindow("Test", {400, 300}));
    ASSERT_EQ(layout.GetCursorPos().x, 0);
    ASSERT_EQ(layout.GetCursorPos().y, 0);
    
    layout.ItemSize({100, 20});
    ASSERT_EQ(layout.GetCursorPos().y, 20 + 4); // LineHeight + ItemSpacingY
    
    layout.SameLine();
    ASSERT_EQ(layout.GetCursorPos().x, 0);
    ASSERT_EQ(layout.GetCursorPos().y, 20 + 4);
    
    layout.EndWindow();
}

TEST_CASE(TestDrawList) {
    ImDrawList draw_list;
    
    draw_list.PrimReserve(6, 4);
    ASSERT_GE(draw_list.IdxBuffer.capacity(), 6);
    ASSERT_GE(draw_list.VtxBuffer.capacity(), 4);
    
    draw_list.AddRect({10, 10}, {50, 50}, 0xFFFFFFFF);
    ASSERT_GT(draw_list.VtxBuffer.size(), 0);
    ASSERT_GT(draw_list.IdxBuffer.size(), 0);
    ASSERT_GT(draw_list.CmdBuffer.size(), 0);
    
    draw_list._ResetForNewFrame();
    ASSERT_EQ(draw_list.VtxBuffer.size(), 0);
    ASSERT_EQ(draw_list.IdxBuffer.size(), 0);
    ASSERT_EQ(draw_list.CmdBuffer.size(), 0);
}

TEST_CASE(TestFontGlyph) {
    ImFont font;
    font.FontSize = 16;
    font.FallbackAdvanceX = 8;
    font.FallbackChar = '?';
    
    ImFontGlyph glyph = {'A', 8, 0,0,8,16, 0,0,1,1};
    font.Glyphs.push_back(glyph);
    font.IndexLookup.resize(128, -1);
    font.IndexLookup['A'] = 0;
    font.IndexAdvanceX.push_back(8);
    
    const ImFontGlyph* g = font.FindGlyph('A');
    ASSERT_TRUE(g != nullptr);
    ASSERT_EQ(g->Codepoint, 'A');
    ASSERT_EQ(g->AdvanceX, 8);
    
    // Missing char -> fallback
    const ImFontGlyph* missing = font.FindGlyph('Z');
    ASSERT_TRUE(missing == nullptr);
}

TEST_CASE(TestFontAtlas) {
    ImFontAtlas atlas;
    ImFont* font = atlas.AddFontDefault();
    ASSERT_TRUE(font != nullptr);
    ASSERT_EQ(atlas.DefaultFont, font);
    
    atlas.Build();
    ASSERT_GT(atlas.TexWidth, 0);
    ASSERT_GT(atlas.TexHeight, 0);
    
    uint8_t* pixels = nullptr;
    int w, h, bpp;
    atlas.GetTexDataAsRGBA32(&pixels, &w, &h, &bpp);
    ASSERT_TRUE(pixels != nullptr);
    ASSERT_EQ(w, atlas.TexWidth);
    ASSERT_EQ(h, atlas.TexHeight);
    ASSERT_EQ(bpp, 4);
}

TEST_CASE(TestDocking) {
    ImGuiDockContext ctx;
    
    ImGuiID root = ctx.AddNode();
    ASSERT_NE(root, 0);
    
    ImGuiID child = ctx.AddNode();
    ASSERT_NE(child, 0);
    
    ctx.SplitNode(root, 0, 0.5f, child);
    
    auto it = ctx.Nodes.find(root);
    ASSERT_TRUE(it != ctx.Nodes.end());
    ASSERT_TRUE(it->second.IsSplitNode());
    
    ctx.DockWindow(root, ImHashStr("Window1"));
    ctx.RemoveNode(child);
    ASSERT_EQ(ctx.Nodes.count(child), 0);
}

int main() { return RUN_ALL_TESTS(); }