/* =============================================================================
   MÓDULO 19 - EJERCICIO 06: Dirty flags — recalcular SOLO lo que cambió
   =============================================================================
   DIFICULTAD: Intermedio
   APOYO: Lección 08 (jerarquía con cache de matMundo, contagio "hacia abajo"
   del flag, recomputar en orden PADRES ANTES QUE HIJOS, y descomponer una
   world TRS — caso 2D/2.5D: rotZ + escala por eje).

   CONSIGNA:

     void marcarDirty(std::vector<Nodo>& jerarquia, int i)
        -> marca dirty A i y A TODOS SUS DESCENDIENTES
           (si ya estaba dirty, CORTÁ AHI: la poda es la mitad del rendimiento)

     int recomputar(std::vector<Nodo>& jerarquia)
        -> recorre el array PADRES ANTES QUE HIJOS: si el nodo está dirty,
           matMundo = (padre? matMundo(padre) : identidad) * matLocal(n),
           limpiar el flag y CONTAR cuántos recomputaste (devolver el contador)

     Decompos descomponer(Mat4 const& w)
        -> recuperar TRS de la world (rotZ + escala por eje, sin corte):
           escala     = (largo(col0), largo(col1), largo(col2))
           rotZ       = atan2(col0.y, col0.x)
           traslacion = col3.xyz

   TESTS (no tocar): usan el MISMO árbol de la lección 08:
     0 raíz <- 1 cuerpo <- 2 propela <- 3 escudo (cadena de 4).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_06_... -o ej06 && ./ej06
   ============================================================================= */
#include <cmath>
#include <vector>
#include "../../include/test_framework.hpp"
#include "../include/mate3d.hpp"

using namespace mate;

// Ayudas DADA (ya las ejercitaste en el 03 — van resueltas para que el 06
// se concentre en EL PUNTO: el cache y su vencimiento):
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

// DADA — la estructura del árbol (idéntica a la lección 08):
struct Nodo {
    int   parent = -1;                       // -1 = raíz
    Vec3  posicionLocal{0.0f, 0.0f, 0.0f};
    float rotZLocal     = 0.0f;
    Vec3  escalaLocal{1.0f, 1.0f, 1.0f};
    bool  mundoDirty = true;                 // EL FLAG
    Mat4  matMundo{};                        // LA CACHE (arranca vencida)
};

Mat4 matLocalDe(Nodo const& n) {             // DADA (del ejercicio 03)
    return matrizTraslacion(n.posicionLocal)
         * matrizRotacionZ(n.rotZLocal)
         * matrizEscala(n.escalaLocal);
}

struct Decompos { Vec3 escala; float rotZ; Vec3 traslacion; };

// ===== TU CÓDIGO (mirá la LECCIÓN 08) =====
void marcarDirty(std::vector<Nodo>& jerarquia, int i) {
    (void)jerarquia; (void)i;
    // TODO 1: flag en el nodo + CASCADE hacia abajo (hijos e hijos de hijos),
    //        con la poda: si el nodo YA estaba dirty, no sigas.
}

int recomputar(std::vector<Nodo>& jerarquia) {
    (void)jerarquia;
    return 0;                    // TODO 2: padres antes que hijos; contar.
}

Decompos descomponer(Mat4 const& w) {
    (void)w;
    return {};                   // TODO 3: largo de columnas + atan2 + col3
}

// ============================= TESTS (no tocar) =============================
// DADA: el árbol de la lección 08 — 0 raíz -> 1 cuerpo -> 2 propela -> 3 escudo.
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
    ASSERT_EQ(cant, 4);                       // los 4 estaban dirty
    // y el mundo del escudo (el fondo de la cadena) queda consistente:
    Vec3 escudo = transformarPunto(j[3].matMundo, Vec3{0.5f, 0.0f, 0.0f});
    ASSERT_TRUE(casiIgualVec3(escudo, Vec3{2.0f, 0.0f, 2.0f}, 1e-4f));
}

TEST_CASE(FrameLimpioNoRecalculaNada) {
    auto j = arbolDeEjemplo();
    recomputar(j);
    ASSERT_EQ(recomputar(j), 0);              // nada cambió: cache viva
}

TEST_CASE(MoverLaHojaRecomputaSoloElla) {
    auto j = arbolDeEjemplo();
    recomputar(j);
    j[3].posicionLocal.x += 0.1f;             // muevo el ESCUDO (hoja)
    marcarDirty(j, 3);
    ASSERT_EQ(recomputar(j), 1);              // solo ese nodo
    // la nueva posición del escudo quedó cacheada bien:
    Vec3 escudo = transformarPunto(j[3].matMundo, Vec3{0.5f, 0.0f, 0.0f});
    ASSERT_TRUE(casiIgualVec3(escudo, Vec3{2.1f, 0.0f, 2.0f}, 1e-4f));
}

TEST_CASE(MoverElMedioArrastraSuSubarbol) {
    auto j = arbolDeEjemplo();
    recomputar(j);
    j[1].posicionLocal.y += 1.0f;             // muevo el CUERPO
    marcarDirty(j, 1);
    ASSERT_EQ(recomputar(j), 3);              // cuerpo + propela + escudo
    Vec3 escudo = transformarPunto(j[3].matMundo, Vec3{0.5f, 0.0f, 0.0f});
    ASSERT_TRUE(casiIgualVec3(escudo, Vec3{2.0f, 1.0f, 2.0f}, 1e-4f));
}

TEST_CASE(MoverLaRaizCascadaTodo) {
    auto j = arbolDeEjemplo();
    recomputar(j);
    j[0].posicionLocal = {5.0f, 0.0f, 0.0f};  // muevo la RAÍZ
    marcarDirty(j, 0);
    ASSERT_EQ(recomputar(j), 4);              // TODOS colgaban de ella
    Vec3 escudo = transformarPunto(j[3].matMundo, Vec3{0.5f, 0.0f, 0.0f});
    ASSERT_TRUE(casiIgualVec3(escudo, Vec3{7.0f, 0.0f, 2.0f}, 1e-4f));
}

TEST_CASE(DescomponerWorldTRSIvUVuelta) {
    // la misma world del final de la lección 08:
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
    std::cout << "EJERCICIO 06: dirty flags y cache de world (lección 08)\n";
    int r = RUN_ALL_TESTS();
    if (r != 0) std::cout << "Sin resolver: los tests están fallando (OK).\n";
    return r;
}
