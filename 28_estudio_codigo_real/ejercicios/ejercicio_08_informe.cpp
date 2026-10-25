/*
 * MÓDULO 28: Estudio de Código Real — Arquitectura de Librerías
 * EJERCICIO 08: Informe Técnico — Matriz de Decisión, Orden y ADR
 *
 * OBJETIVO: Convertir perfiles de evaluación en una decisión de adopción
 * (lección 09): adoptar, envolver, reimplementar o diferir; ordenar los
 * perfiles por score; generar la tabla markdown comparativa y un
 * Architecture Decision Record (ADR) listo para pegar en el informe.
 *
 * Todo es lógica pura: no lee archivos ni accede a la red.
 */

#include <algorithm>
#include <string>
#include <vector>
#include "../../include/test_framework.hpp"

enum class Estrategia { Adoptar, Envolver, Reimplementar, Diferir };

struct PerfilLib {
    std::string nombre;
    int score = 0;                    // 0..100
    bool licencia_cerrada_ok = false; // compatible con un motor propietario
    bool header_only = false;
    int deps_transitivas = 0;
    bool abi_estable = false;
};

// TODO: nombre legible de la estrategia ("Adoptar", "Envolver", ...).
std::string nombre_estrategia(Estrategia e) {
    (void)e;
    return "";
}

// TODO: regla de decisión:
//   - sin licencia compatible -> Diferir
//   - score >= 80 y deps <= 2 -> Adoptar
//   - score >= 60             -> Envolver
//   - score >= 40             -> Reimplementar
//   - resto                   -> Diferir
Estrategia decidir(const PerfilLib& perfil) {
    (void)perfil;
    return Estrategia::Diferir;
}

// TODO: ordenar por score descendente; a igual score, nombre ascendente.
std::vector<PerfilLib> ordenar_por_score(std::vector<PerfilLib> perfiles) {
    (void)perfiles;
    return {};
}

// TODO: tabla markdown con encabezado exacto:
//   | Librería | Score | Estrategia |
//   | --- | --- | --- |
// y una fila por perfil, en el mismo orden que ordenar_por_score().
std::string tabla_markdown(const std::vector<PerfilLib>& perfiles) {
    (void)perfiles;
    return "";
}

// TODO: ADR en markdown con:
//   "# ADR-<id>: <nombre>"
//   "Estado: propuesto" (o "aceptado" si la estrategia es Adoptar)
//   secciones "## Contexto", "## Decisión" y "## Consecuencias"
//   la estrategia elegida debe aparecer en el texto.
std::string generar_adr(const std::string& id, const PerfilLib& perfil) {
    (void)id; (void)perfil;
    return "";
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
    p.licencia_cerrada_ok = false; // licencia incompatible manda
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

    // El orden de las filas respeta el score: primero GPLlib, después miniaudio.
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
