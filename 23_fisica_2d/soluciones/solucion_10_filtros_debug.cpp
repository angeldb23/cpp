/* =============================================================================
   MÓDULO 23 - SOLUCIÓN 10: Filtros de colisión y comandos de debug draw
   ============================================================================= */
#include "../include/fisica2d.hpp"
#include "../../include/test_framework.hpp"

#include <cmath>
#include <cstdint>
#include <vector>

using f2d::AABB;
using f2d::Vec2;

struct Segmento {
    Vec2 a;
    Vec2 b;
};

struct PuntoContacto {
    Vec2 p;
    Vec2 normal;
};

bool debenColisionar(uint16_t catA, uint16_t maskA, int16_t groupA,
                     uint16_t catB, uint16_t maskB, int16_t groupB) {
    if (groupA != 0 && groupA == groupB) return groupA > 0;
    return (catA & maskB) != 0 && (catB & maskA) != 0;
}

std::vector<Segmento> cajaRotada(Vec2 centro, float hx, float hy, float angulo) {
    const float c = std::cos(angulo);
    const float s = std::sin(angulo);
    auto rot = [&](Vec2 p) {
        return Vec2(centro.x + p.x * c - p.y * s,
                    centro.y + p.x * s + p.y * c);
    };
    const Vec2 v0 = rot({-hx, -hy});
    const Vec2 v1 = rot({hx, -hy});
    const Vec2 v2 = rot({hx, hy});
    const Vec2 v3 = rot({-hx, hy});
    return {{v0, v1}, {v1, v2}, {v2, v3}, {v3, v0}};
}

std::vector<Segmento> circulo(Vec2 centro, float radio, int segmentos) {
    std::vector<Segmento> lados;
    if (segmentos <= 0) return lados;
    lados.reserve(static_cast<size_t>(segmentos));
    for (int i = 0; i < segmentos; ++i) {
        const float a0 = 6.2831853f * static_cast<float>(i) / static_cast<float>(segmentos);
        const float a1 = 6.2831853f * static_cast<float>(i + 1) / static_cast<float>(segmentos);
        lados.push_back({{centro.x + std::cos(a0) * radio, centro.y + std::sin(a0) * radio},
                         {centro.x + std::cos(a1) * radio, centro.y + std::sin(a1) * radio}});
    }
    return lados;
}

AABB aabbDeCajaRotada(Vec2 centro, float hx, float hy, float angulo) {
    const float c = std::fabs(std::cos(angulo));
    const float s = std::fabs(std::sin(angulo));
    const float ex = c * hx + s * hy;
    const float ey = s * hx + c * hy;
    return {centro.x - ex, centro.y - ey, centro.x + ex, centro.y + ey};
}

void agregarContacto(std::vector<Segmento>& lineas,
                     std::vector<PuntoContacto>& puntos, Vec2 punto, Vec2 normal,
                     float escala) {
    puntos.push_back({punto, normal});
    lineas.push_back({punto, punto + normal * escala});
}

TEST_CASE(FiltroSimetrico) {
    constexpr uint16_t PARED = 0x0001;
    constexpr uint16_t JUGADOR = 0x0002;
    ASSERT_TRUE(debenColisionar(JUGADOR, PARED, 0, PARED, JUGADOR, 0));
    ASSERT_FALSE(debenColisionar(JUGADOR, PARED, 0, PARED, 0, 0));
    ASSERT_FALSE(debenColisionar(JUGADOR, 0, 0, PARED, JUGADOR, 0));
}

TEST_CASE(FiltroPorGrupo) {
    ASSERT_TRUE(debenColisionar(1, 0, 7, 2, 0, 7));
    ASSERT_FALSE(debenColisionar(1, 0xFFFF, -3, 2, 0xFFFF, -3));
    ASSERT_TRUE(debenColisionar(1, 0xFFFF, 7, 2, 0xFFFF, 8));
    ASSERT_TRUE(debenColisionar(1, 0xFFFF, 0, 2, 0xFFFF, 0));
    ASSERT_FALSE(debenColisionar(1, 0, 0, 2, 0, 0));
}

TEST_CASE(CajaRotadaTieneCuatroLados) {
    const auto lados = cajaRotada({0.0f, 0.0f}, 1.0f, 0.5f, 0.0f);
    ASSERT_EQ(lados.size(), size_t{4});
    ASSERT_NEAR(lados[0].a.x, -1.0f, 1e-6f);
    ASSERT_NEAR(lados[0].a.y, -0.5f, 1e-6f);
    ASSERT_NEAR(lados[0].b.x, 1.0f, 1e-6f);
    ASSERT_NEAR(lados[1].b.y, 0.5f, 1e-6f);
}

TEST_CASE(AabbDeCajaRotada45) {
    const auto b = aabbDeCajaRotada({0.0f, 1.0f}, 0.5f, 0.5f, 0.785398f);
    const float ext = 0.5f * std::sqrt(2.0f);
    ASSERT_NEAR(b.minX, -ext, 1e-4f);
    ASSERT_NEAR(b.maxX, ext, 1e-4f);
    ASSERT_NEAR(b.minY, 1.0f - ext, 1e-4f);
    ASSERT_NEAR(b.maxY, 1.0f + ext, 1e-4f);
}

TEST_CASE(AabbIntercambiaExtensionesAlRotar90) {
    const auto b = aabbDeCajaRotada({0.0f, 0.0f}, 2.0f, 1.0f, 1.570796f);
    ASSERT_NEAR(b.minX, -1.0f, 1e-4f);
    ASSERT_NEAR(b.maxX, 1.0f, 1e-4f);
    ASSERT_NEAR(b.minY, -2.0f, 1e-4f);
    ASSERT_NEAR(b.maxY, 2.0f, 1e-4f);
}

TEST_CASE(CirculoGeneraSegmentos) {
    const auto l = circulo({0.0f, 0.0f}, 1.0f, 8);
    ASSERT_EQ(l.size(), size_t{8});
    ASSERT_NEAR(l[0].a.x, 1.0f, 1e-5f);
    ASSERT_NEAR(l[0].a.y, 0.0f, 1e-5f);
    ASSERT_NEAR(l[2].a.x, 0.0f, 1e-5f);
    ASSERT_NEAR(l[2].a.y, 1.0f, 1e-5f);
}

TEST_CASE(ContactoAgregaPuntoYNormal) {
    std::vector<Segmento> lineas;
    std::vector<PuntoContacto> puntos;
    agregarContacto(lineas, puntos, {1.0f, 2.0f}, {0.0f, 1.0f}, 0.5f);
    ASSERT_EQ(puntos.size(), size_t{1});
    ASSERT_EQ(lineas.size(), size_t{1});
    ASSERT_NEAR(puntos[0].p.y, 2.0f, 1e-6f);
    ASSERT_NEAR(lineas[0].b.x, 1.0f, 1e-6f);
    ASSERT_NEAR(lineas[0].b.y, 2.5f, 1e-6f);
}

int main() {
    std::cout << "Solución 10 (Filtros y debug)...\n";
    return RUN_ALL_TESTS();
}
