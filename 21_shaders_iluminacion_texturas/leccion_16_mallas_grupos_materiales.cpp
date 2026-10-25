/* =============================================================================
   MÓDULO 21 - LECCIÓN 16: Mallas OBJ con materiales: grupos por usemtl,
   pool indexado compartido y DRAW PLAN con glDrawElementsBaseVertex
   =============================================================================
   TEMA 10 de TEMAS.md (cierre del módulo): "carga de mallas (tinyobjloader) y
   materiales por grupo (sub-mesh)". El parser mini de la L18/L09 del 20 quedó
   pequeño; el OBJ REAL de un personaje trae DOCENAS de usemtl cambiando a
   mitad del mesh (face aid, nariz, ojo): acá se juega de verdad.

   --- EL ARCHIVO y sus grupos -----------------------------------------------
   Un OBJ tiene shapes (g/o) y faces; usemtl marca "hasta acá, este
   material". La industria del cargador del motor:
       1. descriptor de datos: tinyobj (header-only de la 20, FetchContent
          tag v2.0.0rc13, TINYOBJLOADER_IMPLEMENTATION una vez).
       2. DEDUP: los índices del OBJ SON tripletas (v,vt,vn); formo la key
          "v/vt/vn" y la guardo en UN POOL ÚNICO de vértices (pos+norm+uv).
          Las caras que COMPARTEN vértice comparten slot: VBO chico.
       3. GRUPOS: recorro las caras EN ORDEN; cuando cambia usemtl cierro el
          grupo anterior y abro otro (float material_id, índices del grupo).
       4. DRAW PLAN: la lista de draws {"material -> (offset, count,
          baseVertex)"} que va directo al renderer SIN arte de por medio.
   LA CLAVE del baseVertex: los índices del grupo los dejo LOCALS (base=0
   dentro del grupo) y viajan al draw con glDrawElementsBaseVertex (núcleo
   desde GL 3.2) que los remapea: pool_base + local. El WIN: puedo reordenar
   / reusar/ cachear índices de grupo sin recalcularlos.

   --- MATERIALES POR GRUPO ---------------------------------------------------
   tinyobj también parsea el MTL (diffuse[3], diffuse_texname...). En esta
   lección (GLSL 3.3, el final del módulo) los materiales viajan como
   UNIFORM (uColorBase + flags): las texturas por material ya las vienen
   EN LA 05, y el upgrade de rol por textura es el ejercicio 10.

   --- ESTA DEMO --------------------------------------------------------------
   Genera UN OBJ "in situ" (string: VALiente UN modelo "avioncito de papel"
   con 2 materiales y usemtl cambiando a mitad del shape), lo parsea con
   tinyobj ObjReader::ParseFromString, arma pool+grupos+plan, imprime el
   plan por consola y lo dibuja girando. Teclas 1/2: highlight de grupo.
   ============================================================================
 */

#include "include/l21.hpp"

// Dependencia del módulo (FetchContent tag v2.0.0rc13: VERIFICADO):
// header-only: UNA TINYOBJLOADER_IMPLEMENTATION por binario.
#define TINYOBJLOADER_IMPLEMENTATION
#include "include/tiny_obj_loader.h"

#include <iostream>
#include <map>
#include <string>
#include <tuple>

namespace {

/* Malla cargada: pool + grupos por material. */
struct Cargada {
  std::vector<float> pool;       // pos3 norm3 uv2 (stride 8 floats)
  std::vector<unsigned> indices; // TODOS los grupos, en orden de draw
  struct Grupo {
    int material;        // índice en materials[]
    size_t ordinalIndex; // offset en "indices" (bytes*4 después)
    int cantIdx;         // índices del grupo
    int baseVertex;      // pool_base + local = veredicto real
    std::string nombre;  // el material MTL de origen
  };
  std::vector<Grupo> grupos;
};

/* OBJ de una mini (string: 2 materiales, usemtl a mitad del shape).
   Los UV fake van de lado: la idea es el pipeline completo. */
const char *OBJ_FUENTE = R"OBJ(
# avioncito de papel: fuselaje con ALA ROJA y CABINA AZUL
v -1.0 0.0 0.6
v  0.0 0.0 0.0
v  1.0 0.0 0.6
v -0.6 0.0 -0.6
v  0.6 0.0 -0.6
v  0.0 0.4 0.2
v  0.0 0.9 0.9
vn 0 1 0
vn 0 -1 0
vt 0 0
vt 1 0
vt 1 1
vt 0 1
usemtl pintura_cuerpo
f 1/1/1 2/2/1 6/3/1
f 2/2/1 3/3/1 6/3/1
f 1/1/1 2/2/1 4/4/1
f 2/2/1 3/3/1 5/4/1
usemtl vidrio_cabina
f 1/1/1 4/4/1 6/2/1
f 3/3/1 5/4/1 6/3/1
)OBJ";

const char *MTL_FUENTE = R"MTL(
newmtl pintura_cuerpo
Kd 0.85 0.25 0.20
newmtl vidrio_cabina
Kd 0.35 0.55 0.95
)MTL";

/* El cargador: parse -> dedup -> grupos -> plan. */
Cargada cargarOBJ(std::string const &objText, std::string const &mtlText) {
  tinyobj::ObjReaderConfig cfg;
  cfg.triangulate = true;
  cfg.vertex_color = false;
  tinyobj::ObjReader reader;
  bool ok = reader.ParseFromString(objText, mtlText, cfg);
  if (!ok || !reader.Valid()) {
    std::cout << "parse OBJ falla: " << reader.Error() << "\n";
    return {};
  }
  if (!reader.Warning().empty())
    std::cout << "parse OBJ aviso: " << reader.Warning() << "\n";

  auto const &attrib = reader.GetAttrib();
  auto const &shapes = reader.GetShapes();
  auto const &materials = reader.GetMaterials();

  Cargada c;
  std::map<std::tuple<int, int, int>, unsigned> unicos;
  auto poolIndex = [&](int v, int vt, int vn) -> unsigned {
    auto it = unicos.find({v, vt, vn});
    if (it != unicos.end())
      return it->second;
    unsigned ret = (unsigned)(c.pool.size() / 8);
    c.pool.push_back(attrib.vertices[3 * (size_t)v + 0]);
    c.pool.push_back(attrib.vertices[3 * (size_t)v + 1]);
    c.pool.push_back(attrib.vertices[3 * (size_t)v + 2]);
    if (vn < 0) {
      c.pool.push_back(0);
      c.pool.push_back(1);
      c.pool.push_back(0);
    } else {
      c.pool.push_back(attrib.normals[3 * (size_t)vn + 0]);
      c.pool.push_back(attrib.normals[3 * (size_t)vn + 1]);
      c.pool.push_back(attrib.normals[3 * (size_t)vn + 2]);
    }
    if (vt < 0) {
      c.pool.push_back(0);
      c.pool.push_back(0);
    } else {
      c.pool.push_back(attrib.texcoords[2 * (size_t)vt + 0]);
      c.pool.push_back(attrib.texcoords[2 * (size_t)vt + 1]);
    }
    unicos[{v, vt, vn}] = ret;
    return ret;
  };

  for (auto const &shape : shapes) {
    auto const &idx = shape.mesh.indices;
    auto const &caraAmt = shape.mesh.num_face_vertices;
    auto const &matIds = shape.mesh.material_ids;
    size_t cara = 0, i = 0;
    while (cara < caraAmt.size()) {
      int matActual = (matIds.empty() ? -1 : matIds[cara]);
      Cargada::Grupo g;
      g.material = matActual;
      g.nombre = (matActual >= 0 && matActual < (int)materials.size())
                     ? materials[matActual].name
                     : "(sin material)";
      g.baseVertex = -1;
      g.ordinalIndex = c.indices.size();
      while (cara < caraAmt.size() &&
             (matIds.empty() || matIds[cara] == matActual)) {
        int amt = caraAmt[cara];
        for (int k = 0; k < amt; ++k) {
          int v = idx[i + k].vertex_index;
          int vt = idx[i + k].texcoord_index;
          int vn = idx[i + k].normal_index;
          unsigned slot = poolIndex(v, vt, vn);
          if (g.baseVertex < 0)
            g.baseVertex = (int)slot;
          int local = (int)slot - g.baseVertex;
          c.indices.push_back((unsigned)local);
        }
        i += (size_t)amt;
        ++cara;
      }
      g.cantIdx = (int)(c.indices.size() - g.ordinalIndex);
      c.grupos.push_back(g);
    }
  }
  return c;
}

const char *VS_MALLA = R"GLSL(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;
uniform mat4 uModel;
uniform mat4 uMatVistaProj;
out vec3 vNormal;
out vec2 vUV;
void main() {
    vNormal = mat3(uModel) * aNormal;
    vUV = aUV;
    gl_Position = uMatVistaProj * (uModel * vec4(aPos, 1.0));
}
)GLSL";

const char *FS_MALLA = R"GLSL(
#version 330 core
in vec3 vNormal;
in vec2 vUV;
out vec4 FragColor;
uniform vec3 uColorBase;
uniform float uRealce;
void main() {
    vec3 N = normalize(vNormal);
    float luz = 0.45f + 0.55f * max(N.y, 0.0f);
    vec3 c = uColorBase * luz;
    c += uRealce * vec3(0.25f);
    FragColor = vec4(c, 1.0);
}
)GLSL";

} // namespace

int main() {
  glu::Contexto ctx = glu::abrir("M21 L16: OBJ + material por grupo", 800, 550);
  if (ctx.ventana == nullptr)
    return 1;
  glEnable(GL_DEPTH_TEST);

  GLuint prog = glu::enlazar(VS_MALLA, FS_MALLA);
  if (prog == 0) {
    glu::cerrar(ctx);
    return 1;
  }

  glu::Uniform uModel, uMat;
  uModel.buscar(prog, "uModel");
  uMat.buscar(prog, "uMatVistaProj");

  /* 1..2: parse + pool + grupos + PLAN. */
  Cargada c = cargarOBJ(OBJ_FUENTE, MTL_FUENTE);
  std::cout << "== POOL: " << c.pool.size() / 8 << " vértices únicos "
            << "(pos+norm+uv, " << c.pool.size() * sizeof(float) << " bytes)\n";
  std::cout << "== DRAW PLAN (draw por grupo, indices + baseVertex):"
            << "\n";
  for (size_t gi = 0; gi < c.grupos.size(); ++gi) {
    auto const &g = c.grupos[gi];
    std::cout << "   draw[" << gi << "] material " << g.material << " ('"
              << g.nombre << "'): offset " << g.ordinalIndex << " idx cant "
              << g.cantIdx << " baseVertex " << g.baseVertex << "\n";
  }

  /* UNA textura (UNA VBO / UNA EBO / UNA VAO) para todo el modelo. */
  f21::MallaGPU gpu;
  {
    GLuint vao = 0, vbo = 0, ebo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(c.pool.size() * sizeof(float)),
                 c.pool.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 (GLsizeiptr)(c.indices.size() * sizeof(unsigned)),
                 c.indices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (const void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (const void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (const void *)(6 * sizeof(float)));
    glBindVertexArray(0);
    gpu = {vao, vbo, ebo, (int)c.indices.size()};
  }

  /* Colores del MTL (diffuse Kd) por grupo, espejados del material. */
  tinyobj::ObjReader rd;
  rd.ParseFromString(OBJ_FUENTE, MTL_FUENTE);
  auto const &mats = rd.GetMaterials();
  std::vector<f21::Vec3> colores(c.grupos.size(), {1.0f, 0.0f, 1.0f});
  for (size_t gi = 0; gi < c.grupos.size(); ++gi) {
    auto const &g = c.grupos[gi];
    if (g.material >= 0 && g.material < (int)mats.size()) {
      auto const &m = mats[g.material];
      colores[gi] = {m.diffuse[0], m.diffuse[1], m.diffuse[2]};
    }
    std::cout << "   material '" << g.nombre << "': Kd " << colores[gi].x << " "
              << colores[gi].y << " " << colores[gi].z << "\n";
  }

  f21::Vec3 ojo{0.0f, 2.0f, 4.5f};
  f21::Mat4 vista = f21::lookAt(ojo, {0.0f, 0.2f, 0.0f}, {0.0f, 1.0f, 0.0f});
  f21::Mat4 proj =
      f21::perspectiva(55.0f, (float)ctx.ancho / (float)ctx.alto, 0.1f, 100.0f);

  const bool *teclas = SDL_GetKeyboardState(nullptr);
  int realzado = -1;
  float yaw = 0.0f;
  while (glu::eventosOk()) {
    float dt = 1.0f / 60.0f;
    yaw += 40.0f * dt;
    if (teclas[SDL_SCANCODE_1])
      realzado = 0;
    else if (teclas[SDL_SCANCODE_2])
      realzado = 1;
    else
      realzado = -1;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(prog);
    f21::subirMat(uMat.lugar, proj * vista);
    f21::subirMat(uModel.lugar, f21::trs({0.0f, 0.2f, 0.0f}, 1.0f, yaw));

    glBindVertexArray(gpu.vao);
    for (size_t gi = 0; gi < c.grupos.size(); ++gi) {
      auto const &g = c.grupos[gi];
      f21::Vec3 kd = colores[gi];
      glUniform3f(glGetUniformLocation(prog, "uColorBase"), kd.x, kd.y, kd.z);
      glUniform1f(glGetUniformLocation(prog, "uRealce"),
                  realzado == (int)gi ? 1.0f : 0.0f);
      glDrawElementsBaseVertex(
          GL_TRIANGLES, g.cantIdx, GL_UNSIGNED_INT,
          (const void *)(g.ordinalIndex * sizeof(unsigned)), g.baseVertex);
    }
    glBindVertexArray(0);
    SDL_GL_SwapWindow(ctx.ventana);
  }

  f21::borrarMalla(gpu);
  glDeleteProgram(prog);
  glu::cerrar(ctx);
  std::cout << "Listo: parse -> pool dedup -> grupos usemtl -> draw plan "
               "con glDrawElementsBaseVertex.\n";
  return 0;
}
