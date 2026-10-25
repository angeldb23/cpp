/* =============================================================================
   MÓDULO 20 - EJERCICIO 06: Estado GL cacheado (delta de comandos)
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 13 (el estado GL es GLOBAL y mutante: cada glDisable/
          glEnable tiene costo y el "estado fantasma" muerde bugs. Un estado
          cacheado que emite SOLO el delta es el núcleo del renderizador).

   CONSIGNA (lógica pura: los "comandos" son strings, NO llamadas GL):
     struct EstadoGL {
         int vx = 0, vy = 0, vw = 0, vh = 0;   // viewport (x, y, w, h)
         bool blend = false;                    // GL_BLEND
         bool ztest = true;                     // GL_DEPTH_TEST
         int  cull = 0;                         // 0 nada, 1 CCW, 2 CW
     };

   El ESTADO ABSOLUTO se describe con comandos (la forma "comandos" es lo
   que el llamador acaba mandando a glViewport/glEnable/glDisable):
     "viewport VX VY VW VH"   (4 ints separados por espacios)
     "blend on" | "blend off"
     "ztest on" | "ztest off"
     "cull 0"   | "cull 1"   | "cull 2"

     1. `delta(prev, cur)`: comandos SOLO de lo que cambia de prev a cur,
        SIEMPRE en el orden viewport → blend → ztest → cull. Si nada
        cambia: lista VACÍA. (El delta con valores ABSOLUTOS es idempotente:
        mandarlo al GPU dos veces deja el mismo estado.)
     2. `aplicar(base, cmds)`: PARSEA cada comando y devuelve el estado
        resultante de aplicárselos a `base` en orden. Un comando mal formado
        (verbos desconocidos) se IGNORA, no aborta.

   TESTS (no tocar). COMPILACIÓN Y CORRIDA:
     g++ -std=c++20 -Wall -Wextra ejercicio_06_estado_delta.cpp -o ej && ./ej
   ============================================================================ */
#include "../../include/test_framework.hpp"
#include <string>
#include <vector>

struct EstadoGL {
    int vx = 0, vy = 0, vw = 0, vh = 0;
    bool blend = false;
    bool ztest = true;
    int  cull  = 0;
};

/* ------------------------------ TU CÓDIGO AQUÍ ---------------------------- */

// TODO 1: el delta de comandos (vacío si no cambia nada)
static std::vector<std::string> delta(const EstadoGL& prev, const EstadoGL& cur) {
    (void)prev; (void)cur;
    return {};
}

// TODO 2: aplicar comandos a un estado (parsear; verbo desconocido = ignorar)
static EstadoGL aplicar(const EstadoGL& base, const std::vector<std::string>& cmds) {
    (void)cmds;
    return base;
}

/* --------------------------- TESTS (no tocar) -------------------------- */
static bool mismoEstado(const EstadoGL& a, const EstadoGL& b) {
    return a.vx == b.vx && a.vy == b.vy && a.vw == b.vw && a.vh == b.vh &&
           a.blend == b.blend && a.ztest == b.ztest && a.cull == b.cull;
}
static bool esComando(const std::vector<std::string>& v, const std::string& cmd) {
    for (const std::string& s : v) if (s == cmd) return true;
    return false;
}

TEST_CASE(DeltaDeNada) {
    EstadoGL a;
    ASSERT_TRUE(delta(a, a).empty());          // mismo estado: 0 comandos
    EstadoGL b{10, 20, 800, 600, true, true, 1};
    ASSERT_TRUE(delta(b, b).empty());          // (pero no el default: absoluto)
}

TEST_CASE(DeltaSoloBlend) {
    EstadoGL prev;
    EstadoGL cur = prev;
    cur.blend = true;
    std::vector<std::string> d = delta(prev, cur);
    ASSERT_EQ(d.size(), (size_t)1);            // guard del stub vacío
    ASSERT_TRUE(esComando(d, "blend on"));
}

TEST_CASE(DeltaMultiplesEnOrden) {
    EstadoGL prev;
    EstadoGL cur{5, 6, 320, 240, false, false, 2};
    std::vector<std::string> d = delta(prev, cur);
    ASSERT_EQ(d.size(), (size_t)3);
    // blend NO figura (false→false); cambian viewport, ztest y cull
    ASSERT_TRUE(esComando(d, "viewport 5 6 320 240"));
    ASSERT_TRUE(esComando(d, "ztest off"));
    ASSERT_TRUE(esComando(d, "cull 2"));
}

TEST_CASE(RoundtripIdempotente) {
    EstadoGL prev;
    EstadoGL cur{12, 0, 480, 320, true, false, 1};
    std::vector<std::string> d = delta(prev, cur);
    ASSERT_FALSE(d.empty());                   // guard: si el stub devolvé {}, no seguimos
    EstadoGL rst = aplicar(prev, d);
    ASSERT_TRUE(mismoEstado(rst, cur));        // aplicar el delta deja en cur
    // IDEMPOTENCIA: el MISMO delta otra vez deja el MISMO estado
    EstadoGL otra = aplicar(rst, d);
    ASSERT_TRUE(mismoEstado(otra, cur));
}

TEST_CASE(VerboDesconocidoIgnora) {
    EstadoGL base{1, 2, 3, 4, false, true, 0};
    std::vector<std::string> cmds = {"cull 1", "voidmain sinSentido", "blend on"};
    EstadoGL r = aplicar(base, cmds);
    ASSERT_EQ(r.cull, 1);
    ASSERT_TRUE(r.blend);
    ASSERT_EQ(r.vx, 1); ASSERT_TRUE(r.ztest);  // lo que no manda el comando, NO cambia
    ASSERT_TRUE(aplicar(base, {"nada", "tampoco"}).vx == base.vx);
}

int main() { return RUN_ALL_TESTS(); }
