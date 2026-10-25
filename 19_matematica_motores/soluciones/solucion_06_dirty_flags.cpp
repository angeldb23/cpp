/* =============================================================================
   SOLUCIÓN 06: dirty flags — recalcular SOLO lo que cambió (lección 08)
   =============================================================================
   - marcarDirty: flag en el nodo + cascada hacia los DESCENDIENTES con poda
     (si ya estaba dirty, su subárbol ya está marcado: corto ahí).
   - recomputar: PARES antes que hijos (el orden del array lo garantiza),
     compone con el mundo del padre, limpia el flag y CUENTA.
   - descomponer: la world TRS se lee de sus COLUMNAS: largo de col = escala,
     atan2 de col0 = ángulo de rotación Z, col3 = traslación.
   ============================================================================= */
#include <cmath>
#include <vector>
#include "../../include/test_framework.hpp"
#include "../include/mate3d.hpp"

using namespace mate;

static Mat4 matrizTraslacion(Vec3 t) {
    Mat4 m = Mat4::identidad();
    m.ponerColumna3(3, t, 1.0f);
    return m;
}
static Mat4 matrizEscala(Vec3 s) {
    Mat4 m = Mat4::identidad();
    m.set(0, 0, s.x); m.set(1, 1, s.y); m.set(2, 2, s.z);
    return m;
}
static Mat4 matrizRotacionZ(float rad) {
    float c = std::cos(rad), s = std::sin(rad);
    Mat4 m = Mat4::identidad();
    m.ponerColumna(0, { c, s, 0.0f });
    m.ponerColumna(1, {-s, c, 0.0f });
    return m;
}
static float largoDe(Vec3 a) {
    return std::sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
}

struct Nodo {
    int   parent = -1;
    Vec3  posicionLocal{0.0f, 0.0f, 0.0f};
    float rotZLocal     = 0.0f;
    Vec3  escalaLocal{1.0f, 1.0f, 1.0f};
    bool  mundoDirty = true;
    Mat4  matMundo{};
};

Mat4 matLocalDe(Nodo const& n) {
    return matrizTraslacion(n.posicionLocal)
         * matrizRotacionZ(n.rotZLocal)
         * matrizEscala(n.escalaLocal);
}

struct Decompos { Vec3 escala; float rotZ; Vec3 traslacion; };

// ===== SOLUCIÓN =====
void marcarDirty(std::vector<Nodo>& jerarquia, int i) {
    if (jerarquia[i].mundoDirty) return;        // PODA: ya está marcado
    jerarquia[i].mundoDirty = true;
    for (int j = 0; j < (int)jerarquia.size(); ++j)
        if (jerarquia[j].parent == i)
            marcarDirty(jerarquia, j);          // contagio hacia ABAJO
}

int recomputar(std::vector<Nodo>& jerarquia) {
    int recomputes = 0;
    // el array está ordenado PADRES antes que hijos (así se lo llena):
    for (size_t i = 0; i < jerarquia.size(); ++i) {
        Nodo& n = jerarquia[i];
        if (!n.mundoDirty) continue;            // cache viva: no toco nada
        Mat4 local = matLocalDe(n);
        n.matMundo   = (n.parent < 0) ? local
                                      : jerarquia[n.parent].matMundo * local;
        n.mundoDirty = false;
        ++recomputes;
    }
    return recomputes;
}

Decompos descomponer(Mat4 const& w) {
    // column-major: la columna c es el eje local c ESTIRADO y GIRADO:
    Vec3 col0{w.at(0, 0), w.at(0, 1), w.at(0, 2)};
    Vec3 col1{w.at(1, 0), w.at(1, 1), w.at(1, 2)};
    Vec3 col2{w.at(2, 0), w.at(2, 1), w.at(2, 2)};
    Vec3 col3{w.at(3, 0), w.at(3, 1), w.at(3, 2)};
    return { {largoDe(col0), largoDe(col1), largoDe(col2)},   // escala
             std::atan2(col0.y, col0.x),                      // rotZ
             col3 };                                          // traslación
}

// ============================= TESTS (del ejercicio — sin cambios) ==========
static std::vector<Nodo> arbolDeEjemplo() {
    std::vector<Nodo> j(4);
    j[0].posicionLocal = {0.0f, 0.0f, 0.0f};
    j[1].parent = 0; j[1].posicionLocal = {1.0f, 0.0f, 0.0f};
    j[2].parent = 1; j[2].posicionLocal = {0.0f, 0.0f, 2.0f};
    j[3].parent = 2; j[3].posicionLocal = {0.5f, 0.0f, 0.0f};
    return j;
}

TEST_CASE(PrimerFrameTodoVencido) {
    auto j = arbolDeEjemplo();
    int cant = recomputar(j);
    ASSERT_EQ(cant, 4);
    Vec3 escudo = transformarPunto(j[3].matMundo, Vec3{0.5f, 0.0f, 0.0f});
    ASSERT_TRUE(casiIgualVec3(escudo, Vec3{2.0f, 0.0f, 2.0f}, 1e-4f));
}

TEST_CASE(FrameLimpioNoRecalculaNada) {
    auto j = arbolDeEjemplo();
    recomputar(j);
    ASSERT_EQ(recomputar(j), 0);
}

TEST_CASE(MoverLaHojaRecomputaSoloElla) {
    auto j = arbolDeEjemplo();
    recomputar(j);
    j[3].posicionLocal.x += 0.1f;
    marcarDirty(j, 3);
    ASSERT_EQ(recomputar(j), 1);
    Vec3 escudo = transformarPunto(j[3].matMundo, Vec3{0.5f, 0.0f, 0.0f});
    ASSERT_TRUE(casiIgualVec3(escudo, Vec3{2.1f, 0.0f, 2.0f}, 1e-4f));
}

TEST_CASE(MoverElMedioArrastraSuSubarbol) {
    auto j = arbolDeEjemplo();
    recomputar(j);
    j[1].posicionLocal.y += 1.0f;
    marcarDirty(j, 1);
    ASSERT_EQ(recomputar(j), 3);
    Vec3 escudo = transformarPunto(j[3].matMundo, Vec3{0.5f, 0.0f, 0.0f});
    ASSERT_TRUE(casiIgualVec3(escudo, Vec3{2.0f, 1.0f, 2.0f}, 1e-4f));
}

TEST_CASE(MoverLaRaizCascadaTodo) {
    auto j = arbolDeEjemplo();
    recomputar(j);
    j[0].posicionLocal = {5.0f, 0.0f, 0.0f};
    marcarDirty(j, 0);
    ASSERT_EQ(recomputar(j), 4);
    Vec3 escudo = transformarPunto(j[3].matMundo, Vec3{0.5f, 0.0f, 0.0f});
    ASSERT_TRUE(casiIgualVec3(escudo, Vec3{7.0f, 0.0f, 2.0f}, 1e-4f));
}

TEST_CASE(DescomponerWorldTRSIvUVuelta) {
    Mat4 w = matrizTraslacion({3.0f, 2.0f, 0.0f})
           * matrizRotacionZ(aRadianes(35.0f))
           * matrizEscala({2.0f, 3.0f, 1.0f});
    Decompos d = descomponer(w);
    ASSERT_TRUE(casiIgual(d.escala.x, 2.0f, 1e-3f));
    ASSERT_TRUE(casiIgual(d.escala.y, 3.0f, 1e-3f));
    ASSERT_TRUE(casiIgual(d.escala.z, 1.0f, 1e-3f));
    ASSERT_TRUE(casiIgual(d.rotZ, aRadianes(35.0f), 1e-3f));
    ASSERT_TRUE(casiIgualVec3(d.traslacion, Vec3{3.0f, 2.0f, 0.0f}, 1e-5f));
}

int main() {
    std::cout << "SOLUCIÓN 06: dirty flags (todos los tests DEBEN pasar)\n";
    int r = RUN_ALL_TESTS();
    if (r != 0) std::cout << "ERROR: la solución debe pasar todos los tests.\n";
    return r;
}
