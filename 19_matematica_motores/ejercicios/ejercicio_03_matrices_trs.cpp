/* =============================================================================
   MÓDULO 19 - EJERCICIO 03: Matrices — traslación, escala, rotación y TRS
   =============================================================================
   DIFICULTAD: Intermedio
   APOYO: Lección 05 (constructores column-major, matrizSRT y orden de
   multiplicación; transformarPunto/transformarDireccion ya están en mate3d).

   CONSIGNA (column-major: los constructores arman COLUMNAS, no filas):

     Mat4 matrizTraslacion(Vec3 t)
        -> identidad con la columna 3 = (t.x, t.y, t.z, 1)
     Mat4 matrizEscala(Vec3 s)
        -> identidad con diagonal = (s.x, s.y, s.z)
     Mat4 matrizRotacionZ(float rad)
        -> col0 = ( cos,  sin, 0)  y col1 = (-sin, cos, 0)
           (gira el eje X hacia el eje Y: antihorario en RH mirando desde +z)
     Mat4 matrizSRT(Vec3 escala, float rotZ, Vec3 traslacion)
        -> la composición:  T * R * S
           (primero escala, después rota, después traslada: casi SIEMPRE
            en ese orden en un juego: ¡el orden de multiplicación importa!)
           (para la rotación usá TU matrizRotacionZ)

   TESTS (no tocar): las tests usan transformarPunto/transformarDireccion
   de mate3d.hpp (ya implementadas en la lib de infraestructura).
   COMPILACIÓN: g++ -std=c++20 -Wall -Wextra ejercicio_03_... -o ej03 && ./ej03
   ============================================================================= */
#include <cmath>
#include "../../include/test_framework.hpp"
#include "../include/mate3d.hpp"

using namespace mate;

// ===== TU CÓDIGO (mirá la LECCIÓN 05) =====
Mat4 matrizTraslacion(Vec3 t) {
    (void)t;
    return Mat4::identidad();   // TODO 1: tocar SOLO la columna 3
}

Mat4 matrizEscala(Vec3 s) {
    (void)s;
    return Mat4::identidad();   // TODO 2: tocar SOLO la diagonal (col c, fila c)
}

Mat4 matrizRotacionZ(float rad) {
    (void)rad;
    return Mat4::identidad();   // TODO 3: col0 y col1 con cos/sin
}

Mat4 matrizSRT(Vec3 escala, float rotZ, Vec3 traslacion) {
    (void)escala; (void)rotZ; (void)traslacion;
    return Mat4::identidad();   // TODO 4: composición T * R * S con TU R
}

// ============================= TESTS (no tocar) =============================
static Vec3 columna3(Mat4 const& m, int c) {   // las 3 primeras filas de la col c
    return { m.at(c, 0), m.at(c, 1), m.at(c, 2) };
}

TEST_CASE(TraslacionSoloColumna3) {
    Mat4 t = matrizTraslacion(Vec3{10.0f, 20.0f, 30.0f});
    ASSERT_NEAR(t.at(3, 0), 10.0f, 1e-5f);    // columna 3, fila 0
    ASSERT_NEAR(t.at(3, 1), 20.0f, 1e-5f);
    ASSERT_NEAR(t.at(3, 2), 30.0f, 1e-5f);
    ASSERT_NEAR(t.at(3, 3), 1.0f, 1e-5f);
    ASSERT_NEAR(t.at(0, 0), 1.0f, 1e-5f);     // el resto sigue siendo identidad
    ASSERT_NEAR(t.at(1, 2), 0.0f, 1e-5f);
    ASSERT_TRUE(casiIgualVec3(transformarPunto(t, Vec3{1.0f, 1.0f, 1.0f}),
                              Vec3{11.0f, 21.0f, 31.0f}));
}

TEST_CASE(EscalaSoloDiagonal) {
    Mat4 m = matrizEscala(Vec3{2.0f, 3.0f, 4.0f});
    ASSERT_NEAR(m.at(0, 0), 2.0f, 1e-5f);
    ASSERT_NEAR(m.at(1, 1), 3.0f, 1e-5f);
    ASSERT_NEAR(m.at(2, 2), 4.0f, 1e-5f);
    ASSERT_NEAR(m.at(0, 1), 0.0f, 1e-5f);
    ASSERT_TRUE(casiIgualVec3(transformarPunto(m, Vec3{1.0f, 1.0f, 1.0f}),
                              Vec3{2.0f, 3.0f, 4.0f}));
}

TEST_CASE(RotacionZ90LlevaXAY) {
    Mat4 r = matrizRotacionZ(aRadianes(90.0f));
    // las COLUMNAS de R son los ejes X local e Y local girados:
    Vec3 ejeXLocal = columna3(r, 0);
    ASSERT_TRUE(casiIgualVec3(ejeXLocal, Vec3{0.0f, 1.0f, 0.0f}, 1e-4f));
    Vec3 ejeYLocal = columna3(r, 1);
    ASSERT_TRUE(casiIgualVec3(ejeYLocal, Vec3{-1.0f, 0.0f, 0.0f}, 1e-4f));
    // y el PUNTO del eje X va a Y:
    Vec3 p = transformarPunto(r, Vec3{1.0f, 0.0f, 0.0f});
    ASSERT_TRUE(casiIgualVec3(p, Vec3{0.0f, 1.0f, 0.0f}, 1e-4f));
}

TEST_CASE(SRTOrdenEscalRotacTraslada) {
    // p=(1,0,0): escala x2 -> (2,0,0); rota 90° -> (0,2,0); traslada -> (10,2,0)
    Mat4 w = matrizSRT(Vec3{2.0f, 2.0f, 2.0f}, aRadianes(90.0f),
                       Vec3{10.0f, 0.0f, 0.0f});
    Vec3 p = transformarPunto(w, Vec3{1.0f, 0.0f, 0.0f});
    ASSERT_TRUE(casiIgualVec3(p, Vec3{10.0f, 2.0f, 0.0f}, 1e-3f));
    // las DIRECCIONES no se trasladan:
    Vec3 d = transformarDireccion(w, Vec3{1.0f, 0.0f, 0.0f});
    ASSERT_TRUE(casiIgualVec3(d, Vec3{0.0f, 2.0f, 0.0f}, 1e-3f));
}

int main() {
    std::cout << "EJERCICIO 03: matrices TRS (lección 05)\n";
    int r = RUN_ALL_TESTS();
    if (r != 0) std::cout << "Sin resolver: los tests están fallando (OK).\n";
    return r;
}
