/* =============================================================================
   MÓDULO 24 - LECCIÓN 05: Profiling UI - frame graph, timers y spikes
   =============================================================================
   TEMA 3 de TEMAS.md (primera lección): "Profiling UI: frame graph (GPU/CPU),
   timers por sistema, memory alloc tracker y graficas en tiempo real".

   QUÉ MEDIR
   ---------
   Tres números distintos que la gente confunde:

   - Frame time: tiempo entre presents. Incluye vsync y espera. Es lo que
     siente el jugador (60 FPS = 16.67 ms).
   - CPU frame: lo que tardó tu código en preparar el frame.
   - GPU frame: lo que tardó la GPU. Se mide con timer queries (módulo 20,
     L17) y llega UN frame tarde: la GPU todavía no terminó cuando la CPU
     pide el resultado. Nunca hagas `glFinish` sólo para medir.

   `gpuMs > cpuMs` sostenido = cuello de botella en GPU. Al revés = CPU-bound.

   PROMEDIO NO ALCANZA
   -------------------
   Un promedio de 16 ms puede esconder un frame de 300 ms por segundo: se ve
   como un tirón aunque el número "esté bien". Por eso se miran percentiles y
   spikes:

   - p95 / p99: el 95%/99% de los frames tardó <= ese valor.
   - spike: frame que supera un umbral (p. ej. 2x el presupuesto de 16.67 ms).
   - max: el peor caso; útil para cazar el hitch exacto.

   Implementación simple y correcta de percentil: ordenar una copia y tomar
   el índice `ceil(p/100 * n) - 1` (nearest-rank). Para 300 muestras alcanza;
   para millones, histograma.

   TIMERS POR SISTEMA
   ------------------
   Un scope RAII mide y acumula:
       { ScopedTimer t("fisica"); step(); }
   En el frame, los tiempos por sistema se muestran como barras: es el "frame
   graph" de CPU. Con GPU, cada pase tiene su query; el frame graph muestra
   ambos y detecta dónde se va el tiempo.

   GRÁFICAS EN TIEMPO REAL
   -----------------------
   `ImGui::PlotLines("frame ms", datos, n, offset, ...)` dibuja un array de
   floats. El "ring buffer" de muestras se pasa como array circular con
   `values_offset`, o se copia a un vector lineal cada frame (más simple).
   `PlotHistogram` sirve para distribuciones (cuántos frames cayeron en cada
   bucket).

   CUIDADO CON LA MEDICIÓN
   -----------------------
   Medir cuesta. Un timer por sistema está bien; un timer por partícula no.
   En Debug, `std::chrono::steady_clock` es monótono y barato, pero no gratis.
   Y no perfiles con el profiler de la UI consumiendo el 20% del frame.
   ============================================================================= */
#include "include/gui_util.hpp"
#include "include/imgui_app.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <deque>
#include <string>
#include <vector>

namespace {

constexpr double kPresupuestoMs = 1000.0 / 60.0;

struct Muestra {
    double frameMs = 0.0;
    double cpuMs = 0.0;
    double gpuMs = 0.0;
};

class Perfil {
public:
    explicit Perfil(std::size_t capacidad) : capacidad_(capacidad) {}

    void agregar(Muestra m) {
        muestras_.push_back(m);
        while (muestras_.size() > capacidad_) muestras_.pop_front();
    }

    double promedioFrame() const {
        if (muestras_.empty()) return 0.0;
        double suma = 0.0;
        for (const auto& m : muestras_) suma += m.frameMs;
        return suma / static_cast<double>(muestras_.size());
    }

    // nearest-rank: ordena una copia y toma ceil(p*n)-1.
    double percentil(double p) const {
        if (muestras_.empty()) return 0.0;
        std::vector<double> v;
        v.reserve(muestras_.size());
        for (const auto& m : muestras_) v.push_back(m.frameMs);
        std::sort(v.begin(), v.end());
        p = gui::clampf(static_cast<float>(p), 0.0f, 100.0f);
        std::size_t idx = static_cast<std::size_t>(std::ceil(p / 100.0 * v.size()));
        if (idx == 0) idx = 1;
        return v[idx - 1];
    }

    std::size_t spikes(double umbralMs = kPresupuestoMs * 2.0) const {
        std::size_t n = 0;
        for (const auto& m : muestras_) if (m.frameMs > umbralMs) ++n;
        return n;
    }

    double maximo() const {
        double m = 0.0;
        for (const auto& x : muestras_) m = std::max(m, x.frameMs);
        return m;
    }

    const std::deque<Muestra>& muestras() const { return muestras_; }

private:
    std::size_t capacidad_;
    std::deque<Muestra> muestras_;
};

Perfil g_perfil(120);
std::vector<float> g_historia(120, 0.0f);

void sembrarDatos() {
    // 119 frames normales y un hitch de 80 ms: el promedio casi no lo nota,
    // el p95 tampoco, el max y el conteo de spikes sí.
    for (int i = 0; i < 119; ++i) {
        const double base = 11.0 + 2.0 * std::sin(i * 0.31);
        g_perfil.agregar({base, base * 0.7, base * 0.9});
    }
    g_perfil.agregar({80.0, 20.0, 78.0});
    std::size_t i = 0;
    for (const auto& m : g_perfil.muestras()) {
        g_historia[i % g_historia.size()] = static_cast<float>(m.frameMs);
        ++i;
    }
}

#if M24_HAS_IMGUI
void dibujarUI() {
    ImGui::SetNextWindowPos(ImVec2(24.0f, 24.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(620.0f, 460.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("M24 / L05 - Frame profiler");

    ImGui::Text("Presupuesto 60 FPS: %.2f ms", kPresupuestoMs);
    ImGui::Text("Promedio: %.2f ms", g_perfil.promedioFrame());
    ImGui::Text("p95: %.2f ms   max: %.2f ms", g_perfil.percentil(95.0), g_perfil.maximo());
    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.3f, 1.0f), "Spikes (>2x): %zu",
                       g_perfil.spikes());

    ImGui::Separator();
    ImGui::PlotLines("frame ms", g_historia.data(), static_cast<int>(g_historia.size()), 0,
                     "ultimos 120 frames", 0.0f, 90.0f, ImVec2(0, 120));

    if (ImGui::BeginTable("timers", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Sistema");
        ImGui::TableSetupColumn("CPU ms");
        ImGui::TableSetupColumn("GPU ms");
        ImGui::TableHeadersRow();
        const char* sistemas[] = {"update", "fisica", "render"};
        const double cpu[] = {3.2, 2.1, 5.7};
        const double gpu[] = {0.0, 0.0, 8.4};
        for (int i = 0; i < 3; ++i) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(sistemas[i]);
            ImGui::TableSetColumnIndex(1); ImGui::Text("%.2f", cpu[i]);
            ImGui::TableSetColumnIndex(2); ImGui::Text("%.2f", gpu[i]);
        }
        ImGui::EndTable();
    }

    ImGui::TextWrapped("El hitch de 80 ms casi no mueve el promedio: por eso se "
                       "miran max, p95/p99 y spikes, no solo la media.");
    ImGui::End();
}
#endif

} // namespace

int main() {
    sembrarDatos();
    std::puts("M24 / L05 - Frame profiler");
    std::puts("--------------------------");

#if M24_HAS_IMGUI
    return gui::runImGuiDemo("M24 L05 - frame profiler", dibujarUI);
#endif

    std::printf("Muestras: %zu\n", g_perfil.muestras().size());
    std::printf("Promedio: %.3f ms\n", g_perfil.promedioFrame());
    std::printf("p50: %.3f ms\n", g_perfil.percentil(50.0));
    std::printf("p95: %.3f ms\n", g_perfil.percentil(95.0));
    std::printf("p99: %.3f ms\n", g_perfil.percentil(99.0));
    std::printf("max:  %.3f ms\n", g_perfil.maximo());
    std::printf("spikes (>2x presupuesto): %zu\n", g_perfil.spikes());
    std::puts("");
    std::puts("El frame de 80 ms es invisible en el promedio y en p95; aparece");
    std::puts("en max y en el conteo de spikes. Esa es la razon de mirar colas.");
    return 0;
}
