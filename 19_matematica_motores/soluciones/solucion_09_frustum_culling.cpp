/* =============================================================================
   SOLUCIÓN 09: frustum y proyección — perspectiva, orto, culling (lección 11)
   =============================================================================
   - matrizPerspectiva / matrizOrtografica: las fórmulas al hueso, con la
     convención GL (right-handed, NDC z en [-1..1], column-major).
     Detalle: la "columna 3" de la perspectiva es (0, 0, -1, 0): el W final
     es -z de vista (los puntos delante de la cámara tienen w > 0).
   - extraerPlanos: Gribb-Hartmann: cada plano es la FILA 3 combinada con
     una fila: fila3 + fila_i para los planos MÍNIMOS (izq/abajo/cerca) y
     fila3 - fila_i para los MÁXIMOS. Es extraer el frustum SIN resolver
     6 planos con puntos/normales: 6 combinaciones lineales y listo.
   ============================================================================= */
#include <cmath>
#include "../../include/test_framework.hpp"
#include "../include/mate3d.hpp"

using namespace mate;

static Mat4 matrizVista(Vec3 pos, Vec3 target, Vec3 up) {
    Vec3 dir = target - pos;
    float l = std::sqrt(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);
    dir = dir / l;
    Vec3 r = { dir.y*up.z - dir.z*up.y, dir.z*up.x - dir.x*up.z,
               dir.x*up.y - dir.y*up.x };
    l = std::sqrt(r.x*r.x + r.y*r.y + r.z*r.z);
    r = r / l;
    Vec3 u = { r.y*dir.z - r.z*dir.y, r.z*dir.x - r.x*dir.z,
               r.x*dir.y - r.y*dir.x };
    Mat4 colocacion = Mat4::identidad();
    colocacion.ponerColumna(0, r);
    colocacion.ponerColumna(1, u);
    colocacion.ponerColumna(2, -dir);
    colocacion.ponerColumna3(3, pos, 1.0f);
    return inversaRigida(colocacion);
}

struct Plano { float a, b, c, d; };

static bool puntoDentroDelPlano(Plano const& pl, Vec3 p) {
    return pl.a*p.x + pl.b*p.y + pl.c*p.z + pl.d >= 0.0f;
}
bool puntoDentroDelFrustum(Plano const planos[6], Vec3 p) {
    for (int i = 0; i < 6; ++i)
        if (!puntoDentroDelPlano(planos[i], p)) return false;
    return true;
}

// ===== SOLUCIÓN =====
Mat4 matrizPerspectiva(float fovYRad, float aspect, float cerca, float lejos) {
    Mat4 m{};                                        // CERO por defecto
    float f = 1.0f / std::tan(fovYRad * 0.5f);
    m.set(0, 0, f / aspect);
    m.set(1, 1, f);
    m.set(2, 2, -(lejos + cerca) / (lejos - cerca));
    m.set(2, 3, -1.0f);                              // w = -z de vista
    m.set(3, 2, -(2.0f * lejos * cerca) / (lejos - cerca));
    return m;
}

Mat4 matrizOrtografica(float izq, float der, float abajo, float arriba,
                       float cerca, float lejos) {
    Mat4 m{};                                        // CERO por defecto
    m.set(0, 0, 2.0f / (der - izq));
    m.set(1, 1, 2.0f / (arriba - abajo));
    m.set(2, 2, -2.0f / (lejos - cerca));
    m.set(0, 3, -(der + izq) / (der - izq));
    m.set(1, 3, -(arriba + abajo) / (arriba - abajo));
    m.set(2, 3, -(lejos + cerca) / (lejos - cerca));
    m.set(3, 3, 1.0f);
    return m;
}

void extraerPlanos(Mat4 const& vp, Plano planos[6]) {
    // fila_i de una Mat4 column-major: (at(0,i), at(1,i), at(2,i), at(3,i)).
    auto fila = [&](int i) {
        return Vec4{vp.at(0, i), vp.at(1, i), vp.at(2, i), vp.at(3, i)};
    };
    Vec4 f0 = fila(0), f1 = fila(1), f2 = fila(2), f3 = fila(3);
    auto mas = [](Vec4 p, Vec4 q) { return Vec4{p.x+q.x, p.y+q.y, p.z+q.z, p.w+q.w}; };
    auto men = [](Vec4 p, Vec4 q) { return Vec4{p.x-q.x, p.y-q.y, p.z-q.z, p.w-q.w}; };
    Vec4 combinaciones[6] = {
        mas(f3, f0), men(f3, f0),     // izquierda, derecha
        mas(f3, f1), men(f3, f1),     // abajo, arriba
        mas(f3, f2), men(f3, f2),     // cerca, lejos
    };
    for (int i = 0; i < 6; ++i) {
        planos[i].a = combinaciones[i].x;
        planos[i].b = combinaciones[i].y;
        planos[i].c = combinaciones[i].z;
        planos[i].d = combinaciones[i].w;
    }
}

// ============================= TESTS (del ejercicio — sin cambios) ==========
TEST_CASE(Perspectiva90Aspecto1) {
    Mat4 p = matrizPerspectiva(aRadianes(90.0f), 1.0f, 1.0f, 100.0f);
    ASSERT_TRUE(casiIgual(p.at(0, 0), 1.0f, 1e-5f));
    ASSERT_TRUE(casiIgual(p.at(1, 1), 1.0f, 1e-5f));
    ASSERT_TRUE(casiIgual(p.at(2, 2), -(100.0f+1.0f)/(100.0f-1.0f), 1e-4f));
    ASSERT_TRUE(casiIgual(p.at(2, 3), -1.0f, 1e-5f));
    ASSERT_TRUE(casiIgual(p.at(3, 2), -(2.0f*100.0f*1.0f)/(100.0f-1.0f), 1e-4f));
    ASSERT_TRUE(casiIgual(p.at(1, 0), 0.0f, 1e-5f));
}

TEST_CASE(OrtograficaSimetrica) {
    Mat4 o = matrizOrtografica(-4.0f, 4.0f, -3.0f, 3.0f, -1.0f, 1.0f);
    ASSERT_TRUE(casiIgual(o.at(0, 0), 2.0f/8.0f, 1e-5f));
    ASSERT_TRUE(casiIgual(o.at(1, 1), 2.0f/6.0f, 1e-5f));
    ASSERT_TRUE(casiIgual(o.at(2, 2), -2.0f/2.0f, 1e-5f));
    ASSERT_TRUE(casiIgual(o.at(3, 3), 1.0f, 1e-5f));
    ASSERT_TRUE(casiIgual(o.at(0, 3), 0.0f, 1e-5f));
    ASSERT_TRUE(casiIgual(o.at(1, 3), 0.0f, 1e-5f));
    ASSERT_TRUE(casiIgual(o.at(2, 3), 0.0f, 1e-5f));
}

TEST_CASE(ProyeccionDelOrigen) {
    Mat4 p = matrizPerspectiva(aRadianes(90.0f), 1.0f, 1.0f, 100.0f);
    Mat4 v = matrizVista(Vec3{0.0f, 0.0f, 10.0f}, V3CERO, V3Y);
    Vec4 clip = p * (v * Vec4{0.0f, 0.0f, 0.0f, 1.0f});
    ASSERT_TRUE(casiIgual(clip.w, 10.0f, 1e-3f));
    float ndcZ = clip.z / clip.w;
    ASSERT_TRUE(ndcZ > 0.80f && ndcZ < 0.83f);
}

TEST_CASE(CullingDeLaEsquina) {
    Mat4 p = matrizPerspectiva(aRadianes(90.0f), 1.0f, 0.5f, 100.0f);
    Mat4 v = matrizVista(Vec3{0.0f, 0.0f, 10.0f}, V3CERO, V3Y);
    Mat4 vp = p * v;
    Plano planos[6];
    extraerPlanos(vp, planos);
    ASSERT_TRUE(puntoDentroDelFrustum(planos, Vec3{0.0f, 0.0f, 0.0f}));
    ASSERT_TRUE(puntoDentroDelFrustum(planos, Vec3{3.0f, 3.0f, 0.0f}));
    ASSERT_FALSE(puntoDentroDelFrustum(planos, Vec3{0.0f, 0.0f, 20.0f}));
    ASSERT_FALSE(puntoDentroDelFrustum(planos, Vec3{0.0f, 0.0f, -95.0f}));
    ASSERT_FALSE(puntoDentroDelFrustum(planos, Vec3{0.0f, -50.0f, 0.0f}));
}

int main() {
    std::cout << "SOLUCIÓN 09: frustum y proyección (todos DEBEN pasar)\n";
    int r = RUN_ALL_TESTS();
    if (r != 0) std::cout << "ERROR: la solución debe pasar todos los tests.\n";
    return r;
}
