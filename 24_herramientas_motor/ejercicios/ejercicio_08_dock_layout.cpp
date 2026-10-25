/* =============================================================================
   MÓDULO 24 - EJERCICIO 08: Layout persistente (ini propio)
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 11 (ini, dockspace y normalización de ratios).

   CONSIGNA
   --------
   Implementá el formato:

       [Docking]
       Escena=0.550000|1
       Consola=0.200000|0

   - `serializar(paneles)`: encabezado `[Docking]\n` y una línea por panel con
     `nombre=ratio|visible` (`%.6f` y `1`/`0`).
   - `parsear(texto)`: sólo lee líneas dentro de `[Docking]`. Ignora vacías,
     comentarios (`;` o `#`), líneas sin `=`, valores sin `|` y ratios no
     numéricos. Los nombres se trimean.
   - `normalizar(paneles, minimo)`: garantiza que la suma sea 1 y que NINGÚN
     ratio quede por debajo de `minimo`. Estrategia: normalizar, fijar los que
     cayeron por debajo del mínimo y repartir el resto proporcionalmente entre
     los libres (agua). Con vector vacío no hace nada. Si la suma queda <= 0,
     reparte 1/n.
   ============================================================================= */
#include "../include/gui_util.hpp"
#include "../../include/test_framework.hpp"

#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

struct Panel {
    std::string nombre;
    float ratio = 0.25f;
    bool visible = true;
};

std::string serializar(const std::vector<Panel>& paneles) {
    // TODO
    (void)paneles;
    return {};
}

std::vector<Panel> parsear(std::string_view texto) {
    // TODO
    (void)texto;
    return {};
}

void normalizar(std::vector<Panel>& paneles, float minimo = 0.05f) {
    // TODO
    (void)paneles;
    (void)minimo;
}

// ============================================================================
// TESTS (no tocar)
// ============================================================================
TEST_CASE(Roundtrip) {
    const std::vector<Panel> p = {{"Escena", 0.5f, true}, {"Consola", 0.25f, false}};
    const auto leido = parsear(serializar(p));
    ASSERT_EQ(leido.size(), 2u);
    ASSERT_EQ(leido[0].nombre, std::string("Escena"));
    ASSERT_NEAR(leido[0].ratio, 0.5f, 1e-5f);
    ASSERT_TRUE(leido[0].visible);
    ASSERT_EQ(leido[1].nombre, std::string("Consola"));
    ASSERT_FALSE(leido[1].visible);
}

TEST_CASE(SerializarFormato) {
    const std::vector<Panel> p = {{"A", 0.5f, true}};
    ASSERT_EQ(serializar(p), std::string("[Docking]\nA=0.500000|1\n"));
}

TEST_CASE(ParsearIgnoraBasura) {
    const std::string texto =
        "; comentario\n"
        "[Otra]\n"
        "Ignore=0.5|1\n"
        "[Docking]\n"
        "# otro comentario\n"
        "\n"
        "  Escena = 0.4 | 1 \n"
        "sinIgual\n"
        "sinBarra=0.3\n"
        "roto=abc|1\n"
        "Consola=0.6|0\n";
    const auto p = parsear(texto);
    ASSERT_EQ(p.size(), 2u);
    ASSERT_EQ(p[0].nombre, std::string("Escena"));
    ASSERT_NEAR(p[0].ratio, 0.4f, 1e-5f);
    ASSERT_EQ(p[1].nombre, std::string("Consola"));
    ASSERT_FALSE(p[1].visible);
}

TEST_CASE(NormalizarSumaUno) {
    std::vector<Panel> p = {{"A", 2.0f, true}, {"B", 3.0f, true}};
    normalizar(p);
    ASSERT_NEAR(p[0].ratio, 0.4f, 1e-5f);
    ASSERT_NEAR(p[1].ratio, 0.6f, 1e-5f);
    ASSERT_NEAR(p[0].ratio + p[1].ratio, 1.0f, 1e-5f);
}

TEST_CASE(NormalizarRespetaMinimo) {
    std::vector<Panel> p = {{"A", 0.001f, true}, {"B", 1.0f, true}};
    normalizar(p, 0.1f);
    ASSERT_GE(p[0].ratio, 0.1f - 1e-5f);
    ASSERT_NEAR(p[0].ratio + p[1].ratio, 1.0f, 1e-5f);
}

TEST_CASE(NormalizarVacioYSumaCero) {
    std::vector<Panel> vacio;
    normalizar(vacio);
    ASSERT_TRUE(vacio.empty());

    std::vector<Panel> p = {{"A", 0.0f, true}, {"B", 0.0f, true}};
    normalizar(p);
    ASSERT_NEAR(p[0].ratio, 0.5f, 1e-5f);
    ASSERT_NEAR(p[1].ratio, 0.5f, 1e-5f);
}

int main() {
    std::cout << "Ejercicio 08 (dock layout)...\n";
    return RUN_ALL_TESTS();
}
