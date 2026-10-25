/* =============================================================================
   MÓDULO 24 - SOLUCIÓN 08: Layout persistente (ini propio)
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
            // línea corrupta: se ignora
        }
    }
    return out;
}

void normalizar(std::vector<Panel>& paneles, float minimo = 0.05f) {
    if (paneles.empty()) return;
    const float n = static_cast<float>(paneles.size());
    if (minimo * n >= 1.0f) {  // el mínimo no entra: reparto igualitario
        for (auto& p : paneles) p.ratio = 1.0f / n;
        return;
    }

    std::vector<bool> fijado(paneles.size(), false);
    float suma = 0.0f;
    for (auto& p : paneles) {
        if (p.ratio < 0.0f) p.ratio = 0.0f;
        suma += p.ratio;
    }
    if (suma <= 0.0f) {
        for (auto& p : paneles) p.ratio = 1.0f / n;
        return;
    }
    for (auto& p : paneles) p.ratio /= suma;

    // Water-filling: los que caen bajo el mínimo se fijan y el resto se
    // reparte proporcionalmente entre los libres.
    for (;;) {
        float totalFijado = 0.0f;
        float totalLibre = 0.0f;
        bool nuevoFijado = false;
        for (std::size_t i = 0; i < paneles.size(); ++i) {
            if (fijado[i]) {
                totalFijado += paneles[i].ratio;
            } else if (paneles[i].ratio < minimo - 1e-7f) {
                paneles[i].ratio = minimo;
                fijado[i] = true;
                totalFijado += minimo;
                nuevoFijado = true;
            } else {
                totalLibre += paneles[i].ratio;
            }
        }
        if (!nuevoFijado) break;
        if (totalLibre <= 1e-9f) {
            for (auto& p : paneles) p.ratio = 1.0f / n;
            return;
        }
        const float resto = std::max(0.0f, 1.0f - totalFijado);
        for (std::size_t i = 0; i < paneles.size(); ++i) {
            if (!fijado[i]) paneles[i].ratio = paneles[i].ratio / totalLibre * resto;
        }
    }
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
    std::cout << "Solución 08 (dock layout)...\n";
    return RUN_ALL_TESTS();
}
