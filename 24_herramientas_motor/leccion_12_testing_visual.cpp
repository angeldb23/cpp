/* =============================================================================
   MÓDULO 24 - LECCIÓN 12: Testing visual (golden frames)
   =============================================================================
   TEMA 8 de TEMAS.md: "Testing visual: image comparison, golden frames y
   regression UI automatizado".

   PROBLEMA
   --------
   Un test unitario no ve que el HUD quedó detrás del panel, que el texto se
   cortó o que un shader cambió de color. El testing visual compara el frame
   renderizado contra una imagen de referencia ("golden"). Si difiere más de
   una tolerancia, falla.

   MÉTRICAS
   --------
   - Hash exacto (FNV-1a, xxHash): rápido, detecta CUALQUIER cambio, pero es
     demasiado estricto: dos GPUs/drivers producen píxeles distintos por
     redondeo. Sirve para comparar la misma máquina y el mismo driver, no para
     CI heterogéneo.
   - RMS (root mean square) del error por canal: 0 = idéntico. Tolerancia
     típica 1-3 niveles de 255. Robusto a ruido leve.
   - PSNR (peak signal-to-noise ratio): 20*log10(255/RMS) dB. Más alto = más
     parecido; idéntico = infinito. 40 dB es "muy parecido"; 30 dB se nota.
   - Píxeles distintos: contás cuántos superan un umbral. Útil para "no más
     del 0.1% de la imagen cambió".
   - Bounding box del diff: te dice DÓNDE está el cambio. Un HUD que se movió
     3 px da un bbox chico; un fondo distinto da toda la pantalla.

   GOLDEN FRAMES EN LA PRÁCTICA
   ----------------------------
   1. Renderizá a un FBO de tamaño fijo (no a la ventana): el tamaño de la
      ventana y el DPI no deben cambiar el test (módulo 20, L15).
   2. Deshabilitá animaciones/tiempo real o sembrá el reloj.
   3. Leé con `glReadPixels` (RGBA8) y compará contra el PNG de referencia.
   4. Guardá el diff como imagen cuando falla, para diagnosticar.
   5. Actualizá los goldens con un flag explícito (`--update-golden`), nunca
      automáticamente en CI.

   UI REGRESSION AUTOMATIZADO
   --------------------------
   Para una herramienta ImGui podés inyectar input sintético (mouse/teclado en
   `ImGuiIO`), correr N frames y comparar. Es más estable que un test de UI
   real porque no depende del compositor. Aun así, el antialiasing y el font
   atlas varían por plataforma: fijá la fuente y la escala.

   ESTA LECCIÓN
   ------------
   Implementa hash, RMS, PSNR y bbox de diff sobre imágenes RGBA generadas en
   memoria. El ejercicio 09 practice lo mismo con tolerancias y guards.
   ============================================================================= */
#include "include/gui_util.hpp"
#include "include/imgui_app.hpp"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <vector>

namespace {

struct Imagen {
    int w = 0;
    int h = 0;
    std::vector<std::uint8_t> rgba;

    Imagen() = default;
    Imagen(int ancho, int alto, std::uint8_t r, std::uint8_t g, std::uint8_t b)
        : w(ancho), h(alto), rgba(static_cast<std::size_t>(ancho) * alto * 4) {
        for (int i = 0; i < w * h; ++i) {
            rgba[static_cast<std::size_t>(i) * 4 + 0] = r;
            rgba[static_cast<std::size_t>(i) * 4 + 1] = g;
            rgba[static_cast<std::size_t>(i) * 4 + 2] = b;
            rgba[static_cast<std::size_t>(i) * 4 + 3] = 255;
        }
    }
};

std::uint64_t hashImagen(const Imagen& img) {
    std::uint64_t h = 1469598103934665603ull;
    for (std::uint8_t b : img.rgba) {
        h ^= b;
        h *= 1099511628211ull;
    }
    return h;
}

double rms(const Imagen& a, const Imagen& b) {
    if (a.w != b.w || a.h != b.h || a.rgba.size() != b.rgba.size()) {
        return std::numeric_limits<double>::infinity();
    }
    double suma = 0.0;
    std::size_t canales = 0;
    for (std::size_t i = 0; i < a.rgba.size(); ++i) {
        if ((i % 4) == 3) continue;  // ignorar alpha
        const double d = static_cast<double>(a.rgba[i]) - static_cast<double>(b.rgba[i]);
        suma += d * d;
        ++canales;
    }
    if (canales == 0) return 0.0;
    return std::sqrt(suma / static_cast<double>(canales));
}

double psnr(const Imagen& a, const Imagen& b) {
    const double r = rms(a, b);
    if (r == 0.0) return std::numeric_limits<double>::infinity();
    if (!std::isfinite(r)) return 0.0;
    return 20.0 * std::log10(255.0 / r);
}

gui::Rect bboxDiff(const Imagen& a, const Imagen& b, int umbral) {
    if (a.w != b.w || a.h != b.h) return {0.0f, 0.0f, 0.0f, 0.0f};
    int minX = a.w, minY = a.h, maxX = -1, maxY = -1;
    for (int y = 0; y < a.h; ++y) {
        for (int x = 0; x < a.w; ++x) {
            const std::size_t i = (static_cast<std::size_t>(y) * a.w + x) * 4;
            const int dr = std::abs(static_cast<int>(a.rgba[i + 0]) - b.rgba[i + 0]);
            const int dg = std::abs(static_cast<int>(a.rgba[i + 1]) - b.rgba[i + 1]);
            const int db = std::abs(static_cast<int>(a.rgba[i + 2]) - b.rgba[i + 2]);
            if (dr > umbral || dg > umbral || db > umbral) {
                if (x < minX) minX = x;
                if (y < minY) minY = y;
                if (x > maxX) maxX = x;
                if (y > maxY) maxY = y;
            }
        }
    }
    if (maxX < 0) return {0.0f, 0.0f, 0.0f, 0.0f};
    return {static_cast<float>(minX), static_cast<float>(minY),
            static_cast<float>(maxX - minX + 1), static_cast<float>(maxY - minY + 1)};
}

#if M24_HAS_IMGUI
void dibujarUI() {
    // Dos "frames" sintéticos: el segundo tiene un rectángulo alterado.
    static Imagen a(64, 64, 40, 60, 80);
    static Imagen b = a;
    static bool iniciado = false;
    if (!iniciado) {
        for (int y = 10; y < 20; ++y) {
            for (int x = 12; x < 30; ++x) {
                const std::size_t i = (static_cast<std::size_t>(y) * b.w + x) * 4;
                b.rgba[i + 0] = 220;
                b.rgba[i + 1] = 80;
                b.rgba[i + 2] = 60;
            }
        }
        iniciado = true;
    }

    ImGui::SetNextWindowPos(ImVec2(24.0f, 24.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(520.0f, 280.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("M24 / L12 - Testing visual");
    const auto box = bboxDiff(a, b, 3);
    ImGui::Text("Hash A: %016llx", static_cast<unsigned long long>(hashImagen(a)));
    ImGui::Text("Hash B: %016llx", static_cast<unsigned long long>(hashImagen(b)));
    ImGui::Text("RMS: %.3f", rms(a, b));
    ImGui::Text("PSNR: %.2f dB", psnr(a, b));
    ImGui::Text("Diff bbox: x=%.0f y=%.0f w=%.0f h=%.0f", box.x, box.y, box.w, box.h);
    ImGui::TextWrapped("El hash detecta cualquier cambio; RMS/PSNR toleran ruido; el "
                       "bbox dice donde mirar. En CI se fija el tamano del FBO y la fuente.");
    ImGui::End();
}
#endif

// Genera una imagen con un cuadrado de color en (x0,y0)-(x1,y1).
Imagen conCuadrado(int ancho, int alto, int x0, int y0, int x1, int y1,
                   std::uint8_t r, std::uint8_t g, std::uint8_t b) {
    Imagen img(ancho, alto, 40, 60, 80);
    for (int y = y0; y < y1; ++y) {
        for (int x = x0; x < x1; ++x) {
            const std::size_t i = (static_cast<std::size_t>(y) * ancho + x) * 4;
            img.rgba[i + 0] = r;
            img.rgba[i + 1] = g;
            img.rgba[i + 2] = b;
        }
    }
    return img;
}

} // namespace

int main() {
    std::puts("M24 / L12 - Testing visual con golden frames");
    std::puts("--------------------------------------------");

    const Imagen a = conCuadrado(48, 32, 8, 8, 24, 20, 220, 80, 60);
    Imagen b = a;
    const Imagen igual = a;
    const Imagen distinta = conCuadrado(48, 32, 30, 4, 40, 14, 20, 200, 120);

    std::printf("hash igual:    %016llx  %s\n",
                static_cast<unsigned long long>(hashImagen(a)),
                hashImagen(a) == hashImagen(igual) ? "(coincide)" : "(NO)");
    std::printf("hash distinta: %016llx\n", static_cast<unsigned long long>(hashImagen(distinta)));
    std::printf("RMS  a vs a: %.3f   PSNR: %s\n", rms(a, igual),
                std::isinf(psnr(a, igual)) ? "inf" : "finito");
    std::printf("RMS  a vs b: %.3f   PSNR: %.2f dB\n", rms(a, distinta), psnr(a, distinta));
    const auto box = bboxDiff(a, distinta, 3);
    std::printf("bbox diff: x=%.0f y=%.0f w=%.0f h=%.0f\n", box.x, box.y, box.w, box.h);

    // Cambio de 1 nivel en un píxel: el hash lo ve, RMS casi no.
    b.rgba[0] = static_cast<std::uint8_t>(b.rgba[0] + 1);
    std::printf("\n1 pixel +1: hash cambia=%s, RMS=%.4f, bbox w=%.0f\n",
                hashImagen(a) != hashImagen(b) ? "si" : "no", rms(a, b),
                bboxDiff(a, b, 0).w);
    std::puts("Por eso el hash no sirve para CI heterogeneo y RMS si.");

#if M24_HAS_IMGUI
    return gui::runImGuiDemo("M24 L12 - testing visual", dibujarUI);
#else
    return 0;
#endif
}
