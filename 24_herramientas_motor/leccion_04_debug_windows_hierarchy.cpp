/* =============================================================================
   MÓDULO 24 - LECCIÓN 04: Scene hierarchy, drag & drop y resource browser
   =============================================================================
   TEMA 2 de TEMAS.md (segunda lección): las otras ventanas de debug.

   SCENE HIERARCHY
   ---------------
   La jerarquía se dibuja con `TreeNodeEx` + `TreePop`. Para que sea usable:

   - IDs estables por entidad (`PushID(id)`), no por fila.
   - `ImGuiTreeNodeFlags_OpenOnArrow`: click en el texto selecciona, flecha
     abre/cierra. Sin esto, seleccionar abre el subárbol.
   - `ImGuiTreeNodeFlags_Leaf` para hojas (sin flecha).
   - `ImGuiTreeNodeFlags_Selected` para resaltar la selección.
   - Recursión con un `std::function` o una pila explícita. Cuidado con
     ciclos: el reparent debe rechazar un padre que sea descendiente.

   DRAG & DROP
   -----------
   El patrón de ImGui es:
       if (ImGui::BeginDragDropSource()) {
           ImGui::SetDragDropPayload("ENTIDAD", &id, sizeof id);
           ImGui::Text("Mover #%u", id);
           ImGui::EndDragDropSource();
       }
       if (ImGui::BeginDragDropTarget()) {
           if (auto* p = ImGui::AcceptDragDropPayload("ENTIDAD")) { ... }
           ImGui::EndDragDropTarget();
       }
   El payload se copia a un buffer interno de ImGui; no guardes punteros al
   stack. El target valida el tipo y el dueño decide si el reparent es legal.

   RESOURCE BROWSER
   ----------------
   Un explorador de assets necesita: filtro de texto, extensión, tamaño y
   estado de carga. `InputTextWithHint` + `Selectable` alcanza; para muchos
   ítems usá `ImGuiListClipper` (virtualiza filas visibles) en vez de emitir
   miles de widgets.

   COMPONENT EDITOR CON DIRTY
   --------------------------
   El editor de componentes es el mismo patrón que el inspector de la L03,
   pero separado por tipo. Para no marcar dirty en cada píxel del arrastre:

       ImGui::DragFloat("x", &v);
       if (ImGui::IsItemDeactivatedAfterEdit()) dirty = true;

   `IsItemDeactivatedAfterEdit` es true UNA vez al soltar, y sólo si el valor
   cambió de verdad. Es la diferencia entre un undo por gesto y cien undos.

   Sin ImGui, la demo headless recorre el árbol y aplica el filtro del
   resource browser sobre una lista de assets.
   ============================================================================= */
#include "include/gui_util.hpp"
#include "include/imgui_app.hpp"

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {

struct Nodo {
    std::uint32_t id = 0;
    std::string nombre;
    std::uint32_t padre = 0;
    std::vector<std::uint32_t> hijos;
    bool activo = true;
};

std::vector<Nodo> g_nodos = {
    {1, "Nivel", 0, {2, 5}, true},
    {2, "Enemigos", 1, {3, 4}, true},
    {3, "Slime A", 2, {}, true},
    {4, "Slime B", 2, {}, false},
    {5, "Decorado", 1, {6}, true},
    {6, "Antorcha", 5, {}, true},
};

struct Asset {
    std::string nombre;
    std::string tipo;
    int bytes = 0;
    bool cargado = false;
};

std::vector<Asset> g_assets = {
    {"jugador.png", "textura", 4096, true},
    {"slime.png", "textura", 2048, true},
    {"bosque.ogg", "audio", 480000, false},
    {"nivel01.tmj", "tilemap", 12000, true},
    {"fuente.ttf", "font", 90000, false},
};

[[maybe_unused]] std::uint32_t g_seleccion = 1;
[[maybe_unused]] bool g_dirty = false;
[[maybe_unused]] char g_filtro[64] = "";

const Nodo* buscar(std::uint32_t id) {
    for (const auto& n : g_nodos) if (n.id == id) return &n;
    return nullptr;
}

// ¿`candidato` es descendiente de `raiz`? Sirve para rechazar ciclos al
// reparentar: no podés colgar un nodo de su propio hijo.
bool esDescendiente(std::uint32_t candidato, std::uint32_t raiz) {
    if (candidato == raiz) return true;
    const Nodo* n = buscar(candidato);
    if (!n || n->padre == 0) return false;
    return esDescendiente(n->padre, raiz);
}

#if M24_HAS_IMGUI
// Recorre el árbol desde la raíz. `padre` filtra los hijos directos.
void dibujarNodo(std::uint32_t id) {
    const Nodo* n = buscar(id);
    if (!n) return;
    ImGui::PushID(static_cast<int>(id));

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
                               ImGuiTreeNodeFlags_SpanAvailWidth;
    if (n->hijos.empty()) flags |= ImGuiTreeNodeFlags_Leaf;
    if (g_seleccion == id) flags |= ImGuiTreeNodeFlags_Selected;

    const bool abierto = ImGui::TreeNodeEx(n->nombre.c_str(), flags);
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) g_seleccion = id;

    if (ImGui::BeginDragDropSource()) {
        ImGui::SetDragDropPayload("ENTIDAD", &n->id, sizeof(n->id));
        ImGui::Text("Mover %s", n->nombre.c_str());
        ImGui::EndDragDropSource();
    }
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* p = ImGui::AcceptDragDropPayload("ENTIDAD")) {
            const auto arrastrado = *static_cast<const std::uint32_t*>(p->Data);
            if (!esDescendiente(n->id, arrastrado)) {
                g_dirty = true;
                std::printf("[jerarquia] reparent %u -> %u\n", arrastrado, n->id);
            }
        }
        ImGui::EndDragDropTarget();
    }

    if (abierto) {
        for (std::uint32_t h : n->hijos) dibujarNodo(h);
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void dibujarUI() {
    ImGui::SetNextWindowPos(ImVec2(24.0f, 24.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(360.0f, 380.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("M24 / L04 - Scene hierarchy");
    dibujarNodo(1);
    ImGui::Separator();
    ImGui::Text("dirty = %s", g_dirty ? "true" : "false");
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(400.0f, 24.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(430.0f, 380.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("M24 / L04 - Resource browser");
    ImGui::InputTextWithHint("##filtro", "filtrar por nombre o tipo...", g_filtro,
                             sizeof(g_filtro));
    for (const auto& a : g_assets) {
        if (!gui::containsNoCase(a.nombre, g_filtro) &&
            !gui::containsNoCase(a.tipo, g_filtro)) {
            continue;
        }
        ImGui::PushID(a.nombre.c_str());
        ImGui::Selectable(a.nombre.c_str(), false);
        ImGui::SameLine();
        ImGui::TextDisabled("%s  %d B  %s", a.tipo.c_str(), a.bytes,
                            a.cargado ? "cargado" : "pendiente");
        ImGui::PopID();
    }
    ImGui::End();
}
#endif

} // namespace

int main() {
    std::puts("M24 / L04 - Scene hierarchy + resource browser");
    std::puts("----------------------------------------------");

#if M24_HAS_IMGUI
    return gui::runImGuiDemo("M24 L04 - jerarquia y assets", dibujarUI);
#endif

    std::puts("ImGui no disponible: demo headless.");
    std::puts("");
    std::puts("Arbol:");
    std::printf("  Nivel #1\n");
    std::printf("    Enemigos #2\n");
    std::printf("      Slime A #3 (activo)\n");
    std::printf("      Slime B #4 (inactivo)\n");
    std::printf("    Decorado #5\n");
    std::printf("      Antorcha #6\n");
    std::puts("");
    std::puts("Reparent legal? #3 bajo #5:");
    std::puts(esDescendiente(5, 3) ? "  NO (ciclo)" : "  SI");
    std::puts("Reparent legal? #5 bajo #3:");
    std::puts(esDescendiente(3, 5) ? "  NO (ciclo)" : "  SI");
    std::puts("");
    std::puts("Resource browser con filtro \"tex\":");
    for (const auto& a : g_assets) {
        if (gui::containsNoCase(a.nombre, "tex") || gui::containsNoCase(a.tipo, "tex")) {
            std::printf("  %-14s %-8s %7d B  %s\n", a.nombre.c_str(), a.tipo.c_str(),
                        a.bytes, a.cargado ? "cargado" : "pendiente");
        }
    }
    return 0;
}
