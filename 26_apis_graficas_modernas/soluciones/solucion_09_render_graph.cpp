/* =============================================================================
   MÓDULO 26 - SOLUCIÓN 09: render graph — intervalos de vida, aliasing y
   barreras inferidas
   =============================================================================
   La solución de ejercicio_09_render_graph.cpp. Es el núcleo del "build
   de resources del frame": los motores reales (aquí simplificamos
   bastante) calculan intervalos sobre un DAG con topological sort, pero
   la IDEA de decición (aliasing por disjunción de intervales + barreras
   por transición de acceso) es exactamente esta. blit/sombra/luz no
   necesitan barrera porque ES read->read.
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

namespace {

enum class Acceso { W, R };

struct Pass {
    std::string nombre;
    std::vector<std::pair<std::string, Acceso>> accesos;
};

struct Intervalo { int primero, ultima; };

/* Intervalo [primera, ultima] de uso por recurso (sin uso: [MAX, -1]). */
std::vector<Intervalo> calcularIntervalos(
        const std::vector<Pass>& passes,
        const std::vector<std::string>& nombres) {
    std::vector<Intervalo> iv(nombres.size(),
                              {std::numeric_limits<int>::max(), -1});
    for (std::size_t p = 0; p < passes.size(); ++p) {
        for (const auto& [id, _] : passes[p].accesos) {
            for (std::size_t r = 0; r < nombres.size(); ++r) {
                if (nombres[r] != id) continue;
                iv[r].primero = std::min(iv[r].primero, int(p));
                iv[r].ultima  = std::max(iv[r].ultima,  int(p));
            }
        }
    }
    return iv;
}

/* Alias posible si los intervalos no se cruzan. */
bool puedeAlias(const Intervalo& a, const Intervalo& b) {
    return a.ultima < b.primero || b.ultima < a.primero;
}

/* Edge por write->algo en passes consecutivos (read->read Es gratis). */
std::size_t contarBarreras(const std::vector<Pass>& passes) {
    std::size_t n = 0;
    for (std::size_t i = 0; i + 1 < passes.size(); ++i) {
        for (const auto& [ida, acc_a] : passes[i].accesos) {
            if (acc_a != Acceso::W) continue;
            for (const auto& [idb, _] : passes[i + 1].accesos) {
                if (ida == idb) ++n;
            }
        }
    }
    return n;
}

/* Compute async: pass que SOLO lee, sin nadie del turno inmediato. */
bool puedeAsync(const Pass& p, const std::vector<Pass>& passes) {
    if (p.accesos.empty()) return false;
    /* índice de p dentro de passes (mismo vector). */
    int idx = -1;
    for (std::size_t i = 0; i < passes.size(); ++i) {
        if (&passes[i] == &p) { idx = static_cast<int>(i); break; }
    }
    if (idx <= 0) return false;   /* el primer pass nunca es async    */

    for (const auto& [id, acc] : p.accesos) {
        if (acc != Acceso::R) return false;   /* solo rows permitidas */
        for (const auto& [idb, accb] : passes[idx - 1].accesos) {
            if (idb == id && accb == Acceso::W) return false;
        }
    }
    return true;
}

} // namespace

/* ------------------------------ TESTS (no tocar)-------------------------- */
static const std::vector<std::string> kRecursos = {"gbuffer", "bloom"};
static const std::vector<Pass> kPasses = {
    {"gbuffer", {{"gbuffer", Acceso::W}}},
    {"sombra",  {{"gbuffer", Acceso::R}}},
    {"luz",     {{"gbuffer", Acceso::R}}},
    {"post",    {{"bloom",   Acceso::W}}},
    {"blit",    {{"bloom",   Acceso::R}}},
};

TEST_CASE(IntervalosDeVida) {
    const auto iv = calcularIntervalos(kPasses, kRecursos);
    ASSERT_EQ(iv.size(), 2u);
    ASSERT_EQ(iv[0].primero, 0);
    ASSERT_EQ(iv[0].ultima, 2);
    ASSERT_EQ(iv[1].primero, 3);
    ASSERT_EQ(iv[1].ultima, 4);
}

TEST_CASE(AliasingGbufferXBloom) {
    const auto iv = calcularIntervalos(kPasses, kRecursos);
    ASSERT_EQ(iv.size(), 2u);              /* guard: no indexar vacío */
    ASSERT_TRUE(puedeAlias(iv[0], iv[1]));
    ASSERT_FALSE(puedeAlias(iv[0], iv[0]));
}

TEST_CASE(NoAliasSiSeCruzan) {
    const auto a = Intervalo{0, 3};
    const auto b = Intervalo{2, 4};
    ASSERT_FALSE(puedeAlias(a, b));
    ASSERT_TRUE(puedeAlias(Intervalo{0, 1}, Intervalo{2, 3}));
}

TEST_CASE(ConteoDeBarreras) {
    ASSERT_EQ(contarBarreras(kPasses), 2u);
}

TEST_CASE(CandidatoComputeAsync) {
    /* blit (índice 4) lee bloom, escrito por "post" (índice 3) en el
       turno inmediato: NO async. */
    ASSERT_FALSE(puedeAsync(kPasses[4], kPasses));
    /* sombra lee gbuffer, escrito por "gbuffer" (índice 0, inmediato): */
    ASSERT_FALSE(puedeAsync(kPasses[1], kPasses));
    /* luz lee gbuffer, pero "sombra" (inmediato) solo lo leyó: cabe */
    ASSERT_TRUE(puedeAsync(kPasses[2], kPasses));
    /* "post" escribe bloom: nunca es candidato compute async. */
    ASSERT_FALSE(puedeAsync(kPasses[3], kPasses));
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
