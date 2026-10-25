/* =============================================================================
   MÓDULO 19 - EJERCICIO 09: Frustum y proyección — perspectiva, orto, culling
   =============================================================================
   DIFICULTAD: Avanzado
   APOYO: Lección 11 (las mismas fórmulas de la lección; la matriz VISTA se
   ejercitó en el 04 — acá va DADA. El glDepth del [0..1] de Vulkan/D3D se
   ve en el módulo 26: acá es GL puro, NDC z en [-1..1]).

   CONSIGNA (right-handed, depth NDC [-1..1], column-major como GLM):

     Mat4 matrizPerspectiva(float fovYRad, float aspect, float cerca, float lejos)
        -> f = 1 / tan(fovY/2)
           m(0,0) = f / aspect        m(1,1) = f
           m(2,2) = -(lejos+cerca)/(lejos-cerca)     m(2,3) = -1
           m(3,2) = -(2 * lejos * cerca)/(lejos-cerca)
           (TODOS los demás elementos de la matriz son 0)

     Mat4 matrizOrtografica(float izq, float der, float abajo, float arriba,
                            float cerca, float lejos)
        -> m(0,0) = 2/(der-izq)      m(1,1) = 2/(arriba-abajo)
           m(2,2) = -2/(lejos-cerca)
           m(0,3) = -(der+izq)/(der-izq)     m(1,3) = -(arriba+abajo)/(arriba-abajo)
           m(2,3) = -(lejos+cerca)/(lejos-cerca)     m(3,3) = 1

     void extraerPlanos(Mat4 const& vp, Plano planos[6])
        -> Gribb-Hartmann, para VP = P * V (column-major, NDC [-1..1]):
           fila_i de VP = (vp.at(0,i), vp.at(1,i), vp.at(2,i), vp.at(3,i))
           izquierda = fila3 + fila0     derecha = fila3 - fila0
           abajo     = fila3 + fila1     arriba  = fila3 - fila1
           cerca     = fila3 + fila2     lejos   = fila3 - fila2

   La ecuación de plano queda (a, b, c, d): un punto está DENTRO del plano
   cuando a*x + b*y + c*z + d >= 0. Frustum completo = los 6 planos a favor.
   La normal (a,b,c) NO va normalizada (solo hace falta para MEDIR distancia).

   TESTS (no tocar): fallan hasta las 3 funciones. La cámara mira desde
   (0,0,10) al origen: en vista, los puntos del jugador van con z negativo.
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_09_... -o ej09 && ./ej09
   ============================================================================= */
#include <cmath>
#include "../../include/test_framework.hpp"
#include "../include/mate3d.hpp"

using namespace mate;

// Ayuda DADA (ya se ejercitó en el ejercicio 04 — Mirá ahí si la olvidás):
static Mat4 matrizVista(Vec3 pos, Vec3 target, Vec3 up) {
    Vec3 dir = target - pos;
    float l = std::sqrt(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
    dir = dir / l;
    Vec3 r = { dir.y*up.z - dir.z*up.y, dir.z*up.x - dir.x*up.z,
               dir.x*up.y - dir.y*up.x };                  // cross(dir, up)
    l = std::sqrt(r.x*r.x + r.y*r.y + r.z*r.z);
    r = r / l;
    Vec3 u = { r.y*dir.z - r.z*dir.y, r.z*dir.x - r.x*dir.z,
               r.x*dir.y - r.y*dir.x };                    // cross(right, dir)
    // la COLOCACIÓN de la cámara: columnas [right, upCam, -dir, pos]...
    // y su INVERSA RÍGIDA es la matriz de vista:
    Mat4 colocacion = Mat4::identidad();
    colocacion.ponerColumna(0, r);
    colocacion.ponerColumna(1, u);
    colocacion.ponerColumna(2, -dir);
    colocacion.ponerColumna3(3, pos, 1.0f);
    return inversaRigida(colocacion);
}

struct Plano { float a, b, c, d; };   // dentro: a*x + b*y + c*z + d >= 0

// DADA — la prueba del punto en UN plano (un solo dot, de la lección 11):
static bool puntoDentroDelPlano(Plano const& pl, Vec3 p) {
    return pl.a*p.x + pl.b*p.y + pl.c*p.z + pl.d >= 0.0f;
}
// DADA — DENTRO del frustum = dentro de los 6 planos:
bool puntoDentroDelFrustum(Plano const planos[6], Vec3 p) {
    for (int i = 0; i < 6; ++i)
        if (!puntoDentroDelPlano(planos[i], p)) return false;
    return true;
}

// ===== TU CÓDIGO (mirá la LECCIÓN 11) =====
Mat4 matrizPerspectiva(float fovYRad, float aspect, float cerca, float lejos) {
    (void)fovYRad; (void)aspect; (void)cerca; (void)lejos;
    return Mat4::identidad();    // TODO 1: f, la diagonal (0,0) (1,1) (2,2),
                                 //        m(2,3) y m(3,2).
}

Mat4 matrizOrtografica(float izq, float der, float abajo, float arriba,
                       float cerca, float lejos) {
    (void)izq; (void)der; (void)abajo; (void)arriba; (void)cerca; (void)lejos;
    return Mat4::identidad();    // TODO 2: tres escalas en la diagonal y
                                 //        tres columnas 3
}

void extraerPlanos(Mat4 const& vp, Plano planos[6]) {
    (void)vp; (void)planos;
    // TODO 3: las 6 COMBINACIONES de la fila 3 con las filas 0..2
    //        (+, -, +, -, +, -) del orden: izq, der, abajo, arriba, cerca, lejos
}

// ============================= TESTS (no tocar) =============================
TEST_CASE(Perspectiva90Aspecto1) {
    // fovY 90° => f = 1/tan(45°) = 1:
    Mat4 p = matrizPerspectiva(aRadianes(90.0f), 1.0f, 1.0f, 100.0f);
    ASSERT_TRUE(casiIgual(p.at(0, 0), 1.0f, 1e-5f));         // f/aspect
    ASSERT_TRUE(casiIgual(p.at(1, 1), 1.0f, 1e-5f));         // f
    ASSERT_TRUE(casiIgual(p.at(2, 2), -(100.0f+1.0f)/(100.0f-1.0f), 1e-4f));
    ASSERT_TRUE(casiIgual(p.at(2, 3), -1.0f, 1e-5f));
    ASSERT_TRUE(casiIgual(p.at(3, 2), -(2.0f*100.0f*1.0f)/(100.0f-1.0f), 1e-4f));
    ASSERT_TRUE(casiIgual(p.at(1, 0), 0.0f, 1e-5f));         // resta 0
}

TEST_CASE(OrtograficaSimetrica) {
    Mat4 o = matrizOrtografica(-4.0f, 4.0f, -3.0f, 3.0f, -1.0f, 1.0f);
    ASSERT_TRUE(casiIgual(o.at(0, 0), 2.0f/8.0f, 1e-5f));    // 2/(der-izq)
    ASSERT_TRUE(casiIgual(o.at(1, 1), 2.0f/6.0f, 1e-5f));    // 2/(arriba-abajo)
    ASSERT_TRUE(casiIgual(o.at(2, 2), -2.0f/2.0f, 1e-5f));   // -2/(lejos-cerca)
    ASSERT_TRUE(casiIgual(o.at(3, 3), 1.0f, 1e-5f));
    ASSERT_TRUE(casiIgual(o.at(0, 3), 0.0f, 1e-5f));         // centrada: 0
    ASSERT_TRUE(casiIgual(o.at(1, 3), 0.0f, 1e-5f));
    ASSERT_TRUE(casiIgual(o.at(2, 3), 0.0f, 1e-5f));
}

TEST_CASE(ProyeccionDelOrigen) {
    // el punto (0,0,0) visto desde (0,0,10): en vista z = -10, y el clip...
    Mat4 p = matrizPerspectiva(aRadianes(90.0f), 1.0f, 1.0f, 100.0f);
    Mat4 v = matrizVista(Vec3{0.0f, 0.0f, 10.0f}, V3CERO, V3Y);
    Vec4 clip = p * (v * Vec4{0.0f, 0.0f, 0.0f, 1.0f});
    ASSERT_TRUE(casiIgual(clip.w, 10.0f, 1e-3f));            // w = -z de vista
    float ndcZ = clip.z / clip.w;                            // [-1..1] GL
    ASSERT_TRUE(ndcZ > 0.80f && ndcZ < 0.83f);               // ≈ 0.8181...
}

TEST_CASE(CullingDeLaEsquina) {
    Mat4 p = matrizPerspectiva(aRadianes(90.0f), 1.0f, 0.5f, 100.0f);
    Mat4 v = matrizVista(Vec3{0.0f, 0.0f, 10.0f}, V3CERO, V3Y);
    Mat4 vp = p * v;
    Plano planos[6];
    extraerPlanos(vp, planos);
    // DENTRO: el origen está delante, a media cámara:
    ASSERT_TRUE(puntoDentroDelFrustum(planos, Vec3{0.0f, 0.0f, 0.0f}));
    // un poco a los costados y arriba (a 10 de distancia, la altura media
    // de un fov de 90° es tan(45°)*10 = 10: cabe de sobra):
    ASSERT_TRUE(puntoDentroDelFrustum(planos, Vec3{3.0f, 3.0f, 0.0f}));
    // DETRÁS de la cámara (z mayor que el de la cámara):
    ASSERT_FALSE(puntoDentroDelFrustum(planos, Vec3{0.0f, 0.0f, 20.0f}));
    // MÁS LEJOS que el plano lejos:
    ASSERT_FALSE(puntoDentroDelFrustum(planos, Vec3{0.0f, 0.0f, -95.0f}));
    // MUY abajo (fuera por el plano abajo):
    ASSERT_FALSE(puntoDentroDelFrustum(planos, Vec3{0.0f, -50.0f, 0.0f}));
}

int main() {
    std::cout << "EJERCICIO 09: frustum y proyección (lección 11)\n";
    int r = RUN_ALL_TESTS();
    if (r != 0) std::cout << "Sin resolver: los tests están fallando (OK).\n";
    return r;
}
