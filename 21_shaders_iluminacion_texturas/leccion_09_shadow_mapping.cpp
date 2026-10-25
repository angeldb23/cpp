/* =============================================================================
   MÓDULO 21 - LECCIÓN 09: Shadow mapping: el pase de profundidad desde la luz
   =============================================================================
   TEMA 5 de TEMAS.md (primera de 2 lecciones): "Sombras: shadow mapping
   (depth map), PCF, bias (acne/peter-panning) y cascaded shadow maps intro".
   PCF, bias refinado y cascadas completan la LECCIÓN 10.

   CÓMO CORRERLA A MANO:
     g++ -std=c++20 -Wall -Wextra leccion_09_shadow_mapping.cpp \
         $(pkg-config --cflags --libs sdl3) -lOpenGL -o lec09 && ./lec09

   --- LA IDEA (1978, Williams; el "depth map") ------------------------------
   El GPU NO sabe qué fragmento está en sombra: la luz de la lección 03
   ilumina TODO lo que tiene la normal a favor. ¿Cómo sabe una luz qué
   fragmentos le bloquean? Respuesta: la luz TIENE SU PROPIA CÁMARA:
     PASO 1: renderizá la escena DESDE la posición/dirección de la luz y
             guardá SOLO LA PROFUNDIDAD (no color) en una textura de depth:
             el "mapa de sombra".
     PASO 2: al iluminar cada fragmento, calculá cómo lo vería la luz
             (transformalo con la cámara de la luz). Si su profundidad desde
             la luz es MAYOR que la guardada en el mapa, algo está en medio:
             hay SOMBRA.
   La comparación la hace el hardware de texturas: se configura la textura
   con GL_TEXTURE_COMPARE_MODE = GL_COMPARE_REF_TO_TEXTURE y
   GL_TEXTURE_COMPARE_FUNC = GL_LEQUAL: el sample devuelto por el
   sampler2DShadow es el RESULTADO de comparar el depth guardado contra el
   tercer componente de la vec3 al samplear (verificado en glcorearb.h:
   0x884C / 0x884D / 0x884E, núcleo GL 3.3 core).

   --- EL FBO DE PROFUNDIDAD (sin color attachment) --------------------------
   El framebuffer del pase 1 NO TIENE color: glDrawBuffers con GL_NONE (y
   glReadBuffer(GL_NONE)). El depth attachment ES UNA TEXTURA (NO un
   renderbuffer: ese no se puede samplear; el 20 L16 usó renderbuffer solo
   porque ese pase era write-only). Formato: GL_DEPTH_COMPONENT32F
   (0x8CAC). La luz mira con una PROYECCIÓN ORTOGRÁFICA (como el sol): se
   elige un rectángulo (izq/der/alto/bajo) que ABARQUE la escena (la caja
   de la sombra) más near/far; TODO lo que quede fuera no recibe y no da
   sombra (los fragmentos lo notan con el clamp-to-border a 1.0: "sin oclusión").

   --- LA COORDENADA [0,1] vs [-1,1] -----------------------------------------
   El depth de clip espacio vive en [-1,1]; el depth TEXTURE guarda [0,1].
   El fragmento remapea la posición vista por la luz:
       vec3 coord = vPosLuz.xyz / vPosLuz.w;   // NDC
       coord = coord * 0.5 + 0.5;              // [0,1] como el mapa
   (formato del mismo NDC del módulo 19, con OTRA cámara.)

   --- LA ESTRUCTURA: la escena se renderiza DOS veces -----------------------
   1. pase luz: viewport = tamaño del mapa (1024x1024), clear depth, sin
      color, programa VS luz (uMatLuz * uModel), dibujar TODO lo que SOMBREA.
   2. pase vista: programa de iluminación; el vertex le agrega
      vPosEnEspacioLuz = uMatLuz * uModel * aPos con la MISMA matLuz del
      pase 1 (subida por CPU dos veces: es UNA matriz compartida).

   --- BIAS: el acne y su par peter-panning ----------------------------------
   Sin corrección, una superficie COMPARA CONTRA SU PROPIA sombra: cuadritos
   oscuros de "auto-sombra" (shadow acne, un artefacto de la resolución del
   mapa). La cura: un bias SUSTRATIVO (poco hacia la luz) al comparar.
   La variante de slope-scale: una cara PARALELA al rayo de luz (dot(N,L)
   bajo) necesita MÁS bias que una cara de frente:
       float bias = max(0.05 * (1.0 - dot(N, L)), 0.005);
   Demasiado bias => sombras despegadas del suelo (peter-panning): objetos
   que parecen "flotar". La lección 10 mezcla PCF + bias ajustado.

   --- ESTA DEMO -------------------------------------------------------------
   Un piso + un cubo arrojando sombra desde una luz direccional (la cámara
   de la luz es UNA ORTOGRÁFICA). Las flechas mueven la luz lateralmente:
   mirá cómo la sombra del cubo la sigue. La demo imprime el tamaño del
   mapa por consola.

   --- APIs (verificadas en glcorearb.h local) -------------------------------
   - GL_DEPTH_COMPONENT32F (0x8CAC) formato del depth texture.
   - GL_TEXTURE_COMPARE_MODE (0x884C), GL_TEXTURE_COMPARE_FUNC (0x884D),
     GL_COMPARE_REF_TO_TEXTURE (0x884E), GL_LEQUAL: el sampler2DShadow.
   - GL_CLAMP_TO_BORDER (0x812D) + glTexParameterfv GL_TEXTURE_BORDER_COLOR.
   - glBindFramebuffer/glFramebufferTexture2D + glDrawBuffer(GL_NONE).
   ============================================================================
 */

#include "include/l21.hpp"

#include <iostream>

namespace {

/* PASO 1: el vertex de la CÁMARA-LUZ (uModel describiendo el mesh). */
const char* VS_LUZ = R"GLSL(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 uMatLuz;        // proyección_luz * vista_luz
uniform mat4 uModel;
void main() { gl_Position = uMatLuz * uModel * vec4(aPos, 1.0); }
)GLSL";

/* Pase 1 fragment: nada de color (glDrawBuffer(GL_NONE)). Aunque no
   escribe, la etapa fragment DEBE existir: el vsolo no linkea. */
const char* FS_LUZ_VACIO = R"GLSL(
#version 330 core
void main() {}
)GLSL";

/* PASO 2: vertex del pase de vista; además de las posiciones de siempre,
   calcula la coordenada "visto por la luz" (4 componentes). */
const char* VS_VISTA = R"GLSL(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
uniform mat4 uModel;
uniform mat4 uMatVista;      // proj * view de la cámara del dueño
uniform mat4 uMatLuz;        // la MISMA matLuz del pase 1
out vec3  vPos;              // punto en MUNDO (N, L, V viven ahí)
out vec3  vNormal;
out vec4  vPosEnEspacioLuz;  // el punto VISTO POR LA LUZ
void main() {
    vec4 mundo = uModel * vec4(aPos, 1.0);
    vPos = mundo.xyz;
    vNormal = mat3(uModel) * aNormal;
    vPosEnEspacioLuz = uMatLuz * mundo;
    gl_Position = uMatVista * mundo;
}
)GLSL";

/* PASO 2: fragmento con el shadow test (compare-mode sampler). */
const char* FS_SOMBRA = R"GLSL(
#version 330 core
in vec3 vPos;
in vec3 vNormal;
in vec4 vPosEnEspacioLuz;
out vec4 FragColor;

uniform sampler2DShadow uMapaSombra;   // sampler de comparación por hardware
uniform vec3  uDirLuz;
uniform vec3  uColorLuz;
uniform vec3  uPosCamara;
uniform float uEscaladaBias;

float factorSombra() {
    // 1) NDC [-1,1] -> UV + depth [0,1]
    vec3 coord = vPosEnEspacioLuz.xyz / vPosEnEspacioLuz.w;
    coord = coord * 0.5 + 0.5;
    // fuera del volumen de la luz (profundidad mayor a 1): sin sombra posible
    if (coord.z > 1.0) return 1.0;

    // 2) BIASES de slope: cara rasante al rayo de luz => más bias
    vec3 N = normalize(vNormal);
    vec3 L = normalize(-uDirLuz);
    float bias = max(0.05 * (1.0 - dot(N, L)), 0.005) * uEscaladaBias;

    // 3) LA COMPARACIÓN HARDWARE (compare mode REF pas uv + LEQUAL):
    //    ~1.0 visible (sin oclusión), ~0.0 en sombra
    float ref = coord.z - bias;
    return texture(uMapaSombra, vec3(coord.xy, ref));
}

void main() {
    vec3 N = normalize(vNormal);
    vec3 L = normalize(-uDirLuz);
    vec3 V = normalize(uPosCamara - vPos);

    float sombra = factorSombra();
    vec3 albedo = vec3(0.75, 0.75, 0.78);
    vec3 color = albedo * 0.10;                                  // ambient
    color += albedo * max(dot(N, L), 0.0) * uColorLuz * sombra;  // difusa
    vec3 reflejo = reflect(-L, N);
    color += vec3(0.2) * pow(max(dot(reflejo, V), 0.0), 24.0)
           * uColorLuz * sombra;                                 // spec
    FragColor = vec4(color, 1.0);
}
)GLSL";

/* La depth texture de la cámara-luz con compare mode LEQUAL */
static void subirDepthTexture(GLuint& tex, int w, int h) {
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, w, h, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // clamp-to-border a 1.0: lo FUERA del volumen de la luz NUNCA sombrea
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borde[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borde);
    // modo COMPARE: cada sample compara el ref contra el depth guardado
    // (verificado: núcleo GL 3.3)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
                    GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
}

}  // namespace

int main() {
    constexpr int TAM_MAPA = 1024;
    std::cout << "Shadow map " << TAM_MAPA << "x" << TAM_MAPA
              << " (GL_DEPTH_COMPONENT32F + compare mode LEQUAL)\n";

    glu::Contexto c = glu::abrir("M21 L09: shadow mapping", 800, 550);
    if (c.ventana == nullptr) return 1;
    glEnable(GL_DEPTH_TEST);

    GLuint progLuz   = glu::enlazar(VS_LUZ, FS_LUZ_VACIO);
    GLuint progVista = glu::enlazar(VS_VISTA, FS_SOMBRA);
    if (progLuz == 0 || progVista == 0) { glu::cerrar(c); return 1; }

    glu::Uniform uMatLuzL, uModelL, uModelV, uMatVista, uMatLuzV;
    uMatLuzL.buscar(progLuz, "uMatLuz");
    uModelL.buscar(progLuz, "uModel");
    uModelV.buscar(progVista, "uModel");
    uMatVista.buscar(progVista, "uMatVista");
    uMatLuzV.buscar(progVista, "uMatLuz");

    f21::Vec3 ojo{0.0f, 2.4f, 5.0f};
    f21::Mat4 vistaCam = f21::lookAt(ojo, {0.0f, 0.0f, 0.0f},
                                     {0.0f, 1.0f, 0.0f});
    f21::Mat4 projCam = f21::perspectiva(55.0f,
        (float)c.ancho / (float)c.alto, 0.1f, 100.0f);

    /* LA CÁMARA DE LA LUZ: posición "sol" + ORTOGRÁFICA que abarca la escena
       (la caja de la sombra; half 6x6, near 0.5, far 25). */
    f21::Vec3 posLuz{3.0f, 5.0f, 2.0f};
    f21::Mat4 projLuz = f21::orto(6.0f, 6.0f, 0.5f, 25.0f);
    f21::Mat4 matLuz = f21::Mat4::identidad();

    f21::MallaGPU cubo = f21::subirMalla(f21::cubo(0.6f));
    f21::MallaGPU piso;
    {
        f21::Malla chapa = f21::cubo(1.0f);
        // el piso: el mismo cubo escalado con la matriz del modelo
        piso = f21::subirMalla(chapa);
    }
    f21::Mat4 chapa = f21::Mat4::identidad();
    chapa.set(0, 0, 6.0f); chapa.set(1, 1, 0.1f); chapa.set(2, 2, 6.0f);
    f21::Mat4 modeloPiso = f21::trs({0.0f, -0.6f, 0.0f}, 1.0f, 0.0f) * chapa
                           * f21::Mat4::identidad();
    f21::Mat4 modeloCubo = f21::trs({0.0f, 0.4f, 0.0f}, 1.0f, 0.0f);
    f21::Vec3 dirLuz = f21::normalizar(f21::Vec3{-3.0f, -5.0f, -2.0f});

    /* Depth texture + FBO SIN color (el pase de la luz). */
    GLuint texSombra = 0;
    subirDepthTexture(texSombra, TAM_MAPA, TAM_MAPA);
    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D, texSombra, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "FBO de sombras incompleto\n";
        glu::cerrar(c);
        return 1;
    }

    const bool* teclas = SDL_GetKeyboardState(nullptr);
    while (glu::eventosOk()) {
        float dt = 1.0f / 60.0f;
        if (teclas[SDL_SCANCODE_LEFT])  posLuz.x -= 1.5f * dt;
        if (teclas[SDL_SCANCODE_RIGHT]) posLuz.x += 1.5f * dt;
        if (teclas[SDL_SCANCODE_UP])    posLuz.y += 1.0f * dt;
        if (teclas[SDL_SCANCODE_DOWN])  posLuz.y -= 1.0f * dt;
        f21::Mat4 vistaLuz = f21::lookAt(posLuz, {0.0f, 0.0f, 0.0f},
                                         {0.0f, 1.0f, 0.0f});
        matLuz = projLuz * vistaLuz;

        /* PASO 1: DEPTH DESDE LA LUZ (viewport = mapa completo). */
        glViewport(0, 0, TAM_MAPA, TAM_MAPA);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glClear(GL_DEPTH_BUFFER_BIT);
        glUseProgram(progLuz);
        f21::subirMat(uMatLuzL.lugar, matLuz);
        f21::subirMat(uModelL.lugar, modeloPiso);
        f21::dibujar(piso);
        f21::subirMat(uModelL.lugar, modeloCubo);
        f21::dibujar(cubo);

        /* PASO 2: ILUMINACIÓN DESDE LA CÁMARA CON shadow test. */
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, c.ancho, c.alto);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(progVista);
        f21::subirMat(uMatVista.lugar, projCam * vistaCam);
        f21::subirMat(uMatLuzV.lugar, matLuz);
        GLint samSombra = glGetUniformLocation(progVista, "uMapaSombra");
        glUniform1i(samSombra, 0);
        GLint dirLuzU = glGetUniformLocation(progVista, "uDirLuz");
        f21::subirVec(dirLuzU, dirLuz);
        GLint colU = glGetUniformLocation(progVista, "uColorLuz");
        f21::subirVec(colU, {1.0f, 0.95f, 0.85f});
        GLint camU = glGetUniformLocation(progVista, "uPosCamara");
        f21::subirVec(camU, ojo);
        GLint escU = glGetUniformLocation(progVista, "uEscaladaBias");
        glUniform1f(escU, 1.0f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texSombra);
        f21::subirMat(uModelV.lugar, modeloPiso);
        f21::dibujar(piso);
        f21::subirMat(uModelV.lugar, modeloCubo);
        f21::dibujar(cubo);

        SDL_GL_SwapWindow(c.ventana);
    }

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &texSombra);
    glDeleteProgram(progLuz);
    glDeleteProgram(progVista);
    f21::borrarMalla(cubo);
    f21::borrarMalla(piso);
    glu::cerrar(c);
    std::cout << "Listo: dos pases: depth desde la luz y sombreado con "
                 "compare-mode en la vista.\n";
    return 0;
}
