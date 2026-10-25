/* =============================================================================
   MÓDULO 21 - LECCIÓN 14: Optimización de shaders: UBO bloques, división de
   branch y texture arrays
   =============================================================================
   TEMA 8 de TEMAS.md: "Optimización: uniforms vs UBO (std140), branch
   divergence, shader variants, texture arrays".

   --- UNIFORMS SUELTOS vs UBO (bloques de uniform) ---------------------------
   Cada glUniformX es un call de CPU por uniform por programa. Si 3 programas
   (blending de materiales, iluminación, post) comparten la MISMA lista de
   luces, la carga 3 veces por draw es desperdicio. La cura, core en 3.3:
   un UBO: un buffer se sube UNA VEZ por frame y se LIGA a un binding point
       glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo);
       glUniformBlockBinding(prog, bloqueIdx, 0);
   y el shader lo declara como BLOQUE con layout std140:
       layout (std140) uniform BloqueLuces { Luces[] ... };
   El std140 es UNA convención de bytes rígida (el layout que ya diseñaste
   en la 04): cada vec3 ocupa 16 bytes con relleno, la base del array sale
   a 16 bytes y el stride de cada elemento es 16 redondeado arriba: hay
   static_asserts ahí mismo que NO DEJAN caer el sistema.
   Repaso del plan (verificable en la impresión de la demo):
        bloque { int cuenta; pad; Luces[8] }  cuenta@0, Luces base@16,
        cada Luz 32 bytes => bloque = 16 + 8*32 = 272 bytes.
   Ojo: la primera regla de optimización REAL es NO subir lo que no cambió:
   los uniforms que cambian por material van en otro UBO del material, y no
   se re-suben por draw si no cambió el material.

   --- BRANCH DIVERGENCE (el "if" dentro del shader) --------------------------
   El GPU corre en batches (warps) de ~32 lanes que ejecutan LA MISMA
   instrucción por ciclo: un if discos los lanes en dos rutas y las dos
   corren AMBAS para el batch (predicación): se pierde tiempo EN VEZ de
   ahorrar. Reglas de cancha:
     - if por FRAGMENTO con datos borrosos (bordes, oscuridad): afina mal:
       las rutas van a divergir casi siempre: mejor máscaras.
     - if por UNIFORM (uModo): todos los lanes van al mismo lado: gratis:
       el compilador de la marca igual puede re-ordenar; no confies.
     - La cura tradicional: unificar (mix/step/max arman máscaras de
       datos) o VARIANTES (compilar el shader con una #define del modo:
       la diferencia se decide EN TIEMPO de compilación, no por cuadro).
   En sombras de la 09-10 ya vimos la variante honesta: el loop 3x3 de PCF
   SE SALTEA solo si la sombra está plenamente en sombra (early exit) etc.

   --- TEXTURE ARRAYS (la lista de texturas en UNA HANDLE) --------------------
   Con muchos materiales, "cambiar la textura por draw" genera cambios de
   binding y cada cambalache rompe el caché del GPU. La cura, core desde
   3.0: la GL_TEXTURE_2D_ARRAY (0x8C1A, glTexImage3D con depth = cantidad
   de capas, lo mismo que la LUT de la 12 pero de planos 2D): el shader
     texture(uAtlas, vec3(uv, layer));   // sampler2DArray, capa por dato
   UN handle, muchas capas, y el muestreo de mip funciona igual que en
   la 2D suelta (glTexSubImage3D por capa: nada de tiles empaquetados y
   sin las costuras del clamp de bordes entre tiles).

   CÓMO CORRERLA A MANO:
     g++ -std=c++20 -Wall -Wextra leccion_14_optimizacion_ubo_arrays.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec14 && ./lec14

   DEMO: la escena de siempre iluminada con el mismo UBO de luces que la
   L04 (misma tabla de 272 bytes), textura 2D array de 4 capas
   procedurales; tecla [T] cicla capa. Impresión del plan de bytes del
   bloque (cross-check del ejercicio 03).
   ============================================================================
 */

#include "include/l21.hpp"

#include <iostream>

namespace {

/* --- El plan std140 CORREGIDO de la L04 (32 bytes por luz exacto) ------- */
struct LuzStd140 { // alignas(16): 32 bytes
  float color[3];
  float intensidad; // @12
  float pos[3];     // @16
  float alcance;    // @28
};
static_assert(sizeof(LuzStd140) == 32, "std140: luz de 32 bytes");
struct BloqueLuces {
  int cuenta;         // @0
  char relleno[12];   // @4..15 (el layout redondea a 16)
  LuzStd140 luces[8]; // @16, stride 32
};
static_assert(sizeof(BloqueLuces) == 16 + 8 * 32, "std140: bloque 272");

const char *VS_PROG = R"GLSL(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;
uniform mat4 uModel;
uniform mat4 uMatVistaProj;
out vec3 vPos;
out vec3 vNormal;
out vec2 vUV;
void main() {
    vec4 mundo = uModel * vec4(aPos, 1.0);
    vPos = mundo.xyz;
    vNormal = mat3(uModel) * aNormal;
    vUV = aUV;
    gl_Position = uMatVistaProj * mundo;
}
)GLSL";

/* Un UBO ligado: BloqueLuces. texture array: sampler2DArray. */
const char *FS_PROG = R"GLSL(
#version 330 core
in vec3 vPos;
in vec3 vNormal;
in vec2 vUV;
out vec4 FragColor;

struct Luz {
    vec3 color;
    float intensidad;
    vec3 pos;
    float alcance;
};

uniform sampler2DArray uAtlas;    // capa por uniform: UN handle
uniform int   uCapa;

layout (std140) uniform BloqueLuces {
    int  cuenta;
    Luz luces[8];
};

void main() {
    vec3 N = normalize(vNormal);
    vec3 albedo;

    // ATLAS: una sola textura mantiene las 4 máscaras; la capa la elige
    // un UNIFORM (dato de CPU, NO de fragmento: divergence de nada).
    vec2 uv = vUV * 2.0f;
    vec3 masc = texture(uAtlas, vec3(uv, float(uCapa))).rgb;

    // masc.r separa los dos albedos del material; masc.g oscurece:
    // la CAPA del atlas es un UNIFORM (dato de CPU, no de fragmento:
    // no hay branch divergence en juego).
    albedo = mix(vec3(0.55f, 0.15f, 0.12f), vec3(0.85f, 0.72f, 0.45f),
                 masc.r);
    albedo *= (1.0f - 0.25f * masc.g);

    vec3 suma = vec3(0.06f);
    for (int i = 0; i < 8; ++i) {
        if (i >= cuenta) break;           // uniform-ish: no diverge feo
        vec3 d = luces[i].pos - vPos;
        float dist = length(d);
        vec3 L = d / dist;
        float at = luces[i].intensidad
                 / (1.0f + 0.09f * dist + 0.032f * dist * dist);
        float cono = (dist < luces[i].alcance) ? 1.0f : 0.0f;
        suma += luces[i].color * max(dot(N, L), 0.0f) * at * cono;
    }
    FragColor = vec4(albedo * suma, 1.0f);
}
)GLSL";

/* --- 4 capas procedurales para la 2D array (mismo patrón de la L05) --- */
std::vector<unsigned char> capaProcedural(int cual, int w, int h) {
  std::vector<unsigned char> d((size_t)w * h * 4);
  for (int y = 0; y < h; ++y)
    for (int x = 0; x < w; ++x) {
      float u = (float)x / (w - 1), v = (float)y / (h - 1);
      float p = 0.0f;
      switch (cual) {
      case 0:
        p = ((int)(u * 8) + (int)(v * 8)) % 2 == 0 ? 1.0f : 0.0f;
        break; // ajedrez
      case 1:
        p = (int)(v * 14) % 2 == 0 ? 1.0f : 0.0f;
        break;
        // mejor: rayitas horizontales
      case 2:
        p = (u * 12.0f < 1.0f + v * 10.0f) ? 1.0f : 0.0f;
        break;
      case 3:
        p = (u > 0.45f && u < 0.55f && v > 0.45f) ? 1.0f : 0.0f;
        break;
      }
      float un = 1.0f - p;
      auto i8 = [](float f) {
        return (unsigned char)(std::min(1.0f, std::max(0.0f, f)) * 255);
      };
      size_t idx = ((size_t)y * w + x) * 4;
      d[idx + 0] = i8(p);
      d[idx + 1] = i8(un);
      d[idx + 2] = (unsigned char)(x % 4 == 0 ? 40 : 0);
      d[idx + 3] = 255;
    }
  return d;
}

void crearAtlas(GLuint &tex, int w, int h, int capas) {
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D_ARRAY, tex); // 0x8C1A
  // 1) ALLOCAR el cubo entero (una sola llamada, sin data)
  glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, w, h, capas, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, nullptr);
  // 2) UNA subida por capa (cada capa = un "material" del atlas)
  for (int c = 0; c < capas; ++c) { // layer index va en LA Z
    auto d = capaProcedural(c, w, h);
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, c, w, h, 1, GL_RGBA,
                    GL_UNSIGNED_BYTE, d.data());
  }
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

/* Plan de bytes impreso (cross-check del ejercicio 03). */
void reporteBloque() {
  std::cout << "== Plan std140 del bloque de luces (como la L04) ==\n";
  std::cout << "   cuenta           int   @0    (+12 de relleno)\n";
  std::cout << "   luces[0..7]      Luz   @16   (stride 32 bytes)\n";
  std::cout << "     de cada luz: color@+0, intensidad@+12, pos@+16, "
               "alcance@+28\n";
  std::cout << "   total = 16 + 8*32 = " << sizeof(BloqueLuces) << " bytes\n\n";
}

} // namespace

int main() {
  reporteBloque();
  std::cout << "Atlas 2D array: 4 capas, UN handle. Tecla T cambia capa, "
            << "U muestra el UBO ligado por binding 0.\n";

  glu::Contexto c = glu::abrir("M21 L14: UBO std140 + texture array", 800, 550);
  if (c.ventana == nullptr)
    return 1;
  glEnable(GL_DEPTH_TEST);

  GLuint prog = glu::enlazar(VS_PROG, FS_PROG);
  if (prog == 0) {
    glu::cerrar(c);
    return 1;
  }

  glu::Uniform uModel, uMat;
  uModel.buscar(prog, "uModel");
  uMat.buscar(prog, "uMatVistaProj");

  /* --- UBO: llenar el bloque EN CPU y subirla UNA VEZ ----------------- */
  BloqueLuces bloque{};
  bloque.cuenta = 3;
  {
    LuzStd140 &l0 = bloque.luces[0];
    l0.color[0] = 1.0f;
    l0.color[1] = 0.95f;
    l0.color[2] = 0.80f;
    l0.intensidad = 2.2f;
    l0.pos[0] = 3.0f;
    l0.pos[1] = 3.0f;
    l0.pos[2] = 2.0f;
    l0.alcance = 14.0f;

    LuzStd140 &l1 = bloque.luces[1];
    // luz de relleno FRÍO cerca del piso
    l1.color[0] = 0.30f;
    l1.color[1] = 0.45f;
    l1.color[2] = 1.00f;
    l1.intensidad = 1.4f;
    l1.pos[0] = -2.5f;
    l1.pos[1] = 1.2f;
    l1.pos[2] = -1.0f;
    l1.alcance = 10.0f;

    LuzStd140 &l2 = bloque.luces[2];
    l2.color[0] = 0.90f;
    l2.color[1] = 0.30f;
    l2.color[2] = 0.25f;
    l2.intensidad = 1.1f;
    l2.pos[0] = 0.0f;
    l2.pos[1] = 2.2f;
    l2.pos[2] = -3.0f;
    l2.alcance = 8.0f;
  }
  GLuint ubo = 0;
  glGenBuffers(1, &ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, ubo);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(BloqueLuces), &bloque,
               GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo); // binding = 0
  GLuint idxBloque = glGetUniformBlockIndex(prog, "BloqueLuces");
  glUniformBlockBinding(prog, idxBloque, 0); // programa a binding 0
  std::cout << "UBO sube ONCE: bloque " << sizeof(BloqueLuces)
            << " bytes en binding 0 (glUniformBlockBinding)\n\n";

  f21::Vec3 ojo{0.0f, 2.0f, 6.5f};
  f21::Mat4 vistaCam = f21::lookAt(ojo, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
  f21::Mat4 projCam =
      f21::perspectiva(55.0f, (float)c.ancho / (float)c.alto, 0.1f, 100.0f);

  f21::MallaGPU piso = f21::subirMalla(f21::cubo(0.5f));
  f21::MallaGPU esfera = f21::subirMalla(f21::esfera(0.50f, 32, 16));
  f21::Mat4 modeloPiso = f21::trs({0.0f, -0.5f, 0.0f}, 1.0f, 0.0f);
  f21::Mat4 modeloEsfera = f21::trs({0.0f, 0.55f, 0.0f}, 1.0f, 0.0f);

  /* --- Atlas 2D array: 4 capas en UNA textura ------------------------ */
  GLuint atlas = 0;
  crearAtlas(atlas, 64, 64, 4);

  const bool *teclas = SDL_GetKeyboardState(nullptr);
  bool teclaTPrev = false;
  int capa = 0;
  while (glu::eventosOk()) {
    bool teclaT = teclas[SDL_SCANCODE_T];
    if (teclaT && !teclaTPrev) {
      capa = (capa + 1) % 4;
      std::cout << "capa del atlas: " << capa << "\n";
    }
    teclaTPrev = teclaT;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(prog);
    f21::subirMat(uMat.lugar, projCam * vistaCam);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, atlas);
    glUniform1i(glGetUniformLocation(prog, "uAtlas"), 0);
    glUniform1i(glGetUniformLocation(prog, "uCapa"), capa);
    f21::subirMat(uModel.lugar, modeloPiso);
    f21::dibujar(piso);
    f21::subirMat(uModel.lugar, modeloEsfera);
    f21::dibujar(esfera);
    SDL_GL_SwapWindow(c.ventana);
  }

  glDeleteTextures(1, &atlas);
  glDeleteBuffers(1, &ubo);
  glDeleteProgram(prog);
  f21::borrarMalla(piso);
  f21::borrarMalla(esfera);
  glu::cerrar(c);
  std::cout << "Listo: UBO sube 1 vez, 3 luces leen del bloque std140, "
               "4 materiales viven en 1 texture array (1 handle).\n";
  return 0;
}
