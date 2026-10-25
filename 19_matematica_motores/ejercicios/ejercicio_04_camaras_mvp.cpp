/* =============================================================================
   MÓDULO 19 - EJERCICIO 04: Cámara y MVP — base de cámara, vista y NDC
   =============================================================================
   DIFICULTAD: Intermedio
   APOYO: Lección 06 (cadena P * V * M, look-at, clip space y NDC; la
   perspectiva completa es el ejercicio 09).

   CONSIGNA (right-handed, la cámara mira hacia -z en SU espacio):

     float productoEscalarDeEje(Vec3 a, Vec3 b)
        -> dot (ya lo sabés: ej 02; lo repetí acá para que el ejercicio
           compile solo y puedas apoyar TODO en las funciones tuyas)

     Vec3 productoVectorialDeEje(Vec3 a, Vec3 b)
        -> cross (ídem)

     MatrizVista matrizVista(Vec3 pos, Vec3 target, Vec3 up)
        -> 1) dir = normaliz(target - pos)
           2) right = normaliz(cross(dir, up))
           3) upCam = cross(right, dir)
           4) matrix "colocación de cámara" = columnas [right, upCam, -dir, pos]
              y la INVERSA RIGIDA (inversaRigida está en mate3d.hpp) como V.

     Vec4 proyectarConMVP(Mat4 const& P, Mat4 const& V, Mat4 const& M, Vec3 p)
        -> el CLIP: transformar como PuntoHomogeneo (p, 1):
           clip = P * V * M * (p, 1)

     Vec3 aNdc(Vec4 clip)
        -> dividir por w: ndc = clip.xyz / clip.w
           (SI w > 0: si w <= 0 el punto está DETRÁS de la cámara: devolvé
            un (0, 0, 0) y que el test de w lo decida)

   TESTS (no tocar): fallan hasta las 5 funciones. La perspectiva usada en
   los tests está definida abajo con fórmula explícita (léela!).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_04_... -o ej04 && ./ej04
   ============================================================================= */
#include <cmath>
#include "../../include/test_framework.hpp"
#include "../include/mate3d.hpp"

using namespace mate;

// Ayuda DADA: los ejes localizados en el espacio normalizado (ej 01/02).
float productoEscalarDeEje(Vec3 a, Vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
Vec3  productoVectorialDeEje(Vec3 a, Vec3 b) {
    return { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
}
float largoDe(Vec3 a) {
    return std::sqrt(productoEscalarDeEje(a, a));
}
Vec3  normalizarDe(Vec3 a) {
    float l = largoDe(a);
    if (l <= 1e-8f) return {0.0f, 0.0f, 0.0f};
    return a / l;
}

// ===== TU CÓDIGO (mirá la LECCIÓN 06) =====
Mat4 matrizVista(Vec3 pos, Vec3 target, Vec3 up) {
    (void)pos; (void)target; (void)up;
    return Mat4::identidad();   // TODO 1: los ejes de la cámara en COLUMNAS
                                //        y la inversaRigida de esa colocación
}

Vec4 proyectarConMVP(Mat4 const& P, Mat4 const& V, Mat4 const& M, Vec3 p) {
    (void)P; (void)V; (void)M; (void)p;
    return {0.0f, 0.0f, 0.0f, 0.0f};   // TODO 2: clip = P * V * M * (p, 1)
}

Vec3 aNdc(Vec4 clip) {
    (void)clip;
    return {0.0f, 0.0f, 0.0f};   // TODO 3: clip.xyz / clip.w
}

// Ayuda DADA (lección 11, por adelantado): proyecto con fixed fov=90°:
Mat4 perspectiva90(float aspecto) {
    // fovY=90 -> f = 1: con RH y depth [-1..1], introduce n=1, f=11:
    float fcero = (11.0f + 1.0f) / (11.0f - 1.0f);   // = 1.2
    Mat4 p{};
    p.set(0, 0, 1.0f / aspecto);    // fovY=90 -> f=1; m00 = f/aspect
    p.set(1, 1, 1.0f);
    p.set(2, 2, -fcero);
    p.set(2, 3, -1.0f);
    p.set(3, 2, -(2.0f * 11.0f * 1.0f) / (11.0f - 1.0f));  // -2.2
    return p;
}

// ============================= TESTS (no tocar) =============================
TEST_CASE(BaseDeCamaraMirandoAlOrigen) {
    // cámara en (0,0,5) mirando al origen: right=+X, upCam=+Y, atrás=+Z (dir=-Z)
    Mat4 v = matrizVista(Vec3{0.0f, 0.0f, 5.0f}, Vec3{0.0f, 0.0f, 0.0f},
                         V3Y);
    // el punto DE LA CÁMARA queda en el origen en espacio de vista:
    Vec3 posEnVista = transformarPunto(v, Vec3{0.0f, 0.0f, 5.0f});
    ASSERT_TRUE(casiIgualVec3(posEnVista, Vec3{0.0f, 0.0f, 0.0f}, 1e-4f));
    // el ORIGEN del mundo queda EN FRENTE (z negativo en vista):
    Vec3 origenEnVista = transformarPunto(v, Vec3{0.0f, 0.0f, 0.0f});
    ASSERT_NEAR(origenEnVista.x, 0.0f, 1e-4f);
    ASSERT_NEAR(origenEnVista.y, 0.0f, 1e-4f);
    ASSERT_NEAR(origenEnVista.z, -5.0f, 1e-4f);
}

TEST_CASE(ClipConModeloTriviado) {
    Mat4 P = perspectiva90(1.0f);     // aspect = 1
    Mat4 V = matrizVista(Vec3{0.0f, 0.0f, 5.0f}, V3CERO, V3Y);
    Mat4 M = Mat4::identidad();
    // punto delante en la mira (0,0,0): en vista z=-5 => clip.w=+5
    Vec4 clip = proyectarConMVP(P, V, M, Vec3{0.0f, 0.0f, 0.0f});
    ASSERT_NEAR(clip.w, 5.0f, 1e-3f);
    Vec3 ndc = aNdc(clip);
    ASSERT_NEAR(ndc.x, 0.0f, 1e-3f);      // centrado horizontal
    ASSERT_NEAR(ndc.y, 0.0f, 1e-3f);      // centrado vertical
    ASSERT_TRUE(ndc.z > 0.7f && ndc.z < 0.8f);   // (z=0.7777… con n=1, f=11)
    // punto DETRÁS de la cámara: w < 0 (el NDC se invierte o descarta):
    Vec4 clipDetras = proyectarConMVP(P, V, M, Vec3{0.0f, 0.0f, 10.0f});
    ASSERT_TRUE(clipDetras.w < 0.0f);
}

TEST_CASE(NdcFueraDelFrustum) {
    // t=2 con punto FUERA del frustum: el NDC campa fuera (|ndc|>1):
    Mat4 P = perspectiva90(1.0f);
    Mat4 V = matrizVista(Vec3{0.0f, 0.0f, 5.0f}, V3CERO, V3Y);
    Vec4 clip = proyectarConMVP(P, V, Mat4::identidad(),
                                Vec3{4.9f, 0.0f, 0.0f});  // cerca del plano z=0
    Vec3 ndc = aNdc(clip);
    ASSERT_TRUE(ndc.x > 0.9f);   // bien AL COSTADO: fuera del frustum
}

int main() {
    std::cout << "EJERCICIO 04: cámara y MVP (lección 06)\n";
    int r = RUN_ALL_TESTS();
    if (r != 0) std::cout << "Sin resolver: los tests están fallando (OK).\n";
    return r;
}
