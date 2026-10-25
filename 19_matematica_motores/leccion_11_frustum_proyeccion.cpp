/* =============================================================================
   MÓDULO 19 - LECCIÓN 11: Frustum y proyección — orto vs perspectiva
   =============================================================================
   TEMA 6 de TEMAS.md (1 lección): ortográfica vs perspectiva, near/far, FOV,
   aspect y matriz de proyección + extracción de planos (culling).

   == PROYECCIÓN ORTOGRÁFICA =================================================

   Los rayos del "ojo" son PARALELOS. No hay cómo se achique con distancia:
   útil para juegos 2D, editar la UI, mapas, y para render de HUD con texturas.

       P_ortho l r b t n f  (right-handed, depth NDC [-1..1], como GL):
         m(0,0) = 2/(r-l)        m(1,1) = 2/(t-b)        m(2,2) = -2/(f-n)
         m(0,3) = -(r+l)/(r-l)
         m(1,3) = -(t+b)/(t-b)
         m(2,3) = -(f+n)/(f-n)
         m(3,3) = 1

   == PROYECCIÓN PERSPECTIVA =================================================

   Simula el ojo: los puntos CERCA son GRANDES, los LEJOS chicos. FOV es el
   ángulo VERTICAL en radianes; el ancho sale del aspect (= ancho/alto):

       P_persp NO (right-handed, NDC z en [-1..1]):
         m(0,0) = 1/(aspect * tan(fovY/2))     m(1,1) = 1/tan(fovY/2)
         m(2,2) = -(f+n)/(f-n)       m(3,2) = -2fn/(f-n)       m(2,3) = -1

   Verificaciones contra GLM (repo 1.0.1, columna-major):
     glm::orthoRH_NO      — glm/ext/matrix_clip_space.hpp (1.0.1:83)
     glm::perspectiveRH_NO — glm/ext/matrix_clip_space.hpp (1.0.1:238)
     (GLM también tiene _LH_ izquierda o _ZO_ cuando quiere depth [0..1],
      como Vulkan/D3D — Módulo 26 lo trayemos al tema.)

   == CULLING: FRUSTUM PLANOS DESDE LA MATRIZ ================================

   El frustum es la pirámide truncada que DEFINA el volumen visible. Son 6
   PLANOS y un punto está visible si queda del lado DENTRO de los 6.

   Extracción GRIBB-HARTMANN (para matrices GL column-major y NDC [-1..1]),
   con M = P * V (el "view-projection" completo):
      row_i = (m[4*0+i], m[4*1+i], m[4*2+i], m[4*3+i])
     izquierda  = row3 + row0      derecha = row3 - row0
     abajo      = row3 + row1      arriba  = row3 - row1
     near       = row3 + row2      far     = row3 - row2

   Cada plano es un vec4 (a, b, c, d) con ecuación punto·xyz + d = 0, y la
   prueba del ltree es un solo dot:

       dentro(a, b, c, d, punto)  <=>  a*x + b*y + c*z + d >= 0

   Para el FRUSTUM un punto está visible si CUMPLE los 6. La normal (a,b,c)
   NO necesita normalizarse para la (mirada) del signo; se normaliza cuando
   querés MEDIR la distancia al plano.

   == CONSEJOS ORTO vs PERSP =================================================

   - z NEAR chicos (0.01) y FAR grandísimo (100000) rompe la PRECISIÓN del
     depth buffer (z-fighting en superficies lejanas). Ajustá n/f por escena.
   - FOV: 45° (largo), 60°-70° (típico FPS: 90 horizontal a 16:9),zení 90+ (ojo).
   ============================================================================= */

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>   // lookAtRH
#include <glm/ext/matrix_clip_space.hpp>  // orthoRH_NO / perspectiveRH_NO
#include "include/mate3d.hpp"
#include <cstdio>
#include <iostream>

using namespace mate;

// ==== PROYECCIONES (column-major; los ejercicios las IMPLEMENTAN) ==========
Mat4 matrizOrtografica(float izq, float der, float abajo, float arriba,
                       float cerca, float lejos) {
    Mat4 m{};
    m.set(0, 0, 2.0f / (der - izq));
    m.set(1, 1, 2.0f / (arriba - abajo));
    m.set(2, 2, -2.0f / (lejos - cerca));
    m.set(0, 3, -(der + izq) / (der - izq));
    m.set(1, 3, -(arriba + abajo) / (arriba - abajo));
    m.set(2, 3, -(lejos + cerca) / (lejos - cerca));
    m.set(3, 3, 1.0f);
    return m;
}

Mat4 matrizPerspectiva(float fovYRad, float aspect, float cerca, float lejos) {
    Mat4 m{};
    float f = 1.0f / std::tan(fovYRad * 0.5f);
    m.set(0, 0, f / aspect);
    m.set(1, 1, f);
    m.set(2, 2, -(lejos + cerca) / (lejos - cerca));
    m.set(2, 3, -1.0f);
    m.set(3, 2, -(2.0f * lejos * cerca) / (lejos - cerca));
    return m;
}

glm::mat4 aGlm(Mat4 const& m) {
    glm::mat4 r = glm::mat4(1.0f);
    for (int c = 0; c < 4; ++c)
        for (int row = 0; row < 4; ++row)
            r[c][row] = m.at(c, row);
    return r;
}

static bool iguales(Mat4 const& a, glm::mat4 const& b, float eps = 1e-5f) {
    for (int c = 0; c < 4; ++c)
        for (int row = 0; row < 4; ++row)
            if (casiIgual(a.at(c, row), b[c][row], eps)) continue;
            else return false;
    return true;
}

// ==== FRUSTUM PLANOS (Gribb-Hartmann) =====================================
struct Plano { float a, b, c, d; };   // dentro <=> a*x + b*y + c*z + d >= 0

// 6 planos desde el VP (en mundo).
void extraerPlanos(Mat4 const& vp, Plano fuera[6]) {
    auto fila = [&](int row) {
        return Vec4{vp.at(0, row), vp.at(1, row), vp.at(2, row), vp.at(3, row)};
    };
    Vec4 r0 = fila(0), r1 = fila(1), r2 = fila(2), r3 = fila(3);
    auto mas = [](Vec4 p, Vec4 q) { return Vec4{p.x+q.x, p.y+q.y, p.z+q.z, p.w+q.w}; };
    auto men = [](Vec4 p, Vec4 q) { return Vec4{p.x-q.x, p.y-q.y, p.z-q.z, p.w-q.w}; };
    Vec4 planos[6] = { mas(r3, r0), men(r3, r0), mas(r3, r1),
                       men(r3, r1), mas(r3, r2), men(r3, r2) };
    for (int i = 0; i < 6; ++i) {
        fuera[i].a = planos[i].x; fuera[i].b = planos[i].y;
        fuera[i].c = planos[i].z; fuera[i].d = planos[i].w;
    }
}

bool puntoDentroDelFrustum(Plano const planos[6], Vec3 p) {
    for (int i = 0; i < 6; ++i) {
        Plano const& pl = planos[i];
        if (pl.a*p.x + pl.b*p.y + pl.c*p.z + pl.d < 0.0f) return false;
    }
    return true;
}

// ==== DEMO ================================================================
int main() {
    std::cout << "== PERSPECTIVA ==\n";
    Mat4 miP = matrizPerspectiva(aRadianes(60.0f), 16.0f/9.0f, 0.5f, 100.0f);
    glm::mat4 gP = glm::perspectiveRH_NO(aRadianes(60.0f), 16.0f/9.0f, 0.5f, 100.0f);
    std::cout << "mi matrizPerspectiva == glm::perspectiveRH_NO? "
              << (iguales(miP, gP) ? "SI" : "NO") << "\n";

    std::cout << "== ORTOGRAFICA ==\n";
    Mat4 miO = matrizOrtografica(-16.0f, 16.0f, -9.0f, 9.0f, -1.0f, 1.0f);
    glm::mat4 gO = glm::orthoRH_NO(-16.0f, 16.0f, -9.0f, 9.0f, -1.0f, 1.0f);
    std::cout << "mi matrizOrtografica == glm::orthoRH_NO? "
              << (iguales(miO, gO) ? "SI" : "NO") << "\n";

    std::cout << "\n== CULLING CON LOS 6 PLANOS (P*V) ==\n";
    glm::mat4 gV = glm::lookAtRH(glm::vec3(0.0f, 0.0f, 10.0f),
                                 glm::vec3(0.0f, 0.0f, 0.0f),
                                 glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 gVP = gP * gV;
    Mat4 vp{};
    for (int c = 0; c < 4; ++c)
        for (int row = 0; row < 4; ++row)
            vp.set(c, row, gVP[c][row]);
    Plano planos[6];
    extraerPlanos(vp, planos);
    Vec3 dentro[] { Vec3{0.0f, 0.0f, 0.0f},   // delante de la cámara
                    Vec3{1.0f, 1.0f, 0.0f} }; // un poco off-center
    Vec3 fuera[]  { Vec3{0.0f, 0.0f, 20.0f},   // DETRÁS (z+f) de la cámara
                    Vec3{0.0f, -50.0f, 0.0f},  // muy abajo: fuera del frustum
                    Vec3{0.0f, 0.0f, 200.0f} };// más lejos que far=100
    for (auto const& p : dentro)
        std::printf("punto (%5.1f,%5.1f,%5.1f)  dentro=%d\n",
                    p.x, p.y, p.z, (int)puntoDentroDelFrustum(planos, p));
    for (auto const& p : fuera)
        std::printf("punto (%5.1f,%5.1f,%5.1f)  dentro=%d (por fuera)\n",
                    p.x, p.y, p.z, (int)puntoDentroDelFrustum(planos, p));

    std::cout << "\n== LOS PARÁMETROS (y por qué tocarlos importa example) ==\n";
    std::cout << "- near muy chico + far very grande => z-fighting (precisión).\n"
                 "- fov: 45° cine, 60-70 típico FPS horizontal ~90 a 16:9.\n"
                 "- ortho para HUD/mapas/menús.\n";

    std::cout << "=== FIN LECCIÓN 11 ===\n";
    return 0;
}
