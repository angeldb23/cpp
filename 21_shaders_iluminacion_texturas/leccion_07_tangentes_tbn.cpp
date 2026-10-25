/* =============================================================================
   MÓDULO 21 - LECCIÓN 07: Espacio tangente: TBN (tangente, bitangente, normal)
   =============================================================================
   TEMA 4 de TEMAS.md (primera de 2 lecciones): "Normal mapping: espacio
   tangente, TBN matrix en vertex shader y unpack de normal map". El mapping
   completo (shader) es la LECCIÓN 08.

   CÓMO CORRERLA A MANO:
     g++ -std=c++20 -Wall -Wextra leccion_07_tangentes_tbn.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec07 && ./lec07
   Impresora: este archivo define TBN y la IMPRIME para 3 triángulos de
   prueba: ES un ejercicio mental ejecutable (los tests del ej 06 reusan la
   misma matemática en C++ puro).

   --- EL PROBLEMA: una esfera lisa quiere ser una naranja -------------------
   Tu esfera tiene ~300 triángulos y una NORMAL por vértice
   suave: la luz la lee "esfera lisa". Una naranja real tiene 》MILES de
   CPU del dueño) O una textura que "miente" la NORMAL: un NORMAL MAP: cada
   texel guarda una normal DESVIADA que dice "acá la superficie se dobla
   un poco". La luz se calcula con esa normal mentirosa: la silueta sigue
   lisa pero el shading parece rugoso: eso ES el normal mapping.

   --- EL PROBLEMA DE COORDENADAS: la normal map vive "en el texel" ----------
   La textura guarda la normal RELATIVA a la superficie local (x => este,
   y => norte, z => arriba). Ese espacio propio de la textura es el ESPACIO
   TANGENTE: 3 ejes nacidos del MESH (no del mundo):
     T (tangent)    : hacia donde CRECE la U de la textura (entre vértice)
     B (bitangent)  : hacia donde CRECE la V
     N (normal)     : la normal de la geometría (la que ya tenías)
   El conjunto (T, B, N) da la MATRIZ TBN: el traductor entre los UV del
   triángulo y el espacio 3D. Con ella:
     - subir normal [0,1] -> [-1,1] (unpack: la L08)
     - llevarla de espacio tangente -> ESPACIO MUNDO (o vice versa)
   Ambos: mat3 multiplicación. La ELECCIÓN "a mundo" (TBN en el shader por
   fragmento) es la que hace LearnOpenGL y esta lección sigue.

   --- CÓMO SE CALCULA T (y su amigo B) --------------------------------------
   Un triángulo (p1, p2, p3) con UVs (uv1, uv2, uv3). El cambio de posición
   con el cambio de UV es un sistema lineal 2x2:
                      dUV: (uv2-uv1, uv3-uv1)
       [e1 e2] * INV(dUV) = [TB]
     e1 = p2-p1, e2 = p3-p1 (vector EDGE en 3D)
     dUV1 = uv2-uv1, dUV2 = uv3-uv1
   Resolviendo el sistema (la "fórmula" de la tangente):
       f = 1 / (dUV1.x * dUV2.y - dUV2.x * dUV1.y)     // determinante
       T = f * (dUV2.y * e1 - dUV1.y * e2)
       B = f * (-dUV2.x * e1 + dUV1.x * e2)
   Dos ajustes OBLIGATORIOS (el detalle que separa el motor de los mods):
     1. Gram-Schmidt: T' = normalize(T - N * dot(N, T)): la T cruda Puede
        quedar inclinada contra N (UVs artísticas); GS la endereza al plano.
     2. HAND (handedness): si dot(cross(N, T'), B) < 0 las UV van al
        espejo: se invierte el signo y SE GUARDA en la componente w del
        atributo tangente (así lo hacen los motores reales al exportar).
   Normalizar TODO: son direcciones, no posiciones.

   --- QUÉ HACE ESTA DEMO ----------------------------------------------------
Un programa SIN ventana: calcula e IMPRIME la TBN de 3 triángulos de
   prueba (antes de que la L08 la meta al shader). Notá que la TBN es LOCAL
   del mesh: girar el modelo NO la cambia (se rota después con el uModel).
   ============================================================================
 */

#include "include/l21.hpp"

#include <iostream>

namespace {

/* Calcula la tangente de UN triángulo (las 3 posiciones + sus UVs) y aplica
   Gram-Schmidt + handedness: EXACTO el código del ejercicio 06 en C++ puro. */
struct Tri { f21::Vec3 p[3]; f21::Vec2 uv[3]; };

static void calcularTBN(Tri const& t, f21::Vec3 N, f21::Vec3& T,
                        f21::Vec3& B) {
    f21::Vec3 e1 = t.p[1] - t.p[0];
    f21::Vec3 e2 = t.p[2] - t.p[0];
    float du1 = t.uv[1].x - t.uv[0].x;
    float dv1 = t.uv[1].y - t.uv[0].y;
    float du2 = t.uv[2].x - t.uv[0].x;
    float dv2 = t.uv[2].y - t.uv[0].y;
    float det = du1 * dv2 - du2 * dv1;
    if (std::fabs(det) < 1e-8f) {       // UV degeneradas: la cara no "dibuja"
        T = f21::normalizar(f21::cross(N, f21::Vec3{0, 1, 0}));
        B = f21::normalizar(f21::cross(N, T));
        return;
    }
    float f = 1.0f / det;
    T = f21::normalizar(f * (dv2 * e1 - dv1 * e2));
    B = f21::normalizar(f * (-du2 * e1 + du1 * e2));
    // Gram-Schmidt: pegar T al plano de N (tapa que la UV sea rasante)
    T = f21::normalizar(T - N * f21::dot(N, T));
    // handedness: si B computado está "al espejo" del sistema T,N
    if (f21::dot(f21::cross(N, T), B) < 0.0f) B = B * -1.0f;
}

/* Verifica: la TBN proyecta el EJE U de la textura ("paso x") en la
   dirección T. Para probar, T· reunion = 1 si T es el eje u del plano. */
static void imprimirTBN(const char* nombre, f21::Vec3 N, f21::Vec3 T,
                        f21::Vec3 B) {
    std::cout << nombre
              << "  N=(" << N.x << "," << N.y << "," << N.z
              << ")  T=(" << T.x << "," << T.y << "," << T.z
              << ")  B=(" << B.x << "," << B.y << "," << B.z << ")\n";
}

}  // namespace

int main() {
    std::cout << "TBN de 3 triángulos de prueba (UV en 2 ejes vivos):\n";

    // TRIÁNGULO 1: piso XY plano con UV lineal (x->u, y->v)
    {   Tri t = {
            {{0,0,0}, {1,0,0}, {0,1,0}},      // posiciones en el plano XY
            {{0,0},   {1,0},   {0,1}},        // UV igual: identidad
        };
        f21::Vec3 T{}, B{};
        calcularTBN(t, {0,0,1}, T, B);
        imprimirTBN("piso XY (N=+Z): ", {0,0,1}, T, B);
        // Esperado: T ~ +X (u crece con x), B ~ +Y (v crece con y)
    }

    // TRIÁNGULO 2: la MISMA geometría con UV ESPEJADA (u = -x): hand = -1
    {
        Tri t = {
            {{0,0,0}, {1,0,0}, {0,1,0}},
            {{1,0},   {0,0},   {1,1}},        // u invertida
        };
        f21::Vec3 T{}, B{};
        calcularTBN(t, {0,0,1}, T, B);
        imprimirTBN("piso XY ESPEJO (flip u): ", {0,0,1}, T, B);
        // Esperado: T apunta a -X: la handedness FUE detectada
    }

    // TRIÁNGULO 3: pared XZ con u ~ x, v ~ z (N=+Y): T debe quedar ~ +X
    {
        Tri t = {
            {{0,0,0}, {1,0,0}, {0,0,1}},
            {{0,0},   {1,0},   {0,1}},
        };
        f21::Vec3 T{}, B{};
        calcularTBN(t, {0,1,0}, T, B);
        imprimirTBN("pared XZ (N=+Y): ", {0,1,0}, T, B);
    }

    // La ROTACIÓN del modelo NO cambia la TBN: es LOCAL del mesh
    f21::Mat4 giro = f21::trs({0,0,0}, 1.0f, 90.0f);
    f21::Vec3 T{}, B{};
    (void)giro; (void)T; (void)B;
    return 0;
}
