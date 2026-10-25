/* =============================================================================
   MÓDULO 24 - EJERCICIO 09: Golden frames (comparación de imágenes)
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 12 (hash, RMS, PSNR, bbox y tolerancias).

   CONSIGNA
   --------
   Implementá sobre imágenes RGBA8:

   1. `hashImagen`: FNV-1a 64 de TODOS los bytes (incluido alpha), con offset
      1469598103934665603ull y primo 1099511628211ull.
   2. `rms(a, b)`: raíz del error cuadrático medio por canal RGB (ignorá
      alpha). Si las dimensiones difieren, `infinity`.
   3. `psnr(a, b)`: `20*log10(255/rms)`. Si rms == 0, `infinity`; si rms no es
      finito, 0.
   4. `bboxDiff(a, b, umbral)`: menor rectángulo que contiene todos los píxeles
      donde ALGÚN canal RGB difiere en más de `umbral` (estricto). Si no hay
      diferencias o las dimensiones no coinciden, rect vacío (w=0,h=0).
   5. `coincide(a, b, maxRms)`: true si `rms(a,b) <= maxRms`.
   ============================================================================= */
#include "../include/gui_util.hpp"
#include "../../include/test_framework.hpp"

#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

struct Imagen {
    int w = 0;
    int h = 0;
    std::vector<std::uint8_t> rgba;
};

std::uint64_t hashImagen(const Imagen& img) {
    // TODO
    (void)img;
    return 0;
}

double rms(const Imagen& a, const Imagen& b) {
    // TODO
    (void)a;
    (void)b;
    return 0.0;
}

double psnr(const Imagen& a, const Imagen& b) {
    // TODO
    (void)a;
    (void)b;
    return 0.0;
}

gui::Rect bboxDiff(const Imagen& a, const Imagen& b, int umbral) {
    // TODO
    (void)a;
    (void)b;
    (void)umbral;
    return {};
}

bool coincide(const Imagen& a, const Imagen& b, double maxRms) {
    // TODO
    (void)a;
    (void)b;
    (void)maxRms;
    return true;
}

// ============================================================================
// TESTS (no tocar)
// ============================================================================
namespace {
Imagen solida(int w, int h, std::uint8_t r, std::uint8_t g, std::uint8_t b) {
    Imagen img{w, h, std::vector<std::uint8_t>(static_cast<std::size_t>(w) * h * 4)};
    for (int i = 0; i < w * h; ++i) {
        img.rgba[static_cast<std::size_t>(i) * 4 + 0] = r;
        img.rgba[static_cast<std::size_t>(i) * 4 + 1] = g;
        img.rgba[static_cast<std::size_t>(i) * 4 + 2] = b;
        img.rgba[static_cast<std::size_t>(i) * 4 + 3] = 255;
    }
    return img;
}
} // namespace

TEST_CASE(HashIgualYDistinto) {
    const auto a = solida(4, 4, 10, 20, 30);
    auto b = a;
    ASSERT_EQ(hashImagen(a), hashImagen(b));
    b.rgba[5] = 99;
    ASSERT_NE(hashImagen(a), hashImagen(b));
}

TEST_CASE(RmsIdenticoYDesplazado) {
    const auto a = solida(4, 4, 10, 20, 30);
    ASSERT_NEAR(rms(a, a), 0.0, 1e-9);

    auto b = a;
    for (std::size_t i = 0; i < b.rgba.size(); i += 4) {
        b.rgba[i + 0] = 20;  // +10 en los tres canales
        b.rgba[i + 1] = 30;
        b.rgba[i + 2] = 40;
    }
    ASSERT_NEAR(rms(a, b), 10.0, 1e-9);
}

TEST_CASE(PsnrIdenticoYFinito) {
    const auto a = solida(2, 2, 0, 0, 0);
    ASSERT_TRUE(std::isinf(psnr(a, a)));

    auto b = a;
    for (std::size_t i = 0; i < b.rgba.size(); i += 4) {
        b.rgba[i + 0] = 10;
        b.rgba[i + 1] = 10;
        b.rgba[i + 2] = 10;
    }
    const double esperado = 20.0 * std::log10(255.0 / 10.0);
    ASSERT_NEAR(psnr(a, b), esperado, 1e-6);
}

TEST_CASE(BboxDelCambio) {
    const auto a = solida(8, 8, 0, 0, 0);
    auto b = a;
    for (int y = 1; y < 3; ++y) {
        for (int x = 2; x < 4; ++x) {
            b.rgba[(static_cast<std::size_t>(y) * 8 + x) * 4 + 1] = 50;
        }
    }
    const auto r = bboxDiff(a, b, 3);
    ASSERT_NEAR(r.x, 2.0f, 1e-6f);
    ASSERT_NEAR(r.y, 1.0f, 1e-6f);
    ASSERT_NEAR(r.w, 2.0f, 1e-6f);
    ASSERT_NEAR(r.h, 2.0f, 1e-6f);

    const auto igual = bboxDiff(a, a, 3);
    ASSERT_TRUE(igual.empty());
}

TEST_CASE(ImagenesDeDistintoTamano) {
    const auto a = solida(2, 2, 0, 0, 0);
    const auto b = solida(3, 2, 0, 0, 0);
    ASSERT_TRUE(std::isinf(rms(a, b)));
    ASSERT_NEAR(psnr(a, b), 0.0, 1e-9);
    ASSERT_TRUE(bboxDiff(a, b, 0).empty());
    ASSERT_FALSE(coincide(a, b, 1.0));
}

TEST_CASE(Tolerancia) {
    const auto a = solida(4, 4, 10, 10, 10);
    auto b = a;
    for (std::size_t i = 0; i < b.rgba.size(); i += 4) b.rgba[i] = 12;
    ASSERT_TRUE(coincide(a, b, 2.0));
    ASSERT_FALSE(coincide(a, b, 1.0));
}

int main() {
    std::cout << "Ejercicio 09 (golden frames)...\n";
    return RUN_ALL_TESTS();
}
