/*
 * MÓDULO 28: Estudio de Código Real — Arquitectura de Librerías
 * SOLUCIÓN 08: Informe Técnico — Matriz de Decisión, Orden y ADR
 *
 * Misma consigna que el ejercicio 08, con las implementaciones completas.
 */

#include <algorithm>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

enum class Estrategia { Adoptar, Envolver, Reimplementar, Diferir };

struct PerfilLib {
    std::string nombre;
    int score = 0;
    bool licencia_cerrada_ok = false;
    bool header_only = false;
    int deps_transitivas = 0;
    bool abi_estable = false;
};

std::string nombre_estrategia(Estrategia e) {
    switch (e) {
        case Estrategia::Adoptar: return "Adoptar";
        case Estrategia::Envolver: return "Envolver";
        case Estrategia::Reimplementar: return "Reimplementar";
        case Estrategia::Diferir: return "Diferir";
    }
    return "Diferir";
}

Estrategia decidir(const PerfilLib& perfil) {
    if (!perfil.licencia_cerrada_ok) return Estrategia::Diferir;
    if (perfil.score >= 80 && perfil.deps_transitivas <= 2) return Estrategia::Adoptar;
    if (perfil.score >= 60) return Estrategia::Envolver;
    if (perfil.score >= 40) return Estrategia::Reimplementar;
    return Estrategia::Diferir;
}

std::vector<PerfilLib> ordenar_por_score(std::vector<PerfilLib> perfiles) {
    std::sort(perfiles.begin(), perfiles.end(), [](const PerfilLib& a, const PerfilLib& b) {
        if (a.score != b.score) return a.score > b.score;
        return a.nombre < b.nombre;
    });
    return perfiles;
}

std::string tabla_markdown(const std::vector<PerfilLib>& perfiles) {
    std::string tabla = "| Librería | Score | Estrategia |\n";
    tabla += "| --- | --- | --- |\n";

    for (const auto& perfil : ordenar_por_score(perfiles)) {
        tabla += "| " + perfil.nombre + " | " + std::to_string(perfil.score) +
                 " | " + nombre_estrategia(decidir(perfil)) + " |\n";
    }
    return tabla;
}

std::string generar_adr(const std::string& id, const PerfilLib& perfil) {
    Estrategia estrategia = decidir(perfil);
    std::string nombre = nombre_estrategia(estrategia);

    std::string adr;
    adr += "# ADR-" + id + ": " + perfil.nombre + "\n\n";
    adr += "Estado: " + std::string(estrategia == Estrategia::Adoptar ? "aceptado" : "propuesto") + "\n\n";
    adr += "## Contexto\n";
    adr += "Evaluación de " + perfil.nombre + " para el motor propio: score " +
           std::to_string(perfil.score) + "/100, " +
           std::to_string(perfil.deps_transitivas) + " dependencias transitivas, licencia " +
           std::string(perfil.licencia_cerrada_ok ? "compatible" : "incompatible") +
           " con un proyecto cerrado.\n\n";
    adr += "## Decisión\n";
    adr += nombre + " " + perfil.nombre + ".\n\n";
    adr += "## Consecuencias\n";
    adr += "Estrategia elegida: " + nombre + ". ";
    if (estrategia == Estrategia::Adoptar) {
        adr += "Se integra por FetchContent con tag fijo y se aísla tras una fachada propia.\n";
    } else if (estrategia == Estrategia::Envolver) {
        adr += "Se usa solo a través de un wrapper con API propia para limitar el acoplamiento.\n";
    } else if (estrategia == Estrategia::Reimplementar) {
        adr += "Se reimplementa lo mínimo necesario y se revisa en la próxima evaluación.\n";
    } else {
        adr += "No se incorpora por ahora; se revisa si cambian licencia o mantenimiento.\n";
    }
    return adr;
}

// ============================================================================
// TESTS (NO MODIFICAR)
// ============================================================================

TEST_CASE(TestEstrategias) {
    PerfilLib p;
    p.nombre = "lib";
    p.licencia_cerrada_ok = true;

    p.score = 95; p.deps_transitivas = 1;
    ASSERT_TRUE(decidir(p) == Estrategia::Adoptar);

    p.score = 85; p.deps_transitivas = 3;
    ASSERT_TRUE(decidir(p) == Estrategia::Envolver);

    p.score = 65; p.deps_transitivas = 0;
    ASSERT_TRUE(decidir(p) == Estrategia::Envolver);

    p.score = 45; p.deps_transitivas = 0;
    ASSERT_TRUE(decidir(p) == Estrategia::Reimplementar);

    p.score = 20; p.deps_transitivas = 0;
    ASSERT_TRUE(decidir(p) == Estrategia::Diferir);

    p.score = 100; p.deps_transitivas = 0;
    p.licencia_cerrada_ok = false;
    ASSERT_TRUE(decidir(p) == Estrategia::Diferir);
}

TEST_CASE(TestNombresEstrategia) {
    ASSERT_EQ(nombre_estrategia(Estrategia::Adoptar), std::string("Adoptar"));
    ASSERT_EQ(nombre_estrategia(Estrategia::Envolver), std::string("Envolver"));
    ASSERT_EQ(nombre_estrategia(Estrategia::Reimplementar), std::string("Reimplementar"));
    ASSERT_EQ(nombre_estrategia(Estrategia::Diferir), std::string("Diferir"));
}

TEST_CASE(TestOrdenPorScore) {
    std::vector<PerfilLib> perfiles = {
        {"Zeta", 70, true, false, 1, true},
        {"Alfa", 70, true, true, 0, true},
        {"Beta", 95, true, false, 2, true},
        {"Gamma", 30, true, false, 4, false},
    };

    auto ordenados = ordenar_por_score(perfiles);

    ASSERT_EQ(ordenados.size(), 4u);
    ASSERT_EQ(ordenados[0].nombre, std::string("Beta"));
    ASSERT_EQ(ordenados[1].nombre, std::string("Alfa"));
    ASSERT_EQ(ordenados[2].nombre, std::string("Zeta"));
    ASSERT_EQ(ordenados[3].nombre, std::string("Gamma"));
}

TEST_CASE(TestTablaMarkdown) {
    std::vector<PerfilLib> perfiles = {
        {"miniaudio", 88, true, true, 0, false},
        {"Box2D", 72, true, false, 2, true},
        {"GPLlib", 99, false, false, 1, false},
    };

    std::string tabla = tabla_markdown(perfiles);

    ASSERT_TRUE(tabla.find("| Librería | Score | Estrategia |") != std::string::npos);
    ASSERT_TRUE(tabla.find("| --- | --- | --- |") != std::string::npos);
    ASSERT_TRUE(tabla.find("| GPLlib | 99 | Diferir |") != std::string::npos);
    ASSERT_TRUE(tabla.find("| miniaudio | 88 | Adoptar |") != std::string::npos);
    ASSERT_TRUE(tabla.find("| Box2D | 72 | Envolver |") != std::string::npos);

    ASSERT_TRUE(tabla.find("GPLlib") < tabla.find("miniaudio"));
}

TEST_CASE(TestADR) {
    PerfilLib p{"GLM", 90, true, true, 1, false};

    std::string adr = generar_adr("007", p);

    ASSERT_TRUE(adr.find("# ADR-007: GLM") != std::string::npos);
    ASSERT_TRUE(adr.find("Estado: aceptado") != std::string::npos);
    ASSERT_TRUE(adr.find("## Contexto") != std::string::npos);
    ASSERT_TRUE(adr.find("## Decisión") != std::string::npos);
    ASSERT_TRUE(adr.find("## Consecuencias") != std::string::npos);
    ASSERT_TRUE(adr.find("Adoptar") != std::string::npos);

    p.score = 45;
    std::string adr2 = generar_adr("008", p);
    ASSERT_TRUE(adr2.find("Estado: propuesto") != std::string::npos);
    ASSERT_TRUE(adr2.find("Reimplementar") != std::string::npos);
}

int main() { return RUN_ALL_TESTS(); }
