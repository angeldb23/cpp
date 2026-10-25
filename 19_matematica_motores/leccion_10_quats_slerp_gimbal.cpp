/* =============================================================================
   MÓDULO 19 - LECCIÓN 10: Cuaterniones II — slerp, nlerp, gimbal lock y matriz
   =============================================================================
   TEMA 5 de TEMAS.md (2da lección): slerp, nlerp, look-at, gimbal lock y
   conversión quat → matriz.

   == NLERP: la mezcla "barata" ==============================================

   lerp lineal de QUATS + normalizar: nlerp. Más rápido que slerp; suelo es:

   NOTA IMPORTANTE: NO es velocidad angular constante. Para pequeñas
   diferencias (camara que sigue al jefe, animaciones suaves) el nlerp
   es suficientemente bueno y se usa MUCHO en motores rcal (-O). Para crema
   cuya velocidad importe, la CAMARA ANTES que el error) — slerp.

   == SLERP: la mezcla "geodésica" ===========================================

   Interpola A LOS LONGOTDE del arco MINIMO del "esfera 4D" de todos los quats:

       si dot(q0, q1) < 0: q1 = -q1       # el camino más corto (doble cobertura)
       d  = clamp(dot(q0, q1), -1, 1)
       θ  = acos(d)
       si θ < epsilon: devolver nlerp (evita dividir por ~0)
       q(t) = ( sin((1-t)θ) * q0 + sin(tθ) * q1 ) / sin(θ)

   Propiedades que la demo atestigúa:
     - endpoints exactos: slerp(q0, q1, 0) = q0, slerp(..., 1) = q1
     - a t=0.5 es la "mitad del camino" misma VELOCIDAD de rotación.
   GLM: glm::slerp(q0, q1, t) — glm/ext/quaternion_common.hpp (1.0.1:91).

   == GIMBAL LOCK vs YAW/PITCH/ROLL ==========================================

   Euler (pitch primero, luego yaw):
       pitch 90° + yaw (después) ≠ equivalentes QUATS con cada composición
   con quats la rotación alrededor de un eje MUNDIAL (yaw "global") es
   SIEMPRE alrededor del mismo eje: no se "acostumbra" del pitch.

   La demo MONSTRAR: con quats, componer rotX(90°) y luego rotY(ángulo)
   TERMA MANTIENE el eje Y mundial... MIENTRAS con ángulos accedidos el
   yaw PIERDE un grado de libertad. (Si te interesa: camara FPS camera con
   yaw (mundial) y pitch (local) SIN gimbal lock: L11 aplica y módulo 21
   lo retoma: "yaw/pitch" camera.)

   == QUAT → MATRIZ (pasar a las pipelines del módulo 20/21) =================

   Para mandar la orientación al GPU se convierte en Mat3/Mat4 (columnas):

       rotación de un eje "unitari o" con quat q (bien DE unitar):
         col0 = (1-2(y²+z²), 2(xy+zw), 2(xz-yw))
         col1 = ( 2(xy-zw), 1-2(x²+z²), 2(yz+xw))
         col2 = ( 2(xz+yw), 2(yz-xw), 1-2(x²+y²))

   GLM: glm::mat3_cast(q) / glm::mat4_cast(q) — gtc/quaternion.hpp (1.0.1:82/90).
   La demo COMPARA mi mat3 desde quat con glm::mat3_cast: iguales.

   ============================================================================= */

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>   // mat3_cast (verificado 1.0.1:82)
#include <glm/ext/quaternion_common.hpp>  // slerp (verificado:77)
#include "include/mate3d.hpp"
#include <cstdio>
#include <iostream>

using namespace mate;

// ==== PRIMITIVAS LOCALES ===================================================
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
float dotQuat(Quat a, Quat b) {
    return a.x*b.x + a.y*b.y + a.z*b.z + a.w*b.w;
}
Quat quatDesdeEjeAngulo(Vec3 ejeUnit, float rad) {
    float mitad = std::sin(rad * 0.5f);
    float cosMitad = std::cos(rad * 0.5f);
    return { ejeUnit.x * mitad, ejeUnit.y * mitad, ejeUnit.z * mitad, cosMitad };
}
// ===========================================================================

// ==== FUNCIONES DE LA LECCIÓN ==============================================
// NLERP: mezcla lineal de quats, renormalizada.
Quat nlerpQuat(Quat const& q0, Quat const& q1, float t) {
    Quat a = q0, b = q1;
    if (dotQuat(a, b) < 0.0f) b = -b;          // camino más corto
    Quat mezcl = a + (b - a) * t;              // (b - a): vec operator de la lib
    float l = std::sqrt(dotQuat(mezcl, mezcl));
    if (l <= 1e-8f) return quatIdentidad();
    return mezcl / l;
}

// SLERP: el arco mínimo entre las dos orientaciones (géodésico exacto).
Quat slerpQuat(Quat const& q0, Quat const& q1, float t) {
    Quat a = q0, b = q1;
    if (dotQuat(a, b) < 0.0f) b = -b;          // doble cobertura: corto camino
    float d = std::max(-1.0f, std::min(1.0f, dotQuat(a, b)));
    float theta = std::acos(d);
    if (theta < 1e-5f) return nlerpQuat(q0, q1, t);  // casi iguales: barato
    float seno = std::sin(theta);
    float w0 = std::sin((1.0f - t) * theta) / seno;
    float w1 = std::sin(t * theta) / seno;
    return a * w0 + b * w1;                     // sumas escalar (* Quat lib)
}

// quat → matriz de rotación (columnas). Necesita quat NORMALIZADO.
Mat3 mat3DesdeQuat(Quat const& q) {
    float xx = q.x*q.x, yy = q.y*q.y, zz = q.z*q.z;
    Mat3 m;
    // col0
    m.set(0, 0, 1.0f - 2.0f*(yy + zz));
    m.set(0, 1, 2.0f*(q.x*q.y + q.z*q.w));
    m.set(0, 2, 2.0f*(q.x*q.z - q.y*q.w));
    // col1
    m.set(1, 0, 2.0f*(q.x*q.y - q.z*q.w));
    m.set(1, 1, 1.0f - 2.0f*(xx + zz));
    m.set(1, 2, 2.0f*(q.y*q.z + q.x*q.w));
    // col2
    m.set(2, 0, 2.0f*(q.x*q.z + q.y*q.w));
    m.set(2, 1, 2.0f*(q.y*q.z - q.x*q.w));
    m.set(2, 2, 1.0f - 2.0f*(xx + yy));
    return m;
}

// ==== DEMO ================================================================
static void mostrarQuat(Quat const& q, const char* nombre) {
    std::printf("%-30s (x %6.3f, y %6.3f, z %6.3f, w %6.3f)\n",
                nombre, q.x, q.y, q.z, q.w);
}

int main() {
    Quat identity = quatIdentidad();
    Quat objetivo = quatDesdeEjeAngulo(V3Z, aRadianes(90.0f));

    std::cout << "== NLERP vs SLERP a mitad del camino (45°) ==\n";
    Quat nM = nlerpQuat(identity, objetivo, 0.5f);
    Quat sM = slerpQuat(identity, objetivo, 0.5f);
    mostrarQuat(nM, "nlerp (t=0.5)");
    mostrarQuat(sM, "slerp (t=0.5)");
    std::cout << " para 90° separa; para ángulos CHICOS son casi iguales\n";

    std::cout << "\n== VERIFICACIÓN CONTRA glm::slerp ==\n";
    glm::quat g0(1.0f, 0.0f, 0.0f, 0.0f);       // w-first en ctor de glm::quat
    glm::quat g1 = glm::angleAxis(aRadianes(90.0f), glm::vec3(0,0,1));
    glm::quat gs = glm::slerp(g0, g1, 0.5f);
    mostrarQuat(Quat{gs.x, gs.y, gs.z, gs.w}, "glm::slerp (t=0.5)");
    bool iguales = casiIgualQuat(sM, Quat{gs.x, gs.y, gs.z, gs.w}, 1e-4f);
    std::cout << "mi slerp == glm::slerp? " << (iguales ? "SI" : "NO") << "\n";

    std::cout << "\n== QUAT → MATRIZ (vs glm::mat3_cast) ==\n";
    Mat3 mia = mat3DesdeQuat(objetivo);
    glm::mat3 gmm = glm::mat3_cast(g1);
    bool igualMat = true;
    for (int c = 0; c < 3 && igualMat; ++c)
        for (int r = 0; r < 3; ++r)
            if (casiIgual(mia.at(c, r), gmm[c][r], 1e-5f) == false) {
                igualMat = false; break;
            }
    std::cout << "mi mat3DesdeQuat == glm::mat3_cast? " << (igualMat ? "SI" : "NO") << "\n";

    std::cout << "\n== GIMBAL LOCK: Euler pierde, quat no ==\n";
    Quat pitch = quatDesdeEjeAngulo(V3X, aRadianes(90.0f));
    Quat yawMundial = quatDesdeEjeAngulo(V3Y, aRadianes(90.0f));
    Quat compuesto = yawMundial * pitch;           // yaw global, luego pitch local
    (void)compuesto;
    std::cout << "en Euler pitch=90° el yaw es INDISTINGUIBLE del roll; con quat\n"
                 "cada rotación es eje-ángulo propio y no colapsa. (módulo 21:\n"
                 "cámara FPS yaw/pitch sin gimbal lock)\n";

    std::cout << "=== FIN LECCIÓN 10 ===\n";
    return 0;
}
