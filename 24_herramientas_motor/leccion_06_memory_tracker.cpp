/* =============================================================================
   MÓDULO 24 - LECCIÓN 06: Memory alloc tracker y gráficas en vivo
   =============================================================================
   TEMA 3 de TEMAS.md (segunda lección): el otro profiler: memoria.

   QUÉ CONTAR
   ----------
   No alcanza con "cuánta RAM usa el proceso": querés saber POR CATEGORÍA
   (texturas, mallas, audio, scripts, general) y cómo evoluciona. Un tracker
   mínimo guarda:

       current[tag]   bytes vivos ahora
       peak[tag]      máximo histórico de esa categoría
       history        muestra del total por frame (ring buffer para la gráfica)
       allocs/frees   cantidad de operaciones (detecta churn)

   `alloc(tag, n)` suma; `free(tag, n)` resta. El free debe CLAMPEAR en cero:
   un free de más es un bug, pero el tracker no debe underflowear a 18 EB.

   PEAK Y LEAKS
   ------------
   `peak` no baja nunca: te dice cuánto necesitó el peor momento (útil para
   presupuestar memoria). Al cerrar el nivel, `current` debería volver a un
   baseline conocido. Lo que queda se reporta como "leak" o como recurso
   todavía referenciado (caché, pools). Distinguí ambos: no todo lo que no es
   cero es una fuga.

   CHURN
   -----
   Muchas allocs/frees del mismo tamaño por frame fragmentan y cuestan. Si el
   contador de operaciones crece más rápido que los bytes, hay churn: conviene
   un pool o arena (módulo 07, L14). El tracker lo hace visible.

   GRÁFICAS
   --------
   `PlotLines` para el total en el tiempo y `PlotHistogram`/barras por
   categoría. Escalá el eje Y al peak: si usás un máximo fijo y el juego usa
   2 MB, la gráfica parece vacía; si usás autoescala, un spike aplasta el
   resto. Una buena UI muestra ambos (auto + peak marcado).

   OVERHEAD
   --------
   Envolver CADA new/delete con un tracker global cuesta y puede recursar
   (el propio tracker asigna). En producción se instrumenta el allocator o se
   usa sampling. Para herramientas de debug, un mapa de punteros -> tamaño
   (o macros de alloc/free del motor) es suficiente.

   QUÉ NO HACER
   ------------
   No midas con `getrusage`/`/proc` por frame: es caro y ruidoso. No guardes
   punteros a memoria liberada "para el reporte". No mezcles bytes reservados
   (capacity) con bytes usados (size) sin decirlo.
   ============================================================================= */
#include "include/gui_util.hpp"
#include "include/imgui_app.hpp"

#include <array>
#include <cstddef>
#include <cstdio>
#include <deque>

namespace {

enum class Tag : std::size_t { General, Textura, Malla, Audio, Script, Count };
constexpr std::size_t kTags = static_cast<std::size_t>(Tag::Count);

const char* nombre(Tag t) {
    switch (t) {
        case Tag::General: return "general";
        case Tag::Textura: return "texturas";
        case Tag::Malla: return "mallas";
        case Tag::Audio: return "audio";
        case Tag::Script: return "scripts";
        default: return "?";
    }
}

class MemoryTracker {
public:
    explicit MemoryTracker(std::size_t capacidadHistoria) : capacidad_(capacidadHistoria) {}

    void alloc(Tag t, std::size_t bytes) {
        auto& c = actual_[static_cast<std::size_t>(t)];
        c += bytes;
        pico_[static_cast<std::size_t>(t)] =
            std::max(pico_[static_cast<std::size_t>(t)], c);
        ++allocs_;
        muestrear();
    }

    void free(Tag t, std::size_t bytes) {
        auto& c = actual_[static_cast<std::size_t>(t)];
        // Un free de más es un bug del que llama; el tracker no underflowea.
        c = (bytes > c) ? 0 : (c - bytes);
        ++frees_;
        muestrear();
    }

    std::size_t actual(Tag t) const { return actual_[static_cast<std::size_t>(t)]; }
    std::size_t pico(Tag t) const { return pico_[static_cast<std::size_t>(t)]; }
    std::size_t total() const {
        std::size_t s = 0;
        for (auto v : actual_) s += v;
        return s;
    }
    std::size_t operaciones() const { return allocs_ + frees_; }
    const std::deque<std::size_t>& historia() const { return historia_; }

private:
    void muestrear() {
        historia_.push_back(total());
        while (historia_.size() > capacidad_) historia_.pop_front();
    }

    std::array<std::size_t, kTags> actual_{};
    std::array<std::size_t, kTags> pico_{};
    std::deque<std::size_t> historia_;
    std::size_t capacidad_ = 0;
    std::size_t allocs_ = 0;
    std::size_t frees_ = 0;
};

MemoryTracker g_mem(240);
std::array<float, kTags> g_barras{};

void simular() {
    g_mem.alloc(Tag::General, 4096);
    for (int i = 0; i < 8; ++i) g_mem.alloc(Tag::Textura, 256 * 1024);
    for (int i = 0; i < 32; ++i) g_mem.alloc(Tag::Malla, 12 * 1024);
    g_mem.alloc(Tag::Audio, 480000);
    g_mem.alloc(Tag::Script, 64 * 1024);

    // El nivel descarga 4 texturas y 16 mallas: el pico queda registrado.
    for (int i = 0; i < 4; ++i) g_mem.free(Tag::Textura, 256 * 1024);
    for (int i = 0; i < 16; ++i) g_mem.free(Tag::Malla, 12 * 1024);
    // Churn: 100 asignaciones y liberaciones de 1 KB.
    for (int i = 0; i < 100; ++i) {
        g_mem.alloc(Tag::General, 1024);
        g_mem.free(Tag::General, 1024);
    }

    for (std::size_t i = 0; i < kTags; ++i) {
        g_barras[i] = static_cast<float>(g_mem.actual(static_cast<Tag>(i))) / 1024.0f;
    }
}

#if M24_HAS_IMGUI
void dibujarUI() {
    ImGui::SetNextWindowPos(ImVec2(24.0f, 24.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(600.0f, 450.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("M24 / L06 - Memoria");

    ImGui::Text("Total vivo: %.1f KiB", static_cast<double>(g_mem.total()) / 1024.0);
    ImGui::Text("Operaciones: %zu", g_mem.operaciones());
    ImGui::Separator();

    for (std::size_t i = 0; i < kTags; ++i) {
        const auto t = static_cast<Tag>(i);
        ImGui::Text("%-10s %8.1f KiB  pico %8.1f KiB", nombre(t),
                    static_cast<double>(g_mem.actual(t)) / 1024.0,
                    static_cast<double>(g_mem.pico(t)) / 1024.0);
    }

    ImGui::Separator();
    ImGui::PlotHistogram("por categoria (KiB)", g_barras.data(), static_cast<int>(kTags), 0,
                         nullptr, 0.0f, 300.0f, ImVec2(0, 100));
    ImGui::TextWrapped("El pico de texturas no baja aunque se liberen: sirve para "
                       "presupuestar el peor caso. Las 100 alloc/free de 1 KiB no "
                       "cambian el total, pero si el contador de operaciones (churn).");
    ImGui::End();
}
#endif

} // namespace

int main() {
    simular();
    std::puts("M24 / L06 - Memory tracker");
    std::puts("--------------------------");

#if M24_HAS_IMGUI
    return gui::runImGuiDemo("M24 L06 - memoria", dibujarUI);
#endif

    for (std::size_t i = 0; i < kTags; ++i) {
        const auto t = static_cast<Tag>(i);
        std::printf("%-10s actual %7zu B   pico %7zu B\n", nombre(t), g_mem.actual(t),
                    g_mem.pico(t));
    }
    std::printf("TOTAL vivo: %zu B\n", g_mem.total());
    std::printf("Operaciones alloc+free: %zu\n", g_mem.operaciones());
    std::printf("Muestras de historia: %zu\n", g_mem.historia().size());
    std::puts("");
    std::puts("El churn (100 ciclos de 1 KiB) no cambia el total vivo, pero si");
    std::puts("el conteo de operaciones: eso es lo que fragmenta y cuesta.");
    return 0;
}
