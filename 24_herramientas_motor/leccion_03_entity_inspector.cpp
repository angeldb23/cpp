/* =============================================================================
   MÓDULO 24 - LECCIÓN 03: Entity inspector para un ECS
   =============================================================================
   TEMA 2 de TEMAS.md (primera lección): "Ventanas de debug: entity inspector
   (ECS), component editor, resource browser y scene hierarchy".

   EL PROBLEMA
   -----------
   Tenés 10.000 entidades y querés ver/editar la seleccionada sin abrir un
   debugger. El inspector es una ventana que muestra los componentes de UNA
   entidad y permite editarlos en vivo.

   La dificultad real no es ImGui: es que un ECS guarda componentes en arrays
   paralelos (SoA) y no tiene una lista de "campos" por entidad. Hay tres
   estrategias:

   1. Reflexión manual: un `if` por componente y por campo.
      Simple, cero magia, se desincroniza cuando agregás un campo.
   2. Reflexión generada: macros/annotations o un generador de código que
      produce la tabla de campos. Es lo que usan motores grandes.
   3. Type erasure con visitors: un `edit(Componente&)` por componente; el
      inspector recorre una lista de editores registrados.

   En este curso usamos la 1 y la 3: la máscara de componentes decide qué
   paneles mostrar, y cada panel conoce sus campos.

   MÁSCARA DE COMPONENTES
   ----------------------
   `mask & kTransform` es barato y cache-friendly. El inspector sólo consulta
   las entidades que tienen el bit. Editar un campo NO cambia la máscara;
   agregar/quitar un componente SÍ (y puede requerir migrar de archetype).

   IDs ESTABLES
   ------------
   Cada entidad necesita un ID estable (generación + índice) y cada widget un
   PushID(entityId). Si usás el índice del vector como ID, al borrar una
   entidad el inspector termina editando a otra. Ver módulo 22 (ECS).

   EDICIÓN EN VIVO
   ---------------
   `DragFloat` devuelve true MIENTRAS se arrastra. Para no recalcular todo el
   mundo cada píxel:
       - marcá `dirty` sólo en `IsItemDeactivatedAfterEdit()` (al soltar), o
       - aplicá el cambio al estado y dejá que el sistema de render lo lea.
   En el inspector conviene lo primero para no inundar el undo/redo.

   Sin ImGui, la demo headless muestra la consulta por máscara y el formato
   de filas del inspector.
   ============================================================================= */
#include "include/gui_util.hpp"
#include "include/imgui_app.hpp"

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {

enum ComponentBit : std::uint32_t {
    kTransform = 1u << 0,
    kHealth = 1u << 1,
    kSprite = 1u << 2,
    kAI = 1u << 3,
};

struct Entidad {
    std::uint32_t id = 0;
    std::string nombre;
    std::uint32_t mascara = 0;
    gui::Vec3 posicion;
    float vida = 100.0f;
    float vidaMax = 100.0f;
    int sprite = 0;
    bool hostil = false;
};

std::vector<Entidad> g_entidades = {
    {1, "Jugador", kTransform | kHealth | kSprite, {0.0f, 0.0f, 0.0f}, 80.0f, 100.0f, 3, false},
    {2, "Slime", kTransform | kHealth | kSprite | kAI, {4.0f, 0.0f, 0.0f}, 25.0f, 40.0f, 7, true},
    {3, "Antorcha", kTransform | kSprite, {1.5f, 0.0f, 0.0f}, 0.0f, 0.0f, 12, false},
    {4, "Boss", kTransform | kHealth | kAI, {-6.0f, 0.0f, 0.0f}, 500.0f, 500.0f, 0, true},
};

[[maybe_unused]] int g_seleccion = 0;

// Consulta típica de un sistema ECS: todas las entidades que tengan TODOS
// los bits pedidos. La usan el inspector y los sistemas de juego.
std::vector<std::uint32_t> consultar(std::uint32_t requeridos) {
    std::vector<std::uint32_t> out;
    for (const auto& e : g_entidades) {
        if ((e.mascara & requeridos) == requeridos) out.push_back(e.id);
    }
    return out;
}

const char* nombreComponente(std::uint32_t bit) {
    switch (bit) {
        case kTransform: return "Transform";
        case kHealth: return "Health";
        case kSprite: return "Sprite";
        case kAI: return "AI";
        default: return "?";
    }
}

#if M24_HAS_IMGUI
void dibujarUI() {
    ImGui::SetNextWindowPos(ImVec2(24.0f, 24.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(460.0f, 480.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("M24 / L03 - Entity inspector");

    ImGui::TextUnformatted("Entidades");
    for (std::size_t i = 0; i < g_entidades.size(); ++i) {
        Entidad& e = g_entidades[i];
        ImGui::PushID(static_cast<int>(e.id));
        const bool seleccionada = (g_seleccion == static_cast<int>(i));
        char label[96];
        std::snprintf(label, sizeof(label), "%03u  %s", e.id, e.nombre.c_str());
        if (ImGui::Selectable(label, seleccionada)) g_seleccion = static_cast<int>(i);
        ImGui::PopID();
    }

    ImGui::Separator();
    if (g_seleccion >= 0 && g_seleccion < static_cast<int>(g_entidades.size())) {
        Entidad& e = g_entidades[static_cast<std::size_t>(g_seleccion)];
        ImGui::PushID(static_cast<int>(e.id));
        ImGui::Text("ID %u  mascara 0x%02x", e.id, e.mascara);
        char nombre[64];
        std::snprintf(nombre, sizeof(nombre), "%s", e.nombre.c_str());
        if (ImGui::InputText("Nombre", nombre, sizeof(nombre))) e.nombre = nombre;

        if (e.mascara & kTransform) {
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::DragFloat3("Posicion", &e.posicion.x, 0.1f);
            }
        }
        if (e.mascara & kHealth) {
            if (ImGui::CollapsingHeader("Health", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::DragFloat("Vida", &e.vida, 1.0f, 0.0f, e.vidaMax);
                ImGui::DragFloat("Vida max", &e.vidaMax, 1.0f, 1.0f, 9999.0f);
            }
        }
        if (e.mascara & kSprite) {
            if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::DragInt("Frame", &e.sprite, 0.2f, 0, 64);
            }
        }
        if (e.mascara & kAI) {
            if (ImGui::CollapsingHeader("AI", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::Checkbox("Hostil", &e.hostil);
            }
        }
        ImGui::PopID();
    }

    ImGui::Separator();
    ImGui::Text("Con Transform+Health: %zu entidades", consultar(kTransform | kHealth).size());
    ImGui::TextWrapped("Cada fila y cada panel hacen PushID(id): la seleccion y "
                       "los widgets no se pisan al cambiar de entidad.");
    ImGui::End();
}
#endif

} // namespace

int main() {
    std::puts("M24 / L03 - Entity inspector (mascara de componentes)");
    std::puts("----------------------------------------------------");

#if M24_HAS_IMGUI
    return gui::runImGuiDemo("M24 L03 - entity inspector", dibujarUI);
#endif

    std::puts("ImGui no disponible: demo headless de la consulta por mascara.");
    std::puts("");
    std::uint32_t consultas[] = {kTransform, kTransform | kHealth, kTransform | kAI,
                                 kSprite | kAI};
    for (std::uint32_t m : consultas) {
        const auto ids = consultar(m);
        std::printf("mascara 0x%02x (%zu):", m, ids.size());
        for (auto id : ids) std::printf(" #%u", id);
        std::puts("");
    }
    std::puts("");
    std::puts("Filas del inspector para la entidad #2:");
    std::printf("  [%s] Transform  pos=(4.0, 0.0, 0.0)\n", nombreComponente(kTransform));
    std::printf("  [%s] Health     vida=25/40\n", nombreComponente(kHealth));
    std::printf("  [%s] Sprite     frame=7\n", nombreComponente(kSprite));
    std::printf("  [%s] AI         hostil=si\n", nombreComponente(kAI));
    return 0;
}
