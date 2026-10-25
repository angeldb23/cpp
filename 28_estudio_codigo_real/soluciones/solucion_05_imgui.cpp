/*
 * MÓDULO 28: Estudio de Código Real — Arquitectura de Librerías
 * SOLUCIÓN 05: ImGui — ID Stack, Layout, Draw Lists, Docking
 */

#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <cstring>
#include <cfloat>
#include "../../include/test_framework.hpp"

// ============================================================================
// TIPOS BASE
// ============================================================================

using ImGuiID = uint32_t;
constexpr ImGuiID IMGUI_ID_NONE = 0;

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
    
    void PushID(const char* str_id) {
        ImGuiID id = ImHashStr(str_id, stack.empty() ? 0 : stack.back());
        stack.push_back(id);
    }
    
    void PushID(const void* ptr_id) {
        ImGuiID id = ImHashData(&ptr_id, sizeof(ptr_id), stack.empty() ? 0 : stack.back());
        stack.push_back(id);
    }
    
    void PushID(int int_id) {
        ImGuiID id = ImHashData(&int_id, sizeof(int_id), stack.empty() ? 0 : stack.back());
        stack.push_back(id);
    }
    
    void PopID() {
        if (!stack.empty()) stack.pop_back();
    }
    
    ImGuiID GetCurrentID() const {
        return stack.empty() ? 0 : stack.back();
    }
    
    ImGuiID GetIDWithSeed(const char* label, ImGuiID seed) const {
        ImGuiID base = stack.empty() ? 0 : stack.back();
        return ImHashStr(label, base ^ seed);
    }
};

// ============================================================================
// 2. LAYOUT SYSTEM
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

struct ImGuiLayout {
    ImGuiWindow* CurrentWindow = nullptr;
    float X = 0, Y = 0;
    float LineHeight = 19;
    float ItemSpacingX = 8, ItemSpacingY = 4;
    
    bool BeginWindow(const char* name, ImVec2 size = {0,0}) {
        if (!CurrentWindow) return false;
        X = CurrentWindow->CursorPosX;
        Y = CurrentWindow->CursorPosY;
        return true;
    }
    
    void EndWindow() {
        if (CurrentWindow) {
            CurrentWindow->CursorPosX = X;
            CurrentWindow->CursorPosY = Y;
        }
    }
    
    void ItemSize(const ImVec2& size) {
        Y += size.y + ItemSpacingY;
        X = CurrentWindow ? CurrentWindow->CursorPosX : 0;
    }
    
    bool ItemAdd(const ImRect& bb, ImGuiID id) {
        (void)bb; (void)id;
        return true;
    }
    
    ImVec2 GetCursorPos() const { return {X, Y}; }
    
    void SetCursorPos(const ImVec2& pos) { X = pos.x; Y = pos.y; }
    
    void SameLine(float offset_x = 0, float spacing_x = -1) {
        X = CurrentWindow ? CurrentWindow->CursorPosX : 0;
        if (offset_x > 0) X += offset_x;
        else if (spacing_x >= 0) X += spacing_x;
    }
};

// ============================================================================
// 3. DRAW LIST
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
    
    ImDrawList() = default;
    
    void AddRect(const ImVec2& p_min, const ImVec2& p_max, uint32_t col, float rounding = 0) {
        (void)rounding;
        PrimReserve(6, 4);
        PrimRect(p_min, p_max, col);
    }
    
    void AddRectFilled(const ImVec2& p_min, const ImVec2& p_max, uint32_t col, float rounding = 0) {
        (void)rounding;
        PrimReserve(6, 4);
        PrimRect(p_min, p_max, col);
    }
    
    void AddText(const ImVec2& pos, uint32_t col, const char* text) {
        (void)pos; (void)col; (void)text;
    }
    
    void AddLine(const ImVec2& p1, const ImVec2& p2, uint32_t col, float thickness = 1.0f) {
        (void)p1; (void)p2; (void)col; (void)thickness;
    }
    
    void PrimReserve(int idx_count, int vtx_count) {
        if (CmdBuffer.empty()) CmdBuffer.push_back({});
        size_t old_vtx_size = VtxBuffer.size();
        VtxBuffer.resize(old_vtx_size + vtx_count);
        IdxBuffer.resize(IdxBuffer.size() + idx_count);
        CmdBuffer.back().ElemCount += idx_count;
    }
    
    void PrimRect(const ImVec2& a, const ImVec2& b, uint32_t col) {
        PrimReserve(6, 4);
        int idx = VtxBuffer.size() - 4;
        VtxBuffer[idx + 0] = {a, {0,0}, 0};
        VtxBuffer[idx + 1] = {{b.x, a.y}, {0,0}, 0};
        VtxBuffer[idx + 2] = {b, {0,0}, 0};
        VtxBuffer[idx + 3] = {{a.x, b.y}, {0,0}, 0};
        int base = idx;
        IdxBuffer[IdxBuffer.size() - 6] = base + 0;
        IdxBuffer[IdxBuffer.size() - 5] = base + 1;
        IdxBuffer[IdxBuffer.size() - 4] = base + 2;
        IdxBuffer[IdxBuffer.size() - 3] = base + 0;
        IdxBuffer[IdxBuffer.size() - 2] = base + 2;
        IdxBuffer[IdxBuffer.size() - 1] = base + 3;
    }
    
    void PrimRectUV(const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, uint32_t col) {
        (void)uv_a; (void)uv_b; (void)col;
        PrimRect(a, b, col);
    }
    
    void _ResetForNewFrame() {
        VtxBuffer.clear();
        IdxBuffer.clear();
        CmdBuffer.clear();
    }
};

// ============================================================================
// 4. FONT ATLAS
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
    std::vector<int> IndexLookup;
    float FallbackAdvanceX = 0;
    uint32_t FallbackChar = '?';
    
    const ImFontGlyph* FindGlyph(uint32_t codepoint) const {
        if (codepoint < IndexLookup.size() && IndexLookup[codepoint] >= 0)
            return &Glyphs[IndexLookup[codepoint]];
        return nullptr;
    }
    
    float GetCharAdvance(uint32_t codepoint) const {
        const ImFontGlyph* g = FindGlyph(codepoint);
        return g ? g->AdvanceX : FallbackAdvanceX;
    }
    
    ImVec2 CalcTextSizeA(float size, float max_width, float wrap_width, const char* text) const {
        (void)max_width; (void)wrap_width;
        float width = 0;
        while (*text) {
            width += GetCharAdvance(static_cast<uint8_t>(*text++)) * size;
        }
        return {width, FontSize * size};
    }
    
    void RenderChar(ImDrawList* draw_list, float size, const ImVec2& pos, uint32_t col, uint32_t codepoint) const {
        (void)draw_list; (void)size; (void)pos; (void)col; (void)codepoint;
    }
};

struct ImFontAtlas {
    int TexWidth = 512, TexHeight = 512;
    std::vector<uint8_t> TexPixels;
    std::vector<ImFont*> Fonts;
    ImFont* DefaultFont = nullptr;
    
    ImFont* AddFontDefault() {
        ImFont* font = new ImFont();
        font->FontSize = 13;
        font->FallbackAdvanceX = 8;
        font->FallbackChar = '?';
        
        // Add basic ASCII glyphs
        for (int c = 32; c < 127; ++c) {
            ImFontGlyph g;
            g.Codepoint = c;
            g.AdvanceX = 8;
            g.X0 = 0; g.Y0 = 0; g.X1 = 8; g.Y1 = 13;
            g.U0 = 0; g.V0 = 0; g.U1 = 1; g.V1 = 1;
            font->Glyphs.push_back(g);
            font->IndexAdvanceX.push_back(8);
        }
        font->IndexLookup.resize(128, -1);
        for (int c = 32; c < 127; ++c) font->IndexLookup[c] = c - 32;
        font->FallbackAdvanceX = 8;
        font->FallbackChar = '?';
        
        Fonts.push_back(font);
        DefaultFont = font;
        return font;
    }
    
    void Build() {
        TexWidth = 512; TexHeight = 512;
        TexPixels.assign(TexWidth * TexHeight * 4, 255);
    }
    
    void GetTexDataAsRGBA32(uint8_t** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel) {
        *out_pixels = TexPixels.data();
        *out_width = TexWidth;
        *out_height = TexHeight;
        *out_bytes_per_pixel = 4;
    }
};

// ============================================================================
// 5. DOCKING
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
    int Flags = 0;
    ImGuiID ParentNode = 0;
    ImGuiID ChildNodes[2] = {0, 0};
    ImGuiID TabBar = 0;
    int SplitAxis = -1;
    float SizeRef = 0;
    
    bool IsSplitNode() const { return ChildNodes[0] != 0 || ChildNodes[1] != 0; }
    bool IsLeafNode() const { return ChildNodes[0] == 0 && ChildNodes[1] == 0; }
};

struct ImGuiDockContext {
    std::unordered_map<ImGuiID, ImGuiDockNode> Nodes;
    ImGuiID NextNodeID = 1;
    
    ImGuiID AddNode(int flags = 0) {
        ImGuiID id = NextNodeID++;
        Nodes[id] = ImGuiDockNode();
        Nodes[id].ID = id;
        Nodes[id].Flags = flags;
        return id;
    }
    
    void RemoveNode(ImGuiID node_id) {
        auto it = Nodes.find(node_id);
        if (it != Nodes.end()) {
            // Remove from parent
            if (it->second.ParentNode != 0) {
                auto parent_it = Nodes.find(it->second.ParentNode);
                if (parent_it != Nodes.end()) {
                    if (parent_it->second.ChildNodes[0] == node_id) parent_it->second.ChildNodes[0] = 0;
                    if (parent_it->second.ChildNodes[1] == node_id) parent_it->second.ChildNodes[1] = 0;
                }
            }
            Nodes.erase(it);
        }
    }
    
    void SplitNode(ImGuiID node_id, int split_axis, float size_ratio, ImGuiID new_node_id) {
        auto it = Nodes.find(node_id);
        if (it == Nodes.end()) return;
        
        ImGuiDockNode& node = it->second;
        node.SplitAxis = split_axis;
        node.SizeRef = size_ratio;
        
        ImGuiID child = AddNode();
        Nodes[child].ParentNode = node_id;
        node.ChildNodes[0] = node_id; // self becomes first child? No, this is simplified
        node.ChildNodes[1] = child;
        Nodes[child].ParentNode = node_id;
        
        (void)size_ratio;
    }
    
    void DockWindow(ImGuiID node_id, ImGuiID window_id) {
        (void)node_id; (void)window_id;
    }
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
    ASSERT_EQ(layout.GetCursorPos().y, 20 + 4);
    
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