/* =============================================================================
   SOLUCIÓN 04: cámara y MVP — base de cámara, vista, clip y NDC (lección 06)
   =============================================================================
   - matrizVista: la cámara como OBJETO (columnas [right, upCam, -dir,
     pos]) y la INVERSA RÍGIDA (mate3d la trae) como V: la vista es la
     colocación de la cámara "al revés".
   - proyectarConMVP: el punto como Vec4 con w=1 y el orden DE APLICACIÓN:
     primero M, después V, después P (la cadena se escribe P * V * M pero
     el factor de la DERECHA es el primero en tocar al punto).
   - aNdc: dividir por w. Si w <= 0 el punto NO se ve (está detrás o en el
     plano mismo): devolvemos cero y el TEST/llamador decide con el w.
   ============================================================================= */
#include <cmath>
#include "../../include/test_framework.hpp"
#include "../include/mate3d.hpp"

using namespace mate;

// Ayuda DADA (ejercicio 01/02, resueltas ahí):
float productoEscalarDeEje(Vec3 a, Vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
Vec3  productoVectorialDeEje(Vec3 a, Vec3 b) {
    return { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
}
float largoDe(Vec3 a) { return std::sqrt(productoEscalarDeEje(a, a)); }
Vec3  normalizarDe(Vec3 a) {
    float l = largoDe(a);
    if (l <= 1e-8f) return {0.0f, 0.0f, 0.0f};
    return a / l;
}

// Ayuda DADA (lección 11, por adelantado): perspectiva con fov=90°:
Mat4 perspectiva90(float aspecto) {
    float fcero = (11.0f + 1.0f) / (11.0f - 1.0f);   // = 1.2
    Mat4 p{};
    p.set(0, 0, 1.0f / aspecto);
    p.set(1, 1, 1.0f);
    p.set(2, 2, -fcero);
    p.set(2, 3, -1.0f);
    p.set(3, 2, -(2.0f * 11.0f * 1.0f) / (11.0f - 1.0f));
    return p;
}

// ===== SOLUCIÓN =====
Mat4 matrizVista(Vec3 pos, Vec3 target, Vec3 up) {
    // 1) la dirección HACIA donde mira la cámara:
    Vec3 dir = normalizarDe(target - pos);
    // 2) el "right" de la cámara:
    Vec3 right = normalizarDe(productoVectorialDeEje(dir, up));
    // 3) el "up" REAL de la cámara (perpendicular a los otros dos):
    Vec3 upCam = productoVectorialDeEje(right, dir);
    // 4) la COLOCACIÓN de la cámara como objeto del mundo:
    Mat4 colocacion = Mat4::identidad();
    colocacion.ponerColumna(0, right);      // X local
    colocacion.ponerColumna(1, upCam);      // Y local
    colocacion.ponerColumna(2, -dir);       // Z local: hacia ATRÁS (RH)
    colocacion.ponerColumna3(3, pos, 1.0f); // dónde está el "cero" de ella
    // 5) la vista es como la colocación INVERTIDA (inversaRigida la trae):
    return inversaRigida(colocacion);
}

Vec4 proyectarConMVP(Mat4 const& P, Mat4 const& V, Mat4 const& M, Vec3 p) {
    Vec4 puntoHomogeneo{p.x, p.y, p.z, 1.0f};
    return P * (V * (M * puntoHomogeneo));    // M primero, V, y P al final
}

Vec3 aNdc(Vec4 clip) {
    if (clip.w <= 0.0f) return {0.0f, 0.0f, 0.0f};   // detrás: NO se ve
    return {clip.x / clip.w, clip.y / clip.w, clip.z / clip.w};
}

// ============================= TESTS (del ejercicio — sin cambios) ==========
TEST_CASE(BaseDeCamaraMirandoAlOrigen) {
    Mat4 v = matrizVista(Vec3{0.0f, 0.0f, 5.0f}, Vec3{0.0f, 0.0f, 0.0f},
                         V3Y);
    Vec3 posEnVista = transformarPunto(v, Vec3{0.0f, 0.0f, 5.0f});
    ASSERT_TRUE(casiIgualVec3(posEnVista, Vec3{0.0f, 0.0f, 0.0f}, 1e-4f));
    Vec3 origenEnVista = transformarPunto(v, Vec3{0.0f, 0.0f, 0.0f});
    ASSERT_NEAR(origenEnVista.x, 0.0f, 1e-4f);
    ASSERT_NEAR(origenEnVista.y, 0.0f, 1e-4f);
    ASSERT_NEAR(origenEnVista.z, -5.0f, 1e-4f);
}

TEST_CASE(ClipConModeloTriviado) {
    Mat4 P = perspectiva90(1.0f);
    Mat4 V = matrizVista(Vec3{0.0f, 0.0f, 5.0f}, V3CERO, V3Y);
    Mat4 M = Mat4::identidad();
    Vec4 clip = proyectarConMVP(P, V, M, Vec3{0.0f, 0.0f, 0.0f});
    ASSERT_NEAR(clip.w, 5.0f, 1e-3f);
    Vec3 ndc = aNdc(clip);
    ASSERT_NEAR(ndc.x, 0.0f, 1e-3f);
    ASSERT_NEAR(ndc.y, 0.0f, 1e-3f);
    ASSERT_TRUE(ndc.z > 0.7f && ndc.z < 0.8f);
    Vec4 clipDetras = proyectarConMVP(P, V, M, Vec3{0.0f, 0.0f, 10.0f});
    ASSERT_TRUE(clipDetras.w < 0.0f);
}

TEST_CASE(NdcFueraDelFrustum) {
    Mat4 P = perspectiva90(1.0f);
    Mat4 V = matrizVista(Vec3{0.0f, 0.0f, 5.0f}, V3CERO, V3Y);
    Vec4 clip = proyectarConMVP(P, V, Mat4::identidad(),
                                Vec3{4.9f, 0.0f, 0.0f});
    Vec3 ndc = aNdc(clip);
    ASSERT_TRUE(ndc.x > 0.9f);
}

int main() {
    std::cout << "SOLUCIÓN 04: cámara y MVP (todos los tests DEBEN pasar)\n";
    int r = RUN_ALL_TESTS();
    if (r != 0) std::cout << "ERROR: la solución debe pasar todos los tests.\n";
    return r;
}
