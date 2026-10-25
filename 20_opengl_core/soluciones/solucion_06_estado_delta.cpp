/* =============================================================================
   MÓDULO 20 - SOLUCIÓN 06: Estado GL cacheado (delta de comandos)
   =============================================================================
   DIFICULTAD: Media
   APOYO: Lección 13. Resuelve el ejercicio_06_estado_delta.cpp.

   El delta con VALORES ABSOLUTOS (no on/off relativos) es idempotente:
   mandar el mismo delta dos veces al GPU deja el mismo estado. Eso es lo
   que hace seguro re-enviar el comando tras un "glGetError() ==".
   ============================================================================ */
#include "../../include/test_framework.hpp"
#include <sstream>
#include <string>
#include <vector>

struct EstadoGL {
    int vx = 0, vy = 0, vw = 0, vh = 0;
    bool blend = false;
    bool ztest = true;
    int  cull  = 0;
};

// TODO 1
static std::vector<std::string> delta(const EstadoGL& prev, const EstadoGL& cur) {
    std::vector<std::string> cmds;
    if (prev.vx != cur.vx || prev.vy != cur.vy || prev.vw != cur.vw || prev.vh != cur.vh) {
        cmds.push_back("viewport " + std::to_string(cur.vx) + " " +
                       std::to_string(cur.vy) + " " + std::to_string(cur.vw) + " " +
                       std::to_string(cur.vh));
    }
    if (prev.blend != cur.blend) cmds.push_back(cur.blend ? "blend on" : "blend off");
    if (prev.ztest != cur.ztest) cmds.push_back(cur.ztest ? "ztest on" : "ztest off");
    if (prev.cull != cur.cull)   cmds.push_back("cull " + std::to_string(cur.cull));
    return cmds;
}

// TODO 2
static EstadoGL aplicar(const EstadoGL& base, const std::vector<std::string>& cmds) {
    EstadoGL e = base;
    for (const std::string& cmd : cmds) {
        std::istringstream in(cmd);
        std::string verbo;
        if (!(in >> verbo)) continue;
        if (verbo == "viewport") {
            int a, b, c, d;
            if (in >> a >> b >> c >> d) { e.vx = a; e.vy = b; e.vw = c; e.vh = d; }
        } else if (verbo == "blend") {
            std::string v;
            if (in >> v) e.blend = (v == "on");
        } else if (verbo == "ztest") {
            std::string v;
            if (in >> v) e.ztest = (v == "on");
        } else if (verbo == "cull") {
            int c;
            if (in >> c) e.cull = c;
        }
        // verbo desconocido: se ignora
    }
    return e;
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
    ASSERT_TRUE(delta(a, a).empty());
    EstadoGL b{10, 20, 800, 600, true, true, 1};
    ASSERT_TRUE(delta(b, b).empty());
}

TEST_CASE(DeltaSoloBlend) {
    EstadoGL prev;
    EstadoGL cur = prev;
    cur.blend = true;
    std::vector<std::string> d = delta(prev, cur);
    ASSERT_EQ(d.size(), (size_t)1);
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
    ASSERT_FALSE(d.empty());
    EstadoGL rst = aplicar(prev, d);
    ASSERT_TRUE(mismoEstado(rst, cur));
    EstadoGL otra = aplicar(rst, d);
    ASSERT_TRUE(mismoEstado(otra, cur));
}

TEST_CASE(VerboDesconocidoIgnora) {
    EstadoGL base{1, 2, 3, 4, false, true, 0};
    std::vector<std::string> cmds = {"cull 1", "voidmain sinSentido", "blend on"};
    EstadoGL r = aplicar(base, cmds);
    ASSERT_EQ(r.cull, 1);
    ASSERT_TRUE(r.blend);
    ASSERT_EQ(r.vx, 1); ASSERT_TRUE(r.ztest);
    ASSERT_TRUE(aplicar(base, {"nada", "tampoco"}).vx == base.vx);
}

int main() { return RUN_ALL_TESTS(); }
