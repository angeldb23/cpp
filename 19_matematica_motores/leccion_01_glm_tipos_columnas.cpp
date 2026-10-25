/* =============================================================================
   MÓDULO 19 - LECCIÓN 01: GLM mínimo — tipos, column-major y handedness
   =============================================================================
   TEMA 1 de TEMAS.md (1ra lección): GLM tipos (vec2/3/4, mat2/3/4, quat),
   value_ptr (recién acá la mencionamos:detalle completo en L02), convención
   column-major y handedness.

   == QUÉ ES GLM Y POR QUÉ ESTE TAG ===========================================

   GLM (OpenGL Mathematics) es la librería de matemática oficial de facto en
   motores 3D C++. Es header-only, imita la notación de GLSL (vec3, mat4,
   dot, normalize...), y por eso mismo es la "puerta de entrada" natural del
   programador de shaders de los módulos 20 y 21.

   El curso la trae con CMake FetchContent con TAG CONCRETO (nunca master).
   El tag que usamos es 1.0.1 (verificado contra el repo oficial en la
   sesión de construcción: https://github.com/g-truc/glm, tag 1.0.1):

       include(FetchContent)
       FetchContent_Declare(
           glm
           URL https://github.com/g-truc/glm/archive/refs/tags/1.0.1.tar.gz
           # O bien GIT_REPOSITORY + GIT_TAG 1.0.1
       )
       FetchContent_MakeAvailable(glm)
       # target_link_libraries(mine PRIVATE glm::glm glm::glm-header-only)

   En 1.0.1 (miramos el CMakeLists.txt oficial): define targets `glm`,
   `glm::glm` y `glm::glm-header-only` (ALIAS). Con concepto GLM_BUILD_LIBRARY
   en OFF queda *puro header-only* (más rápido de configurar: no compila el
   .cpp que empaqueta las implementaciones).

   == LOS TIPOS ================================================================

   GLM parametriza TODO por plantillas: glm::vec<2, float> == glm::vec2,
   glm::mat<4, 4, float> == glm::mat4. Nombres cortos = float (que es lo
   mismo que la GPU espera: f32). Tenemos:

     glm::vec2 / vec3 / vec4     -- puntos, direcciones, colores
     glm::mat2 / mat3 / mat4     -- rotaciones 2D / rotaciones+normales / model
     glm::quat                   -- orientaciones (L09 y L10)
     glm::ivec2... , glm::dvec3  -- int y double versiones

   Cualquiera llega a la forma general: glm::vec<4, float, glm::highp>.
   default es highp float para CPU, pero con f32 explícitos queda claro.

   == COLUMN-MAJOR (STORAGE) ===================================================

   GLM guarda la MATRIZ por COLUMNAS contiguas en memoria:

     glm::mat4 m;
     // m[0] es la COLUMNA 0 (un vec4), m[3] es la COLUMNA 3 (traslación)

   Por dentro es un float m[16], donde el índice de (fila r, col c) es c*4+r:
     m[c*4 + r]

   ¿Por qué columnas contiguas? Porque la operación más común es
       clip = M * v
   (matriz por columna). Cada componente del resultado es
       sum_k M(r, k) * v[k]
   — es decir: dot producto entre VECTOR COLUMN de M y (READS contiguas) y v.
   A su vez GLSL y el bus GPU (glUniformMatrix4fv con transpose=GL_FALSE)
   esperan column-major: el float* que subís es la primera columna primero.

   En las API del motor: NUNCA "transpones" al vuelo para debuguear; si algo
   sale transponeado, revisá primero si subis con transpose=GL_TRUE o si
   cargás filas pensando que son columnas.

   == HANDEDNESS ===============================================================

   Decidimos "x a la derecha, y para arriba" y queda PENDIENTE: ¿hacia dónde
   apunta +z?

     Right-handed (RH, OpenGL y GLM por defecto): +z hacia EL ESPECTADOR,
     la cámara mira hacia -z. El producto vectorial sigue la regla de la
     mano derecha (usar x cross y = +z en RH y -z en LH).

     Left-handed (LH, típico de Direct3D en APIs viejas + algunos motores):
     +z hacia adentro de la pantalla.

   GLM 1.0.1 trae funciones XX_RH / XX_LH explícitas (verificado en el
   repo: perspectiveRH_NO, perspectiveRH_ZO, lookAtRH...) — en este curso
   usamos SIEMPRE right-handed y depth [-1..1] (las funciones *_NO: OpenGL
   style, NO-Vulkan; las *_ZO son depth [0..1], para D3D/Vulkan).

   En el Módulo 26 (Vulkan) vas a ver GLM_FORCE_DEPTH_ZERO_TO_ONE — fijado
   ANTES de incluir GLM cambia el depth range de las funciones "plain"
   (documento; acá no lo usamos: OpenGL 3.3 espera [-1, 1] en NDC).

   == COMO PROBAR ESTO ========================================================

   Con CMakeFetchContent (CMakeLists.txt de la carpeta del módulo):
     cmake -B build -S . -DGLM_BUILD_LIBRARY=OFF && cmake --build build -j2
     ./build/leccion_01_glm_tipos_columnas

   O con GLM bajado a mano:
     g++ -std=c++20 -Wall -Wextra -I<carpeta_de_glm> leccion_01... -o leccion01
   ============================================================================= */

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>  // glm::translate (verificado 1.0.1)
#include <glm/gtc/type_ptr.hpp>          // glm::value_ptr (verificado 1.0.1)
#include <cstdio>
#include <iomanip>
#include <iostream>

static void imprimirMatriz(const char* nombre, glm::mat4 const& m) {
    std::cout << nombre << " (column-major: cada línea es UNA columna):\n";
    for (int c = 0; c < 4; ++c) {
        std::cout << "  col" << c << " = [";
        for (int r = 0; r < 4; ++r) {
            if (r > 0) std::cout << ", ";
            std::cout << std::setw(7) << m[c][r];
        }
        std::cout << "]\n";
    }
}

int main() {
    std::cout << "== TAMAÑOS Y DIRECCIONES DE MEMORIA ==\n";
    glm::mat4 m{};
    std::cout << "sizeof(glm::mat4) = " << sizeof(m)
              << " (= 16 floats = 64 bytes)\n";
    // Ojo: en GLM 1.0.1, glm::mat4{} viene en CERO (la demo lo imprime),
    // la IDENTIDAD es glm::mat4(1.0f). No te confíes del default.
    glm::mat4 id = glm::mat4(1.0f);
    imprimirMatriz("glm::mat4(1.0f) (identidad)", id);

    std::cout << "\n== COLUMNAS CONTIGUAS ==\n";
    glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3{2.0f, 3.0f, 4.0f});
    // glm::translate vive en <glm/ext/matrix_transform.hpp> (verificado en el
    // repo 1.0.1): las funciones de armar modelos NO vienen en glm/glm.hpp.
    imprimirMatriz("glm::translate(id, (2,3,4))", t);
    std::cout << "(solo la COLUMNA 3 cambió: la traslación vive ahí)\n";

    std::cout << "\n== EL ÍNDICE REAL: m[c*4+r] ==\n";
    std::cout << "m[3] (columna 3, traslación): ("
              << t[3].x << ", " << t[3].y << ", " << t[3].z << ")\n";
    std::cout << "t[0][1] (col 0, fila 1) = " << t[0][1] << "  (debe ser 0)\n";

    float const* planos = glm::value_ptr(t);   // verificado: glm/gtc/type_ptr.hpp
    std::cout << "\nvalue_ptr t en memoria (primera columna primero): ";
    for (int i = 0; i < 4; ++i) std::cout << planos[i] << " ";
    std::cout << "\n";

    std::cout << "\n== HANDEDNESS: cross product ==\n";
    glm::vec3 x{1, 0, 0}, y{0, 1, 0};
    glm::vec3 crossXY = glm::cross(x, y);      // glm::cross: <glm/geometric.hpp>
    std::cout << "x cross y = (" << crossXY.x << ", " << crossXY.y << ", "
              << crossXY.z << ")  — +z: ESTA es la convención RIGHT-HANDED\n";
    std::cout << "En LH, x cross y daría -z; GLM siempre computa RH:\n"
                 "el handedness es de las FUNCIONES de cámara/proyección "
                 "(_RH/_LH), no de los operadores.\n";

    std::cout << "=== FIN LECCIÓN 01 ===\n";
    return 0;
}
