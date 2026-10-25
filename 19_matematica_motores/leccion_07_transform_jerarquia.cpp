/* =============================================================================
   MÓDULO 19 - LECCIÓN 07: Jerarquía de transforms — local↔mundo
   =============================================================================
   TEMA 4 de TEMAS.md (1ra lección): local-to-world, world-to-local y jerarquía.

   == EL PROBLEMA: OBJETOS QUE COLGAN DE OTROS ===============================

   Un brazo de robot: hombro → codo → mano/scripts. La MANO se mueve con el
   codo, y el codo con el hombro. Nadie quiere guardar la posición "mundial"
   de todo a mano: se guarda cada nodo en su PROPIO sistema (local) y el
   "espacio del mundo" se caREA recorriendo la jerarquía:

       matMundo(hijo) = matMundo(padre) * matLocal(hijo)

   — con column-major, "el hijo entra por la derecha": primero su TRS local
   (L05), después el mundo de su padre. La multiplicación encadenada es el
   ITERADOR de la jerarquía:

       W_hijo = W_diceabuelo * W_padre * W_hijo_local

   (El orden importa: un W de dos matrices lo mismo no conmuta.)

   == LOCAL ← MUNDO (la operación inversa) ===================================

   ¿Dónde queda un punto del destino del MUNDO dentro del espacio del hijo?

       P_local = inv(W_hijo) * P_mundo        (a mano: CONCATENA las inversas)

   Para toda la cadena, de izquierda a derecha hay que INVERTIR el producto
   completo: inv(padre_p ... hijo) = inv(localHijo)*inv(W_Padre), etc. La función
   inversaRigida de mate3d.hpp (okdo solo ROS + traslación, L07 anexo) ya está
   en el include del módulo para matrices ROTACIÓN+TRASLACIÓN (sin escala).

   Para escala hay que descomponer (L08). Por eso el juego típico usa
   local→mundo SIEMPRE (jugar directo: jerarquía recursiva), y world→local
   solo cuando el nodo es rígido (cámara, picking de mouse, jerarquías que
   se animan sin escala).

   == TRANSFORM LOCAL: C interiors (TRZ) ====================================

   ¿Qué guardo por nodo? La "receta" (pos, rotación, escala) + la mat a código:

       struct Transform {
           Vec3   position;   // local
           float  rotZ;       // (simplificado: giro en Z, en radianes)
           Vec3   scale;
           Transform* padre;  // o índice, en SoA continental (L17: arreglos)
           Mat4   matMundo;   // CACHE del resultado compilado (¡ver L08!)
       };

       Mat4 local = T*R*S, y "world" se compone subiendo la cadena
       de la jerarquía USTED. Diseño clásico y barato.

   == DEMO ====================================================================

   La demo construye 3 nodos (hombro/codo/mano) con locales, compone el MaOOdu
   y comprueba:
   - La mano del mundo desciende SE es la COMPOSICIÓN de hombro..mano.
   - La inversa devuelve el punto mundo a local (vuelve al valor original).
   ============================================================================= */

#include "include/mate3d.hpp"
#include <cstdio>
#include <iostream>

using namespace mate;

// ==== PRIMITIVAS LOCALES (L05: comprimir acá para compilar sola) ============
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
Mat4 matrizRotacionZ(float rad) {
    float c = std::cos(rad), s = std::sin(rad);
    Mat4 m = Mat4::identidad();
    m.ponerColumna(0, { c, s, 0.0f });
    m.ponerColumna(1, {-s, c, 0.0f });
    return m;
}
Mat4 matrizSRT(Vec3 escala, float rotZ, Vec3 traslacion) {
    // el "matLocal" del nodo: T * R * S con rotación z (simplificada)
    return matrizTraslacion(traslacion) * matrizRotacionZ(rotZ) * matrizEscala(escala);
}
// ===========================================================================

struct Transform {
    Vec3  position{0.0f, 0.0f, 0.0f};
    float rotZ    = 0.0f;
    Vec3  scale{1.0f, 1.0f, 1.0f};
    Mat4  matLocal() const {
        return matrizSRT(scale, rotZ, position);
    }
};

// ==== FUNCIONES CLAVE ======================================================
// local→mundo subiendo por la cadena de parents (recursiva, clara).
Mat4 matMundoDe(Transform const& nodo, Transform const* padre,
                Mat4 const& mundoPadre) {
    Mat4 local = nodo.matLocal();
    return (padre == nullptr) ? local : mundoPadre * local;
}

// world→local (rigida, sin escala ni corte): inversa del mundo del nodo.
Mat4 matMundoALocal(Mat4 const& mundoDelNodo) {
    return inversaRigida(mundoDelNodo);
}

// ==== DEMO =================================================================
int main() {
    std::cout << "== LA JERARQUÍA: hombro → codo → mano ==\n";
    Transform hombro{{2.0f, 3.0f, 0.0f}, aRadianes(20.0f), {1.0f, 1.0f, 1.0f}};
    Transform codo  {{1.0f, 0.0f, 0.0f}, aRadianes(30.0f), {1.0f, 1.0f, 1.0f}};
    Transform mano  {{1.5f, 0.0f, 0.0f}, 0.0f,               {1.0f, 1.0f, 1.0f}};

    Mat4 W_hombro = hombro.matLocal();                    // raiz: mundo = local
    Mat4 W_codo   = W_hombro * codo.matLocal();
    Mat4 W_mano   = W_codo   * mano.matLocal();           // LA cadena completa

    Vec3 puntaLocal{2.0f, 0.0f, 0.0f};                    // dedo en el local de mano
    Vec3 puntaMundo = transformarPunto(W_mano, puntaLocal);
    std::printf("la punta de la mano queda en (%.3f, %.3f, %.3f) del mundo\n",
                puntaMundo.x, puntaMundo.y, puntaMundo.z);

    std::cout << "'la mano gira con el codo': los dos comparten la jerarquía\n";

    std::cout << "\n== MUNDO → LOCAL (vuelta de la transformación) ==\n";
    Vec3 puntoMundo{6.0f, 5.0f, 1.0f};
    Vec3 enManoLocal = transformarPunto(matMundoALocal(W_mano), puntoMundo);
    std::printf("el punto6,5,1 del mundo, en el espacio de la MANO local: "
                "(%.3f, %.3f, %.3f)\n", enManoLocal.x, enManoLocal.y, enManoLocal.z);
    // y la vuelta: W_mano * local == el punto original del mundo:
    Vec3 vuelta = transformarPunto(W_mano, enManoLocal);
    std::printf("... y volviendo con W_mano: (%.3f, %.3f, %.3f)\n",
                vuelta.x, vuelta.y, vuelta.z);
    std::cout << "(≈ 6, 5, 1: la composición/inversa es COHERENTE)\n";

    std::cout << "\n== ¿PARA QUÉ SIRVE 'LOCAL'? (picking / audios) ==\n";
    std::cout << "p. ej.: el punto donde clickeaste (en pantalla→mundo con\n"
                 "la matriz inv(P*V)) es mejor pensado EN ESPACIO LOCAL del\n"
                 "objeto para decidir discojunción contra su shape local.\n";

    std::cout << "=== FIN LECCIÓN 07 ===\n";
    return 0;
}
