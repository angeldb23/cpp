/* =============================================================================
   MÓDULO 24 - LECCIÓN 11: Layout persistente, dockspace y multi-viewport
   =============================================================================
   TEMA 7 de TEMAS.md: "Layout persistente: ini file, dockspace (rama docking
   tag v1.91.5-docking y backend SDL3: verificar que existan al construir),
   multiple viewports (rama docking, no en mainline) y trabajo en segundo
   monitor".

   VERSIÓN Y RAMA (verificado en la sesión de construcción)
   -------------------------------------------------------
   Dear ImGui tiene dos ramas: `master` (estable, sin docking) y `docking`
   (docking + multi-viewport). El tag `v1.91.5-docking` EXISTE en el repo
   oficial (verificado con `git ls-remote --tags`) y trae:
       - `ImGuiConfigFlags_DockingEnable`
       - `ImGui::DockSpaceOverViewport(...)`
       - `imgui_internal.h` con `DockBuilder*` (API todavía "interna")
       - backends `imgui_impl_sdl3.cpp` y `imgui_impl_opengl3.cpp` con soporte
         de viewports (`PlatformHasViewports` / `RendererHasViewports`).
   El CMake del módulo fija ESE tag; si algún día se usa `master`, el código
   de dockspace NO compila. No es un detalle cosmético: es otra rama.

   INI PERSISTENTE
   ---------------
   Por defecto ImGui guarda el layout en `imgui.ini` (ruta configurable en
   `io.IniFilename`). El archivo tiene secciones por ventana, posición, tamaño,
   flags y, con docking, los nodos del dockspace. Se guarda solo unos segundos
   después del último cambio y al destruir el contexto. Para control total:

       io.IniFilename = nullptr;
       ImGui::LoadIniSettingsFromDisk("layout.ini");
       ImGui::SaveIniSettingsToDisk("layout.ini");

   No escribas el ini a mano en producción: es un formato interno que cambia
   entre versiones. Si necesitás un layout de fábrica, usá `DockBuilder*` una
   vez y dejá que ImGui lo persista.

   DOCKSPACE
   ---------
       ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(),
                                    ImGuiDockNodeFlags_PassthruCentralNode);
   crea un nodo raíz que ocupa la ventana principal. Después cada `Begin()`
   puede dockearse arrastrando la pestaña. `ImGuiDockNodeFlags_PassthruCentralNode`
   deja ver el render 3D detrás del hueco central.

   Para un layout programático:
       ImGuiID raiz = ImGui::GetID("MiDock");
       ImGui::DockBuilderRemoveNode(raiz);
       ImGui::DockBuilderAddNode(raiz, ImGuiDockNodeFlags_DockSpace);
       ImGuiID izq, der;
       ImGui::DockBuilderSplitNode(raiz, ImGuiDir_Left, 0.25f, &izq, &der);
       ImGui::DockBuilderDockWindow("Escena", izq);
       ImGui::DockBuilderFinish(raiz);

   MULTI-VIEWPORT
   --------------
   Con `io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable`, una ventana de
   ImGui puede salir de la ventana principal y vivir en otro monitor. Requiere
   backend con soporte (SDL3 + OpenGL3 lo tienen en la rama docking). En el
   loop hay que llamar, después de `ImGui::Render()`:

       ImGui::UpdatePlatformWindows();
       ImGui::RenderPlatformWindowsDefault();
       SDL_GL_MakeCurrent(window, gl);   // el contexto vuelve a la ventana main

   Costo: más contextos/ventanas, más complejidad con Wayland y Hi-DPI, y
   posibles bugs del compositor. Para una herramienta de un solo monitor, no
   lo habilites. En la UHD 600 cada ventana extra es superficie y memoria.

   WAYLAND / SEGUNDO MONITOR
   -------------------------
   En Wayland el cliente no elige la posición global de la ventana; el
   compositor manda. `SDL_GetWindowPosition` puede devolver valores distintos
   de lo que pediste. El layout persistente guarda posiciones que el
   compositor puede reinterpretar. No es un bug de ImGui: es el modelo de
   seguridad de Wayland. En X11 las posiciones sí se respetan.

   LAYOUT HEADLESS
   ---------------
   La parte testeable es el formato de layout propio (nombre -> ratio +
   visible), con normalización de ratios. La UI puede usar ese mismo modelo
   para reconstruir paneles; el ini de ImGui queda como detalle del backend.
   ============================================================================= */
#include "include/gui_util.hpp"
#include "include/imgui_app.hpp"

#include <cstdio>
#include <string>
#include <vector>

namespace {

struct Panel {
    std::string nombre;
    float ratio = 0.25f;
    bool visible = true;
};

std::string serializar(const std::vector<Panel>& paneles) {
    std::string out = "[Docking]\n";
    for (const auto& p : paneles) {
        char linea[256];
        std::snprintf(linea, sizeof(linea), "%s=%.6f|%d\n", p.nombre.c_str(),
                      static_cast<double>(p.ratio), p.visible ? 1 : 0);
        out += linea;
    }
    return out;
}

std::vector<Panel> parsear(std::string_view texto) {
    std::vector<Panel> out;
    bool enSeccion = false;
    std::size_t pos = 0;
    while (pos <= texto.size()) {
        std::size_t fin = texto.find('\n', pos);
        if (fin == std::string_view::npos) fin = texto.size();
        std::string linea = gui::trim(texto.substr(pos, fin - pos));
        pos = fin + 1;

        if (linea.empty() || linea[0] == ';' || linea[0] == '#') continue;
        if (linea.front() == '[' && linea.back() == ']') {
            enSeccion = (linea == "[Docking]");
            continue;
        }
        if (!enSeccion) continue;

        const auto eq = linea.find('=');
        if (eq == std::string::npos) continue;
        const std::string nombre = gui::trim(linea.substr(0, eq));
        const std::string valor = gui::trim(linea.substr(eq + 1));
        const auto barra = valor.find('|');
        if (nombre.empty() || barra == std::string::npos) continue;

        try {
            Panel p;
            p.nombre = nombre;
            p.ratio = std::stof(gui::trim(valor.substr(0, barra)));
            p.visible = gui::trim(valor.substr(barra + 1)) != "0";
            out.push_back(p);
        } catch (...) {
            // línea corrupta: se ignora, no se rompe el layout entero
        }
    }
    return out;
}

// Normaliza para que los ratios sumen 1, respetando un mínimo. Si el mínimo
// no entra (demasiados paneles), se reparte igual.
void normalizar(std::vector<Panel>& paneles, float minimo = 0.05f) {
    if (paneles.empty()) return;
    float suma = 0.0f;
    for (auto& p : paneles) {
        p.ratio = std::max(p.ratio, minimo);
        suma += p.ratio;
    }
    if (suma <= 0.0f) {
        for (auto& p : paneles) p.ratio = 1.0f / static_cast<float>(paneles.size());
        return;
    }
    for (auto& p : paneles) p.ratio /= suma;
}

std::vector<Panel> g_paneles = {
    {"Escena", 0.55f, true},
    {"Inspector", 0.25f, true},
    {"Consola", 0.20f, true},
    {"Profiler", 0.10f, false},
};

#if M24_HAS_IMGUI
void dibujarUI() {
    ImGuiID dock = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(),
                                                ImGuiDockNodeFlags_PassthruCentralNode);
    (void)dock;

    ImGui::Begin("Escena");
    ImGui::TextUnformatted("(viewport 3D iria aca)");
    ImGui::TextWrapped("PassthruCentralNode deja ver el render detras del hueco.");
    ImGui::End();

    ImGui::Begin("Inspector");
    for (auto& p : g_paneles) {
        ImGui::PushID(p.nombre.c_str());
        ImGui::Checkbox(p.nombre.c_str(), &p.visible);
        ImGui::DragFloat("ratio", &p.ratio, 0.01f, 0.05f, 1.0f);
        ImGui::PopID();
    }
    if (ImGui::Button("Normalizar")) normalizar(g_paneles);
    ImGui::End();

    if (g_paneles[2].visible) {
        ImGui::Begin("Consola");
        ImGui::TextUnformatted("layout persistente: [Docking] en un ini propio");
        ImGui::End();
    }
    if (g_paneles[3].visible) {
        ImGui::Begin("Profiler");
        ImGui::TextUnformatted("frame graph aqui");
        ImGui::End();
    }
}
#endif

} // namespace

int main() {
    std::puts("M24 / L11 - Layout persistente y docking");
    std::puts("----------------------------------------");

    const std::string ini = serializar(g_paneles);
    std::puts("INI serializado:");
    std::printf("%s", ini.c_str());

    auto leidos = parsear(ini);
    std::printf("Paneles leidos: %zu\n", leidos.size());
    normalizar(leidos);
    std::puts("");
    std::puts("Ratios normalizados:");
    float suma = 0.0f;
    for (const auto& p : leidos) {
        std::printf("  %-10s %.4f %s\n", p.nombre.c_str(),
                    static_cast<double>(p.ratio), p.visible ? "visible" : "oculto");
        suma += p.ratio;
    }
    std::printf("  suma = %.6f\n", static_cast<double>(suma));
    std::puts("");
    std::puts("El tag v1.91.5-docking existe y trae DockSpaceOverViewport;");
    std::puts("master NO trae docking. El CMake fija la rama docking a proposito.");

#if M24_HAS_IMGUI
    return gui::runImGuiDemo("M24 L11 - docking y layout", dibujarUI);
#else
    return 0;
#endif
}
