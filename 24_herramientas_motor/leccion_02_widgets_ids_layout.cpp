/* =============================================================================
   MÓDULO 24 - LECCIÓN 02: Widgets, IDs y layout
   =============================================================================
   TEMA 1 de TEMAS.md (segunda lección): la práctica de ImGui: widgets, el
   stack de IDs, tablas y layout.

   EL ID ES LA CLAVE
   -----------------
   ImGui no guarda punteros a widgets: guarda estado por ID (¿está abierto el
   TreeNode?, ¿cuál Slider está arrastrando el mouse?, ¿qué InputText tiene
   foco?). El ID se calcula hasheando:

       ventana actual + PushID(...) + label del widget

   Consecuencia: dos widgets con el MISMO label en el mismo scope colisionan.
   El segundo hereda el estado del primero (el clásico "dos botones se mueven
   juntos"). Soluciones:

       1. Etiquetas distintas: "Vida##jugador1" y "Vida##jugador2".
          Lo que va después de ## se usa para el ID pero NO se dibuja.
       2. PushID(i) / PopID() alrededor de cada iteración. Es lo correcto en
          bucles y en listas de entidades.
       3. PushID(ptr) para objetos con dirección estable.

   `ImGui::GetID("texto")` te da el ID calculado sin dibujar nada: útil para
   storage propio, drag & drop y tests.

   WIDGETS QUE VAS A USAR TODO EL TIEMPO
   -------------------------------------
   - `Text` / `TextWrapped` / `TextColored`: salida.
   - `Button`, `SmallButton`, `Selectable`: devuelven true en el frame del click.
   - `Checkbox("God", &bool)`, `SliderFloat`, `DragFloat`: editan in-place y
     devuelven true si el valor cambió.
   - `InputText("Nombre", buf, size)`: buffer de tamaño FIJO; ImGui no reserva.
   - `BeginCombo` / `Selectable` / `EndCombo`: desplegable.
   - `TreeNodeEx` / `TreePop`: jerarquía plegable.
   - `BeginTable` / `TableSetupColumn` / `TableNextRow` / `TableSetColumnIndex`
     / `EndTable`: tablas con headers, bordes y row background.

   LAYOUT
   ------
   ImGui apila widgets verticalmente. `SameLine()` pone el siguiente al lado;
   `Spacing()`, `Separator()` y `Dummy()` crean aire; `SetNextItemWidth()`
   limita el ancho del próximo widget. El layout es inmediato: si cambiás el
   orden de las llamadas, cambia el resultado en el próximo frame.

   REGLA DE ORO: el código de UI no debe contener lógica de juego. Los widgets
   editan el estado y devuelven "cambió"; el resto del motor reacciona a ese
   estado. En la lección 03 ese estado es una escena ECS.

   Sin ImGui, la demo headless muestra cómo el stack de IDs separa labels
   repetidos (misma cuenta que hace ImGui, con FNV-1a).
   ============================================================================= */
#include "include/gui_util.hpp"
#include "include/imgui_app.hpp"

#include <cstdio>
#include <cstring>

namespace {

struct Estado {
    char nombre[64] = "Jugador";
    float velocidad = 3.5f;
    bool godMode = false;
    int arma = 0;
    int seleccion = 1;
};

Estado g;

#if M24_HAS_IMGUI
void dibujarUI() {
    ImGui::SetNextWindowPos(ImVec2(24.0f, 24.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(620.0f, 430.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("M24 / L02 - widgets, IDs y layout");

    ImGui::InputText("Nombre", g.nombre, sizeof(g.nombre));
    ImGui::DragFloat("Velocidad", &g.velocidad, 0.1f, 0.0f, 20.0f, "%.1f m/s");
    ImGui::Checkbox("God mode", &g.godMode);

    const char* armas[] = {"Pistola", "Escopeta"};
    if (ImGui::BeginCombo("Arma", armas[g.arma])) {
        for (int i = 0; i < 2; ++i) {
            if (ImGui::Selectable(armas[i], g.arma == i)) g.arma = i;
        }
        ImGui::EndCombo();
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Entidades (el PushID separa los IDs):");
    if (ImGui::BeginTable("entidades", 3,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("Entidad");
        ImGui::TableSetupColumn("Vida");
        ImGui::TableSetupColumn("Estado");
        ImGui::TableHeadersRow();

        for (int i = 0; i < 3; ++i) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::PushID(i);  // sin esto, los tres "Enemigo" comparten estado
            char label[32];
            std::snprintf(label, sizeof(label), "Enemigo %d", i);
            if (ImGui::Selectable(label, g.seleccion == i)) g.seleccion = i;
            ImGui::PopID();
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%d", 100 - i * 25);
            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(i == 1 ? "herido" : "ok");
        }
        ImGui::EndTable();
    }

    ImGui::Separator();
    ImGui::Text("Seleccionado: indice %d, arma %s, god=%s", g.seleccion, armas[g.arma],
                g.godMode ? "si" : "no");
    ImGui::TextWrapped("Probar: sin PushID los tres Selectable 'Enemigo' comparten "
                       "estado. Con PushID(i) cada fila tiene su propio ID.");
    ImGui::End();
}
#endif

} // namespace

int main() {
    std::puts("M24 / L02 - Widgets, IDs y layout");
    std::puts("--------------------------------");

#if M24_HAS_IMGUI
    return gui::runImGuiDemo("M24 L02 - widgets e IDs", dibujarUI);
#endif

    // Demo headless: el mismo cálculo de IDs que hace ImGui (FNV-1a con seed
    // del scope). Demuestra por qué PushID evita colisiones.
    std::puts("ImGui no disponible: demo headless del stack de IDs.");
    std::puts("");
    std::uint32_t raiz = gui::hashId("Ventana/Entidades");
    std::printf("ID de la ventana: %08x\n", raiz);
    std::printf("%-28s %-10s %s\n", "scope", "label", "ID resultante");
    for (int i = 0; i < 3; ++i) {
        std::uint32_t scope = gui::hashId("Enemigo", raiz ^ gui::hashId(std::to_string(i)));
        std::uint32_t id = gui::hashId("Enemigo", scope);
        std::printf("PushID(%d)%*s %-10s %08x\n", i, 19, "", "Enemigo", id);
    }
    std::uint32_t sinPush = gui::hashId("Enemigo", raiz);
    std::printf("%-28s %-10s %08x\n", "(sin PushID)", "Enemigo", sinPush);
    std::puts("");
    std::puts("Los tres primeros IDs son distintos; el ultimo seria compartido.");
    std::puts("En ImGui eso se traduce en: hover/foco/estado por fila correctos.");
    return 0;
}
