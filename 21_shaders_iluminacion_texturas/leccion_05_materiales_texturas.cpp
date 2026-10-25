/* =============================================================================
   MÓDULO 21 - LECCIÓN 05: Materiales: propiedades por objeto y texturas por rol
   =============================================================================
   TEMA 3 de TEMAS.md (primera de 2 lecciones): "Materiales: propiedades por
   objeto, texturas difusa/especular/normal/emissive y PBR básico
   (albedo/metalness/roughness)". El PBR completo es la LECCIÓN 06.

   CÓMO CORRERLA A MANO:
     g++ -std=c++20 -Wall -Wextra leccion_05_materiales_texturas.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec05 && ./lec05

   --- EL PROBLEMA: el "uniform de luz" no alcanza ----------------------------
   La lección 01 daba el spec con UNA intensidad global (0.35) y UN exponente
   (brillo). Con eso, TODOS los objetos del nivel se ven igual: el piso brilla
   como el auto. Un MATERIAL es la bolsa de propiedades de UN objeto:
       - albedo   : el color base por textura (lo que ya sabés: 20 L09-L11)
       - especular: CUÁNTO y CÓMO brilla, por textura: las manchas del metal
                    plateado siguen la silueta; madera y plástico apenas brillan.
       - normal   : (lección 07-08) "arrugas" de la superficie sin geometría
       - emissive : luz QUE EL OBJETO EMITE (pantallas, lava, ojos en dark)
   y el cálculo por rol es SIEMPRE el de la 01, con los datos cambiantes.

   --- TEXTURA ESPECULAR: la salida más barata con más Revoluciones -----------
   Una especular map en ESCALA DE GRIS multiplicada sobre el spec de la 01:
       spec = pow(max(dot(R, V), 0.0), brillo) * texture(uSpec, uv).r
   La MISMA mancha de la lección 01 queda multiplicada por el gris del
   material canal por canal.
   Suele ser un canal gris (el GLSL lee el .r y listo): el abuelo directo
   del PBR roughness de la lección 06.

   --- EMISSIVE: la luz que porta el shader del objeto ------------------------
   NO es una luz del sistema (no ilumina a los VECINOS): solo suma color
   al fragmento propio: el "ojo brilla" pero el piso de al lado sigue oscuro.
       color += texture(uEmissive, uv).rgb * uIntensidadEmissive;
   Para que una lava ILLUMINE el piso de al lado falta un pase de luz extra
   (lo tiende un motor con 'luces por área'; fuera del alcance del curso).

   --- PROYECCIÓN DE UV vs LOCAL: la identidad GLSL del módulo 20 ------------
   Los UV vienen interpolados de la geometría. La misma textura de albedo
   puede REPETIRSE (wrap GL_REPEAT + mipmaps, visto en 20 L10): el material
   guarda su escala de tiles y el shader multiplica: uv * uRepetir.

   --- ESTA DEMO --------------------------------------------------------------
   3 esferas en fila: MATE (spec 0), YESO emisivo (spec bajo, franjas emisoras)
   y METAL (spec alto). Las máscaras son procedurales (ajedrez, rayitas de
   cepillado, franjas de calor). Flechas mueven la luz: mirá cómo SOLO cambia
   por datos de material.
   ============================================================================
 */

#include "include/l21.hpp"

#include <iostream>

namespace {

const char* VS_FUENTE = R"GLSL(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;
out vec3  vPos;
out vec3  vNormal;
out vec2  vUV;
void main() {
    vec4 mundo = uModel * vec4(aPos, 1.0);
    vPos = mundo.xyz;
    vNormal = mat3(uModel) * aNormal;
    vUV = aUV;
    gl_Position = uProj * uView * mundo;
}
)GLSL";

/* FRAGMENTO MATERIAL: los 4 roles de textura. uTieneSpec/uTieneEmissive son
   "brazos del material": en un motor real un material se arma con ID de
   textura por rol y flags (la clase CPU del ejercicio 05 modela exacto eso). */
const char* FS_MATERIAL = R"GLSL(
#version 330 core
in vec3 vPos;
in vec3 vNormal;
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uAlbedo;
uniform sampler2D uSpecular;    // gris: canal r multiplicado al spec
uniform sampler2D uEmissive;

uniform vec3  uPosCamara;
uniform vec3  uPosLuz;
uniform vec3  uColorLuz;
uniform float uBrillo;
uniform float uSpecFactor;
uniform vec2  uRepetir;         // escala UV: mismo material, many tiles
uniform float uIntensidadEmissive;
uniform float uTieneEmissive;

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(uPosLuz - vPos);
    vec3 V = normalize(uPosCamara - vPos);

    // el mismo material con REPETICIÓN uv ( wrapping del 20 L10)
    vec2 uv = vUV * uRepetir;

    // DIFUSA
    vec3 albedo = texture(uAlbedo, uv).rgb;

    // DIFUSO + AMBIENT...
    vec3 color = albedo * (0.10 + uColorLuz * max(dot(N, L), 0.0));

    // SPECULAR multiplicado por la especular map (canal GRIS r)
    vec3 reflejo = reflect(-L, N);
    float specTex = texture(uSpecular, uv).r;
    float specGeo = pow(max(dot(reflejo, V), 0.0), uBrillo);
    color += uColorLuz * uSpecFactor * specTex * specGeo;

    // EMISSIVE: se SUMA después del shading (la emisión no siwait "luz")
    if (uTieneEmissive > 0.5) {
        color += texture(uEmissive, uv).rgb * uIntensidadEmissive;
    }
    FragColor = vec4(color, 1.0);
}
)GLSL";

}  // namespace

/* Textura PROCEDURAL por rol: 0 ajedrez (albedo), 1 cepillado (spec),
   2 franjas de calor (emissive). */
static std::vector<unsigned char> texturaGris(int w, int h, int modo) {
    std::vector<unsigned char> pix(w * h * 4, 255);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            float v = 0.0f;
            switch (modo) {
            case 0: v = ((x / 8 + y / 8) % 2) ? 0.85f : 0.35f; break; // ajedrez
            case 1:  // "cepillado": flejes horizontales, algo de ruido
                v = 0.55f + 0.35f * std::sin(y * 0.9f);
                break;
            case 2:  // "emisiva": franjas de calor (la rasca del material)
                v = 0.5f + 0.5f * std::sin(x * 0.35f + y * 0.1f);
                break;
            }
            std::size_t i = (std::size_t(y) * w + std::size_t(x)) * 4;
            unsigned char g = (unsigned char)(v * 255.0f + 0.5f);
            pix[i] = pix[i + 1] = pix[i + 2] = g;
            pix[i + 3] = 255;
        }
    return pix;
}

static GLuint subirGris(const std::vector<unsigned char>& pix, int w, int h) {
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pix.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return tex;
}

int main() {
    glu::Contexto c = glu::abrir("M21 L05: materiales por rol", 900, 400);
    if (c.ventana == nullptr) return 1;
    glEnable(GL_DEPTH_TEST);

    GLuint prog = glu::enlazar(VS_FUENTE, FS_MATERIAL);
    if (prog == 0) { glu::cerrar(c); return 1; }

    glu::Uniform uModel, uView, uProj, uPosCamara, uPosLuz, uColorLuz,
                 uBrillo, uSpecFactor, uRepetir, uIntE, uTieneE;
    uModel.buscar(prog, "uModel");    uView.buscar(prog, "uView");
    uProj.buscar(prog, "uProj");      uPosCamara.buscar(prog, "uPosCamara");
    uPosLuz.buscar(prog, "uPosLuz");  uColorLuz.buscar(prog, "uColorLuz");
    uBrillo.buscar(prog, "uBrillo");  uSpecFactor.buscar(prog, "uSpecFactor");
    uRepetir.buscar(prog, "uRepetir");
    uIntE.buscar(prog, "uIntensidadEmissive");
    uTieneE.buscar(prog, "uTieneEmissive");
    GLint samplers[3] = { glGetUniformLocation(prog, "uAlbedo"),
                          glGetUniformLocation(prog, "uSpecular"),
                          glGetUniformLocation(prog, "uEmissive") };
    glUniform1i(samplers[0], 0);
    glUniform1i(samplers[1], 1);
    glUniform1i(samplers[2], 2);

    // 3 texturas procedurales: albedo ajedrez, spec líneas, emiss franjas
    GLuint texAlbedo = subirGris(texturaGris(64, 64, 0), 64, 64);
    GLuint texSpec   = subirGris(texturaGris(64, 64, 1), 64, 64);
    GLuint texEmiss  = subirGris(texturaGris(64, 64, 2), 64, 64);

    // TRES materiales (la clase CPU del ej 05 usa ESTA bolsa):
    struct Material {
        float specFactor, brillo;
        float intensidadE; bool tieneEmiss;
        const char* nombre;
    } mats[] = {
        {0.0f,  32.0f, 0.0f, false, "MATE   (sin spec)"},
        {0.15f, 128.0f, 0.6f, true,  "YESO   (spec bajo + emissive)"},
        {0.9f,  64.0f, 0.0f, false, "METAL  (spec alto)"},
    };

    f21::MallaGPU esfera = f21::subirMalla(f21::esfera(0.5f));
    f21::Vec3 ojo{0.0f, 1.2f, 4.2f};
    f21::Mat4 vista = f21::lookAt(ojo, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
    f21::Mat4 proj = f21::perspectiva(55.0f,
        (float)c.ancho / (float)c.alto, 0.1f, 100.0f);
    f21::Vec3 luzPos{2.0f, 2.0f, 2.5f};
    const bool* teclas = SDL_GetKeyboardState(nullptr);
    static const int unidades[3] = {GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2};

    while (glu::eventosOk()) {
        float dt = 1.0f / 60.0f;
        if (teclas[SDL_SCANCODE_LEFT])  luzPos.x -= 1.5f * dt;
        if (teclas[SDL_SCANCODE_RIGHT]) luzPos.x += 1.5f * dt;

        glClearColor(0.05f, 0.05f, 0.07f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(prog);
        f21::subirMat(uView.lugar, vista);
        f21::subirMat(uProj.lugar, proj);
        f21::subirVec(uPosCamara.lugar, ojo);
        f21::subirVec(uColorLuz.lugar, {1.0f, 0.97f, 0.9f});

        for (int i = 0; i < 3; ++i) {
            float x = -2.2f + 2.2f * (float)i;
            f21::subirMat(uModel.lugar, f21::trs({x, 0.0f, 0.0f}, 1.0f, 0.0f));
            f21::subirVec(uPosLuz.lugar, luzPos);
            glUniform1f(uBrillo.lugar, mats[i].brillo);
            glUniform1f(uSpecFactor.lugar, mats[i].specFactor);
            glUniform2f(uRepetir.lugar, 2.0f, 2.0f);
            glUniform1f(uIntE.lugar, mats[i].intensidadE);
            glUniform1f(uTieneE.lugar, mats[i].tieneEmiss ? 1.0f : 0.0f);
            // las TRES texturas activas en sus 3 unidades (u0 albedo, etc.)
            for (int u = 0; u < 3; ++u) {
                glActiveTexture(unidades[u]);
                GLuint t = (u == 0) ? texAlbedo : (u == 1) ? texSpec : texEmiss;
                glBindTexture(GL_TEXTURE_2D, t);
            }
            f21::dibujar(esfera);
        }
        SDL_GL_SwapWindow(c.ventana);
    }

    glDeleteProgram(prog);
    glDeleteTextures(1, &texAlbedo);
    glDeleteTextures(1, &texSpec);
    glDeleteTextures(1, &texEmiss);
    f21::borrarMalla(esfera);
    glu::cerrar(c);
    std::cout << "Listo: 3 materiales distintos (MATE, emisivo, METAL) con la "
                 "misma luz: la diferencia vive en los datos del material.\n";
    return 0;
}
