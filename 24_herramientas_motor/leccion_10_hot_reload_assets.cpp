/* =============================================================================
   MÓDULO 24 - LECCIÓN 10: Hot-reload de assets
   =============================================================================
   TEMA 6 de TEMAS.md: "Hot-reload de assets: watchers, callback a resource
   manager y UI de estado de carga".

   QUÉ SIGNIFICA HOT-RELOAD
   ------------------------
   Editás `slime.png` en Aseprite, guardás, y el juego muestra la textura
   nueva sin reiniciar. Lo mismo con shaders, tilemaps o audio. El ciclo es:

       cambio en disco -> watcher -> debounce -> recarga -> swap -> UI

   WATCHERS
   --------
   Dos estrategias:

   1. Polling: cada N ms recorrés los archivos y comparás `last_write_time`
      (o tamaño + mtime). Portable, simple, sin dependencias; cuesta I/O.
      Con pocos assets y cada 250-500 ms es invisible.
   2. Notificación del SO: inotify en Linux, ReadDirectoryChangesW en Windows,
      FSEvents en macOS. Eficiente pero multiplataforma y con edge cases
      (renombres atómicos: el editor escribe un temporal y lo mueve).

   Para el curso usamos polling: la lógica (`snapshot` + `diff`) es pura y
   testeable sin tocar el disco. El adaptador real usa `std::filesystem`.

   DEBOUNCE
   --------
   Guardar en un editor puede disparar varios eventos seguidos (temporal,
   rename, escritura). Si recargás en cada uno, leés archivos a medio escribir.
   Debounce: esperá X ms desde el PRIMER evento y recién ahí procesá. Si el
   mismo archivo vuelve a cambiar, NO reinicies el timer (coalescencia), o
   vas a esperar para siempre mientras el artista sigue guardando.

   CALLBACK AL RESOURCE MANAGER
   ----------------------------
   El watcher no sabe recargar texturas. Emite eventos:
       {path, Added | Modified | Removed}
   y el resource manager decide: buscar el handle, recargar, invalidar la
   GPU, notificar a los sistemas. Si el asset está en uso, el swap debe ser
   seguro: cargar el nuevo recurso, cambiar el handle y liberar el viejo
   cuando nadie lo referencia (módulo 22, L07).

   UI DE ESTADO
   ------------
   Una ventana con: archivo, estado (ok/pendiente/error), tiempo de la última
   carga y mensaje de error. El hot-reload falla silenciosamente si no ves el
   error: "shader compilado con warnings" tiene que aparecer en la consola.

   CUIDADO
   -------
   No recargues en medio de un frame que está usando el recurso. Encolá el
   evento y aplicá el swap al inicio del frame siguiente (o en un punto
   seguro). Y no watchees la carpeta de builds ni los logs.
   ============================================================================= */
#include "include/gui_util.hpp"
#include "include/imgui_app.hpp"

#include <cstdint>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

namespace {

enum class Cambio { Agregado, Modificado, Borrado };

const char* nombreCambio(Cambio c) {
    switch (c) {
        case Cambio::Agregado: return "agregado";
        case Cambio::Modificado: return "modificado";
        case Cambio::Borrado: return "borrado";
    }
    return "?";
}

struct Evento {
    std::string path;
    Cambio cambio = Cambio::Modificado;
};

using Snapshot = std::map<std::string, std::int64_t>;

// Diff puro de dos fotos del directorio. Ordenado por path para que la UI y
// los tests sean deterministas.
std::vector<Evento> diffSnapshots(const Snapshot& antes, const Snapshot& despues) {
    std::vector<Evento> out;
    for (const auto& [path, mtime] : despues) {
        auto it = antes.find(path);
        if (it == antes.end()) {
            out.push_back({path, Cambio::Agregado});
        } else if (it->second != mtime) {
            out.push_back({path, Cambio::Modificado});
        }
    }
    for (const auto& [path, mtime] : antes) {
        (void)mtime;
        if (despues.find(path) == despues.end()) {
            out.push_back({path, Cambio::Borrado});
        }
    }
    return out;
}

class Debouncer {
public:
    explicit Debouncer(double cooldownSegundos) : cooldown_(cooldownSegundos) {}

    // El primer aviso arma el timer; los siguientes NO lo reinician: queremos
    // coalescer la ráfaga de guardado, no esperar a que el artista pare.
    void notificar(const std::string& path, double ahora) {
        pendientes_.emplace(path, ahora);
    }

    std::vector<std::string> listos(double ahora) {
        std::vector<std::string> out;
        for (auto it = pendientes_.begin(); it != pendientes_.end();) {
            if (ahora - it->second >= cooldown_) {
                out.push_back(it->first);
                it = pendientes_.erase(it);
            } else {
                ++it;
            }
        }
        return out;
    }

    std::size_t pendientes() const { return pendientes_.size(); }

private:
    double cooldown_;
    std::map<std::string, double> pendientes_;
};

enum class Estado { Ok, Pendiente, Error };

struct Slot {
    std::string path;
    Estado estado = Estado::Ok;
    double ultimaCarga = 0.0;
    std::string mensaje;
};

std::map<std::string, Slot> g_slots;

Debouncer g_debounce(0.25);

void sembrar() {
    g_slots["jugador.png"] = {"jugador.png", Estado::Ok, 1.0, ""};
    g_slots["bosque.ogg"] = {"bosque.ogg", Estado::Ok, 1.2, ""};
    g_slots["shader_luz.glsl"] = {"shader_luz.glsl", Estado::Ok, 1.5, ""};
}

void simular() {
    Snapshot antes = {{"jugador.png", 100}, {"bosque.ogg", 200}, {"shader_luz.glsl", 300}};
    Snapshot despues = {{"jugador.png", 101}, {"bosque.ogg", 200},
                        {"shader_luz.glsl", 301}, {"nivel02.tmj", 400}};

    const auto eventos = diffSnapshots(antes, despues);
    std::puts("Eventos detectados:");
    for (const auto& e : eventos) {
        std::printf("  %-18s %s\n", e.path.c_str(), nombreCambio(e.cambio));
        g_slots[e.path] = {e.path, Estado::Pendiente, 0.0, ""};
        g_debounce.notificar(e.path, 10.0);
        g_debounce.notificar(e.path, 10.05);  // ráfaga del editor: no reinicia
        g_debounce.notificar(e.path, 10.10);
    }

    std::puts("");
    std::printf("Pendientes en t=10.20: %zu\n", g_debounce.pendientes());
    const auto listos = g_debounce.listos(10.30);
    std::printf("Listos en t=10.30: %zu\n", listos.size());
    for (const auto& p : listos) {
        g_slots[p].estado = (p == "shader_luz.glsl") ? Estado::Error : Estado::Ok;
        g_slots[p].ultimaCarga = 10.30;
        g_slots[p].mensaje = (p == "shader_luz.glsl") ? "linea 12: falta ;" : "";
    }
}

#if M24_HAS_IMGUI
void dibujarUI() {
    ImGui::SetNextWindowPos(ImVec2(24.0f, 24.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(560.0f, 320.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("M24 / L10 - Hot reload");
    ImGui::TextUnformatted("Assets vigilados (polling + debounce 250 ms):");
    if (ImGui::BeginTable("assets", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Archivo");
        ImGui::TableSetupColumn("Estado");
        ImGui::TableSetupColumn("Ultima carga");
        ImGui::TableSetupColumn("Mensaje");
        ImGui::TableHeadersRow();
        for (const auto& [path, slot] : g_slots) {
            (void)path;
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(slot.path.c_str());
            ImGui::TableSetColumnIndex(1);
            ImVec4 color(0.5f, 0.9f, 0.5f, 1.0f);
            if (slot.estado == Estado::Pendiente) color = ImVec4(1.0f, 0.8f, 0.3f, 1.0f);
            if (slot.estado == Estado::Error) color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
            ImGui::TextColored(color, "%s", slot.estado == Estado::Ok ? "ok" :
                                              slot.estado == Estado::Pendiente ? "pendiente" :
                                                                                "error");
            ImGui::TableSetColumnIndex(2);
            if (slot.ultimaCarga > 0.0) ImGui::Text("%.2f s", slot.ultimaCarga);
            else ImGui::TextUnformatted("-");
            ImGui::TableSetColumnIndex(3); ImGui::TextUnformatted(slot.mensaje.c_str());
        }
        ImGui::EndTable();
    }
    ImGui::TextWrapped("El swap real se aplica al inicio del frame siguiente. El watcher "
                       "solo emite eventos; el resource manager decide como recargar.");
    ImGui::End();
}
#endif

} // namespace

int main() {
    sembrar();
    simular();
    std::puts("");
    for (const auto& [path, slot] : g_slots) {
        std::printf("%-18s %-9s ultima=%.2f %s\n", path.c_str(),
                    slot.estado == Estado::Ok ? "ok" :
                    slot.estado == Estado::Pendiente ? "pendiente" : "error",
                    slot.ultimaCarga, slot.mensaje.c_str());
    }

#if M24_HAS_IMGUI
    return gui::runImGuiDemo("M24 L10 - hot reload", dibujarUI);
#else
    return 0;
#endif
}
