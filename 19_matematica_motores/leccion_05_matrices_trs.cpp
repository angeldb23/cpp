/* =============================================================================
   MÓDULO 19 - LECCIÓN 05: Matrices — identidad, traslación, escala, TRS
   =============================================================================
   TEMA 3 de TEMAS.md (1ra lección): identidad, traslación, rotación, escala,
   TRS y orden de multiplicación.

   == POR QUÉ MATRICES =======================================================

   Cada transformación geométrica (mover, girar, estirar) se puede escribir
   como una función lineal sobre coordenadas: esa función es multiplicar por
   una matriz. La GRACIA es componer:

       punto' = (T * R * S) * punto

   Una sola multiplicación aplica TODO. El objeto del juego tiene SU matriz
   de mundo (model) y la GPU hace mvp = P * V * M (L06).

   == IDENTIDAD Y COLUMN-MAJOR ===============================================

   La identidad I: I * M = M = M * I (no hace nada). En memoria column-major
   (igual que GLM y que la lección 01):

       I = [ 1 0 0 0 ]  (columna 0)
           [ 0 1 0 0 ]  (columna 1)
           [ 0 0 1 0 ]  (columna 2)
           [ 0 0 0 1 ]  (columna 3)

   Los VECTORES CRECE en COLUMNAS (m[c*4+r]): la 4a columna (c=3) es la
   traslación, SIN escala/rotación en sus primeras 3 componentes.

   == LOS CONSTRUCTORES (column-major) ======================================

   Traslación T (t=traslación): pinta columnas como identidad y la col 3:
       col3 = (tx, ty, tz, 1)   — con punto (1.0f en w)

   Escala S (s=escalado por ejes):
       col0 = (s.x, 0,   0, 0); col1 = (0,   s.y, 0, 0); col2 = (0,   0, s.z, 0)

   Rotación Z (ángulo θ, right-handed, la rotación es ANTIreloj mirando
   desde +z hacia el origen... en realidad: en RH aplicado en column-major
   la matriz que gira PUNTOS en sentido antihorario en el plano XY es esta):
       col0 = (cosθ,  sinθ, 0)     -- cuota "x" del resultado alcanza de X.
       col1 = (-sinθ, cosθ, 0)
       col2 = (0, 0, 1)
   Homologa con GLM: glm::rotate(id, θ, vec3(0,0,1)) produce ESTA matriz
   (verificado en la demo de la L06 contra glm::rotate).

   Rotación X (gira el plano Y-Z):
       col1 = (0, cosθ, sinθ); col2 = (0, -sinθ, cosθ)
   Rotación Y (gira el plano X-Z):
       col0 = (cosθ, 0, -sinθ); col2 = (sinθ, 0, cosθ)

   == ORDEN DE MULTIPLICACIÓN: TRS ==========================================

   Construir el "world del objeto": escalar, LUEGO rotar, LUEGO trasladar:

       world = T * R * S       # se lee: primero S, después R, después T

   ¿Por qué este orden? (en column-major, el factor de la DERECHA se aplica
   primero al punto):
     1) S escala el objeto local (desde SU origen).
     2) R gira ese objeto ya escalado.
     3) T lo traslada al lugar del mundo.
   Si lo hubieras escrito S * R * T, primero trasladarías (y el doble local
   TODo el objeto escala completo atravesado con la traslación).

   == GLM EQUIVALENCIAS (verificadas en el repo 1.0.1) ======================

     glm::translate(I, t)  = T        glm/ext/matrix_transform.hpp
     glm::scale(I, s)      = S        glm/ext/matrix_transform.hpp
     glm::rotate(I, θ, eje) = R       glm/ext/matrix_transform.hpp

   (En GLM debes pasar el CONJUNTO de R en un eje normalizado; sin cuenta
   externa la demo de L06 que mi matrizRotacionZ es EXACTAMENTE la misma
   que generate glm::rotate(id, θ, (0,0,1)).)

   == COMO PROBAR ESTO ========================================================

   g++ -std=c++20 -Wall -Wextra -I. leccion_05_... -o leccion05 && ./leccion05
   (El "include/mate3d.hpp" del módulo trae Mat4 con producto mat*mat, mat*vec
   y transformarPunto/transformarDireccion de infraestructura.)
   ============================================================================= */

#include "include/mate3d.hpp"
#include <cstdio>
#include <iostream>

using namespace mate;

// ==== CONSTRUCTORES DE LA LECCIÓN (los ejercicios los IMPLEMENTAN) =========
Mat4 matrizTraslacion(Vec3 t) {
    Mat4 m = Mat4::identidad();
    m.ponerColumna3(3, t, 1.0f);
    return m;   // solo cambia la columna 3
}

Mat4 matrizEscala(Vec3 s) {
    Mat4 m = Mat4::identidad();
    m.set(0, 0, s.x);
    m.set(1, 1, s.y);
    m.set(2, 2, s.z);
    return m;   // solo cambia la diagonal (col c, fila c)
}

Mat4 matrizRotacionZ(float rad) {
    float c = std::cos(rad), s = std::sin(rad);
    Mat4 m = Mat4::identidad();
    m.ponerColumna(0, { c, s, 0.0f });   // col 0 es "hacia dónde va el eje X local"
    m.ponerColumna(1, {-s, c, 0.0f });   // col 1 es el eje Y luego de la rotación
    return m;
}

Mat4 matrizRotacionX(float rad) {
    float c = std::cos(rad), s = std::sin(rad);
    Mat4 m = Mat4::identidad();
    m.ponerColumna(1, { 0.0f, c, s });
    m.ponerColumna(2, { 0.0f, -s, c });
    return m;
}

Mat4 matrizRotacionY(float rad) {
    float c = std::cos(rad), s = std::sin(rad);
    Mat4 m = Mat4::identidad();
    m.ponerColumna(0, { c, 0.0f, -s });
    m.ponerColumna(2, { s, 0.0f, c });
    return m;
}

// La COMPOSICIÓN del tema: world = T * R * S.
Mat4 matrizSRT(Vec3 escala, Mat4 const& rotacion, Vec3 traslacion) {
    return matrizTraslacion(traslacion) * rotacion * matrizEscala(escala);
}

// ==== DEMO ================================================================
static void mostrar(Mat4 const& m, const char* nombre) {
    std::cout << nombre << ":\n";
    for (int row = 0; row < 4; ++row) {
        std::cout << "  [";
        for (int c = 0; c < 4; ++c) std::printf(" %7.3f", m.at(c, row));
        std::cout << " ]\n";
    }
}

int main() {
    std::cout << "== ROTACION Z 90 GRADOS ==\n";
    mostrar(matrizRotacionZ(aRadianes(90)), "rotZ(90°)");
    Vec3 p = transformarPunto(matrizRotacionZ(aRadianes(90.0f)), {1, 0, 0});
    std::printf("rotZ(90°) * (1,0,0)  =  (%.3f, %.3f, %.3f)   (el eje X va a Y)\n",
                p.x, p.y, p.z);

    std::cout << "\n== SRT EN UNA CAJA ==\n";
    // Caja unitaria centrada en el origen (2 x 1 x 1) girada 30° en Y,
    // puesta en (5, 0, 0) con escala (2)
    Mat4 world = matrizSRT(Vec3{2.0f, 1.0f, 1.0f},
                           matrizRotacionY(aRadianes(30.0f)),
                           Vec3{5.0f, 0.0f, 0.0f});
    Vec3 esquinaLocal{0.5f, 0.5f, 0.0f};
    Vec3 esquinaMundo = transformarPunto(world, esquinaLocal);
    std::printf("esquina local (0.5, 0.5, 0)  ->  mundo (%.3f, %.3f, %.3f)\n",
                esquinaMundo.x, esquinaMundo.y, esquinaMundo.z);

    std::cout << "\n== DIRECCIONES NO SE TRASLADAN ==\n";
    Vec3 dirLocal = V3Z;                       // "apunta en +z"
    Vec3 dirMundo = transformarDireccion(world, dirLocal);
    std::printf("dirección +z local  ->  mundo (%.3f, %.3f, %.3f) — sin mover\n",
                dirMundo.x, dirMundo.y, dirMundo.z);

    std::cout << "\n== GLM: LA MISMA TRS (snippet de la lección) ==\n";
    std::cout << "// glm::mat4 world =\n"
                 "//   glm::translate(I, t) * glm::rotate(I, ang, eje) * glm::scale(I, esc);\n"
                 "(con I = glm::mat4(1.0f) — la identidad en GLM)\n";

    std::cout << "=== FIN LECCIÓN 05 ===\n";
    return 0;
}
