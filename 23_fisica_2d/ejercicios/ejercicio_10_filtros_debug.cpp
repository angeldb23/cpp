/* =============================================================================
   MÓDULO 23 - EJERCICIO 10: Filtros de colisión y comandos de debug draw
   =============================================================================
   DIFICULTAD: Medio
   APOYO: Lección 11 (filtros por categoría/máscara/grupo) y lección 14
          (lista de comandos de debug sin backend).

   CONSIGNA:
     - `debenColisionar(...)`: reproducir la regla de b2Filter SIN Box2D.
         * Si groupA == groupB y groupA != 0: colisionan si groupA > 0,
           nunca si groupA < 0.
         * Si no: (catA & maskB) != 0 && (catB & maskA) != 0.
     - `cajaRotada`: devolver 4 segmentos (los lados) de una caja de
       semiextensiones hx,hy centrada en `centro` y rotada `angulo` radianes.
       Orden: abajo, derecha, arriba, izquierda (sentido antihorario).
     - `circulo`: devolver `segmentos` segmentos que aproximan la circunferencia.
     - `aabbDeCajaRotada`: AABB que envuelve la caja rotada.
     - `agregarContacto`: añadir un punto de contacto a `puntos` y un segmento
       desde el punto hasta punto + normal*escala a `lineas`.
   ============================================================================= */
#include "../include/fisica2d.hpp"
#include "../../include/test_framework.hpp"

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
    // TODO
    (void)catA;
    (void)maskA;
    (void)groupA;
    (void)catB;
    (void)maskB;
    (void)groupB;
    return false;
}

std::vector<Segmento> cajaRotada(Vec2 centro, float hx, float hy, float angulo) {
    // TODO
    (void)centro;
    (void)hx;
    (void)hy;
    (void)angulo;
    return {};
}

std::vector<Segmento> circulo(Vec2 centro, float radio, int segmentos) {
    // TODO
    (void)centro;
    (void)radio;
    (void)segmentos;
    return {};
}

AABB aabbDeCajaRotada(Vec2 centro, float hx, float hy, float angulo) {
    // TODO
    (void)centro;
    (void)hx;
    (void)hy;
    (void)angulo;
    return {};
}

void agregarContacto(std::vector<Segmento>& lineas,
                     std::vector<PuntoContacto>& puntos, Vec2 punto, Vec2 normal,
                     float escala = 0.5f) {
    // TODO
    (void)lineas;
    (void)puntos;
    (void)punto;
    (void)normal;
    (void)escala;
}

// ============================================================================
// TESTS (no tocar)
// ============================================================================
TEST_CASE(FiltroSimetrico) {
    constexpr uint16_t PARED = 0x0001;
    constexpr uint16_t JUGADOR = 0x0002;
    // Ambos se aceptan.
    ASSERT_TRUE(debenColisionar(JUGADOR, PARED, 0, PARED, JUGADOR, 0));
    // Solo uno acepta: NO colisionan (la regla es simétrica).
    ASSERT_FALSE(debenColisionar(JUGADOR, PARED, 0, PARED, 0, 0));
    ASSERT_FALSE(debenColisionar(JUGADOR, 0, 0, PARED, JUGADOR, 0));
}

TEST_CASE(FiltroPorGrupo) {
    // Mismo grupo positivo: colisionan aunque las máscaras estén vacías.
    ASSERT_TRUE(debenColisionar(1, 0, 7, 2, 0, 7));
    // Mismo grupo negativo: nunca colisionan aunque las máscaras se acepten.
    ASSERT_FALSE(debenColisionar(1, 0xFFFF, -3, 2, 0xFFFF, -3));
    // Grupos distintos no se pisan: vuelven a las máscaras.
    ASSERT_TRUE(debenColisionar(1, 0xFFFF, 7, 2, 0xFFFF, 8));
    // Grupo 0 (sin grupo) usa las máscaras.
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
    std::cout << "Ejercicio 10 (Filtros y debug)...\n";
    return RUN_ALL_TESTS();
}
