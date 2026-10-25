/* =============================================================================
   MÓDULO 19 - LECCIÓN 02: GLM y la GPU — value_ptr y configuración con defines
   =============================================================================
   TEMA 1 de TEMAS.md (2da lección): value_ptr, configuración de GLM y por qué
   NO forzamos SIMD.

   == VALUE_PTR: pasar una matriz a la GPU ====================================

   Cuando una API gráfica (glUniformMatrix4fv del módulo 20, o cualquier
   backend del módulo 26) pide un puntero a 16 floats, no podés pasarle
   `&matriz` así como vienes: glm::mat4 no expone el array como miembro
   estable de su API. Para eso existe glm::value_ptr (header verificado en
   el repo tag 1.0.1: glm/gtc/type_ptr.hpp):

       glm::mat4 m = glm::mat4(1.0f);
       float const* data = glm::value_ptr(m);   // como si fuera float[16]

   value_ptr te da el puntero a los 16 floats EN EL ORDEN DE ALMACENAMIENTO
   de GLM: column-major, primero la columna 0. Sirve igual para vec2/3/4
   y para quat (16/16 floats).

   Regla práctica para el módulo 20:

       glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mvp));

   GL_FALSE significa "no transpongas al subir": el layout en RAM de GLM
   (column-major) ya es el que GLSL espera. Si te olvidás y pasás GL_TRUE,
   la escena se mira arruinada: cada rotación queda rara y la traslación
   termina pegada al fondo. Eso casi siempre lo arreglás quitando "el
   transpose" BRUTAL (GL_TRUE) o revisando si tu matriz ya venía traspuesta.

   == CONFIGURAR GLM CON DEFINES =============================================

   Todos los defines se ponen ANTES de incluir GLM:

       #define GLM_FORCE_DEPTH_ZERO_TO_ONE
       #include <glm/glm.hpp>

   Los defines acá (verificados en el repo oficial del tag 1.0.1 en
   detail/setup.hpp):
     GLM_FORCE_DEPTH_ZERO_TO_ONE  -- cambia el rango de profundidad de las
                                     funciones "plain" (perspective, ortho,
                                     etc.) de NDC z en [-1..1] (OpenGL) a
                                     [0..1] (Direct3D/Vulkan). En Módulo 26
                                     se explica mejor. El curso usa OpenGL:
                                     NO lo definimos.
     GLM_FORCE_LEFT_HANDED / GLM_FORCE_RIGHT_HANDED -- cámara y proyección en
                                     sistema izquierdo/derecho. El curso usa
                                     right-handed (RH) SIEMPRE.
     GLM_FORCE_PURE               -- fuerza TODO escalar, sin instrucciones
                                     SIMD. Útil para debuguear: si un bug de
                                     matemática desaparece con GLM_FORCE_PURE
                                     y aparece sin él, koordinados indirectos.
     GLM_FORCE_MESSAGES           -- GLM imprime en la compilación qué camino
                                     eligió (ideal para aprender cómo elige).
     GLM_FORCE_DEFAULT_ALIGNED_GENTYPES -- alinea vec3/vec4 a 16 bytes para
                                     SIMD interno. Cambia sizeof() y layouts:
                                     ROMPE cálculos de memoria (memcpy,
                                     upload a GPU). NO lo usemos.

   == ¿QUÉ ELIGE GLM POR DEFECTO EN NUESTRA MÁQUINA? (sin forzar nada) =======

   Sin defines de fuerza, GLM 1.0.1 elige instrucciones según los macros que
   el COMPILADOR define por los flags (-march=...):
     - x86-64 baseline (TODAS las CPU AMD64, la del dueño incluida): tiene
       SSE2 siempre, así que GCC/Clang definen __SSE2__ y GLM usa intrinsics
       SSE2 por defecto. Esto es seguro en la Celeron N4020 (sin AVX/AVX2).
     - Si compilás con -march=native o -mavx2 aparecen __AVX2__ y GLM puede
       elegir AVX: el binario se cuelga con SIGILL en cualquier CPU que no
       tenga AVX. REGLA DEL CURSO (ver módulo 29 y L17): nunca distribuir
       binarios compilados con -march=native sin verificar la CPU destino.

   == CÓMO PROBAR ESTO =======================================================

   cmake -B build -S . -DGLM_BUILD_LIBRARY=OFF && cmake --build build -j2
   ./build/leccion_02_glm_config_value_ptr

   La demo no define nada especial: usa GLM como viene (el camino seguro).
   ============================================================================= */

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>  // glm::translate (verificado 1.0.1)
#include <glm/gtc/type_ptr.hpp>          // glm::value_ptr (verificado 1.0.1)
#include <iomanip>
#include <iostream>

int main() {
    std::cout << "== VALUE_PTR: mismo dato, dos 'trajes' ==\n";
    glm::mat4 id = glm::mat4(1.0f);  // la identidad en GLM 1.0.1
    float const* data = glm::value_ptr(id);

    std::cout << "Los 16 floats en memoria (columna por columna):\n";
    for (int i = 0; i < 16; ++i) {
        std::cout << std::setw(6) << data[i] << ((i % 4 == 3) ? "\n" : " ");
    }
    std::cout << "(los 1 están en las posiciones c*4+c: diagonal column-major)\n";

    std::cout << "\n== TRASLACIÓN EN LA COLUMNA 3 ==\n";
    glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3{1.0f, 2.0f, 3.0f});
    float const* a = glm::value_ptr(t);
    std::cout << "Los últimos 4 floats (== la columna 3): ";
    for (int i = 12; i < 16; ++i) std::cout << a[i] << " ";
    std::cout << "\n(la traslación vive al final: posiciones 12..15)\n";

    std::cout << "\n== QUÉ CAMBIA CON GLM_FORCE_DEPTH_ZERO_TO_ONE ==\n";
    std::cout << "Sin definirlo: NDC z en [-1..1] (OpenGL clásico).\n"
                 "Definido:      NDC z en [0..1]  (D3D/Vulkan). Las funciones\n"
                 "*_RH_NO / *_RH_ZO lo eligen EXPLÍCITAMENTE sin necesidad de\n"
                 "defines — verificado en glm/ext/matrix_clip_space.hpp.\n";

    std::cout << "=== FIN LECCIÓN 02 ===\n";
    return 0;
}
