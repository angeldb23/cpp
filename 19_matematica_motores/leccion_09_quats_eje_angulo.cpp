/* =============================================================================
   MÓDULO 19 - LECCIÓN 09: Cuaterniones I — eje-ángulo, rotar vectores, cobertura
   =============================================================================
   TEMA 5 de TEMAS.md (1ra lección): cuaterniones axis-angle, producto,
   identidad, rotación de vectores.

   == POR QUÉ NO EUCLIDES (ángulos de Euler) ==================================

   Guardar una orientación con 3 ángulos (pitch/yaw/roll) tiene tres
   problemas clásicos:
     - GIMBAL LOCK: al llegar a ±90° de pitch, dos ejes quedan alineados y
       PERDES UN GRADO DE LIBERTAD (yaw y roll dejan de ser independientes).
     - COMPOSICIÓN oscura: combinar dos orientaciones a mano es feo y
       sensible al orden.
     - Interpolar ángulos individuales da caminos OSLANY (la cara no gira
       "por el camino más corto": gira cada eje por separado — L10 vé.

   El quaternion tiene 4 números (x, y, z, w) con UNA sola unidad:
     identidad = (0, 0, 0, 1)   -> "sin rotación"
   Restricción de unitar: x² + y² + z² + w² = 1 (me quedo en el hemisferio OK).

   == EJE-ÁNGULO → QUAT ======================================================

   Rotación de θ radianes alrededor del eje unitario ń:

       q = ( ń * sin(θ/2),  cos(θ/2) )      # xyz = eje, w = cos

   Intuitivo: el VALOR W codifica "cuánto giraste" (cos de MEDIA vuelta),
   el VECT RESTR codifica "alrededor de quién".

   RotZ a 90°: eje=(0,0,1), θ=π/2 → q = (0, 0, sin45°, cos45°).

   == ROTACIÓN DE UN VECTOR (la fórmula "rápida") =============================

   Formulás: hacer v' = q * v_puro * conj(q) es la DEFINITION (triple product
   de quats, funciona TIENE doble). La forma expandida (sin quats):

       u = q.xyz (el eje, escalado por sin(θ/2))
       t = 2 * cross(u, v)
       v' = v + q.w * t + cross(u, t)

   (Verificado en la demo: coincide con rotar con matriz y con matrices.)
   ES diferencial O(1) SIN crear matrices: perfecto para girar un solo vector
   p. ej. una dirección de luz o rayo (módulo 21 lo usa todo el tiempo).

   == DOBLE COBERTURA (muy despriano de L10) =================================

   q y −q representan LA MISMA rotación (girar 360° censado serie −1 = id):
     dot(qA, qB) > 0  → "van por el mismo camino"
     dot(qA, qB) < 0  → "van por caminos opuestos" (la mezcla a seguir rompe)
   mate3d.hpp incluye casiIgualQuat() que respeta esto.

   == PRODUCTO: COMPOSICIÓN DE ROTACIONES ====================================

   q3 = q1 * q2 aplica PRIMERO q2 y DESPUÉS q1 (miro mi círculo: igual que
   matrices A * B). RotZ45° * RotZ45° = RotZ90° (demo lo pruebla).

   == GLM EQUIVALENTES (repo 1.0.1 verificado) ===============================

     glm::angleAxis(θ, eje)        → quat: glm/ext/quaternion_trigonometric.hpp
     glm::rotate / normalize (q)   →  quaternion_operational/geometric
     glm::slerp                     → glm/ext/quaternion_common.hpp (L10)

   ============================================================================= */

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>                // definición de glm::quat (1.0.1)
#include <glm/ext/quaternion_trigonometric.hpp>  // glm::angleAxis (verificado)
#include "include/mate3d.hpp"
#include <cstdio>
#include <iostream>

using namespace mate;

// ==== PRIMITIVAS LOCALES (L03/L04) =========================================
float productoEscalar(Vec3 a, Vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
Vec3  productoVectorial(Vec3 a, Vec3 b) {
    return { a.y*b.z - a.z*b.y,
             a.z*b.x - a.x*b.z,
             a.x*b.y - a.y*b.x };
}
float largo(Vec3 a) { return std::sqrt(productoEscalar(a, a)); }
Vec3  normalizar(Vec3 a) {
    float l = largo(a);
    if (l <= 1e-8f) return {0.0f, 0.0f, 0.0f};
    return a / l;
}
// ===========================================================================

// ==== FUNCIONES DE LA LECCIÓN ==============================================
Quat quatDesdeEjeAngulo(Vec3 ejeUnit, float rad) {
    float mitsa = std::sin(rad * 0.5f);
    float cosMitad = std::cos(rad * 0.5f);
    return { ejeUnit.x * mitsa, ejeUnit.y * mitsa, ejeUnit.z * mitsa, cosMitad };
}

Quat conjugar(Quat q) { return {-q.x, -q.y, -q.z, q.w}; }

// Rotación de un vector con quat, SIN crear matriz (fórmula expandida):
Vec3 rotarVectorQuat(Quat const& q, Vec3 v) {
    Vec3 u{q.x, q.y, q.z};
    Vec3 t = productoVectorial(u, v) * 2.0f;
    return v + t * q.w + productoVectorial(u, t);
}

// ==== DEMO ================================================================
static void mostrar(Vec3 const& v, const char* nombre) {
    std::printf("%-26s (%6.3f, %6.3f, %6.3f)\n", nombre, v.x, v.y, v.z);
}

int main() {
    std::cout << "== rotZ 90° CON QUAT ==\n";
    Quat q = quatDesdeEjeAngulo(V3Z, aRadianes(90.0f));
    std::printf("q = (%.4f, %.4f, %.4f, %.4f)   (eje*sin45, cos45)\n",
                q.x, q.y, q.z, q.w);
    mostrar(rotarVectorQuat(q, V3X), "rotq(90°) * (1,0,0)");
    std::cout << " (debería quedar (0, 1, 0) ≈ Igual que la matriz rotZ L05)\n";

    std::cout << "\n== EQUIVALENCIA CON GLM angleAxis ==\n";
    glm::quat qGlm = glm::angleAxis(aRadianes(90.0f), glm::vec3(0, 0, 1));
    std::printf("glm::angleAxis same = (%.4f, %.4f, %.4f, %.4f)\n",
                qGlm.x, qGlm.y, qGlm.z, qGlm.w);
    std::cout << " (casiIgualQuat: " << (casiIgualQuat(q, Quat{qGlm.x, qGlm.y, qGlm.z, qGlm.w})
        ? "SI (la misma orientación)" : "NO") << ")\n";

    std::cout << "\n== DOBLE COBERTURA ==\n";
    Quat qNeg{ -q.x, -q.y, -q.z, -q.w };
    Vec3 conNegativo = rotarVectorQuat(qNeg, V3X);
    mostrar(conNegativo, "rot(-q) * (1,0,0)");
    std::cout << "q y -q giran IGUAL (mi misma orientación). La comparación de "
                 "rotaciones es por |dot| (casiIgualQuat).\n";

    std::cout << "\n== PRODUCTO: rotZ45 * rotZ45 = rotZ90 ==\n";
    Quat a = quatDesdeEjeAngulo(V3Z, aRadianes(45.0f));
    Quat c = a * a;
    Vec3 r90 = rotarVectorQuat(c, V3X);
    mostrar(r90, "(q45*q45) * (1,0,0)");
    std::cout << " (otra vez (0, 1, 0): composición de rotaciones)\n";

    std::cout << "=== FIN LECCIÓN 09 ===\n";
    return 0;
}
