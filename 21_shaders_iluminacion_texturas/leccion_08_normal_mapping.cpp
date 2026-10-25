/* =============================================================================
   MÓDULO 21 - LECCIÓN 08: Normal mapping: TBN en vertex shader y unpack
   =============================================================================
   TEMA 4 de TEMAS.md (segunda de 2 lecciones): "Normal mapping: espacio
   tangente, TBN matrix en vertex shader y unpack de normal map".
   La matemática de la TBN (cómo SE CONSTRUYE) es la LECCIÓN 07.

   CÓMO CORRERLA A MANO:
     g++ -std=c++20 -Wall -Wextra leccion_08_normal_mapping.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec08 && ./lec08

   --- EL STACK COMPLETO (recap de la 07) ------------------------------------
   La GEOMETRÍA aporta: aPos, aNormal (la "lisa"), aUV y aTangent (vec3: la
   T construida por la 07; la W del handedness puede ir en un vec4). El
   SHADER arma la base (T, B, N) y construye mat3(TBN) para:
     vec3 N = TBN * normalTexel;   // tangente -> MUNDO (por fragmento)
   (la alternativa "mundo -> tangente" convierte cada luz al espacio de
   textura; más cara y retinta si la rotación cambia por fragmento.)
   Se elige "a mundo": N sale en el MISMO espacio que las luces ya viven,
   sin convertir cada posición de luz por fragmento.

   --- UNPACK del normal map (la receta EXACTA) ------------------------------
   La textura guarda la normal COM PRIMIDA en RGB [0..1]:
       packed = (normal + 1.0) * 0.5   (siendo la "flat" (0,0,1) -> (0.5, 0.5, 1))
   el UNPACK es el INVERSO:
       vec3 n = texture(uNormalMap, uv).rgb * 2.0 - 1.0;   // ¡2x - 1!
   Con la normal "por defecto" (0,0,1) la textura x a "flat" gris clarito
   (0.5, 0.5, 1.0): la normal izquierda en el color de textura típico.
   El "normal map azul" clásico viene de EXO (z alto). "Normal map verde"
   (swapa canales A y G, ejertos DXT5nm en GLES): si te topas una, se
   swapea o cambia el unpack: LEÉL DEL NOTEBOOK (cada fuente es un mundo).

   --- LA gramática del vertex: la mat3 se arma EN EL FRAGMENT --------------
   El vertex NO manda una mat3 interpolada: la tornillada de un vec3 por
   vértice con exacto blending no es garantizada CORRECTA; lo que se hace:
   mandar 3 varyings (T, B, N) y en el FRAGMENT:
       mat3 TBN = mat3(T, B, N);
   Los tres son direcciones rotadas con el mat3 del modelo; en el fragment
   se RECTIFICA con GS (de la punta de la lección 07) y se arman la vec

   --- ESTA DEMO ------------------------------------------------------------
   UN cubo rotando con la normal map procedural ("piel" de rayas calculadas
   con senos). aTangent sale de la MISMA fórmula de la L07 (por cara). Las
   teclas N/M cambian entre "usar normal map" y "usar normal lisa": ver
   ahí como la silueta NO cambia (el cubo sigue un cubo) pero el shading
   SI rescate: eso es el normal mapping.

   --- APIs ------------------------------------------------------------------
   Nada NUEVO de API: el shader agrega MAT3 y texture por SEGUNDA unidad.
   DE MENCION: la suite "tangent-space derivativa" calcula T y B en el
   FRAGMENT con las derivadas dFdx/dFdy (es la que implementan los motores
   de alto volumen para no traer tangentes por vértice; fuera de curso).
   ============================================================================
 */

#include "include/l21.hpp"

#include <iostream>

namespace {

/* VÉRTICE con el NUEVO atributo tangente (location 3). La T viene de la
   misma fórmula CPU-side de la lección 07 (esta demo la calcula por cara).
   En el VERTEX se rota TODO con el mat3 del modelo (lamisma rotación que la
   normal) y se envían los tres vectores como varyings. */
const char* VS_FUENTE = R"GLSL(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;
layout (location = 3) in vec3 aTangent;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

out vec3 vPos;
out vec3 vNormal;        // normal GEOMETRÍA en mundo (para TBN)
out vec2 vUV;
out vec3 vTangent;       // T en mundo

void main() {
    vec4 mundo = uModel * vec4(aPos, 1.0);
    vPos = mundo.xyz;
    vNormal   = mat3(uModel) * aNormal;
    vTangent  = mat3(uModel) * aTangent;    // mismo mat3: T es dirección
    vUV = aUV;
    gl_Position = uProj * uView * mundo;
}
)GLSL";

/* FRAGMENTO: armar TBN, UNPACK de la normal map, y LUCIR con la normal
   mentirosa. La narrativa "usar normal GEOM" vs "usar normal TEXTURA" es el
   uniform uUsarNormalMap: el interruptor pedagógico de la demo. */
const char* FS_NORMAL_MAP = R"GLSL(
#version 330 core
in vec3 vPos;
in vec3 vNormal;
in vec2 vUV;
in vec3 vTangent;
out vec4 FragColor;

uniform sampler2D uAlbedo;
uniform sampler2D uNormalMap;

uniform vec3  uPosCamara;
uniform vec3  uPosLuz;
uniform vec3  uColorLuz;
uniform float uBrillo;
uniform float uUsarNormalMap;    // 1: unpack y usar; 0: normal geométrica

void main() {
    vec3 Ngeo = normalize(vNormal);

    // BITANGENTE RECONSTRUIDA en el fragment: cross(N, T) es ORTOGONAL a
    // ambos (la W del handedness quedara por ahora +1: UVs al derecho)
    vec3 T = normalize(vTangent - Ngeo * dot(Ngeo, vTangent)); // GS in-shader
    vec3 B = cross(Ngeo, T);
    mat3 TBN = mat3(T, B, Ngeo);          // columnas = ejes tangente

    vec3 N = Ngeo;
    if (uUsarNormalMap > 0.5) {
        // UNPACK: la normal map guarda [0,1]; la normal vive en [-1,1]
        vec3 nTan = texture(uNormalMap, vUV).rgb * 2.0 - 1.0;
        N = normalize(TBN * nTan);       // tangente -> MUNDO para la luz
    }

    vec3 L = normalize(uPosLuz - vPos);
    vec3 V = normalize(uPosCamara - vPos);

    vec3 albedo = texture(uAlbedo, vUV).rgb;
    vec3 color = albedo * (0.12 + max(dot(N, L), 0.0));
    vec3 reflejo = reflect(-L, N);
    color += vec3(0.35)
           * pow(max(dot(reflejo, V), 0.0), uBrillo);
    FragColor = vec4(color, 1.0);
}
)GLSL";

/* Genera una normal map PROCEDURAL: bump suave determinista con senos.
   La "flat" es (0.5, 0.5, 1.0): el conocido "normal map azul". */
static std::vector<unsigned char> normalMapProcedural(int w, int h) {
    std::vector<unsigned char> pix(w * h * 4);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            // fuente de altura: 2 senos cruzados (bump de "hoja de pieles")
            float fx = std::sin(x * 0.35f) * 0.5f
                     + std::sin(y * 0.22f + x * 0.08f) * 0.5f;
            float dy = (std::sin((x + 1) * 0.35f) - std::sin((x - 1) * 0.35f))
                     * 0.25f;
            float dx = (std::sin((y + 1) * 0.22f + (x + 1) * 0.08f)
                      - std::sin((y - 1) * 0.22f + (x - 1) * 0.08f)) * 0.25f;
            // la normal es (-dx, -dy, 1) normalizado y PACKED: *0.5+0.5
            float nz = 1.0f;
            float nx = -dx, ny = -dy;
            float l = std::sqrt(nx*nx + ny*ny + nz*nz);
            nx /= l; ny /= l; nz /= l;
            std::size_t i = (std::size_t(y) * w + std::size_t(x)) * 4;
            pix[i]     = (unsigned char)((nx * 0.5f + 0.5f) * 255.0f + 0.5f);
            pix[i + 1] = (unsigned char)((ny * 0.5f + 0.5f) * 255.0f + 0.5f);
            pix[i + 2] = (unsigned char)((nz * 0.5f + 0.5f) * 255.0f + 0.5f);
            pix[i + 3] = 255;
            (void)fx;
        }
    return pix;
}

/* Malla a medida con el atributo EXTRA tangente (la geometría del l21 no
   trae T: se calcula por cara con la fórmula de la lección 07). */
struct DataTang { std::vector<float> verts; std::vector<unsigned> idx; };

static DataTang cuboConTangentes(float mitad) {
    f21::Malla base = f21::cubo(mitad);
    DataTang d;
    d.idx = base.indices;
    // por cara (4 vértices consecutivos): la T ~ dirección creciente de UV.x
    for (std::size_t v = 0; v < base.vertices.size() / 8; v += 4) {
        f21::Vec3 N{base.vertices[v + 3], base.vertices[v + 4],
                    base.vertices[v + 5]};
        f21::Vec3 U{base.vertices[v + 0], base.vertices[v + 1],
                    base.vertices[v + 2]};
        f21::Vec3 V2{base.vertices[v + 8], base.vertices[v + 9],
                     base.vertices[v + 10]};
        // en la geometría del l21 los UV avanzan con los vértices consecutivos
        f21::Vec3 e = V2 - U;
        f21::Vec3 T = e - N * f21::dot(N, e);
        T = f21::normalizar(T);
        for (int k = 0; k < 4; ++k) {
            d.verts.push_back(U.x); d.verts.push_back(U.y);
            d.verts.push_back(U.z);
            d.verts.push_back(N.x); d.verts.push_back(N.y);
            d.verts.push_back(N.z);
            // reproducción del uv del l21 (uv en slots 6..7 del vértice k)
            float uvi = base.vertices[v + (std::size_t)k * 8 + 6];
            float uvj = base.vertices[v + (std::size_t)k * 8 + 7];
            d.verts.push_back(uvi); d.verts.push_back(uvj);
            d.verts.push_back(T.x); d.verts.push_back(T.y); d.verts.push_back(T.z);
        }
    }
    return d;
}

}  // namespace

int main() {
    glu::Contexto c = glu::abrir("M21 L08: normal mapping", 800, 500);
    if (c.ventana == nullptr) return 1;
    glEnable(GL_DEPTH_TEST);

    GLuint prog = glu::enlazar(VS_FUENTE, FS_NORMAL_MAP);
    if (prog == 0) { glu::cerrar(c); return 1; }

    glu::Uniform uModel, uView, uProj, uPosCamara, uPosLuz, uColorLuz,
                 uBrillo, uUsaNorm;
    uModel.buscar(prog, "uModel");    uView.buscar(prog, "uView");
    uProj.buscar(prog, "uProj");      uPosCamara.buscar(prog, "uPosCamara");
    uPosLuz.buscar(prog, "uPosLuz");  uColorLuz.buscar(prog, "uColorLuz");
    uBrillo.buscar(prog, "uBrillo");  uUsaNorm.buscar(prog, "uUsarNormalMap");
    GLint samAlbedo = glGetUniformLocation(prog, "uAlbedo");
    GLint samNormal = glGetUniformLocation(prog, "uNormalMap");
    glUniform1i(samAlbedo, 0);
    glUniform1i(samNormal, 1);

    // normal map procedural (la "flat" se ve azulaucarada por el z alto)
    auto nm = normalMapProcedural(128, 128);
    GLuint texNormal = 0;
    glGenTextures(1, &texNormal);
    glBindTexture(GL_TEXTURE_2D, texNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 128, 128, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, nm.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // albedo ajedrez (reuso el mini-textura builder de la 02)
    std::vector<unsigned char> ajedrez(64 * 64 * 4);
    for (int y = 0; y < 64; ++y)
        for (int x = 0; x < 64; ++x) {
            bool par = ((x / 8) + (y / 8)) % 2 == 0;
            float v = par ? 0.7f : 0.3f;
            std::size_t i = (std::size_t(y) * 64 + std::size_t(x)) * 4;
            ajedrez[i] = ajedrez[i + 1] = ajedrez[i + 2] =
                (unsigned char)(v * 255.0f + 0.5f);
            ajedrez[i + 3] = 255;
        }
    GLuint texAlbedo = 0;
    glGenTextures(1, &texAlbedo);
    glBindTexture(GL_TEXTURE_2D, texAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, 64, 64, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, ajedrez.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    DataTang cubo = cuboConTangentes(1.0f);
    GLuint vao = 0, vbo = 0, ebo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(cubo.verts.size() * sizeof(float)),
                 cubo.verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 (GLsizeiptr)(cubo.idx.size() * sizeof(unsigned)),
                 cubo.idx.data(), GL_STATIC_DRAW);
    // stride 44 bytes: pos3(0) normal3(12) uv2(24) tangent3(32)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float),
                          (const void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float),
                          (const void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float),
                          (const void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float),
                          (const void*)(8 * sizeof(float)));
    glBindVertexArray(0);

    f21::Vec3 ojo{0.0f, 1.5f, 4.0f};
    f21::Mat4 vista = f21::lookAt(ojo, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
    f21::Mat4 proj = f21::perspectiva(55.0f,
        (float)c.ancho / (float)c.alto, 0.1f, 100.0f);
    float angulo = 0.0f;
    bool usarNormalMap = true;

    const bool* teclas = SDL_GetKeyboardState(nullptr);
    float brillo = 24.0f;
    while (glu::eventosOk()) {
        float dt = 1.0f / 60.0f;
        angulo += 25.0f * dt;
        if (teclas[SDL_SCANCODE_N]) usarNormalMap = true;
        if (teclas[SDL_SCANCODE_M]) usarNormalMap = false;  // M: normal lisa
        if (teclas[SDL_SCANCODE_UP])   brillo *= 1.02f;
        if (teclas[SDL_SCANCODE_DOWN]) brillo /= 1.02f;

        glClearColor(0.05f, 0.05f, 0.07f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(prog);
        f21::subirMat(uView.lugar, vista);
        f21::subirMat(uProj.lugar, proj);
        f21::subirMat(uModel.lugar, f21::trs({0.0f, 0.0f, 0.0f}, 1.0f, angulo));
        f21::subirVec(uPosCamara.lugar, ojo);
        f21::subirVec(uPosLuz.lugar, {2.0f, 2.0f, 2.0f});
        f21::subirVec(uColorLuz.lugar, {1.0f, 0.96f, 0.9f});
        glUniform1f(uBrillo.lugar, brillo);
        glUniform1f(uUsaNorm.lugar, usarNormalMap ? 1.0f : 0.0f);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texAlbedo);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texNormal);
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, (GLsizei)cubo.idx.size(),
                       GL_UNSIGNED_INT, nullptr);
        SDL_GL_SwapWindow(c.ventana);
    }

    glDeleteProgram(prog);
    glDeleteTextures(1, &texNormal);
    glDeleteTextures(1, &texAlbedo);
    glDeleteBuffers(1, &ebo); glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glu::cerrar(c);
    std::cout << "Listo: N = normal map, M = geometría lisa; flechas cambian "
                 "el brillo, el cubo rota solo.\n";
    return 0;
}
