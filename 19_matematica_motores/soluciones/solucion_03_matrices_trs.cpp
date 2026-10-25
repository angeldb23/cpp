/* =============================================================================
   SOLUCIÓN 03: matrices TRS — traslación, escala, rotación Z, SRT (lección 05)
   =============================================================================
   Column-major (como GLM): CADA FUNCIÓN ARMA COLUMNAS, no filas:
   - traslación: única columna que cambia es la 3 (con w=1).
   - escala: UNA diagonal (columna c llevado a fila c... m.set(c, c, s)).
   - rotación Z: col0 = (cos, sin, 0) y col1 = (-sin, cos, 0): las columnas
     SON los ejes del sistema local girado — X local apunta a donde el giro
     lo mandan (a 90°, la X local se paró sobre la Y del mundo).
   - SRT: T * R * S — el orden de la MULTIPLICACIÓN es el que se aplica EN
     EL INSULTO: objeto de la escena => primero escalar, después girar,
     después traer; casi SIEMPRE es ese orden en un juego.
   ============================================================================= */
#include <cmath>
#include "../../include/test_framework.hpp"
#include "../include/mate3d.hpp"

using namespace mate;

// ===== SOLUCIÓN =====
Mat4 matrizTraslacion(Vec3 t) {
    Mat4 m = Mat4::identidad();
    m.ponerColumna3(3, t, 1.0f);      // SOLO la columna 3
    return m;
}

Mat4 matrizEscala(Vec3 s) {
    Mat4 m = Mat4::identidad();
    m.set(0, 0, s.x);                 // SOLO la diagonal: (c, c)
    m.set(1, 1, s.y);
    m.set(2, 2, s.z);
    return m;
}

Mat4 matrizRotacionZ(float rad) {
    float c = std::cos(rad), s = std::sin(rad);
    Mat4 m = Mat4::identidad();
    m.ponerColumna(0, { c, s, 0.0f });     // X local girada
    m.ponerColumna(1, { -s, c, 0.0f });    // Y local girada
    return m;
}

Mat4 matrizSRT(Vec3 escala, float rotZ, Vec3 traslacion) {
    // aplicando DE DERECHA A IZQ: punto => S, desp. R, desp. T:
    return matrizTraslacion(traslacion)
         * matrizRotacionZ(rotZ)
         * matrizEscala(escala);
}

// ============================= TESTS (del ejercicio — sin cambios) ==========
static Vec3 columna3(Mat4 const& m, int c) {
    return { m.at(c, 0), m.at(c, 1), m.at(c, 2) };
}

TEST_CASE(TraslacionSoloColumna3) {
    Mat4 t = matrizTraslacion(Vec3{10.0f, 20.0f, 30.0f});
    ASSERT_NEAR(t.at(3, 0), 10.0f, 1e-5f);
    ASSERT_NEAR(t.at(3, 1), 20.0f, 1e-5f);
    ASSERT_NEAR(t.at(3, 2), 30.0f, 1e-5f);
    ASSERT_NEAR(t.at(3, 3), 1.0f, 1e-5f);
    ASSERT_NEAR(t.at(0, 0), 1.0f, 1e-5f);
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
    Vec3 ejeXLocal = columna3(r, 0);
    ASSERT_TRUE(casiIgualVec3(ejeXLocal, Vec3{0.0f, 1.0f, 0.0f}, 1e-4f));
    Vec3 ejeYLocal = columna3(r, 1);
    ASSERT_TRUE(casiIgualVec3(ejeYLocal, Vec3{-1.0f, 0.0f, 0.0f}, 1e-4f));
    Vec3 p = transformarPunto(r, Vec3{1.0f, 0.0f, 0.0f});
    ASSERT_TRUE(casiIgualVec3(p, Vec3{0.0f, 1.0f, 0.0f}, 1e-4f));
}

TEST_CASE(SRTOrdenEscalRotacTraslada) {
    Mat4 w = matrizSRT(Vec3{2.0f, 2.0f, 2.0f}, aRadianes(90.0f),
                       Vec3{10.0f, 0.0f, 0.0f});
    Vec3 p = transformarPunto(w, Vec3{1.0f, 0.0f, 0.0f});
    ASSERT_TRUE(casiIgualVec3(p, Vec3{10.0f, 2.0f, 0.0f}, 1e-3f));
    Vec3 d = transformarDireccion(w, Vec3{1.0f, 0.0f, 0.0f});
    ASSERT_TRUE(casiIgualVec3(d, Vec3{0.0f, 2.0f, 0.0f}, 1e-3f));
}

int main() {
    std::cout << "SOLUCIÓN 03: matrices TRS (todos los tests DEBEN pasar)\n";
    int r = RUN_ALL_TESTS();
    if (r != 0) std::cout << "ERROR: la solución debe pasar todos los tests.\n";
    return r;
}
