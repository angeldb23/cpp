/* =============================================================================
   MÓDULO 24 - LECCIÓN 09: Logging y consola en pantalla
   =============================================================================
   TEMA 5 de TEMAS.md: "Logging y consola: ring buffer, filtros por
   nivel/categoria, busqueda y export".

   POR QUÉ UN RING BUFFER
   ----------------------
   Un juego puede loguear miles de líneas por minuto. Guardar todo en un
   `std::vector` hace crecer la memoria sin techo. El ring buffer mantiene las
   últimas N entradas y descarta las más viejas: memoria acotada y las líneas
   recientes (las que importan cuando algo falla) siempre están.

   `std::deque` es la opción simple: `push_back` + `pop_front` O(1). Si querés
   cero allocaciones por línea, usá un array circular fijo con un índice de
   escritura y un contador. Para una consola de debug, `deque` alcanza.

   NIVELES Y CATEGORÍAS
   --------------------
   Nivel: Trace < Debug < Info < Warn < Error. El filtro "mínimo" muestra ese
   nivel y los superiores. Categoría: "render", "fisica", "audio", "script".
   Se combinan: `nivel >= minimo && (categoria == filtro || filtro vacío)`.
   La búsqueda de texto se hace DESPUÉS de esos filtros y sin distinguir
   mayúsculas (si no, "Error" no encuentra "error").

   ORDEN Y EXPORT
   --------------
   Las entradas se muestran en orden cronológico (viejo -> nuevo). El export
   suele ser texto plano:

       [E][render] shader no compila
       [W][audio] no se encontro bosque.ogg

   Un número de secuencia monotónico ayuda a ordenar y a deduplicar cuando
   varias hebras escriben. Si el log es multihilo, el buffer necesita un mutex
   y el export toma una copia bajo lock (módulo 14).

   CONSOLA INTERACTIVA
   -------------------
   Además de mostrar, una consola acepta comandos: `help`, `clear`, `filter
   render`, `level warn`. El parser separa por espacios y el comando devuelve
   una respuesta que también se loguea. El historial de comandos con flecha
   arriba es un ring buffer de strings.

   COSTO
   -----
   Formatear un string por cada log es caro. En release se compilan fuera los
   niveles bajos (`#ifdef NDEBUG`) o se usa un logger con niveles. No loguees
   dentro del loop interno de partículas.
   ============================================================================= */
#include "include/gui_util.hpp"
#include "include/imgui_app.hpp"

#include <cstdint>
#include <cstdio>
#include <deque>
#include <string>
#include <vector>

namespace {

enum class Nivel : int { Trace = 0, Debug = 1, Info = 2, Warn = 3, Error = 4 };

const char* nombreNivel(Nivel n) {
    switch (n) {
        case Nivel::Trace: return "T";
        case Nivel::Debug: return "D";
        case Nivel::Info: return "I";
        case Nivel::Warn: return "W";
        case Nivel::Error: return "E";
    }
    return "?";
}

struct Entrada {
    Nivel nivel = Nivel::Info;
    std::string categoria;
    std::string texto;
    std::uint64_t secuencia = 0;
};

class Consola {
public:
    explicit Consola(std::size_t capacidad) : capacidad_(capacidad) {}

    void log(Nivel n, std::string categoria, std::string texto) {
        entradas_.push_back({n, std::move(categoria), std::move(texto), ++secuencia_});
        while (entradas_.size() > capacidad_) entradas_.pop_front();
    }

    void setNivelMinimo(Nivel n) { minimo_ = n; }
    void setCategoria(std::string c) { categoria_ = std::move(c); }

    std::vector<Entrada> visibles(std::string_view busqueda) const {
        std::vector<Entrada> out;
        for (const auto& e : entradas_) {
            if (static_cast<int>(e.nivel) < static_cast<int>(minimo_)) continue;
            if (!categoria_.empty() && e.categoria != categoria_) continue;
            if (!busqueda.empty() && !gui::containsNoCase(e.texto, busqueda) &&
                !gui::containsNoCase(e.categoria, busqueda)) {
                continue;
            }
            out.push_back(e);
        }
        return out;
    }

    std::string exportar() const {
        std::string out;
        for (const auto& e : entradas_) {
            out += "[";
            out += nombreNivel(e.nivel);
            out += "][";
            out += e.categoria;
            out += "] ";
            out += e.texto;
            out += "\n";
        }
        return out;
    }

    std::size_t size() const { return entradas_.size(); }
    std::size_t capacidad() const { return capacidad_; }

private:
    std::size_t capacidad_;
    std::deque<Entrada> entradas_;
    Nivel minimo_ = Nivel::Trace;
    std::string categoria_;
    std::uint64_t secuencia_ = 0;
};

Consola g_consola(8);
[[maybe_unused]] char g_busqueda[64] = "";
[[maybe_unused]] int g_nivel = 0;
[[maybe_unused]] char g_categoria[32] = "";

void sembrar() {
    g_consola.log(Nivel::Info, "render", "OpenGL 3.3 core listo");
    g_consola.log(Nivel::Debug, "fisica", "broad-phase: 42 pares");
    g_consola.log(Nivel::Warn, "audio", "no se encontro bosque.ogg");
    g_consola.log(Nivel::Error, "render", "Shaders no compilan: falta #version");
    g_consola.log(Nivel::Info, "script", "lua: nivel cargado");
    g_consola.log(Nivel::Trace, "fisica", "contacto #12 normal (0,1)");
}

#if M24_HAS_IMGUI
void dibujarUI() {
    ImGui::SetNextWindowPos(ImVec2(24.0f, 24.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(680.0f, 420.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("M24 / L09 - Consola");

    ImGui::SetNextItemWidth(220.0f);
    if (ImGui::InputTextWithHint("##buscar", "buscar...", g_busqueda, sizeof(g_busqueda))) {
        // el filtro se aplica al dibujar; no hace falta reconstruir nada
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f);
    ImGui::Combo("nivel", &g_nivel, "Trace\0Debug\0Info\0Warn\0Error\0");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(120.0f);
    ImGui::InputTextWithHint("##cat", "categoria", g_categoria, sizeof(g_categoria));

    if (ImGui::Button("Exportar")) {
        std::printf("%s", g_consola.exportar().c_str());
    }
    ImGui::SameLine();
    if (ImGui::Button("Limpiar")) {
        // una consola real tendria clear(); aca se muestra el boton
    }

    ImGui::Separator();
    g_consola.setNivelMinimo(static_cast<Nivel>(g_nivel));
    g_consola.setCategoria(g_categoria);
    const auto filas = g_consola.visibles(g_busqueda);
    for (const auto& e : filas) {
        ImVec4 color(0.8f, 0.8f, 0.8f, 1.0f);
        if (e.nivel == Nivel::Warn) color = ImVec4(1.0f, 0.8f, 0.2f, 1.0f);
        if (e.nivel == Nivel::Error) color = ImVec4(1.0f, 0.35f, 0.35f, 1.0f);
        if (e.nivel == Nivel::Debug) color = ImVec4(0.55f, 0.75f, 1.0f, 1.0f);
        ImGui::TextColored(color, "[%s][%s] %s", nombreNivel(e.nivel), e.categoria.c_str(),
                           e.texto.c_str());
    }
    ImGui::Separator();
    ImGui::Text("%zu/%zu entradas | %zu visibles", g_consola.size(), g_consola.capacidad(),
                filas.size());
    ImGui::End();
}
#endif

} // namespace

int main() {
    sembrar();
    std::puts("M24 / L09 - Consola con ring buffer");
    std::puts("-----------------------------------");

    std::printf("Capacidad %zu, entradas %zu\n", g_consola.capacidad(), g_consola.size());
    std::puts("");
    std::puts("Visibles con minimo Warn:");
    g_consola.setNivelMinimo(Nivel::Warn);
    for (const auto& e : g_consola.visibles("")) {
        std::printf("  [%s][%s] %s\n", nombreNivel(e.nivel), e.categoria.c_str(),
                    e.texto.c_str());
    }
    std::puts("");
    std::puts("Busqueda \"SHADER\" (sin distinguir mayusculas):");
    g_consola.setNivelMinimo(Nivel::Trace);
    for (const auto& e : g_consola.visibles("SHADER")) {
        std::printf("  [%s][%s] %s\n", nombreNivel(e.nivel), e.categoria.c_str(),
                    e.texto.c_str());
    }
    std::puts("");
    std::puts("Export:");
    std::printf("%s", g_consola.exportar().c_str());

#if M24_HAS_IMGUI
    return gui::runImGuiDemo("M24 L09 - consola", dibujarUI);
#else
    return 0;
#endif
}
