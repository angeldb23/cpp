/* =============================================================================
   MÓDULO 19 - LECCIÓN 06: Modelo-Vista-Proyección y orden de multiplicación
   =============================================================================
   TEMA 3 de TEMAS.md (2da lección): modelo-vista-proyección, column-major en
   la práctica y COMPUTER cross-check contra GLM.

   == LA CADENA MVP ==========================================================

   Para dibujar un punto del mundo en la pantalla el shader (módulo 20)
   multiplica:

       gl_Position = P * V * M * vec4(posLocal, 1.0)

       M = model  : del ESPACIO LOCAL del objeto al MUNDO (L05: T*R*S).
       V = view   : del MUNDO a los OJOS de LA CÁMARA (inverse del TR de la
                    cámara: es la vista "existe" de una cámara mirando a −z).
       P = projection : de los ojos a CLIP (la homogenia que aplana: L11).

   El orden NO es negociable: con column-major "A * B aplica B primero",
   y en la cadena P * V * M "el punto entra por M" (después del igual, el
   orden de aplicacions es M -> V -> P).

   == LA MATRIZ VISTA (cámara simple "look-at") ==============================

   Cámara en `pos`, mirando al `target`, con `up` "aprox" (0,1,0):

       eje  (forward/back): nuevo "z" de la cámara: zCam = -dir
            dir = normalizar(target - pos)   — camara mira hacia
        (recordar: en RH la cámara mira hacia SU −z, o sea el forward es −zCam)
       right = normalizar(cross(dir, up))
       upCam = cross(right, dir)   (ya unitario, es perpendicular a los dos)

   La matriz VISTA es la inversa del TR que "colocaría la cámara" en el mundo
   con esa SOBREVOLACIÓN: los ejes de la cámara vuelven como COLUMNAS:

       V = [  right.x   right.y   right.z    dot(right, −pos)... ]

   — en notación material: V = inversaRigida(colocarCamara) donde
     colocarCamara tiene columnas right, upCam, zCam−(−dir), traslación pos.
   (En la L07 se DECOMPONE cómo funciona inversaRigida: limpio para no confundir.)

   == NDC Y CLIP (prólogo de la L11) ========================================

   Después de P * V * M queda CLIP:

       clip = ( x_c, y_c, z_c, w_c ) con w_c = -(z_view)  (en perspectiva)

   La división reveladora: ndc = clip.xyz / clip.w. Un punto está DENTRO de
   la proyección si  |ndc.x| <= 1, |ndc.y| <= 1, y (OpenGL) -1 <= ndc.z <= 1.

   == VERIFICACIÓN CRUZADA CONTRA GLM (repo 1.0.1) ==========================

   Verificado en el repo (ejercirtar):
     glm::lookAtRH        —  glm/ext/matrix_transform.hpp   (lookAtRH, lookAtLH)
     glm::perspectiveRH_NO — glm/ext/matrix_clip_space.hpp  (RH + depth -1..1)
     glm::rotate          —  glm/ext/matrix_transform.hpp

   La demo COMPILA MI matrizVista vs glm::lookAtRH y MI rotZ vs glm::rotate
   y se imprime true/false en cada comparación. Corrías tú para verlo.

   ============================================================================= */

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>  // lookAtRH, rotate (verificado 1.0.1)
#include <glm/ext/matrix_clip_space.hpp> // perspectiveRH_NO (verificado 1.0.1)
#include "include/mate3d.hpp"
#include <cstdio>
#include <iostream>

using namespace mate;

// ==== PRIMITIVAS LOCALES (ya enseñadas en L03-L05; acá van de paso para
//      que ESTA lección compile por sí sola) =================================
float productoEscalar(Vec3 a, Vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}
Vec3 productoVectorial(Vec3 a, Vec3 b) {
    return { a.y*b.z - a.z*b.y,
             a.z*b.x - a.x*b.z,
             a.x*b.y - a.y*b.x };
}
float largo(Vec3 a) { return std::sqrt(productoEscalar(a, a)); }
Vec3 normalizar(Vec3 a) {
    float l = largo(a);
    if (l <= 1e-8f) return {0.0f, 0.0f, 0.0f};
    return a / l;
}
Mat4 matrizRotacionZ(float rad) {
    float c = std::cos(rad), s = std::sin(rad);
    Mat4 m = Mat4::identidad();
    m.ponerColumna(0, { c, s, 0.0f });
    m.ponerColumna(1, {-s, c, 0.0f });
    return m;
}
Mat4 matrizTraslacion(Vec3 t) {
    Mat4 m = Mat4::identidad();
    m.ponerColumna3(3, t, 1.0f);
    return m;
}
Mat4 matrizEscala(Vec3 s) {
    Mat4 m = Mat4::identidad();
    m.set(0, 0, s.x); m.set(1, 1, s.y); m.set(2, 2, s.z);
    return m;
}
Mat4 matrizSRT(Vec3 escala, Mat4 const& rotacion, Vec3 traslacion) {
    return matrizTraslacion(traslacion) * rotacion * matrizEscala(escala);
}
// ===========================================================================

// ==== FUNCIONES DE LA LECCIÓN ==============================================
// La matriz que "colocaría la cámara" en el mundo con esos ejes.
Mat4 colocacionDeCamara(Vec3 pos, Vec3 target, Vec3 up) {
    Vec3 dir   = normalizar(target - pos);              // hacia el target
    Vec3 right = normalizar(productoVectorial(dir, up));    // mirar a la par
    Vec3 upCam = productoVectorial(right, dir);             // arriba de la cámara
    Mat4 m = Mat4::identidad();
    m.ponerColumna(0, right);   // ejes de la cámara como COLUMNAS
    m.ponerColumna(1, upCam);
    m.ponerColumna(2, -dir);    // z cam = "atrás" = -dir (RH: mira a -z)
    m.ponerColumna3(3, pos, 1.0f);
    return m;
}

// Vista = inversa (rigida) de la colocación. (Es MVP's V.)
Mat4 matrizVista(Vec3 pos, Vec3 target, Vec3 up) {
    return inversaRigida(colocacionDeCamara(pos, target, up));
}

// ==== conversion de extenrao de Mat4 nuestro a glm::mat4 ====================
glm::mat4 aGlm(Mat4 const& m) {
    glm::mat4 r = glm::mat4(1.0f);
    for (int c = 0; c < 4; ++c)
        for (int row = 0; row < 4; ++row)
            r[c][row] = m.at(c, row);
    return r;
}

static void mostrarMatriz(glm::mat4 const& m, const char* nombre) {
    std::cout << nombre << ":\n";
    for (int row = 0; row < 4; ++row) {
        std::cout << "  [";
        for (int c = 0; c < 4; ++c) std::printf(" %7.3f", m[c][row]);
        std::cout << " ]\n";
    }
}

int main() {
    std::cout << "== MI rotZ IGUALES glm::rotate ==\n";
    float ang = aRadianes(37.0f);
    glm::vec3 ejeZ{0.0f, 0.0f, 1.0f};
    // GLM rotate sobre eje normalizado (el modulo.), y mi col como rows.
    Mat4 miRotZ = matrizRotacionZ(ang);
    glm::mat4 glmRotZ = glm::rotate(glm::mat4(1.0f), ang, ejeZ);
    bool igualesRotZ = true;
    for (int c = 0; c < 4 && igualesRotZ; ++c)
        for (int row = 0; row < 4; ++row)
            if (casiIgual(miRotZ.at(c, row), glmRotZ[c][row], 1e-5f) == false) {
                igualesRotZ = false; break;
            }
    std::cout << "mi matrizRotacionZ(ang, z) == glm::rotate(I, ang, (0,0,1))? "
              << (igualesRotZ ? "SI" : "NO") << "\n";

    std::cout << "\n== MI matrizVista IGUALES glm::lookAtRH ==\n";
    Vec3 posCam{0.0f, 2.0f, 5.0f}, target{0.0f, 0.5f, 0.0f}, up{0.0f, 1.0f, 0.0f};
    Mat4 miV = matrizVista(posCam, target, up);
    glm::mat4 glmV = glm::lookAtRH(glm::vec3(posCam.x, posCam.y, posCam.z),
                                   glm::vec3(target.x, target.y, target.z),
                                   glm::vec3(up.x, up.y, up.z));
    bool igualesV = true;
    for (int c = 0; c < 4 && igualesV; ++c)
        for (int row = 0; row < 4; ++row)
            if (casiIgual(miV.at(c, row), glmV[c][row], 1e-5f) == false) {
                igualesV = false; break;
            }
    std::cout << "mi matrizVista(pos, target, up) == glm::lookAtRH(...)? "
              << (igualesV ? "SI" : "NO") << "\n";
    mostrarMatriz(glmV, "glm::lookAtRH");

    std::cout << "\n== CADENA MVP (conjunto con perspectiva) ==\n";
    glm::mat4 P = glm::perspectiveRH_NO(aRadianes(60.0f), 16.0f/9.0f, 0.5f, 100.0f);
    glm::mat4 Mundo = aGlm(matrizSRT(Vec3{1.0f, 2.0f, 1.0f},
                                     matrizRotacionZ(aRadianes(35.0f)),
                                     Vec3{0.0f, 0.0f, 0.0f}));
    glm::mat4 MVP = P * glmV * Mundo;
    // un punto de la esquina local
    glm::vec4 clip = MVP * glm::vec4(0.5f, 1.0f, 0.0f, 1.0f);
    glm::vec3 ndc = glm::vec3(clip) / clip.w;
    std::printf("clip     = (%.4f, %.4f, %.4f, w=%.4f)\n",
                clip.x, clip.y, clip.z, clip.w);
    std::printf("ndc      = (%.4f, %.4f, %.4f)  -> dentro del frustum si "
                "todos |coord| <= 1 y z en [-1..1]\n", ndc.x, ndc.y, ndc.z);

    std::cout << "\n== COMO SUBIR A LA GPU (módulo 20, anticipo) ==\n";
    std::cout << "glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(MVP));\n"
                 "(con value_ptr de la L02 — la matriz ya está column-major)\n";

    std::cout << "=== FIN LECCIÓN 06 ===\n";
    return 0;
}
