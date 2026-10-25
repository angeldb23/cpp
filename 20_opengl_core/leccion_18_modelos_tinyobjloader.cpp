/* =============================================================================
   MÓDULO 20 - LECCIÓN 18: Modelos con tinyobjloader — OBJ/MTL, mallas
   indexadas y materiales básicos (assimp queda como mención)
   =============================================================================
   TEMA 12 de TEMAS.md: tinyobjloader (header-only, OBJ/MTL), mallas
   indexadas, materiales básicos; assimp solo como mención.

   CÓMO CORRERLA A MANO (desde la carpeta del módulo):
     g++ -std=c++20 -Wall -Wextra leccion_18_modelos_tinyobjloader.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec18 && ./lec18
     (ó con el CMakeLists.txt del módulo, que trae tinyobjloader con
      FetchContent: tag v2.0.0rc13, VERIFICADO contra el repo:
      refs/tags/v2.0.0rc13 existe y es la última release de la serie 2.x
      (la "2.0" definitiva nunca salió de rc; así lo consumen todos)).

   --- POR QUÉ UN PARSEADOR EXTERNO -------------------------------------------
   El formato OBJ es un TEXTO con líneas "v"/"vn"/"vt"/"f"/"usemtl"/...
   Parasearlo a mano ES posible (y lo haces patron en el ejercicio 09 con
   la versión mini): pero el OBJ REAL llega con:
     - caras con N vértices (fan: no solo triángulos);
     - índices "v/vt/vn" por componente: 1-based, pueden faltar (v//vn);
     - materiales MTL con nombres de archivo de textura....
   tinyobjloader maneja TODO ESO y devuelve:
     attrib_t  : pools de datos planos (attrib.vertices: floats xyz;
                 attrib.normals; attrib.texcoords).
     shape_t   : shapes[] con mesh.indexes + un material_id cada cara.
                 (!NOTA de API: el struct se llama tinyobj::shape_t y el
                  campo mesh.index_t: UNO index_t con 3 ids (vertex_index,
                  normal_index, texcoord_index; -1 si la cara no tiene).)
     material_t: diffuse_texname, ambient, diffuse[3]...

   --- MALLAS INDEXADAS (la forma real que usan los FPS) ---------------------
   La CONVERSIÓN que hace el cargador del motor (el que da el también):
     OBJ no garantiza SHARED vertices (una cara con vn/vt propios genera
     3 "vértices lógicos" distintos aunque la pos sea LA MISMA)... 
     La clave del índice es la TRIPLETA (v, vt, vn). La receta:
       1. Por cada index_t de cada shape: formo key = (v,vt,vn).
       2. Si la key NO estaba en mi map uniqueVertex["v/vt/vn"]:
          - la agrego al array de vértices únicos (pos[3]+uv[2]+norm[3])
          - registro el índice nuevo.
       3. El índice del triángulo va al EBO.
   RESULTADO: mucho menos VBO (el share vertex se comparte) y el draw es
   glDrawElements como la lección 06. ES EXACTO el ejercicio 09 (parser
   mini OBJ + dedup), solo que ACÁ lo hace la librería y el ejercicio lo
   armás vos (con std::map<> de strings... para las claves "v/vt/vn").

   --- MATERIALES (básicos) ----------------------------------------------------
   El MTL da color difuso y nombre de textura por material; el mesh trae
   material_id por cara. El render del 21 los convierte en uniforms
   (uColorBase) y en texturas (por la lección 09); acá queda el read
   básico: si un día necesitás FBX/GLTF/skinned meshes, ese es el mundo
   de assimp (la lib grande de formatos): pesa demasiado para compilar en
   esta máquina (TEMAS.md y DECISIONES: mención, no compilación).
   --------------------------------------------------------------------------- */

#include "include/gl_util.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <iostream>
#include <map>
#include <string>
#include <vector>

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL_Init fallo: " << SDL_GetError() << "\n";
        return 1;
    }

    glu::Contexto c = glu::abrir("Modulo 20 - L18 modelos OBJ", 800, 600);
    if (c.contexto == nullptr) { SDL_Quit(); return 1; }

    /* Un OBJ mínimo con UN cubo y texto bbox para probar normales y
       materiales (el cubo real se escribiría del lado del arte: aquí
       usamos UN triángulo + una CUBO con caras indexadas)... también la
       lección recomienda mirar el arte real (módulo 25: maneras).        */
    const std::string rutaObj = "modelo_18_trivial.obj";
    {
        std::ofstream out(rutaObj);
        out << "# leccion 18\n"
            << "v 0 0 0\n v 1 0 0\n v 0.5 1 0\n"
            << "f 1 2 3\n";     // UNA cara, 3 vértices, sin uv/vn (tinyobj
        out.close();            // devuelve indices a -1 para la que falta)
    }
    std::cout << "escrito " << rutaObj << " (3 verts, 1 cara)\n";

    tinyobj::attrib_t                attrib;
    std::vector<tinyobj::shape_t>    shapes;
    std::vector<tinyobj::material_t> materiales;
    std::string warn, err;

    bool ok = tinyobj::LoadObj(&attrib, &shapes, &materiales, &warn, &err,
                               rutaObj.c_str());
    if (!ok) {
        std::cout << "LoadObj fallo: '" << warn << "' / '" << err << "'\n";
        glu::cerrar(c);
        SDL_Quit();
        return 1;
    }
    std::cout << "LoadObj ok: shapes=" << shapes.size()
              << " materiales=" << materiales.size()
              << " verts_entradas=" << attrib.vertices.size() / 3 << "\n";

    /* ---- CONVERSIÓN A LA MALLA INDEXADA del curso ------------------------
       (struct Vertex layout de la L08: pos(3) + normal(3); el ej 09 agrega
        el uv y el dedup POR CLAVE STRING; acá es por (v,vt,vn) enteros). */
    struct Malla {
        std::vector<float>  verts;   // pos.xyz + normal.xyz (6 floats/vert)
        std::vector<unsigned> idx;   // EBO
    };
    Malla m;
    std::map<std::string, unsigned> unicos;
    for (const tinyobj::shape_t& shape : shapes) {
        for (const tinyobj::index_t& ix : shape.mesh.indices) {
            std::string key =
                std::to_string(ix.vertex_index) + "/" +
                std::to_string(ix.texcoord_index) + "/" +
                std::to_string(ix.normal_index);
            auto it = unicos.find(key);
            if (it == unicos.end()) {
                unsigned nuevo = (unsigned)(m.verts.size() / 6);
                unicos[key] = nuevo;
                m.verts.push_back(attrib.vertices[3 * (size_t)ix.vertex_index + 0]);
                m.verts.push_back(attrib.vertices[3 * (size_t)ix.vertex_index + 1]);
                m.verts.push_back(attrib.vertices[3 * (size_t)ix.vertex_index + 2]);
                if (ix.normal_index >= 0) {
                    m.verts.push_back(attrib.normals[3 * (size_t)ix.normal_index + 0]);
                    m.verts.push_back(attrib.normals[3 * (size_t)ix.normal_index + 1]);
                    m.verts.push_back(attrib.normals[3 * (size_t)ix.normal_index + 2]);
                } else {          // el OBJ no trae normales: la normal CARA?
                    m.verts.push_back(0); m.verts.push_back(0); m.verts.push_back(1);
                }
                m.idx.push_back(nuevo);
            } else {
                m.idx.push_back(it->second);
            }
        }
    }
    std::cout << "malla indexada: verts=" << m.verts.size() / 6
              << " indices=" << m.idx.size() << "\n";
    std::cout << "(con UNA cara: 3 verts / 3 idx; con el cubo del arte real: "
              << "8 verts / 36 idx en vez de 72 verts: ESO es el share.)\n";

    /* ---- SUBIR LA MALLA AL GPU (lecciones 06 + 08 juntas) ---------------- */
    GLuint vao = 0, vbo = 0, ebo = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, m.verts.size() * sizeof(float),
                 m.verts.data(), GL_STATIC_DRAW);   // malla: cambia NUNCA
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m.idx.size() * sizeof(unsigned),
                 m.idx.data(), GL_STATIC_DRAW);
    // pos = location 0 (3 floats, offset 0, stride 24), normal = loc 1 (byte 12)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (const void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    std::cout << "VBO/EBO/VAO listos: la lección 21 (iluminación) consume esta\n"
              << "malla con normales; acá solo quedó cargada y verificada.\n";

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
    glu::cerrar(c);
    SDL_Quit();
    return 0;
}
