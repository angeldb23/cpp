/* =============================================================================
   MÓDULO 21 - include/l21.hpp — helpers comunes de las lecciones de shaders
   =============================================================================
   Reutiliza lo que ya existe en el curso (requisito de TEMAS.md):
     - 20_opengl_core/include/gl_util.hpp : contexto 3.3 core, eventos,
       compilación/enlazado de shaders con logs (namespace glu).
     - 19_matematica_motores/include/mate3d.hpp : Vec2/3/4 y Mat4
       column-major (namespace mate).

   Acá SOLO hay infraestructura: las fórmulas de iluminación, TBN, sombras,
   bloom, etc. están en el .cpp de cada lección. Todo apunta a OpenGL 3.3 CORE.

   VERIFICADO contra headers locales el 22/10/2026:
   - GL/glcorearb.h de glvnd: GL_TEXTURE_COMPARE_MODE (0x884C),
     GL_TEXTURE_COMPARE_FUNC (0x884D), GL_COMPARE_REF_TO_TEXTURE (0x884E),
     GL_TEXTURE_2D_ARRAY (0x8C1A), GL_TEXTURE_3D (0x806F), GL_RGBA32F
     (0x8814), GL_RGBA16F (0x881A), GL_R8 (0x8229), GL_CLAMP_TO_BORDER
     (0x812D), GL_DEPTH_COMPONENT32F (0x8CAC), glDrawElementsBaseVertex
     (núcleo desde GL 3.2), glTexImage3D, glDrawBuffers, glGetStringi
     + GL_NUM_EXTENSIONS (0x821D).
   - SDL3 3.4.18 (SDL_video.h): mismas APIs GL que el módulo 20.

   Compilar una lección (desde la carpeta del módulo):
     g++ -std=c++20 -Wall -Wextra leccion_XX_foo.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec && ./lec
   (mismo patrón del módulo 20; ninguna lección compila hago falta una ventana).
   ============================================================================ */
#pragma once

#include "../../20_opengl_core/include/gl_util.hpp"
#include "../../19_matematica_motores/include/mate3d.hpp"

#include <cmath>
#include <vector>
#include <string>

namespace f21 {  // "fx 21": infra del módulo de iluminación

using mate::Vec2;
using mate::Vec3;
using mate::Vec4;
using mate::Mat3;
using mate::Mat4;
using mate::transformarPunto;

/* ---------------------------------------------------------------------------
   MATE DE CÁMARA (fórmulas de las lecciones 11-13 del módulo 19; acá quedan
   empaquetadas porque TODAS las lecciones de luces/sombras las necesitan).
 ------------------------------------------------------------------------- */

inline float aRad(float grados) { return mate::aRadianes(grados); }

/* Producto vectorial (el mate3d lo trae como ejercicio, no como lib). */
inline Vec3 cross(Vec3 a, Vec3 b) {
    return {a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x};
}

inline float dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

inline Vec3 normalizar(Vec3 v) {
    float l = std::sqrt(dot(v, v));
    return (l > 0.0f) ? v / l : Vec3{0.0f, 0.0f, 0.0f};
}

inline float largo(Vec3 v) { return std::sqrt(dot(v, v)); }

/* Proyección PERSPECTIVA estándar (column-major, módulo 19 L11):
   vertical + near/far en profundidad POSITIVA (la cámara mira -Z).
   Verificada contra GLM 1.0.1 en la sesión del módulo 19 (ndc z 0.8181
   para near 0.1 / far 100 / z 100). */
inline Mat4 perspectiva(float fovYGrados, float aspect, float near, float far) {
    Mat4 m{};
    float f = 1.0f / std::tan(aRad(fovYGrados) * 0.5f);
    m.set(0, 0, f / aspect);
    m.set(1, 1, f);
    m.set(2, 2, (far + near) / (near - far));
    m.set(2, 3, 2.0f * far * near / (near - far));
    m.set(3, 2, -1.0f);
    m.set(3, 3, 0.0f);
    return m;
}

/* Proyección ORTOGRÁFICA simétrica en ancho/alto (para sombras direccional
   y spot frustum del techo). cerca->lejos profundidad positiva. */
inline Mat4 orto(float halfW, float halfH, float near, float far) {
    Mat4 m = Mat4::identidad();
    m.set(0, 0, 1.0f / halfW);
    m.set(1, 1, 1.0f / halfH);
    m.set(2, 2, -2.0f / (far - near));
    m.set(2, 3, -(far + near) / (far - near));
    return m;
}

/* Cámara "mira a": filas de la rotación = s (derecha), u (arriba), -f
   (hacia delante de la cámara OPUESTO: GL mira -Z) + traslación negada. */
inline Mat4 lookAt(Vec3 ojo, Vec3 centro, Vec3 arriba) {
    Vec3 f = normalizar(centro - ojo);
    Vec3 s = normalizar(cross(f, arriba));
    Vec3 u = cross(s, f);
    Mat4 m = Mat4::identidad();
    // fila 0 = s, fila 1 = u, fila 2 = -f  -> (column y fila invertidas)
    m.set(0, 0, s.x); m.set(1, 0, s.y); m.set(2, 0, s.z);
    m.set(0, 1, u.x); m.set(1, 1, u.y); m.set(2, 1, u.z);
    m.set(0, 2, -f.x); m.set(1, 2, -f.y); m.set(2, 2, -f.z);
    m.set(3, 0, -dot(s, ojo));
    m.set(3, 1, -dot(u, ojo));
    m.set(3, 2, dot(f, ojo));
    return m;
}

/* TRS básica (ya vista en 19 L06): paraarmar el plan del object. */
inline Mat4 trs(Vec3 pos, float esc, float yawDeg) {
    Mat4 t = Mat4::identidad();
    t.ponerColumna3(3, pos, 1.0f);
    float c = std::cos(aRad(yawDeg)), s = std::sin(aRad(yawDeg));
    Mat4 r = Mat4::identidad();
    // rotación alrededor de Y (módulo 19 L06): x' = c*x - s*z, z' = s*x + c*z
    r.set(0, 0, c);  r.set(0, 2, -s);
    r.set(1, 0, 0);  r.set(1, 1, 1);  r.set(1, 2, 0);
    r.set(2, 0, s);  r.set(2, 2, c);
    Mat4 sm = Mat4::identidad();
    sm.set(0, 0, esc); sm.set(1, 1, esc); sm.set(2, 2, esc);
    return t * r * sm;
}

/* Sube una Mat4 column-major a un uniform mat4 (transpose FALSE: ya almacena
   columnas, igual convención que GLM). */
inline void subirMat(GLint lugar, Mat4 const& m) {
    glUniformMatrix4fv(lugar, 1, GL_FALSE, m.m);
}

inline void subirVec(GLint lugar, Vec3 v) {
    glUniform3f(lugar, v.x, v.y, v.z);
}

/* ---------------------------------------------------------------------------
   GEOMETRÍA: un cubo y una esfera con POS+NORMAL+UV interleaveados (la base
   de cualquier demo de iluminación: 8 líneas para armar todo el VAO).
   Formato de vértice (más lección 08 del módulo 20):
       float[3] pos | float[3] normal | float[2] uv  -> stride 32 bytes
 ------------------------------------------------------------------------- */
struct Malla {
    std::vector<float> vertices;   // pos3 normal3 uv2 float
    std::vector<unsigned> indices;
};

/* Devuelve geometría con cara al frente, 6 caras con su propia NORMAL. */
inline Malla cubo(float mitad = 0.5f) {
    Malla g;
    // centro del cubo en el origen, cada cara con derecha-arriba-izquierda-abajo
    Vec3 dirs[6] = {
        {+mitad, 0, 0}, {-mitad, 0, 0}, {0, +mitad, 0},
        {0, -mitad, 0}, {0, 0, +mitad}, {0, 0, -mitad}};
    for (int cara = 0; cara < 6; ++cara) {
        Vec3 n = dirs[cara];
        // vectores tangente y bitangente genuinos para separar UV
        Vec3 t = (std::fabs(n.x) > 0.5f) ? Vec3{0, 0, -1.0f} : Vec3{1, 0, 0};
        Vec3 b = cross(n, t);
        for (int i = 0; i < 4; ++i) {
            float du = (i == 1 || i == 2) ? 2.0f * mitad : -2.0f * mitad;
            float dv = (i == 2 || i == 3) ? 2.0f * mitad : -2.0f * mitad;
            Vec3 p = {n.x + t.x * du * 0.5f + b.x * dv * 0.5f,
                      n.y + t.y * du * 0.5f + b.y * dv * 0.5f,
                      n.z + t.z * du * 0.5f + b.z * dv * 0.5f};
            g.vertices.push_back(p.x); g.vertices.push_back(p.y);
            g.vertices.push_back(p.z);
            g.vertices.push_back(n.x); g.vertices.push_back(n.y);
            g.vertices.push_back(n.z);
            float uvx = (i == 0 || i == 3) ? 0.0f : 1.0f;
            float uvy = (i == 0 || i == 1) ? 0.0f : 1.0f;
            g.vertices.push_back(uvx); g.vertices.push_back(uvy);
        }
        unsigned base = static_cast<unsigned>(cara) * 4u;
        g.indices.insert(g.indices.end(),
                         {base + 0u, base + 1u, base + 2u,
                          base + 0u, base + 2u, base + 3u});
    }
    return g;
}

/* Esfera UV gruesa (lat/lon): normal = posición normalizada. Suficiente para
   ver specular y normal map; sin índices deduplicados (lección 16 arreglará
   eso discutir la creación de la VAO desde una malla OBJ). */
inline Malla esfera(float radio, int segU = 24, int segV = 12) {
    Malla g;
    for (int v = 0; v <= segV; ++v) {
        float phi = -3.14159265f * 0.5f + 3.14159265f * (float)v / (float)segV;
        for (int u = 0; u <= segU; ++u) {
            float th = 3.14159265f * 2.0f * (float)u / (float)segU;
            Vec3 n = {std::cos(phi) * std::cos(th), std::sin(phi),
                      std::cos(phi) * std::sin(th)};
            g.vertices.push_back(n.x * radio);
            g.vertices.push_back(n.y * radio);
            g.vertices.push_back(n.z * radio);
            g.vertices.push_back(n.x); g.vertices.push_back(n.y);
            g.vertices.push_back(n.z);
            g.vertices.push_back((float)u / (float)segU);
            g.vertices.push_back((float)v / (float)segV);
        }
    }
    auto at = [&](int v, int uu) {
        return static_cast<unsigned>(v * (segU + 1) + uu);
    };
    for (int v = 0; v < segV; ++v) {
        for (int u = 0; u < segU; ++u) {
            g.indices.push_back(at(v, u));
            g.indices.push_back(at(v + 1, u));
            g.indices.push_back(at(v + 1, u + 1));
            g.indices.push_back(at(v, u));
            g.indices.push_back(at(v + 1, u + 1));
            g.indices.push_back(at(v, u + 1));
        }
    }
    return g;
}

/* VAO+VBO+EBO listo para dibujar (los detalles en 20 L06-L08).
   Devuelve {vao, cantidadDeIndices}. */
struct MallaGPU { GLuint vao = 0, vbo = 0, ebo = 0; int indices = 0; };

inline MallaGPU subirMalla(Malla const& g) {
    GLuint vao = 0, vbo = 0, ebo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(g.vertices.size() * sizeof(float)),
                 g.vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 (GLsizeiptr)(g.indices.size() * sizeof(unsigned)),
                 g.indices.data(), GL_STATIC_DRAW);
    // pos 3 (offset 0) + normal 3 (offset 12) + uv 2 (offset 24) -> stride 32
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (const void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (const void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (const void*)(6 * sizeof(float)));
    glBindVertexArray(0);
    return {vao, vbo, ebo, (int)g.indices.size()};
}

inline void dibujar(MallaGPU v) {
    glBindVertexArray(v.vao);
    glDrawElements(GL_TRIANGLES, v.indices, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

/* Liberación espejo del alta (regla RAII discutida en el módulo 07). */
inline void borrarMalla(MallaGPU v) {
    glDeleteBuffers(1, &v.ebo);
    glDeleteBuffers(1, &v.vbo);
    glDeleteVertexArrays(1, &v.vao);
}

}  // namespace f21
