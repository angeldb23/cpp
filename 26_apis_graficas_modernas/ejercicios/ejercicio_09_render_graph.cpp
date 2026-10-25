/* =============================================================================
   MÓDULO 26 - EJERCICIO 09: render graph — intervalos de vida, aliasing y
   barreras inferidas
   =============================================================================
   DIFICULTAD: Media-difícil
   APOYO: Lección 09 (frame graph, resource aliasing, attachments
   transitorias y async compute).

   CONSIGNA
   --------
   1. `calcularIntervalos(passes, nombres)`: por cada recurso, el rango
      [primera, ultima] de passes que lo usan. Si nadie lo usa:
      primero = INT_MAX, ultima = -1.
   2. `puedeAlias(a, b)`: los intervalos NO se cruzan
      (a.ultima < b.primero || b.ultima < a.primero).
   3. `contarBarreras(passes)`: entre passes CONSECUTIVOS, un edge por
      recurso que sale de un pass escrito (W) y se usa en el siguiente
      (R o W: write->read y write->write).
   4. `puedeAsync(pass, passes)`: un pass es candidato a la familia de
      compute dedicada si TODOS sus accesos son READ y ninguno de esos
      recursos fue escrito en el pass INMEDIATAMENTE anterior.
   ============================================================================= */
#include "../../include/test_framework.hpp"

#include <cstdint>
#include <utility>
#include <vector>

namespace {

enum class Acceso { W, R };

struct Pass {
    std::string nombre;
    std::vector<std::pair<std::string, Acceso>> accesos;
};

struct Intervalo { int primero, ultima; };

std::vector<Intervalo> calcularIntervalos(
        const std::vector<Pass>& passes,
        const std::vector<std::string>& nombres) {
    // TODO
    (void)passes; (void)nombres;
    return {};   // stub vacío (el test protege con size)
}

bool puedeAlias(const Intervalo& a, const Intervalo& b) {
    // TODO
    return false;
}

std::size_t contarBarreras(const std::vector<Pass>& passes) {
    // TODO
    return 99;   // stub: valor imposible
}

bool puedeAsync(const Pass& p, const std::vector<Pass>& passes) {
    // TODO (p vive dentro de passes: buscá el índice)
    (void)p; (void)passes;
    return false;
}

} // namespace

/* ------------------------------ TESTS (no tocar)-------------------------- */
/* Pipeline del frame del curso (la de la lección 09). */
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
    ASSERT_EQ(iv.size(), 2u);              /* guard: no indexar vacío */
    ASSERT_EQ(iv[0].primero, 0);
    ASSERT_EQ(iv[0].ultima, 2);
    ASSERT_EQ(iv[1].primero, 3);
    ASSERT_EQ(iv[1].ultima, 4);
}

TEST_CASE(AliasingGbufferXBloom) {
    const auto iv = calcularIntervalos(kPasses, kRecursos);
    ASSERT_EQ(iv.size(), 2u);              /* guard: no indexar vacío */
    /* gbuffer muere en el pass 2 y bloom nace en el 3: memoria compartida */
    ASSERT_TRUE(puedeAlias(iv[0], iv[1]));
    /* el mismo intervalo consigo mismo: NO se puede alias */
    ASSERT_FALSE(puedeAlias(iv[0], iv[0]));
}

TEST_CASE(NoAliasSiSeCruzan) {
    const auto a = Intervalo{0, 3};
    const auto b = Intervalo{2, 4};
    ASSERT_FALSE(puedeAlias(a, b));        /* 2 <= 3: cruzan */
    /* adyacentes sin tocarse: alias posible */
    ASSERT_TRUE(puedeAlias(Intervalo{0, 1}, Intervalo{2, 3}));
}

TEST_CASE(ConteoDeBarreras) {
    /* gbuffer W->R (0->1) y bloom W->R (3->4): 2 edges
       (las pasadas sombra->luz y luz->post no tienen W). */
    ASSERT_EQ(contarBarreras(kPasses), 2u);
}

TEST_CASE(CandidatoComputeAsync) {
    /* blit (índice 4) lee bloom, y el pass INMEDIATO (3, "post") lo
       escribió: tiene que esperar la barrera: NO async. */
    ASSERT_FALSE(puedeAsync(kPasses[4], kPasses));
    /* sombra lee gbuffer y el pass anterior (0, "gbuffer") lo escribió:
       NO async. */
    ASSERT_FALSE(puedeAsync(kPasses[1], kPasses));
    /* luz lee gbuffer y su anterior (1, "sombra") solo lo leyó:
       candidata a la familia de compute. */
    ASSERT_TRUE(puedeAsync(kPasses[2], kPasses));
    /* un pass que escribe algo ("post" escribe bloom) nunca es async. */
    ASSERT_FALSE(puedeAsync(kPasses[3], kPasses));
}

int main() {
    return TestFramework::TestRunner::instance().run();
}
