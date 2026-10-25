/* =============================================================================
   MÓDULO 21 - LECCIÓN 06: PBR básico: albedo / metalness / roughness
   =============================================================================
   TEMA 3 de TEMAS.md (segunda de 2 lecciones): "Materiales ... y PBR básico
   (albedo/metalness/roughness)".

   CÓMO CORRERLA A MANO:
     g++ -std=c++20 -Wall -Wextra leccion_06_pbr_basico.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec06 && ./lec06

   --- EL SALTO DE CONCEPTO -------------------------------------------------
   Phong/Blinn son MODELOS DE APARIENCIA: un exponente "brillo" y una
   intensidad de spec se ajustan a ojo. PBR (Physically Based Rendering)
   reemplaza esos ajustes por propiedades del MATERIAL REAL:
       albedo    : color de la difusa (en metal, también el color del
                   reflejo especular);
       metalness : 0 = dieléctrico (plásticos, piedra), 1 = metal. Es un
                   interruptor con valores intermedios para mezclas (óxido,
                   pintura). En el workflow glTF es un mapa propio;
       roughness : rugosidad microscópica 0..1, reemplaza el exponente
                   "brillo" (roughness baja ~= exponente alto del Phong);
       AO        : oclusión ambient (mapa R). En este módulo, con una luz,
                   es un multiplicador del ambient y nada más.
   La demo esfera "con la misma luz y materiales distintos al 100% por datos"
   es la gran diferencia didáctica (la oración de la lección 05, en PBR fijo).

   --- F0: la ancla ----------------------------------------------------------
   F0 = reflectividad del material a incidencia RECTA. En dieléctricos es un
   valor pequeño: 0.04 para casi todo. En metales es el COLOR del metal: es
   el albedo el que responde al espejo:
       F0 = mix(vec3(0.04), albedo, metalness);
   Esa línea se copió a todo motor moderno (es el corazón del "metalness
   workflow").

   --- FRESNEL-SCHLICK -------------------------------------------------------
   A incidencia rasante el reflejo tiende a 1 (el piso mojado visto de lado).
   F = F0 + (1-F0)*(1-cosTheta)^5. Barato: un pow de exponente 5.

   --- D y G (Cook-Torrance abreviado) ---------------------------------------
   El spec PBR no es un pow: es una BRDF con dos términos (piezas estándar,
   del curso Siggraph 2012 y replicadas por LearnOpenGL y glTF 2.0):
     D (GGX): densidad de microfacetas JUSTO alineadas con H:
            D = a2 / (pi * ((NdotH^2)*(a2-1)+1)^2),  a = roughness^2.
     G (Smith): sombreado entre microfacetas:
            G_Schlick(x,k) = x/(x*(1-k)+k), con k = (roughness+1)^2/8
            para luces puntuales (la variante IBL usa k = a/2, otro tramo).
     BRDF specular = D * F * G / (4 * NdotV * NdotL)
   La DIFFUSA de Lambert queda con /pi (conservación de energía).
   La suma por luz es la integral DISCRETIZADA (IBL es otro camino).

   --- ESTA DEMO -------------------------------------------------------------
   3 esferas con la misma luz: aluminio (metal 1, rough 0.14), plástico rojo
   (metal 0, rough 0.25), piedra (metal 0, rough 0.9). Flechas ARRIBA/ABAJO
   ajustan una roughness global y el pico del spec se degrada en los tres.

   --- Notas de implementación en 3.3 ----------------------------------------
   - const float PI = 3.14159265359: GLSL no trae una constante pi.
   - Las divisiones por NdotV y NdotL exigen max(..., epsilon).
   - El tonemap + gamma final es obligatorio: con D*G el spec pasa de 1.0
     fácil y sin eso se clipea a blanco (la lección 11 despliega tonemaps).
   ============================================================================
 */

#include "include/l21.hpp"

#include <iostream>

namespace {

const char* VS_FUENTE = R"GLSL(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;
out vec3  vPos;
out vec3  vNormal;
void main() {
    vec4 mundo = uModel * vec4(aPos, 1.0);
    vPos = mundo.xyz;
    vNormal = mat3(uModel) * aNormal;
    gl_Position = uProj * uView * mundo;
}
)GLSL";

/* FRAGMENTO PBR: Cook-Torrance directo, sin IBL ni sombras. Cada función
   está comentada línea por línea; el material son 3 uniforms. */
const char* FS_PBR = R"GLSL(
#version 330 core
in vec3 vPos;
in vec3 vNormal;
out vec4 FragColor;

uniform vec3  uPosCamara;
uniform vec3  uPosLuz;
uniform vec3  uColorLuz;
uniform vec3  uAlbedo;
uniform float uMetal;     // 0 dieléctrico .. 1 metal
uniform float uRough;     // 0 espejo .. 1 mate
const float PI  = 3.14159265359;

// D GGX: densidad de microfacetas alineadas EXACTO con H.
float DistributionGGX(float NdotH, float a2) {
    float tmp  = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * tmp * tmp);
}

// G de Schlick: visibilidad de una microfacetas bajo x (0..1).
float GeometrySchlickGGX(float NdotX, float k) {
    return NdotX / (NdotX * (1.0 - k) + k);
}

// G de SMITH: producto de la atenuación luce vs vista.
float GeometrySmith(float NdotV, float NdotL, float k) {
    return GeometrySchlickGGX(NdotV, k) * GeometrySchlickGGX(NdotL, k);
}

// F de Schlick: F0 en la perpendicular, cayendo a lo rasante.
vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main() {
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uPosCamara - vPos);
    vec3 L = normalize(uPosLuz - vPos);
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.001);  // epsilon: no dividir por 0
    float NdotH = max(dot(N, H), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    // PROPIEDADES del material
    vec3  F0     = mix(vec3(0.04), uAlbedo, uMetal);
    float a      = uRough * uRough;
    float a2     = a * a;
    float k      = (uRough + 1.0) * (uRough + 1.0) / 8.0;

    // BRDF SPECULAR de Cook-Torrance
    float D = DistributionGGX(NdotH, a2);
    float G = GeometrySmith(NdotV, NdotL, k);
    vec3  F = FresnelSchlick(HdotV, F0);
    vec3  brdfSpec = (D * G * F) / (4.0 * NdotV * NdotL);

    // DIFFUSA de Lambert con conservación de energía: /pi
    vec3 kd = (vec3(1.0) - F) * (1.0 - uMetal);   // el metal no difunde
    vec3 difusa = (kd * uAlbedo) / PI;

    // SUMA POR LUZ (acá una; sumar más luces = repetir el bloque)
    vec3 brdfLuz = difusa + brdfSpec;
    vec3 color   = brdfLuz * uColorLuz * NdotL;

    // ambient mínimo (con AO map sería * texture(uAO, uv))
    color += 0.03 * uAlbedo;

    // TONEMAP + GAMMA: con D*G el spec puede pasar de 1.0 fácil
    color = color / (color + vec3(1.0));   // Reinhard (la L11 la despliega)
    color = pow(color, vec3(1.0 / 2.2));   // gamma de salida
    FragColor = vec4(color, 1.0);
}
)GLSL";

}  // namespace

int main() {
    glu::Contexto c = glu::abrir("M21 L06: PBR (metal / plástico / piedra)",
                                 900, 400);
    if (c.ventana == nullptr) return 1;
    glEnable(GL_DEPTH_TEST);

    GLuint prog = glu::enlazar(VS_FUENTE, FS_PBR);
    if (prog == 0) { glu::cerrar(c); return 1; }

    glu::Uniform uModel, uView, uProj, uPosCamara, uPosLuz, uColorLuz,
                 uAlbedo, uMetal, uRough;
    uModel.buscar(prog, "uModel");    uView.buscar(prog, "uView");
    uProj.buscar(prog, "uProj");      uPosCamara.buscar(prog, "uPosCamara");
    uPosLuz.buscar(prog, "uPosLuz");  uColorLuz.buscar(prog, "uColorLuz");
    uAlbedo.buscar(prog, "uAlbedo");  uMetal.buscar(prog, "uMetal");
    uRough.buscar(prog, "uRough");

    f21::MallaGPU esf = f21::subirMalla(f21::esfera(0.5f));
    f21::Vec3 ojo{0.0f, 1.1f, 4.0f};
    f21::Mat4 vista = f21::lookAt(ojo, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f});
    f21::Mat4 proj = f21::perspectiva(55.0f,
        (float)c.ancho / (float)c.alto, 0.1f, 100.0f);

    struct PBRMaterial { f21::Vec3 albedo; float metal, rough; } mats[] = {
        {{0.82f, 0.84f, 0.86f}, 1.0f, 0.14f},   // aluminio pulido
        {{0.85f, 0.10f, 0.08f}, 0.0f, 0.25f},   // plástico rojo
        {{0.45f, 0.42f, 0.40f}, 0.0f, 0.90f},   // piedra mate
    };
    float roughGlobal = 0.5f;
    float angulo = 0.0f;

    const bool* teclas = SDL_GetKeyboardState(nullptr);
    while (glu::eventosOk()) {
        float dt = 1.0f / 60.0f;
        if (teclas[SDL_SCANCODE_LEFT])  angulo -= 40.0f * dt;
        if (teclas[SDL_SCANCODE_RIGHT]) angulo += 40.0f * dt;
        if (teclas[SDL_SCANCODE_UP])    roughGlobal += 0.15f * dt;
        if (teclas[SDL_SCANCODE_DOWN])  roughGlobal -= 0.15f * dt;
        if (roughGlobal < 0.02f) roughGlobal = 0.02f;
        if (roughGlobal > 1.0f)  roughGlobal = 1.0f;
        angulo += 20.0f * dt;
        float rad = f21::aRad(angulo);
        f21::Vec3 luz{ std::cos(rad) * 2.5f, 2.5f, std::sin(rad) * 2.5f };

        glClearColor(0.04f, 0.04f, 0.06f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(prog);
        f21::subirMat(uView.lugar, vista);
        f21::subirMat(uProj.lugar, proj);

        for (int i = 0; i < 3; ++i) {
            float x = -2.2f + 2.2f * (float)i;
            f21::subirMat(uModel.lugar, f21::trs({x, 0.0f, 0.0f}, 1.0f, 0.0f));
            f21::subirVec(uPosCamara.lugar, ojo);
            f21::subirVec(uPosLuz.lugar, luz);
            f21::subirVec(uColorLuz.lugar, {1.0f, 0.98f, 0.94f});
            f21::subirVec(uAlbedo.lugar, mats[i].albedo);
            glUniform1f(uMetal.lugar, mats[i].metal);
            // roughness final = mezcla material + control global (jugable)
            glUniform1f(uRough.lugar,
                        (mats[i].rough + roughGlobal) * 0.5f);
            f21::dibujar(esf);
        }
        SDL_GL_SwapWindow(c.ventana);
    }

    glDeleteProgram(prog);
    f21::borrarMalla(esf);
    glu::cerrar(c);
    std::cout << "Listo: aluminio / plástico / piedra con el MISMO shader "
                 "PBR: el material solo son 3 números.\n";
    return 0;
}
