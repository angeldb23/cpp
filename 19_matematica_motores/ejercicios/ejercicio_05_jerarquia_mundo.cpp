/* =============================================================================
   MÓDULO 19 - EJERCICIO 05: Jerarquía — local↔mundo y la inversa
   =============================================================================
   DIFICULTAD: Intermedio
   APOYO: Lección 07 (matMundo(padre) * matLocal(hijo); inversaRigida ya está
   en mate3d.hpp; matrizSRT se practicó en el ejercicio 03 — para no repetir,
   ACÁ el matLocal de cada nodo es UNA rotación Z + traslación, sin escala).

   CONSIGNA:

     Mat4 matrizLocal(Vec3 pos, float rotZ)
        -> T(pos) * R_Z(rotZ)   (en column-major el factor de la DERECHA se
           aplica primero: el punto local se rota y DESPUÉS se traslada)

     Mat4 compuestoDe(Mat4 const& mundoPadre, Mat4 const& localHijo)
        -> mundoPadre * localHijo   (¡el ORDEN es la lección entera!)

     Vec3 localizarEnNodo(Mat4 const& mundoDelNodo, Vec3 puntoMundo)
        -> puntoMundo visto EN EL ESPACIO del nodo:
           transformarPunto(inversaRigida(mundoDelNodo), puntoMundo)

   TESTS (no tocar): recomponen el brazo del robot de la lección:
     hombro en (2,3) rot 20° -> codo local (1,0) rot 30° -> mano local (1.5,0)
     y la punta del dedo (local de mano (2,0,0)) da (5.189, 6.023) en mundo.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_05_... -o ej05 && ./ej05
   ============================================================================= */
#include <cmath>
#include "../../include/test_framework.hpp"
#include "../include/mate3d.hpp"

using namespace mate;

// ===== TU CÓDIGO (mirá la LECCIÓN 07) =====
Mat4 matrizLocal(Vec3 pos, float rotZ) {
    (void)pos; (void)rotZ;
    return Mat4::identidad();   // TODO 1: T(pos) * R_Z(rotZ) — armá T y R a mano
}

Mat4 compuestoDe(Mat4 const& mundoPadre, Mat4 const& localHijo) {
    (void)mundoPadre; (void)localHijo;
    return Mat4::identidad();   // TODO 2: el producto GRANDE (¿quién va a la
                                //        izquierda y quién a la derecha?)
}

Vec3 localizarEnNodo(Mat4 const& mundoDelNodo, Vec3 puntoMundo) {
    (void)mundoDelNodo; (void)puntoMundo;
    return {0.0f, 0.0f, 0.0f};  // TODO 3: transformarPunto con la INVERSA
}

// ============================= TESTS (no tocar) =============================
// (ayudas DADA: traslación y rotación Z armadas a mano, del ejercicio 03)
static Mat4 matTraslacionDada(Vec3 t) {
    Mat4 m = Mat4::identidad();
    m.ponerColumna3(3, t, 1.0f);
    return m;
}
static Mat4 matRotacionZDada(float rad) {
    float c = std::cos(rad), s = std::sin(rad);
    Mat4 m = Mat4::identidad();
    m.ponerColumna(0, { c, s, 0.0f });
    m.ponerColumna(1, {-s, c, 0.0f });
    return m;
}

TEST_CASE(RoundtripDeUnPunto) {
    // Un nodo con rotación 37° y traslación (3,1,0), y un punto del mundo:
    Mat4 w = matTraslacionDada({3.0f, 1.0f, 0.0f}) * matRotacionZDada(aRadianes(37.0f));
    Vec3 puntoMundo{6.0f, 5.0f, 1.0f};
    Vec3 enLocal = localizarEnNodo(w, puntoMundo);
    // transformando el local con el W del nodo VUELVE al punto del mundo:
    Vec3 vuelta = transformarPunto(w, enLocal);
    ASSERT_NEAR(vuelta.x, 6.0f, 1e-3f);
    ASSERT_NEAR(vuelta.y, 5.0f, 1e-3f);
    ASSERT_NEAR(vuelta.z, 1.0f, 1e-3f);
}

TEST_CASE(OrdenDelCompuesto) {
    // hijo (2,0,0) local + padre T(10,0,0)*R(90°): la persona que espera en
    // el mundo al origen del HIJO mide (10,2,0): la traslación local va en
    // el eje +x DEL HIJO, que el padre lo giró 90° hacia +y:
    Mat4 wPadre = matTraslacionDada({10.0f, 0.0f, 0.0f})
                * matRotacionZDada(aRadianes(90.0f));
    Mat4 lHijo = matrizLocal(Vec3{2.0f, 0.0f, 0.0f}, 0.0f);
    Vec3 p = transformarPunto(compuestoDe(wPadre, lHijo), Vec3{0.0f, 0.0f, 0.0f});
    ASSERT_TRUE(casiIgualVec3(p, Vec3{10.0f, 2.0f, 0.0f}, 1e-3f));
    // ¿el orden INVERSO da lo mismo? lHijo * wPadre: el punto (0,0,0) pasa
    // PRIMERO por el padre ((10,0,0) sin rotarlo por... el hijo traslada el
    // RESULTADO: da (12,0,0) — un punto DISTINTO. El orden ES la matemática:
    Mat4 alReves = matrizLocal(Vec3{2.0f, 0.0f, 0.0f}, 0.0f)
                 * (matTraslacionDada({10.0f, 0.0f, 0.0f})
                 *  matRotacionZDada(aRadianes(90.0f)));
    Vec3 pReves = transformarPunto(alReves, Vec3{0.0f, 0.0f, 0.0f});
    ASSERT_TRUE(casiIgualVec3(pReves, Vec3{12.0f, 0.0f, 0.0f}, 1e-3f));
    ASSERT_FALSE(casiIgualVec3(pReves, p, 1e-3f));   // demuestran que NO es igual
}

TEST_CASE(BrazoDelRobotCompleto) {
    // hombro (2,3) rot 20°, codo local (1,0) rot 30°, mano local (1.5, 0):
    Mat4 W_hombro = matrizLocal(Vec3{2.0f, 3.0f, 0.0f}, aRadianes(20.0f));
    Mat4 L_codo   = matrizLocal(Vec3{1.0f, 0.0f, 0.0f}, aRadianes(30.0f));
    Mat4 L_mano   = matrizLocal(Vec3{1.5f, 0.0f, 0.0f}, 0.0f);
    Mat4 W_codo   = compuestoDe(W_hombro, L_codo);
    Mat4 W_mano   = compuestoDe(W_codo, L_mano);
    // el DEDO de la mano (punta local (2,0,0)) en el mundo:
    Vec3 punta = transformarPunto(W_mano, Vec3{2.0f, 0.0f, 0.0f});
    // el valor de la lección 07:
    ASSERT_NEAR(punta.x, 5.189f, 0.02f);
    ASSERT_NEAR(punta.y, 6.023f, 0.02f);
    // Y LA INVERSA: de mundo a local de la mano recupera los (2,0,0):
    Vec3 enMano = localizarEnNodo(W_mano, punta);
    ASSERT_TRUE(casiIgualVec3(enMano, Vec3{2.0f, 0.0f, 0.0f}, 1e-3f));
}

int main() {
    std::cout << "EJERCICIO 05: jerarquía local↔mundo (lección 07)\n";
    int r = RUN_ALL_TESTS();
    if (r != 0) std::cout << "Sin resolver: los tests están fallando (OK).\n";
    return r;
}
