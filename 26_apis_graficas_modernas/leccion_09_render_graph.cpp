/* =============================================================================
   MÓDULO 26 - LECCIÓN 09: render graph moderno — frame graph, aliasing de
   recursos, attachments transitorias y async compute
   =============================================================================
   TEMA 7: "Render graph moderno: frame graph, resource aliasing, transient
   attachments y async compute" — 1 lección.

   UN FRAME GRAPH ES UN SISTEMA DE BUILD PARA LOS PASOS DEL FRAME
   ---------------------------------------------------------------
   Declarás passes (shadow, gbuffer, lighting, bloom, tonemap, UI) y los
   recursos que CADA UNO lee/escribe. Con ese grafo el motor:
     1. calcula el INTERVALO de vida de cada recurso: qué passes lo usan,
        de la primera a la última;
     2. aplica ALIASING: dos recursos transitorios cuyos intervalos NO se
        superponen se mapean a la MISMA memoria física (como un pool con
        memoria reciclada). El gbuffer del frame puede REUTILIZAR los
        buffers del bloom del post-proceso, si sus vidas no se cruzan;
     3. infiere las BARRERAS (write->read, write->write, read->write)
        entre passes en lugar de vkCmdPipelineBarrier a mano;
     4. decide el queue: renders en la GRAPHICS queue, el compute pesado
        en la ASYNC COMPUTE queue usando solo semáforos en los edges de
        dependencia entre ambas.

   EL MODELO DE ESTA LECCIÓN (y del ejercicio 09)
   ----------------------------------------------
   Simulador en C++ puro: passes en orden de frame (con índice), cada uno
   con accesos {recurso, R/W}. El "build" calcula intervalos y el
   aliasing clásico:
     overlap(a, b) == false => alias posible.
   Las barreras: para cada par de passes CONSECUTIVOS, un edge por cada
   recurso compartido cuyo acceso cambió de W a R (o W a W). Esto SUENA
   simplista porque es la versión "por frame" del DAG real (con orden
   topológico); la idea conceptual es la MISMA.

   ASYNC COMPUTE
   --------------
   Un pass de compute puro puede ir a la familia de compute dedicada
   SOLO si no depende en barrera de la pasada de render inmediata: el
   semáforo cuelga en UN solo edge, no en todos los recursos. En la
   UHD 600 (LECCIÓN 02: la family 0 ya incluye COMPUTE) no hay
   ganancia real: el async puro no rinde ahí. En dGPU con familia de
   compute separada sí. En el simulador marcamos "puede async" cuando
   el pass SOLO LEE recursos que fueron escritos hace 2 o más passes
   (no depende del turno inmediato).
   ============================================================================= */
#include <algorithm>
#include <cstdint>
#include <limits>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

namespace {

enum class Acceso { W, R };

struct Pass {
    std::string nombre;
    std::vector<std::pair<std::string, Acceso>> accesos;
};

struct Intervalo { int primero; int ultima; };

/* Intervalo [primera, ultima] de passes que USAN cada recurso. */
std::vector<Intervalo> calcularIntervalos(
        const std::vector<Pass>& passes,
        const std::vector<std::string>& nombres) {
    std::vector<Intervalo> iv(nombres.size(),
                              {std::numeric_limits<int>::max(), -1});
    for (std::size_t p = 0; p < passes.size(); ++p) {
        for (const auto& [id, _] : passes[p].accesos) {
            for (std::size_t r = 0; r < nombres.size(); ++r) {
                if (nombres[r] == id) {
                    iv[r].primero = std::min(iv[r].primero, int(p));
                    iv[r].ultima  = std::max(iv[r].ultima,  int(p));
                }
            }
        }
    }
    return iv;
}

/* Alias posible si los intervalos no se cruzan (un recurso escribe
   después de que el otro ya nadie lo quiere). */
bool puedeAlias(const Intervalo& a, const Intervalo& b) {
    return a.ultima < b.primero || b.ultima < a.primero;
}

/* Barreras inferidas entre passes consecutivos: un edge por recurso que
   pasa de escrito a leído (o de escrito a escrito) entre pass i e i+1. */
std::size_t contarBarreras(const std::vector<Pass>& passes) {
    std::size_t n = 0;
    for (std::size_t i = 0; i + 1 < passes.size(); ++i) {
        for (const auto& [ida, acc_a] : passes[i].accesos) {
            if (acc_a != Acceso::W) continue;   /* termina en read: no edge */
            for (const auto& [idb, acc_b] : passes[i + 1].accesos) {
                if (ida == idb) ++n;            /* W->R o W->W: edge real  */
            }
        }
    }
    return n;
}

} // namespace

int main() {
    const std::vector<std::string> recursos = {"gbuffer", "bloom"};
    const std::vector<Pass> passes = {
        {"gbuffer", {{"gbuffer", Acceso::W}}},
        {"sombra",  {{"gbuffer", Acceso::R}}},
        {"luz",     {{"gbuffer", Acceso::R}}},
        {"post",    {{"bloom",   Acceso::W}}},
        {"blit",    {{"bloom",   Acceso::R}}},
    };

    const auto iv = calcularIntervalos(passes, recursos);
    std::printf("intervalos: gbuffer=[%d..%d] bloom=[%d..%d]\n",
                iv[0].primero, iv[0].ultima, iv[1].primero, iv[1].ultima);
    const bool alias = puedeAlias(iv[0], iv[1]);
    std::printf("gbuffer y bloom: %s alias (memoria física %s)\n",
                alias ? "SÍ pueden" : "NO pueden",
                alias ? "compartida" : "separada");
    std::printf("barreras inferidas write->read: %zu\n",
                contarBarreras(passes));
    /* Async: un pass que SOLO LEE y cuyo recurso no fue escrito por el
       pass inmediato anterior es candidato a la familia de compute
       dedicada. "luz" (lee gbuffer; "sombra" antes solo lo leyó) es
       candidato; "post" escribe bloom, así que nunca lo es. */
    return 0;
}
