/* =============================================================================
   SOLUCIÓN 05: jerarquía — local↔mundo y la inversa (lección 07)
   =============================================================================
   - matrizLocal: T(pos) * R_Z(rotZ) — el factor de la DERECHA (la rotación)
     toca el punto local PRIMERO y después lo traslada.
   - compuestoDe: mundoPadre * localHijo: la traslación local del hijo viaja
     en los ejes DEL HIJO, girados por el padre... por eso el orden de la
     multiplicación ES la jerarquía entera (multiplicarlo al revés da un
     punto distinto, y el test lo demuestra con un contraejemplo).
   - localizarEnNodo: la inversa RÍGIDA del mundo del nodo aplicada al
     punto: "el punto visto EN coordenadas del nodo" (para UI anclada,
     scripts, localización de daño, editores...).
   ============================================================================= */
#include <cmath>
#include "../../include/test_framework.hpp"
#include "../include/mate3d.hpp"

using namespace mate;

// ===== SOLUCIÓN =====
static Mat4 matrizTraslacionDe(Vec3 t) {
    Mat4 m = Mat4::identidad();
    m.ponerColumna3(3, t, 1.0f);
    return m;
}
static Mat4 matrizRotacionZDe(float rad) {
    float c = std::cos(rad), s = std::sin(rad);
    Mat4 m = Mat4::identidad();
    m.ponerColumna(0, { c, s, 0.0f });
    m.ponerColumna(1, {-s, c, 0.0f });
    return m;
}

Mat4 matrizLocal(Vec3 pos, float rotZ) {
    // el punto local se ROTA primero (derecha) y DESPUÉS se traslada:
    return matrizTraslacionDe(pos) * matrizRotacionZDe(rotZ);
}

Mat4 compuestoDe(Mat4 const& mundoPadre, Mat4 const& localHijo) {
    return mundoPadre * localHijo;   // el PADRE a la izquierda: primero se
                                     // aplica el local del hijo
}

Vec3 localizarEnNodo(Mat4 const& mundoDelNodo, Vec3 puntoMundo) {
    // inversa rígida: [R^T | -R^T*t] — la mate3d la trae resuelta:
    return transformarPunto(inversaRigida(mundoDelNodo), puntoMundo);
}

// ============================= TESTS (del ejercicio — sin cambios) ==========
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
    Mat4 w = matTraslacionDada({3.0f, 1.0f, 0.0f}) * matRotacionZDada(aRadianes(37.0f));
    Vec3 puntoMundo{6.0f, 5.0f, 1.0f};
    Vec3 enLocal = localizarEnNodo(w, puntoMundo);
    Vec3 vuelta = transformarPunto(w, enLocal);
    ASSERT_NEAR(vuelta.x, 6.0f, 1e-3f);
    ASSERT_NEAR(vuelta.y, 5.0f, 1e-3f);
    ASSERT_NEAR(vuelta.z, 1.0f, 1e-3f);
}

TEST_CASE(OrdenDelCompuesto) {
    Mat4 wPadre = matTraslacionDada({10.0f, 0.0f, 0.0f})
                * matRotacionZDada(aRadianes(90.0f));
    Mat4 lHijo = matrizLocal(Vec3{2.0f, 0.0f, 0.0f}, 0.0f);
    Vec3 p = transformarPunto(compuestoDe(wPadre, lHijo), Vec3{0.0f, 0.0f, 0.0f});
    ASSERT_TRUE(casiIgualVec3(p, Vec3{10.0f, 2.0f, 0.0f}, 1e-3f));
    Mat4 alReves = matrizLocal(Vec3{2.0f, 0.0f, 0.0f}, 0.0f)
                 * (matTraslacionDada({10.0f, 0.0f, 0.0f})
                 *  matRotacionZDada(aRadianes(90.0f)));
    Vec3 pReves = transformarPunto(alReves, Vec3{0.0f, 0.0f, 0.0f});
    ASSERT_TRUE(casiIgualVec3(pReves, Vec3{12.0f, 0.0f, 0.0f}, 1e-3f));
    ASSERT_FALSE(casiIgualVec3(pReves, p, 1e-3f));
}

TEST_CASE(BrazoDelRobotCompleto) {
    Mat4 W_hombro = matrizLocal(Vec3{2.0f, 3.0f, 0.0f}, aRadianes(20.0f));
    Mat4 L_codo   = matrizLocal(Vec3{1.0f, 0.0f, 0.0f}, aRadianes(30.0f));
    Mat4 L_mano   = matrizLocal(Vec3{1.5f, 0.0f, 0.0f}, 0.0f);
    Mat4 W_codo   = compuestoDe(W_hombro, L_codo);
    Mat4 W_mano   = compuestoDe(W_codo, L_mano);
    Vec3 punta = transformarPunto(W_mano, Vec3{2.0f, 0.0f, 0.0f});
    ASSERT_NEAR(punta.x, 5.189f, 0.02f);
    ASSERT_NEAR(punta.y, 6.023f, 0.02f);
    Vec3 enMano = localizarEnNodo(W_mano, punta);
    ASSERT_TRUE(casiIgualVec3(enMano, Vec3{2.0f, 0.0f, 0.0f}, 1e-3f));
}

int main() {
    std::cout << "SOLUCIÓN 05: jerarquía local↔mundo (todos los tests DEBEN pasar)\n";
    int r = RUN_ALL_TESTS();
    if (r != 0) std::cout << "ERROR: la solución debe pasar todos los tests.\n";
    return r;
}
