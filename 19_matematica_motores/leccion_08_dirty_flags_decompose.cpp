/* =============================================================================
   MÓDULO 19 - LECCIÓN 08: Dirty flags y cache de world matrices — descomposición
   =============================================================================
   TEMA 4 de TEMAS.md (2da lección): jerarquía con dirty flags y cache.

   == EL COSTO DE "RECALCULAR TODO" ==========================================

   La L07 compuso la cadena completa para cada nodo en cada frame:
   recalcular el mundo de TODO el árbol cuando solo MOVERON una hoja es
   RAFAGA de multiplicaciones al pedo (una jerarquía del juego tiene miles
   de nodos: huesos, física, shaders, partículas-ancord...).

   == DIRTY FLAGS: recalcular SOLO lo que cambió ==============================

   La idea es simple y LA época clásica del "cachear con flag":

       struct Nodo {
           ...
           bool mundoDirty = true;   // "mi matMundo está vencida"
           Mat4 matMundo;            // la CACHE
       };

   Reglas:
     1) CAMBIAR algo local (posición, rotación, escala) marca dirty AL NODO
        y a TODOS sus DESCENDIENTES (su mundo depende de ancestors:
        un descendiente depende del mundo del ancestro: si el ancestro
        cambia, TODOS los hijos cambian: "contagio hacia abajo").
     2) La RECOMPUTACIÓN se hace por edición, PLANO: primero sube para
        dejar consistente cada descendiente (el algoritm estándar).
     3) Si un nodo NO está dirty, SU SUBÁRBOL COMPLETO no re-computa
        (por eso la cascada solo "hacia abajo": los descendientes vuelven
        heredar mi NEW mundo al computar — el algoritmo mayores no afetent).
   (La version correcto más general: un DAG completo de matemática con
   "sukładnost topológica". Acá vamos con "padres antes que hijos" en orden
   del array — es lo que analizamos declarativo SOA en L17.)

   == DESCOMPOSICIÓN TRS (rotación z + escala: caso común en 2D/2.5D) =======

   PERO MIRA: NO TODO es directo. ALGUNS veces estás dado una MUNDO y el
   juego necesita RECUPERAR los datos locales. Para matrices del curso TRS
   (escala positiva uniforme por ejes, rotación z — caso común de 2D/2.5D del
   módulo 17), la matMundo column-major permite descomponer:

       scale.x = largo(col0)      (columna 0 es donde termina el eje X local)
       scale.y = largo(col1)
       rotZ    = atan2(col0.y, col0.x)   (ángulo del eje X local en el mundo)
       traslación = col3.xyz

   Limitación (que la demo anota): con ROTACIÓN EN 3 EJES o CORTE (shear)
   NO funciona. Para el caso general: descomponer con cuaterniones (L09-L10)
   o usar glm::quat_cast(glm::mat3(m3)) (verificado en gtc/quaternion.hpp
   — la función quat_cast(Mat3) del 1.0.1) + normalizar columnas.

   == DEMO ====================================================================

   La demo marca dirty a la RAÍZ y sube mis covers con contador:
     - un frame "nada cambia"       → 0 recomputes
     - "mover una hoja"             → solo su subárbol
     - "mover la raíz"              → TODO el árbol
   Y descompone una world para verificar ida-vuelta.
   ============================================================================= */

#include "include/mate3d.hpp"
#include <cstdio>
#include <iostream>
#include <vector>

using namespace mate;

// ==== PRIMITIVAS LOCALES (L03/L04/L05) =====================================
float productoEscalar(Vec3 a, Vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
float largo(Vec3 a) { return std::sqrt(productoEscalar(a, a)); }
Mat4 matrizTraslacion(Vec3 t) {
    Mat4 m = Mat4::identidad();
    m.ponerColumna3(3, t, 1.0f);
    return m;
}
Mat4 matrizEscala(Vec3 s) {
    Mat4 m = Mat4::identidad();
    m.set(0, 0, s.x); m.set(1, 1, s.y); m.set(2, 2, s.z);
    return m;
}
Mat4 matrizRotacionZ(float rad) {
    float c = std::cos(rad), s = std::sin(rad);
    Mat4 m = Mat4::identidad();
    m.ponerColumna(0, { c, s, 0.0f });
    m.ponerColumna(1, {-s, c, 0.0f });
    return m;
}
// ===========================================================================

struct Nodo {
    int   parent = -1;                      // índice en la jerarquía; -1 = raíz
    Vec3  posicionLocal{0.0f, 0.0f, 0.0f};
    float rotZLocal     = 0.0f;
    Vec3  escalaLocal{1.0f, 1.0f, 1.0f};
    bool  mundoDirty = true;                // el CAMBIO con marcar
    Mat4  matMundo{};                       // LA CACHE
};

Mat4 matLocalDe(Nodo const& n) {
    return matrizTraslacion(n.posicionLocal)
         * matrizRotacionZ(n.rotZLocal)
         * matrizEscala(n.escalaLocal);
}

void marcarDirty(std::vector<Nodo>& jerarquia, int i) {
    if (jerarquia[i].mundoDirty) return;        // ya marcado: el HIT nº
    jerarquia[i].mundoDirty = true;             // el FLINGO
    for (int j = 0; j < (int)jerarquia.size(); ++j) {
        if (jerarquia[j].parent == i)
            marcarDirty(jerarquia, j);          // contagio hacia ABAJO
    }
}

// Recorre PADRES ANTES QUE HIJOS (orden del array) y recompone de congregación.
int recomputar(std::vector<Nodo>& jerarquia) {
    int recomputes = 0;
    for (size_t i = 0; i < jerarquia.size(); ++i) {
        Nodo& n = jerarquia[i];
        if (!n.mundoDirty) continue;  // nodo limpio: este y su subárbol quedan
        Mat4 local   = matLocalDe(n);
        n.matMundo = (n.parent < 0) ? local : jerarquia[n.parent].matMundo * local;
        n.mundoDirty = false;
        ++recomputes;
    }
    return recomputes;
}

// Descomposición (2D/2.5D: rotZ + escala positiva). Devuelve por STRUT.
struct Decompos { Vec3 escala; float rotZ; Vec3 traslacion; };
Decompos descomponer(Mat4 const& w) {
    Vec3 col0{w.at(0,0), w.at(0,1), w.at(0,2)};
    Vec3 col1{w.at(1,0), w.at(1,1), w.at(1,2)};
    Vec3 col2{w.at(2,0), w.at(2,1), w.at(2,2)};
    Vec3 col3{w.at(3,0), w.at(3,1), w.at(3,2)};   // la COLUMNA 3 es la traslación
    Decompos d{};
    d.escala     = {largo(col0), largo(col1), largo(col2)};
    d.rotZ       = std::atan2(col0.y, col0.x);
    d.traslacion = col3;
    return d;
}

// ==== DEMO ================================================================
int main() {
    std::vector<Nodo> jerarquia(4);
    // 0 raiz (avion) -> 1 cuerpo -> 2 propela ; 2-> 3 escudo
    jerarquia[0].posicionLocal = {0.0f, 0.0f, 0.0f};
    jerarquia[1].parent = 0; jerarquia[1].posicionLocal = {1.0f, 0.0f, 0.0f};
    jerarquia[2].parent = 1; jerarquia[2].posicionLocal = {0.0f, 0.0f, 2.0f};
    jerarquia[3].parent = 2; jerarquia[3].posicionLocal = {0.5f, 0.0f, 0.0f};

    std::cout << "== EL ARBOL (0 raiz - 1..3 hijos) ==\n";
    std::cout << "  raiz(0)\n    └── cuerpo(1)\n          └── propela(2)\n"
                 "                └── escudo(3)\n";

    int r = recomputar(jerarquia);
    std::cout << "primer frame (todo dirty): recomputes = " << r << "\n";

    r = recomputar(jerarquia);
    std::cout << "frame sin cambios:        recomputes = " << r << " (nada cambió)\n";

    // muevo la HOJA: solo esa recomputa
    jerarquia[3].posicionLocal.x += 0.1f;
    marcarDirty(jerarquia, 3);
    r = recomputar(jerarquia);
    std::cout << "mover la hoja escudo:    recomputes = " << r << " (subárbol chico)\n";

    // muevo la RAIZ: TODA la cascada
    jerarquia[0].posicionLocal.y += 0.5f;
    marcarDirty(jerarquia, 0);
    r = recomputar(jerarquia);
    std::cout << "mover la raíz:           recomputes = " << r << " (todo cuelga)\n";

    std::cout << "\n== DESCOMPOSICIÓN TRS (idas y vueltas) ==\n";
    Mat4 w = matrizTraslacion({3.0f, 2.0f, 0.0f})
           * matrizRotacionZ(aRadianes(35.0f))
           * matrizEscala({2.0f, 3.0f, 1.0f});
    Decompos d = descomponer(w);
    std::printf("descompone: escala=(%.3f, %.3f, %.3f) rotZ=%.1f° trasl=(%.1f, %.1f, %.1f)\n",
                d.escala.x, d.escala.y, d.escala.z, aGrados(d.rotZ),
                d.traslacion.x, d.traslacion.y, d.traslacion.z);
    std::cout << "(rotación 3D general o corte: cuaterniones — L09/L10 y "
                 "glm::quat_cast(mat3) verificado del 1.0.1)\n";

    std::cout << "=== FIN LECCIÓN 08 ===\n";
    return 0;
}
