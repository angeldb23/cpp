/* =============================================================================
   MÓDULO 24 - LECCIÓN 08: Property grid, curve editor y color picker
   =============================================================================
   TEMA 4 de TEMAS.md (segunda lección): las tres herramientas de edición que
   acompañan al gizmo.

   PROPERTY GRID
   -------------
   Una tabla nombre -> valor con edición in-place. Los problemas prácticos:

   - Tipos heterogéneos: `std::variant<int, float, bool, std::string, Color>`.
   - Read-only: algunas propiedades se muestran pero no se editan.
   - Dirty / revert: guardar el valor original para poder cancelar.
   - Filtrar por nombre: en un material con 80 campos, el filtro es la feature.
   - Undo: un cambio por gesto, no por píxel (`IsItemDeactivatedAfterEdit`).

   En la versión headless, el grid es un vector de propiedades y el test
   verifica set/get/dirty/revert, igual que el ejercicio 03.

   CURVE EDITOR
   ------------
   Una curva es una lista de keyframes (tiempo, valor) ordenada por tiempo.
   Entre dos keys se interpola:

   - Step: mantiene el valor de la key izquierda hasta la siguiente.
   - Linear: lerp.
   - Smooth: smoothstep, derivada 0 en los extremos (sin "esquinas").
   - Bézier/Catmull-Rom: necesita tangentes; el módulo 19 (L15) las cubre.

   Antes de la primera key y después de la última, se CLAMPEA al extremo (no
   se extrapola). Insertar una key en un tiempo existente la reemplaza. La
   edición es visual, pero toda la matemática es testeable.

   COLOR PICKER
   ------------
   ImGui guarda colores como float RGBA lineal o sRGB según el modo. El picker
   trabaja en HSV para que sea intuitivo; la conversión HSV<->RGB es la parte
   que conviene tener clara:

       C = V * S
       X = C * (1 - |((H/60) mod 2) - 1|)
       m = V - C
       (R,G,B) = (C,X,0) / (X,C,0) / ... según el sector de H

   `ColorEdit4` con `ImGuiColorEditFlags_DisplayHSV` deja elegir el espacio;
   `ColorPicker4` muestra el picker grande con alpha y cuentagotas.

   GAMMA
   -----
   El color que elegís en pantalla es sRGB; el shader ilumina en lineal. Si
   pasás el sRGB crudo al shader, las luces salen mal (módulo 21, L02). La UI
   puede mostrar sRGB y convertir al subir el uniform.
   ============================================================================= */
#include "include/gui_util.hpp"
#include "include/imgui_app.hpp"

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

namespace {

enum class Interp { Step, Linear, Smooth };

struct Key {
    float tiempo = 0.0f;
    float valor = 0.0f;
    Interp interp = Interp::Linear;
};

std::vector<Key> g_keys = {
    {0.0f, 0.0f, Interp::Smooth},
    {1.0f, 1.0f, Interp::Linear},
    {2.0f, 0.5f, Interp::Step},
    {3.0f, 1.5f, Interp::Smooth},
};

float evaluarCurva(const std::vector<Key>& keys, float t) {
    if (keys.empty()) return 0.0f;
    if (t <= keys.front().tiempo) return keys.front().valor;
    if (t >= keys.back().tiempo) return keys.back().valor;
    for (std::size_t i = 0; i + 1 < keys.size(); ++i) {
        const Key& a = keys[i];
        const Key& b = keys[i + 1];
        if (t < a.tiempo || t > b.tiempo) continue;
        const float u = (b.tiempo - a.tiempo) > 0.0f
                            ? (t - a.tiempo) / (b.tiempo - a.tiempo)
                            : 0.0f;
        switch (a.interp) {
            case Interp::Step: return a.valor;
            case Interp::Linear: return gui::lerpf(a.valor, b.valor, u);
            case Interp::Smooth: return gui::lerpf(a.valor, b.valor, gui::smooth01(u));
        }
    }
    return keys.back().valor;
}

// HSV con H en grados [0,360), S y V en [0,1].
gui::Color hsvARgb(float h, float s, float v) {
    h = std::fmod(h, 360.0f);
    if (h < 0.0f) h += 360.0f;
    s = gui::clampf(s, 0.0f, 1.0f);
    v = gui::clampf(v, 0.0f, 1.0f);

    const float c = v * s;
    const float x = c * (1.0f - std::fabs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
    const float m = v - c;
    float r = 0.0f, g = 0.0f, b = 0.0f;
    if (h < 60.0f)       { r = c; g = x; }
    else if (h < 120.0f) { r = x; g = c; }
    else if (h < 180.0f) { g = c; b = x; }
    else if (h < 240.0f) { g = x; b = c; }
    else if (h < 300.0f) { r = x; b = c; }
    else                 { r = c; b = x; }
    return {r + m, g + m, b + m, 1.0f};
}

#if M24_HAS_IMGUI
void dibujarUI() {
    ImGui::SetNextWindowPos(ImVec2(24.0f, 24.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(600.0f, 520.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("M24 / L08 - Property grid + curva + color");

    if (ImGui::CollapsingHeader("Property grid", ImGuiTreeNodeFlags_DefaultOpen)) {
        static bool activo = true;
        static float velocidad = 3.5f;
        static int vidas = 3;
        static char nombre[48] = "Slime";
        ImGui::Checkbox("Activo", &activo);
        ImGui::DragFloat("Velocidad", &velocidad, 0.05f, 0.0f, 20.0f);
        ImGui::DragInt("Vidas", &vidas, 0.1f, 0, 99);
        ImGui::InputText("Nombre", nombre, sizeof(nombre));
    }

    if (ImGui::CollapsingHeader("Curve editor", ImGuiTreeNodeFlags_DefaultOpen)) {
        static float t = 1.5f;
        ImGui::SliderFloat("Tiempo", &t, 0.0f, 3.0f);
        float valores[64];
        for (int i = 0; i < 64; ++i) {
            valores[i] = evaluarCurva(g_keys, 3.0f * static_cast<float>(i) / 63.0f);
        }
        ImGui::PlotLines("curva", valores, 64, 0, "keys: step/linear/smooth", 0.0f, 1.6f,
                         ImVec2(0, 100));
        ImGui::Text("f(%.2f) = %.3f", t, evaluarCurva(g_keys, t));
        for (std::size_t i = 0; i < g_keys.size(); ++i) {
            ImGui::PushID(static_cast<int>(i));
            ImGui::DragFloat("t", &g_keys[i].tiempo, 0.01f, 0.0f, 3.0f);
            ImGui::SameLine();
            ImGui::DragFloat("v", &g_keys[i].valor, 0.01f, -2.0f, 2.0f);
            ImGui::PopID();
        }
    }

    if (ImGui::CollapsingHeader("Color picker", ImGuiTreeNodeFlags_DefaultOpen)) {
        static float color[4] = {0.85f, 0.25f, 0.20f, 1.0f};
        ImGui::ColorEdit4("Albedo", color,
                          ImGuiColorEditFlags_DisplayHSV | ImGuiColorEditFlags_AlphaBar);
        ImGui::ColorPicker4("Picker", color, ImGuiColorEditFlags_DisplayHSV);
    }

    ImGui::End();
}
#endif

} // namespace

int main() {
    std::puts("M24 / L08 - Property grid, curva y color");
    std::puts("-----------------------------------------");

    std::puts("Curva de keys (0,0 smooth) (1,1 linear) (2,0.5 step) (3,1.5 smooth):");
    for (float t = 0.0f; t <= 3.001f; t += 0.5f) {
        std::printf("  f(%.2f) = %.3f\n", t, evaluarCurva(g_keys, t));
    }
    std::puts("");
    std::puts("HSV -> RGB:");
    for (float h : {0.0f, 60.0f, 120.0f, 240.0f, 300.0f}) {
        const auto c = hsvARgb(h, 1.0f, 1.0f);
        std::printf("  h=%5.0f s=1 v=1 -> (%.1f, %.1f, %.1f)\n", h, c.r, c.g, c.b);
    }
    std::puts("");
    std::puts("El grid guarda el valor original para revertir; el editor de curva");
    std::puts("reemplaza keys con el mismo tiempo y clampea fuera de rango.");

#if M24_HAS_IMGUI
    return gui::runImGuiDemo("M24 L08 - grid, curva y color", dibujarUI);
#else
    return 0;
#endif
}
